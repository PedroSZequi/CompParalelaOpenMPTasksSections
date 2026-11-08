#!/bin/bash
# Compila todas as versoes das tres atividades
# Uso: ./compilar.sh
set -e
CFLAGS="-O3 -Wall -fopenmp"

gcc $CFLAGS atividade1/analise_inicial.c      -o atividade1/analise_inicial      -lm
gcc $CFLAGS atividade1/analise_sections.c     -o atividade1/analise_sections     -lm
gcc $CFLAGS atividade2/simulacao_inicial.c    -o atividade2/simulacao_inicial
gcc $CFLAGS atividade2/simulacao_sections.c   -o atividade2/simulacao_sections
gcc $CFLAGS atividade3/contalinhas_inicial.c  -o atividade3/contalinhas_inicial
gcc $CFLAGS atividade3/contalinhas_tasks.c    -o atividade3/contalinhas_tasks
gcc $CFLAGS atividade3/contalinhas_tasks_v2.c -o atividade3/contalinhas_tasks_v2

echo "Compilacao concluida:"
ls -1 atividade1/analise_inicial atividade1/analise_sections \
      atividade2/simulacao_inicial atividade2/simulacao_sections \
      atividade3/contalinhas_inicial atividade3/contalinhas_tasks \
      atividade3/contalinhas_tasks_v2
