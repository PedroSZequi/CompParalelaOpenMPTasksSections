#!/bin/bash
# Mede o tempo de um programa com 1, 2, 3 e 4 threads (OMP_NUM_THREADS).
# Usa o tempo que o proprio programa imprime (omp_get_wtime) e calcula a media.
# Antes de cada configuracao faz 1 execucao de aquecimento (descartada).
#
# Uso: ./medir.sh <repeticoes> <executavel> [argumentos...]
# Ex.: ./medir.sh 5 ./atividade1/analise_inicial

REPS=$1; shift
echo "Programa: $* | $REPS execucoes por configuracao (+1 aquecimento)"
printf "%-8s %-45s %s\n" "threads" "tempos (s)" "media (s)"
for T in 1 2 3 4; do
    OMP_NUM_THREADS=$T "$@" > /dev/null            # aquecimento
    tempos=""
    for r in $(seq "$REPS"); do
        t=$(OMP_NUM_THREADS=$T "$@" | grep "Tempo de execu" | grep -oE "[0-9]+\.[0-9]+")
        tempos="$tempos $t"
    done
    media=$(echo $tempos | awk '{s=0; for(i=1;i<=NF;i++) s+=$i; printf "%.4f", s/NF}')
    printf "%-8s %-45s %s\n" "$T" "$tempos" "$media"
done
