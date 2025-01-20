#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

// Aluno João Guilherme Silva Ribeiro
// Mat: 222011641


#define QTO 3  		// Capacidade criação de ordens
#define QTB	10		// Capacidade recebimendo de ordens
#define MMAX 40		// tamanho máximo de uma mensagem
#define CAPOMBO 2	// Capacidade do pombo

#define G 2			// Capacidade Generais
#define C 3			// Capacidade Cabos
#define T 3			// Capacidade Tenentes
#define P 2			// Capacidade Pombos

// Range Inclusivo []
#define RG_INIT 20
#define RG_END 126
#define RANGE RG_END - RG_INIT + 1


// Buffers dos processos 
char* mesa_general[QTO];
char* mochila_envio[CAPOMBO];
char* mochila_destino[CAPOMBO];
char* mesa_cabo[QTB];

// Índices dos buffers
int ptr_general = 1;
int ptr_mochila = 0;
int ptr_cabo = 0;
int ptr_tenente = 0;


// Material para geração de ordens
char specs[6][15] = {
	"Infantaria",
	"Cavalaria",
	"Artilharia",
	"Drone",
	"Tanque",
	"Objetivo",
};

char cordenadas[6][10] = {
	"alpha",
	"bravo",
	"charlie",
	"delta",
	"epsilon",
	"phi"
};
char* chave = "gigantepelaproprianatureza";

// Mecanismos de sincronização dos processos
pthread_mutex_t lock_mensagem = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_mensagem = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock_pombo = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_tenente = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_pombo = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock_cabo = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_cabo = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock_msn_unica = PTHREAD_MUTEX_INITIALIZER;

// Atores do problema
void* general(void* meuid);
void* tenente(void* meuid);
void* pombo(void* meuid);
void* cabo(void* meuid);

// funções Auxiliares
char* pensa_ordem();
char* codifica(char* mensagem, char* chave);
char* decodifica(char* cifra, char* chave);
int modulo(int a, int b);

int main(){
	pthread_t gen[G], cab[C], ten[T], pru[P];
	int* ptr_gen;
	for (int i = 0; i < G; i++){
		ptr_gen = (int*) malloc(sizeof(int));
		*ptr_gen = i;
		pthread_create(&gen[i], NULL, general, ptr_gen);	
	}
	
	for (int i = 0; i < T; i++){
		pthread_create(&ten[i], NULL, tenente, NULL);	
	}
	
	for (int i = 0; i < P; i++){
		pthread_create(&pru[i], NULL, pombo, NULL);	
	}

	for (int i = 0; i < C; i++){
		pthread_create(&cab[i], NULL, cabo, NULL);	
	}
	
	pthread_join(gen[0], NULL);
	return 0;
}

void* general(void* meuid){
	int ptr_mod;
	while(1){
		// Pensa e insere a ordem
		char* ordem = pensa_ordem(*((int*) meuid));
		sleep(rand() % 3);
		pthread_mutex_lock(&lock_mensagem);
			while (ptr_general % QTO == ptr_tenente % QTO){
				pthread_cond_wait(&cond_mensagem, &lock_mensagem);
			}
			ptr_mod = ptr_general % QTO;
			mesa_general[ptr_mod] = ordem;
			printf("Ordem: \"%s\". Inserida na posicao %d da mesa general\n", mesa_general[ptr_mod], ptr_mod);;
			ptr_general++;
			if ((ptr_general - 2) % QTO == ptr_tenente % QTO){
				pthread_cond_signal(&cond_mensagem);
			}
		pthread_mutex_unlock(&lock_mensagem);
	}
  	pthread_exit(0);
}

