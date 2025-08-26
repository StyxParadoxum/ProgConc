#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Programa auxiliar para gerar vetores e o produto interno esperado
// Uso: ./gerador <tamanho> <arquivo_saida>

// Define o intervalo dos números gerados
#define MAX 5000.0f
#define MIN -2000.0f

// Gera número aleatório no intervalo [MIN, MAX]
#define RAND_INTERVALO() ((float) rand() / RAND_MAX * (MAX - MIN) + MIN)

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <N> <arquivo_saida>\n", argv[0]);
        return 1;
    }

    long n = atol(argv[1]);
    const char *nomeArq = argv[2];

    float *v1 = (float*) malloc(n * sizeof(float));
    float *v2 = (float*) malloc(n * sizeof(float));

    srand(time(NULL));

    for (long i = 0; i < n; i++) {
        v1[i] = RAND_INTERVALO();
        v2[i] = RAND_INTERVALO();
    }

    double produto = 0.0;
    for (long i = 0; i < n; i++) {
        produto += (double) v1[i] * v2[i];
    }

    FILE *f = fopen(nomeArq, "wb");
    fwrite(&n, sizeof(long), 1, f);
    fwrite(v1, sizeof(float), n, f);
    fwrite(v2, sizeof(float), n, f);
    fwrite(&produto, sizeof(double), 1, f);
    fclose(f);

    printf("Arquivo %s gerado. Intervalo [%.2f, %.2f], produto interno = %.6lf\n",
           nomeArq, MIN, MAX, produto);

    free(v1);
    free(v2);
    return 0;
}
