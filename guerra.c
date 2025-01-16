#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>


#define QTO 5  		// Capacidade criação de ordens
#define QTB	10		// Capacidade recebimendo de ordens
#define MMAX 30		// tamanho máximo de uma mensagem
#define CAPOMBO 2	// Capacidade do pombo

// Range Inclusivo []
#define RG_INIT 20
#define RG_END 126
#define RANGE RG_END - RG_INIT + 1



char* mesa_general[QTO];
char* mochila_envio[CAPOMBO];
char* mochila_destino[CAPOMBO];
char* mesa_cabo[QTB];

int ptr_general = 0;
int ptr_mochila = 0;
int ptr_cabo = 0;

char ordens_possiveis[4][MMAX] = {
	"inimigo em alpha",
	"inimigo em bravo",
	"inimigo em charlie",
	"inimigo em delta"
};
char* chave = "brasilacimadetudo";
pthread_mutex_t lock_mensagem = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_mensagem = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock_pombo = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_tenente = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_pombo = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock_cabo = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_cabo = PTHREAD_COND_INITIALIZER;

void* general(void* meuid);
char* pensa_ordem();

void* tenente(void* meuid);
char* codifica(char* mensagem, char* chave);
char* decodifica(char* cifra, char* chave);
int modulo(int a, int b);

void* pombo(void* meuid);



int main(){
	pthread_t gen, ten, pru;
	pthread_create(&gen, NULL, general, NULL);
	pthread_create(&ten, NULL, tenente, NULL);
	pthread_create(&pru, NULL, pombo, NULL);
	pthread_join(gen, NULL);
	return 0;
}

void* general(void* meuid){
	while(1){
		char* ordem = pensa_ordem();
		sleep(rand() % 10);
		pthread_mutex_lock(&lock_mensagem);
			while (ptr_general == QTO){
				pthread_cond_wait(&cond_mensagem, &lock_mensagem);
			}
			mesa_general[ptr_general] = ordem;
			printf("Ordem: \"%s\" inserida na posicao %d\n", mesa_general[ptr_general], ptr_general);
			ptr_general++;
			if (ptr_general == 1){
				pthread_cond_signal(&cond_mensagem);
			}
		pthread_mutex_unlock(&lock_mensagem);

		
	}
  	pthread_exit(0);
}

void* tenente(void* meuid){
 	while(1){
		sleep(rand() % 10);
		pthread_mutex_lock(&lock_mensagem);
			while (ptr_general == 0){
				pthread_cond_wait(&cond_mensagem, &lock_mensagem);
			}
			ptr_general--;
			char* mensagem = mesa_general[ptr_general];
			printf("Ordem: \"%s\" retirada da posicao %d\n", mensagem, ptr_general);
			if (ptr_general == QTO - 1){
				pthread_cond_signal(&cond_mensagem);
			}
		pthread_mutex_unlock(&lock_mensagem);
		char* cripta = codifica(mensagem, chave);
		free(mensagem);
		pthread_mutex_lock(&lock_pombo);
			while(ptr_mochila >= CAPOMBO){
				pthread_cond_wait(&cond_tenente, &lock_pombo);
			}
			mochila_envio[ptr_mochila] = cripta;
			printf("Cripta \"%s\" colocada na posicao %d da mochila\n", mochila_envio[ptr_mochila], ptr_mochila);
			ptr_mochila++;
			if (ptr_mochila == CAPOMBO){
				pthread_cond_signal(&cond_pombo);
			}
		pthread_mutex_unlock(&lock_pombo);
  	}
  	pthread_exit(0);
}

void* pombo(void* meuid){
	while(1){
		pthread_mutex_lock(&lock_pombo);
			while (ptr_mochila != CAPOMBO){
				pthread_cond_wait(&cond_pombo, &lock_pombo);
			}
			for (int i = 0; i < CAPOMBO; i++){
				mochila_destino[i] = mochila_envio[i];
			}
			ptr_mochila = 0;
			pthread_cond_broadcast(&cond_tenente);
		pthread_mutex_unlock(&lock_pombo);

		printf("Tranferindo Mensagens...\n");
		
		pthread_mutex_lock(&lock_cabo);
			while (ptr_cabo > QTB - CAPOMBO){
				pthread_cond_wait(&cond_cabo, &lock_cabo);
			}
			for (int i = 0; i < CAPOMBO; i++){
				mesa_cabo[ptr_cabo + i] = mochila_destino[i];
				printf("Cripta: \"%s\" transferida na posicao %d\n", mesa_cabo[ptr_cabo + i], ptr_cabo + i);
			}
			ptr_cabo += CAPOMBO;

			if (ptr_cabo ==  CAPOMBO){
				pthread_cond_signal(&cond_cabo);
			}
		pthread_mutex_unlock(&lock_cabo);

	}
}

char* pensa_ordem(){
	char* ordem = (char*) malloc(MMAX);
	strcpy(ordem, ordens_possiveis[rand() % 4]);
	return ordem;
}

char* codifica(char* mensagem, char* chave){
    int tmn_chave = strlen(chave);
    int tmn_mensagem = strlen(mensagem);
    char* cifra = (char*) malloc(tmn_mensagem + 1);
    int c_mensagem, c_chave, c_cifra;

    for (int i = 0; i < tmn_mensagem; i++){
        c_mensagem = mensagem[i];
        c_chave = chave[i % tmn_chave];

        c_cifra = modulo( (c_chave - RG_INIT ) + (c_mensagem - RG_INIT), RANGE) + RG_INIT;
        cifra[i] = c_cifra;
    }
    cifra[tmn_mensagem] = '\0';
    return cifra;
}

int modulo(int a, int b) {
  int m = a % b;
  if (m < 0) {
    m += (b < 0) ? -b : b; 
  }
  return m;
}
