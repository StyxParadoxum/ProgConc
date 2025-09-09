/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Codigo: Comunicação entre threads usando variável compartilhada e exclusao mutua com bloqueio */

#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>

long int soma = 0; // variavel compartilhada entre as threads
pthread_mutex_t mutex; // variavel de lock para exclusao mutua
pthread_cond_t cond_extra, cond_tarefa; 
int pronto = 0; // flag de controle
int fim = 0; // nova flag para indicar término

// funcao executada pelas threads
void *ExecutaTarefa (void *arg) {
  long int id = (long int) arg;
  printf("Thread : %ld esta executando...\n", id);

  for (int i=0; i<100000; i++) {
     pthread_mutex_lock(&mutex); 

     // se atingiu múltiplo de 1000
     if (soma % 1000 == 0) {
        pronto = 1;  
        pthread_cond_signal(&cond_extra); // avisa extra
        pthread_cond_wait(&cond_tarefa, &mutex); // espera extra imprimir

        if (fim) { // se extra já finalizou, encerra
           pthread_mutex_unlock(&mutex);
           break;
        }
     }

     soma++;
     pthread_mutex_unlock(&mutex);
  }
  printf("Thread : %ld terminou!\n", id);
  pthread_exit(NULL);
}

//funcao executada pela thread de log
void *extra (void *args) {
  printf("Extra : esta executando...\n");
  while (1) {
     pthread_mutex_lock(&mutex);

     while (!pronto && soma < 100000) {
        pthread_cond_wait(&cond_extra, &mutex);
     }

     if (pronto) {
        printf("soma = %ld \n", soma);
        pronto = 0;
        pthread_cond_signal(&cond_tarefa); 
     }

     if (soma >= 100000) {
        fim = 1; // avisa que acabou
        pthread_cond_broadcast(&cond_tarefa); // libera todas que estejam presas
        pthread_mutex_unlock(&mutex);
        break;
     }

     pthread_mutex_unlock(&mutex);
  }
  printf("Extra : terminou!\n");
  pthread_exit(NULL);
}

//fluxo principal
int main(int argc, char *argv[]) {
   pthread_t *tid; //identificadores das threads no sistema
   int nthreads; //qtde de threads (passada linha de comando)

   //--le e avalia os parametros de entrada
   if(argc<2) {
      printf("Digite: %s <numero de threads>\n", argv[0]);
      return 1;
   }
   nthreads = atoi(argv[1]);

   //--aloca as estruturas
   tid = (pthread_t*) malloc(sizeof(pthread_t)*(nthreads+1));
   if(tid==NULL) {puts("ERRO--malloc"); return 2;}

   //--inicializa o mutex (lock de exclusao mutua) e as variaveis de condicao
   pthread_mutex_init(&mutex, NULL);
   pthread_cond_init(&cond_extra, NULL);
   pthread_cond_init(&cond_tarefa, NULL);

   //--cria as threads
   for(long int t=0; t<nthreads; t++) {
     if (pthread_create(&tid[t], NULL, ExecutaTarefa, (void *)t)) {
       printf("--ERRO: pthread_create()\n"); exit(-1);
     }
   }

   //--cria thread de log
   if (pthread_create(&tid[nthreads], NULL, extra, NULL)) {
      printf("--ERRO: pthread_create()\n"); exit(-1);
   }

   //--espera todas as threads terminarem
   for (int t=0; t<nthreads+1; t++) {
     if (pthread_join(tid[t], NULL)) {
         printf("--ERRO: pthread_join() \n"); exit(-1); 
     } 
   } 

   //--finaliza o mutex
   pthread_mutex_destroy(&mutex);
   pthread_cond_destroy(&cond_extra);
   pthread_cond_destroy(&cond_tarefa);
   
   printf("Valor de 'soma' = %ld\n", soma);

   return 0;
}
