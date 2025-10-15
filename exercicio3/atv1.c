#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>

// Macros
#define N_CONSUMIDORES 1000

// Entrada
int n_elementos; 
int tam_buffer;    

// Buffer compartilhado
int *buffer = NULL;

// Variaveis de controle
int contador = 0;        
int consumidos = 0;

// Semaforos      
sem_t mutex_prod;          
sem_t mutex_cons;       
sem_t mutex_total;
sem_t buffer_vazio;         
sem_t buffer_cheio;           

// Variáveis de Rastreamento
int *n_primos_thread = NULL;       // Contador de primos por thread
pthread_t *tid_cons = NULL;       // Vetor com os IDs das threads consumidoras

int eh_primo(long long int n) {
    int i;

    if (n <= 1) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;

    for (i = 3; i < sqrt(n) + 1; i += 2) {
        if (n % i == 0) return 0;
    }

    return 1;
}


int inicializa_semaforos(void) {
    if (sem_init(&buffer_vazio, 0, 1) != 0) {
        return 0;
    }

    if (sem_init(&buffer_cheio, 0, 0) != 0) {
        return 0;
    }

    if (sem_init(&mutex_prod, 0, 1) != 0) {
        return 0;
    }

    if (sem_init(&mutex_cons, 0, 1) != 0) {
        return 0;
    }

    if (sem_init(&mutex_total, 0, 1) != 0) {
        return 0;
    }

    return 1;
}

void *produtor(void *arg) {
    
    static int valor = 0;
    int n_inserir;

    printf("---INICIO DA PRODUCAO---\n");

    while (valor < n_elementos) {
        
        // Espera o buffer estar vazio
        sem_wait(&buffer_vazio);
        sem_wait(&mutex_prod);

        n_inserir = tam_buffer;
        if (valor + n_inserir > n_elementos) {
            n_inserir = n_elementos - valor;
        }

        printf("Produtor vai inserir %d elementos\n", n_inserir);

        for (int i = 0; i < n_inserir; i++) {
            valor++;
            buffer[i] = valor;
            printf("Valor %d inserido na posicao %d do buffer\n", buffer[i], i);
        }

        contador = n_inserir;

        sem_post(&mutex_prod);

        for (int i = 0; i < n_inserir; i++) {
            sem_post(&buffer_cheio);
        }

        printf("Produtor inseriu %d elementos (%d de %d)\n",
            n_inserir, valor, n_elementos);
    }

    printf("--- FIM DA PRODUCAO ---\n");
    pthread_exit(NULL);
}


