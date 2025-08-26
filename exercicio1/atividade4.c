#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include "timer.h"

// Estrutura com os dados que cada thread vai processar
typedef struct {
    int id;
    int nThreads;
    long dimensao;
    float *x, *y;
    double acumulado;
} InfoThread;

// Função que será executada pelas threads
void *calcula_parte(void *arg) {
    InfoThread *dados = (InfoThread*) arg;

    long inicio = (dados->id) * (dados->dimensao / dados->nThreads);
    long fim = (dados->id == dados->nThreads - 1) ? dados->dimensao : inicio + (dados->dimensao / dados->nThreads);

    dados->acumulado = 0.0;
    for (long i = inicio; i < fim; i++) {
        dados->acumulado += (double) dados->x[i] * dados->y[i];
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso correto: %s <qtd_threads> <arquivo_teste>\n", argv[0]);
        return 1;
    }

    int nThreads = atoi(argv[1]);
    const char *arquivo = argv[2];

    // Leitura do arquivo binário
    long n;
    float *vetX, *vetY;
    double esperado;

    FILE *fp = fopen(arquivo, "rb");
    if (!fp) {
        perror("Erro abrindo arquivo");
        return 2;
    }

    fread(&n, sizeof(long), 1, fp);
    vetX = (float*) malloc(n * sizeof(float));
    vetY = (float*) malloc(n * sizeof(float));
    fread(vetX, sizeof(float), n, fp);
    fread(vetY, sizeof(float), n, fp);
    fread(&esperado, sizeof(double), 1, fp);
    fclose(fp);

    // Criação das threads
    pthread_t *threads = (pthread_t*) malloc(nThreads * sizeof(pthread_t));
    InfoThread *dados = (InfoThread*) malloc(nThreads * sizeof(InfoThread));

    double inicio, fim;
    GET_TIME(inicio);

    for (int t = 0; t < nThreads; t++) {
        dados[t].id = t;
        dados[t].nThreads = nThreads;
        dados[t].dimensao = n;
        dados[t].x = vetX;
        dados[t].y = vetY;
        pthread_create(&threads[t], NULL, calcula_parte, (void*) &dados[t]);
    }

    // Redução dos resultados
    double resultadoConc = 0.0;
    for (int t = 0; t < nThreads; t++) {
        pthread_join(threads[t], NULL);
        resultadoConc += dados[t].acumulado;
    }

    GET_TIME(fim);

    // Cálculo do erro relativo
    double erroRel = fabs(esperado - resultadoConc) / fabs(esperado);

    printf("Produto interno (sequencial): %.6lf\n", esperado);
    printf("Produto interno (concorrente): %.6lf\n", resultadoConc);
    printf("Erro relativo: %e\n", erroRel);
    printf("Tempo gasto: %lf segundos\n", fim - inicio);

    free(vetX);
    free(vetY);
    free(threads);
    free(dados);

    return 0;
}
