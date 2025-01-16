#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Range Inclusivo []
#define RG_INIT 20
#define RG_END 126

int Range = RG_END - RG_INIT + 1;

char* codificador(char* mensagem, char* chave);
char* decodificador(char* cifra, char* chave);
int modulo(int a, int b);

int main(){
    char* chave = "Tame Impala";
    char* mensagem = "Qual seria o seu nome?";
    char* cifra = codificador(mensagem, chave); 
    printf("A cifra eh: %s\n", cifra);
    mensagem = decodificador(cifra, chave);
    printf("A mensagem eh: %s\n", mensagem);
    free(cifra);
    free(mensagem);
    return 0;
}


char* codificador(char* mensagem, char* chave){
    int tmn_chave = strlen(chave);
    int tmn_mensagem = strlen(mensagem);
    char* cifra = (char*) malloc(tmn_mensagem + 1);
    int c_mensagem, c_chave, c_cifra;

    for (int i = 0; i < tmn_mensagem; i++){
        c_mensagem = mensagem[i];
        c_chave = chave[i % tmn_chave];

        c_cifra = modulo( (c_chave - RG_INIT ) + (c_mensagem - RG_INIT), Range) + RG_INIT;
        cifra[i] = c_cifra;
    }
    cifra[tmn_mensagem] = '\0';
    return cifra;
}


char* decodificador(char* cifra, char* chave){
    int tmn_chave = strlen(chave);
    int tmn_cifra = strlen(cifra);
    char* mensagem = (char*) malloc(tmn_cifra + 1);
    int c_cifra, c_chave, c_mensagem;

    for (int i = 0; i < tmn_cifra; i++){
        c_cifra = cifra[i];
        c_chave = chave[i % tmn_chave];

        c_mensagem = modulo(c_cifra - c_chave, Range) + RG_INIT;
        mensagem[i] = c_mensagem;
    }
    mensagem[tmn_cifra] = '\0';
    return mensagem;
}

int modulo(int a, int b) {
  int m = a % b;
  if (m < 0) {
    m += (b < 0) ? -b : b; 
  }
  return m;
}