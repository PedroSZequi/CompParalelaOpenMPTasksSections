#define _GNU_SOURCE // Para scandir e asprintf (asprintf e extensao GNU; com _XOPEN_SOURCE gera aviso)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>   // Para operações de diretório
#include <sys/stat.h> // Para stat
#include <errno.h>
#include <omp.h>      // Usaremos para timing inicialmente

// Função para contar linhas em um único arquivo
long long count_lines_in_file(const char *filepath) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        // fprintf(stderr, "Erro ao abrir arquivo %s: %s\n", filepath, strerror(errno));
        return 0; // Ignora arquivos que não pode abrir
    }
    long long count = 0;
    int ch;
    // getc_unlocked: le o caractere SEM travar o FILE. E seguro aqui porque
    // cada arquivo e lido por uma unica tarefa (um unico thread).
    while ((ch = getc_unlocked(fp)) != EOF) {
        if (ch == '\n') {
            count++;
        }
    }
    fclose(fp);
    // Adiciona 1 se o arquivo não terminar com newline mas não for vazio
    // Simplificação: vamos apenas contar os '\n'
    return count;
}

// Contador global de linhas, atualizado pelas tarefas com #pragma omp atomic
long long total_lines_global = 0;

// Função recursiva para atravessar diretórios (versão com tasks)
// Em vez de retornar a soma, cada tarefa soma diretamente no contador global.
void traverse_directory(const char *dirpath) {
    struct dirent **namelist;
    int n;

    n = scandir(dirpath, &namelist, NULL, alphasort);
    if (n < 0) {
        fprintf(stderr, "Erro ao escanear diretório %s: %s\n", dirpath,
                strerror(errno));
        return;
    }

    for (int i = 0; i < n; i++) {
        // Ignora "." e ".."
        if (strcmp(namelist[i]->d_name, ".") == 0
            || strcmp(namelist[i]->d_name, "..") == 0) {
            free(namelist[i]);
            continue;
        }

        // Monta o caminho completo
        char *fullpath = NULL;
        if (asprintf(&fullpath, "%s/%s", dirpath, namelist[i]->d_name) == -1) {
            fprintf(stderr, "Erro de alocação em asprintf\n");
            free(namelist[i]);
            continue; // Pula este item
        }

        struct stat path_stat;
        if (stat(fullpath, &path_stat) != 0) {
            fprintf(stderr, "Erro no stat para %s: %s\n", fullpath, strerror(errno));
            free(fullpath);
            free(namelist[i]);
            continue;
        }

        // Se for um diretório: cria uma TAREFA para explorá-lo.
        // fullpath é firstprivate: a tarefa recebe sua própria cópia do ponteiro
        // e fica responsável por liberar a memória quando terminar.
        if (S_ISDIR(path_stat.st_mode)) {
            #pragma omp task firstprivate(fullpath)
            {
                traverse_directory(fullpath);
                free(fullpath);
            }
        }
        // Se for um arquivo .c ou .h: cria uma TAREFA para contar as linhas.
        else if (S_ISREG(path_stat.st_mode)) {
            size_t len = strlen(namelist[i]->d_name);
            if (len > 2 && (strcmp(namelist[i]->d_name + len - 2, ".c") == 0 ||
                            strcmp(namelist[i]->d_name + len - 2, ".h") == 0))
            {
                #pragma omp task firstprivate(fullpath)
                {
                    long long linhas = count_lines_in_file(fullpath);
                    #pragma omp atomic
                    total_lines_global += linhas;   // soma protegida
                    free(fullpath);
                }
            } else {
                free(fullpath);   // não é .c/.h: nenhuma tarefa usa
            }
        } else {
            free(fullpath);       // nem diretório nem arquivo regular
        }

        free(namelist[i]);
    }
    free(namelist);
    // Não é preciso taskwait aqui: o resultado vai para o contador global
    // e a barreira implícita do fim da região parallel espera todas as tarefas.
}

int main(int argc, char *argv[]) {
    const char *start_dir = "."; // Diretório atual por padrão
    if (argc > 1) {
        start_dir = argv[1];
    }

    printf("Contando linhas em .c/.h a partir de: %s (versão tasks + getc_unlocked, %d threads)\n",
           start_dir, omp_get_max_threads());

    double start_time, end_time;
    start_time = omp_get_wtime();

    // --- Bloco a ser otimizado ---
    // Uma thread (single) inicia a travessia e vai criando as tarefas;
    // as outras threads da equipe executam as tarefas do pool.
    #pragma omp parallel
    {
        #pragma omp single
        traverse_directory(start_dir);
    } // barreira implícita: todas as tarefas terminaram
    long long total_lines = total_lines_global;
    // --- Fim do Bloco ---

    end_time = omp_get_wtime();

    printf("\n--- Resultado ---\n");
    printf("Total de linhas encontradas: %lld\n", total_lines);
    printf("Tempo de execução: %.4f segundos\n", end_time - start_time);

    return 0;
}