void *consumidor(void *arg) {
    long id = (long) arg;
    int elemento;
    int primos_encontrados = 0;
    int item_consumido_local = 0;
    int item_atual_total;
    static int out = 0;

    /* Validacao do ID */
    if (id < 0 || id >= N_CONSUMIDORES) {
        printf("ID de consumidor invalido\n");
        pthread_exit(NULL);
    }

    printf("--- INICIO DA THREAD CONSUMIDORA %ld ---\n", id);

    while (1) {
        sem_wait(&mutex_total);
        if (consumidos >= n_elementos) {
            sem_post(&mutex_total);
            break;
        }
        consumidos++;
        item_atual_total = consumidos;
        item_consumido_local++;
        sem_post(&mutex_total);

        sem_wait(&buffer_cheio);
        sem_wait(&mutex_cons);

        elemento = buffer[out];
        printf("THREAD CONSUMIDORA %ld CONSUMIU %d DA POSICAO %d (Número Total %d de %d / Número Local %d)\n",
            id, elemento, out, item_atual_total, n_elementos, item_consumido_local);

        out = (out + 1) % tam_buffer;
        contador--;

        if (contador == 0) {
            printf("--LOG: Buffer esvaziado, liberando produtor\n");
            out = 0;
            sem_post(&buffer_vazio);
        }

        sem_post(&mutex_cons);

        if (eh_primo((long long int)elemento)) {
            primos_encontrados++;
            printf("THREAD CONSUMIDORA %ld ENCONTROU PRIMO: %d (total de primos encontrados por essa thread: %d)\n",
                id, elemento, primos_encontrados);
        }
    }

    n_primos_thread[id] = primos_encontrados;

    printf("Número de Primos Encontrados pela Thread %ld: %d \n", id, primos_encontrados);
    printf("--- FIM DA THREAD CONSUMIDORA %ld ---\n", id);
    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {
    pthread_t tid_prod;
    int i;
    int total_primos = 0;
    int max_primos = 0;
    int vencedora = 0;

    if (argc != 3) {
        printf("Número de Argumentos não é o Esperado\n");
        printf("Forma Correta -> *Número de Elementos* *Tamanho do Buffer (M < N)*\n");
        return 1;
    }

    n_elementos = atoi(argv[1]);
    tam_buffer = atoi(argv[2]);

    if (n_elementos <= 0) {
        printf("Número de Elementos deve ser maior que 0\n");
        return 1;
    }

    if (tam_buffer <= 0) {
        printf("Tamanho do Buffer deve ser maior que 0\n");
        return 1;
    }

    if (tam_buffer > n_elementos) {
        printf("M deve ser menor ou igual a N\n");
        return 1;
    }

    // Alocação do buffer
    buffer = (int *) malloc(tam_buffer * sizeof(int));
    if (buffer == NULL) {
        printf("Falha na alocacao do buffer\n");
        return 1;
    }

    // Alocação do vetor de contagem de primos
    n_primos_thread = (int *) calloc(N_CONSUMIDORES, sizeof(int));
    if (n_primos_thread == NULL) {
        printf("Falha na alocacao do vetor com numero de primos por thread\n");
        return 1;
    }

    // Alocação do vetor com os IDs das threads consumidoras
    tid_cons = (pthread_t *) malloc(N_CONSUMIDORES * sizeof(pthread_t));
    if (tid_cons == NULL) {
        printf("Falha na alocacao do vetor com os IDs das threads consumidoras\n");
        return 1;
    }

    if (!inicializa_semaforos()) {
        printf("Falha na inicializacao dos semaforos\n");
        return 1;
    }


    // Criação da Thread Produtora
    if (pthread_create(&tid_prod, NULL, produtor, NULL) != 0) {
        printf("Falha na criacao da thread produtora\n");
        return 1;
    }

    // Criação das Threads Consumidoras (Quantidade definida pela macro N_CONSUMIDORES)
    for (i = 0; i < N_CONSUMIDORES; i++) {
        if (pthread_create(&tid_cons[i], NULL, consumidor, (void *) (long) i) != 0) {
            printf("Falha na criacao da thread consumidora %d\n", i);
            pthread_join(tid_prod, NULL);
            for (int j = 0; j < i; j++) {
                pthread_join(tid_cons[j], NULL);
            }
            return 1;
        }
    }


    // Join da Thread Produtora
    pthread_join(tid_prod, NULL);

    // Join das Threads Consumidoras
    for (i = 0; i < N_CONSUMIDORES; i++) {
        pthread_join(tid_cons[i], NULL);
    }

    // Calcula o total de primos e identifica a thread vencedora
    for (i = 0; i < N_CONSUMIDORES; i++) {
        total_primos += n_primos_thread[i];
        if (n_primos_thread[i] > max_primos) {
            max_primos = n_primos_thread[i];
            vencedora = i;
        }
    }

    printf("\n\t=== RESULTADOS ===\n");
    printf("Total de primos encontrados: %d\n", total_primos);
    printf("Número da Thread Vencedora: %d\nNúmero de Primos encontrados pela Thread Vencedora: %d\n", vencedora, max_primos);

    free(buffer);
    free(n_primos_thread);
    free(tid_cons);
    sem_destroy(&mutex_prod);
    sem_destroy(&mutex_cons);
    sem_destroy(&mutex_total);
    sem_destroy(&buffer_vazio);
    sem_destroy(&buffer_cheio);

    printf("\n\t=== FIM DO PROGRAMA ===\n");

    return 0;
}