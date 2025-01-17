#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define PR 5 //número de produtores
#define CN 1 // número de consumidores
#define N 5  //tamanho do buffer


int buffer[N];


int ptr_prod = 0;
int ptr_cons = -1;

pthread_mutex_t buffer_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_cond = PTHREAD_COND_INITIALIZER;

void * produtor(void * meuid);
void * consumidor (void * meuid);


void main(){
	int erro;
	int i, n, m;
	int *id;

	pthread_t tid[PR];
	for (i = 0; i < PR; i++){
		id = (int *) malloc(sizeof(int));
		*id = i;
		erro = pthread_create(&tid[i], NULL, produtor, (void *) (id));

		if(erro){
			printf("erro na criacao do thread %d\n", i);
			exit(1);
		}
	}

	pthread_t tCid[CN];
	for (i = 0; i < CN; i++){
		id = (int *) malloc(sizeof(int));
		*id = i;
		erro = pthread_create(&tCid[i], NULL, consumidor, (void *) (id));

		if(erro){
			printf("erro na criacao do thread %d\n", i);
			exit(1);
		}
	}
	
	pthread_join(tid[0],NULL);

}

void * produtor (void* pi){
  	int item;
	while(1){
		item = rand() % 10;
		//sleep(10);
		pthread_mutex_lock(&buffer_lock);
			while (ptr_prod % N == ptr_cons % N){
				pthread_cond_wait(&buffer_cond, &buffer_lock);
			}
			buffer[ptr_prod % N] = item;
			printf("Item %d inserido na posicao %d\n", buffer[ptr_prod % N], ptr_prod % N);
			ptr_prod++;
			if ((ptr_prod - 2) % N == ptr_cons % N){
				pthread_cond_broadcast(&buffer_cond);
			}
		pthread_mutex_unlock(&buffer_lock);
	}
  	pthread_exit(0);
}

void * consumidor (void* pi){
	int item;
 	while(1){
		//sleep(rand() % 3);
		pthread_mutex_lock(&buffer_lock);
			while ((ptr_cons + 1) % N == (ptr_prod % N) ){
				pthread_cond_wait(&buffer_cond, &buffer_lock);
			}
			ptr_cons++;
			item = buffer[ptr_cons % N];
			printf("Item %d retirado da posicao %d\n", item, ptr_cons % N);
			buffer[ptr_cons] = 0;
			if ((ptr_prod + 1) % N == ptr_cons % N){
				pthread_cond_broadcast(&buffer_cond);
			}
		pthread_mutex_unlock(&buffer_lock);
  	}
  	pthread_exit(0);
}
