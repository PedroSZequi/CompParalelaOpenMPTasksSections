# Atividades – OpenMP Tasks e Sections

Lab de Computação Paralela – Faculdade de Computação e Informática (FCI)
Aluno: Pedro de Souza Zequi – RA 10419805

## Estrutura

| Pasta | Arquivo | Descrição |
|---|---|---|
| `atividade1/` | `analise_inicial.c` | Código inicial: min, max, soma e soma dos quadrados em 3 laços `parallel for` |
| | `analise_sections.c` | Desafio: uma única região `parallel sections`, uma `section` por métrica, com `reduction` |
| `atividade2/` | `simulacao_inicial.c` | Código inicial: posição e velocidade das partículas em 2 laços `parallel for` |
| | `simulacao_sections.c` | Desafio: os 2 laços em `parallel sections` (mais uma conferência do resultado) |
| `atividade3/` | `contalinhas_inicial.c` | Código inicial: contagem recursiva e sequencial de linhas em arquivos `.c`/`.h` |
| | `contalinhas_tasks.c` | Desafio: uma `task` por subdiretório e por arquivo, com contador global e `atomic` |
| | `contalinhas_tasks_v2.c` | Igual à versão com tasks, mas lendo os arquivos com `getc_unlocked` |
| raiz | `compilar.sh` | Compila todas as versões |
| | `medir.sh` | Mede o tempo médio com 1, 2, 3 e 4 threads |

## Requisitos

- gcc com suporte a OpenMP (testado no GitHub Codespaces, Ubuntu, gcc 13)
- Linux/POSIX (a atividade 3 usa `scandir`, `stat` e `asprintf`)

## Compilação

Todas as versões de uma vez:

```bash
chmod +x compilar.sh medir.sh
./compilar.sh
```

Ou uma a uma, com as flags pedidas no enunciado:

```bash
gcc -O3 -Wall -fopenmp atividade1/analise_inicial.c     -o atividade1/analise_inicial     -lm
gcc -O3 -Wall -fopenmp atividade1/analise_sections.c    -o atividade1/analise_sections    -lm
gcc -O3 -Wall -fopenmp atividade2/simulacao_inicial.c   -o atividade2/simulacao_inicial
gcc -O3 -Wall -fopenmp atividade2/simulacao_sections.c  -o atividade2/simulacao_sections
gcc -O3 -Wall -fopenmp atividade3/contalinhas_inicial.c -o atividade3/contalinhas_inicial
gcc -O3 -Wall -fopenmp atividade3/contalinhas_tasks.c   -o atividade3/contalinhas_tasks
gcc -O3 -Wall -fopenmp atividade3/contalinhas_tasks_v2.c -o atividade3/contalinhas_tasks_v2
```

## Execução

```bash
./atividade1/analise_inicial
./atividade1/analise_sections

./atividade2/simulacao_inicial
./atividade2/simulacao_sections

# a atividade 3 recebe o diretório a ser varrido (padrão: diretório atual)
./atividade3/contalinhas_inicial  /usr/include
./atividade3/contalinhas_tasks    /usr/include
./atividade3/contalinhas_tasks_v2 /usr/include
```

O número de threads é definido pela variável de ambiente `OMP_NUM_THREADS`, por exemplo:

```bash
OMP_NUM_THREADS=2 ./atividade1/analise_sections
```

## Medição de desempenho

`medir.sh` roda o programa com 1, 2, 3 e 4 threads. Para cada configuração, faz uma execução de aquecimento (descartada) e depois N execuções, e mostra os tempos e a média. O tempo usado é o que o próprio programa mede com `omp_get_wtime()`.

```bash
./medir.sh 5 ./atividade1/analise_inicial
./medir.sh 5 ./atividade1/analise_sections
./medir.sh 5 ./atividade3/contalinhas_tasks /usr/include
```

## Observações

- **Atividade 3, código inicial:** a primeira linha foi trocada de `#define _XOPEN_SOURCE 700` para `#define _GNU_SOURCE`, porque `asprintf` é uma extensão GNU. Com a definição original, o gcc avisa que a função está declarada implicitamente.
- **Atividade 2:** as duas `sections` acessam o mesmo campo `vy` ao mesmo tempo (uma lê e a outra escreve), o que é uma condição de corrida. O programa conta quantas partículas tiveram a posição calculada com a velocidade já atualizada. Mais detalhes estão no relatório.
- **Atividade 3, `getc_unlocked`:** com mais de uma thread, cada `fgetc` trava e destrava o `FILE`, o que deixa a leitura muito mais lenta. A versão v2 usa `getc_unlocked`, que é seguro aqui porque cada arquivo é lido por uma única tarefa.