void* tenente(void* meuid){
	char* mensagem; 
	char* cripta;
 	while(1){
		sleep(rand() % 3);
		// Retira a ordem da mesa do General
		pthread_mutex_lock(&lock_mensagem);
			while ((ptr_tenente + 1) % QTO == (ptr_general % QTO) ){
				pthread_cond_wait(&cond_mensagem, &lock_mensagem);
			}
			ptr_tenente++;
			int ptr_mod = ptr_tenente %  QTO;
			mensagem = mesa_general[ptr_mod];
			printf("Ordem: \"%s\" retirada da posicao %d da mesa general\n", mensagem, ptr_mod);
			if ((ptr_general + 1) % QTO == ptr_mod){
				pthread_cond_broadcast(&cond_mensagem);
			}
		pthread_mutex_unlock(&lock_mensagem);

		// Ciptografa a mensagem
		cripta = codifica(mensagem, chave);
		free(mensagem);

		// Coloca a cifra na mochila de envio
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
		// Transfere as mensagens entre as mochilas
		pthread_mutex_lock(&lock_pombo);
			while (ptr_mochila != CAPOMBO){
				pthread_cond_wait(&cond_pombo, &lock_pombo);
			}
			for (int i = 0; i < CAPOMBO; i++){
				mochila_destino[i] = mochila_envio[CAPOMBO - i - 1];
			}
			ptr_mochila = 0;
			pthread_cond_broadcast(&cond_tenente);
		pthread_mutex_unlock(&lock_pombo);

		printf("Tranferindo Mensagens...\n");
		sleep(rand() % 3);
		
		// Adiciona as mensagem na mesa do Cabo
		pthread_mutex_lock(&lock_cabo);
			while (ptr_cabo > QTB - CAPOMBO){
				pthread_cond_wait(&cond_cabo, &lock_cabo);
			}
			for (int i = 0; i < CAPOMBO; i++){
				mesa_cabo[ptr_cabo + i] = mochila_destino[i];
				printf("Cripta: \"%s\" transferida na posicao %d para mesa cabo\n", mesa_cabo[ptr_cabo + i], ptr_cabo + i);
			}
			ptr_cabo += CAPOMBO;

			if (ptr_cabo ==  CAPOMBO){
				pthread_cond_signal(&cond_cabo);
			}
		pthread_mutex_unlock(&lock_cabo);
	}
}

void* cabo(void* meuid){
	char* cripta;
	char* mensagem;
	while(1){
		// Retira as cifra de sua mesa
		pthread_mutex_lock(&lock_cabo);
				while (ptr_cabo == 0){
					pthread_cond_wait(&cond_cabo, &lock_cabo);
				}
				ptr_cabo--;
				cripta = mesa_cabo[ptr_cabo];
				printf("Cripta: \"%s\" retirada da posicao %d da mesa cabo\n", cripta, ptr_cabo);
				if (ptr_cabo == QTO - 1){
					pthread_cond_signal(&cond_cabo);
				}
		pthread_mutex_unlock(&lock_cabo);

		// Desciptrografa a mensagem
		mensagem = decodifica(cripta, chave);
		free(cripta);
		sleep(1);
		// Executa a ordem
		printf("A Mensagem eh: \"%s\"\n", mensagem);
		free(mensagem);
	}

}

char* pensa_ordem(int id){
	char* ordem = (char*) malloc(MMAX);
	sprintf(ordem, "%s em %s", specs[(rand() + id) % 6], cordenadas[(rand() + id) % 6]);
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
		// Calcula o deslocamento da criptografia
        c_cifra = modulo( (c_chave - RG_INIT ) + (c_mensagem - RG_INIT), RANGE) + RG_INIT;
        cifra[i] = c_cifra;
    }
    cifra[tmn_mensagem] = '\0';
    return cifra;
}

char* decodifica(char* cifra, char* chave){
    int tmn_chave = strlen(chave);
    int tmn_cifra = strlen(cifra);
    char* mensagem = (char*) malloc(tmn_cifra + 1);
    int c_cifra, c_chave, c_mensagem;

    for (int i = 0; i < tmn_cifra; i++){
		// Desfaz o deslocamento da criptografia
        c_cifra = cifra[i];
        c_chave = chave[i % tmn_chave];
        c_mensagem = modulo(c_cifra - c_chave, RANGE) + RG_INIT;
        mensagem[i] = c_mensagem;
    }
    mensagem[tmn_cifra] = '\0';
    return mensagem;
}

// Calcula o módulo matemático entre dois numeros inteiros
int modulo(int a, int b) {
  int m = a % b;
  if (m < 0) {
    m += (b < 0) ? -b : b; 
  }
  return m;
}
