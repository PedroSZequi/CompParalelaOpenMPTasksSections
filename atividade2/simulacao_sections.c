#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define NUM_PARTICLES (1024 * 1024 * 10)
#define DT 0.01 // Time step

typedef struct {
    double x, y, z;    // Posição
    double vx, vy, vz; // Velocidade
} Particle;

int main() {
    Particle *particles = (Particle*) malloc(NUM_PARTICLES * sizeof(Particle));
    if (!particles) { perror("malloc falhou"); return 1; }

    long long i;
    // Inicializa partículas
    printf("Inicializando partículas...\n");
    #pragma omp parallel for
    for (i = 0; i < NUM_PARTICLES; i++) {
        particles[i].x = (double)rand() / RAND_MAX * 100.0;
        particles[i].y = (double)rand() / RAND_MAX * 100.0;
        particles[i].z = (double)rand() / RAND_MAX * 100.0;
        particles[i].vx = (double)rand() / RAND_MAX * 10.0 - 5.0;
        particles[i].vy = (double)rand() / RAND_MAX * 10.0 - 5.0;
        particles[i].vz = (double)rand() / RAND_MAX * 10.0 - 5.0;
    }
    printf("Partículas inicializadas.\n");

    // Copia de y e vy ANTES do passo, so para conferir o resultado depois
    // (fora da medicao de tempo)
    double *y0  = (double*) malloc(NUM_PARTICLES * sizeof(double));
    double *vy0 = (double*) malloc(NUM_PARTICLES * sizeof(double));
    if (!y0 || !vy0) { perror("malloc falhou"); return 1; }
    for (i = 0; i < NUM_PARTICLES; i++) {
        y0[i]  = particles[i].y;
        vy0[i] = particles[i].vy;
    }

    double start_time, end_time;
    start_time = omp_get_wtime();

    // --- Bloco a ser otimizado ---
    printf("Simulando um passo (versão sections)...\n");
    const double GRAVITY = -9.81;

    // Uma unica regiao "parallel sections": cada laco vira uma section,
    // executada inteira por uma thread. As duas secoes rodam ao mesmo tempo.
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {   // Secao 1: atualiza posicoes (le vx, vy, vz)
            for (i = 0; i < NUM_PARTICLES; i++) {
                particles[i].x += particles[i].vx * DT;
                particles[i].y += particles[i].vy * DT;
                particles[i].z += particles[i].vz * DT;
            }
        }

        #pragma omp section
        {   // Secao 2: atualiza velocidades (escreve vy)
            for (i = 0; i < NUM_PARTICLES; i++) {
                particles[i].vy += GRAVITY * DT;
            }
        }
    } // barreira implicita
    printf("  Posições e velocidades atualizadas.\n");
    // --- Fim do Bloco ---

    end_time = omp_get_wtime();

    printf("\nSimulação concluída.\n");

    // Conferencia: a posicao deveria usar a velocidade ANTIGA (y0 + vy0*DT).
    // Conta quantas particulas usaram a velocidade ja atualizada pela secao 2.
    long long inconsistentes = 0;
    for (i = 0; i < NUM_PARTICLES; i++) {
        if (particles[i].y != y0[i] + vy0[i] * DT) inconsistentes++;
    }
    printf("Partículas com y calculado usando a velocidade NOVA: %lld de %d (%.1f%%)\n",
           inconsistentes, NUM_PARTICLES, 100.0 * inconsistentes / NUM_PARTICLES);
    free(y0); free(vy0);
    // Verificar uma partícula (opcional)
    // printf("Partícula 0: Pos(%.2f, %.2f, %.2f) Vel(%.2f, %.2f, %.2f)\n",
    //        particles[0].x, particles[0].y, particles[0].z,
    //        particles[0].vx, particles[0].vy, particles[0].vz);
    printf("Tempo de execução (simulação): %.4f segundos\n", end_time - start_time);

    free(particles);
    return 0;
}
