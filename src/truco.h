#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>

#define MAX_CARTAS 40 // Quantidade de cartas em um baralho de truco
#define TOTAL_CONECTIONS 4 // Quantas conexões o servidor vai aceitar


typedef struct Carta Carta;

Carta Escolhe_Carta(Carta *cartas, int tam);

Carta *Define_Forca(Carta *cartas);

Carta* Pega_Baralho();

Carta Armazena_Carta(Carta carta, char buffer[]);

void Verifica_Queda(int sockfd, char *buffer);

int Define_Truco(int sockfd);

char *Define_Valor(int valor);

int* Distribui_Cartas();

int Ganhador(Carta carta1, Carta carta2, Carta carta3, Carta carta4);

int Analisa_Rodada(int *rodadas);

int* Remove_Carta_Cliente(char valor, char naipe, Carta *cartas, int *vet, int id);

int *Atualiza_Vetor(int *vet, int valor, int tam);

int Verifica_Jogo(int *vet, int qtd);

void Zera_Variaveis_Queda(int *empate, int *recusado, int*multiplicador, int*rodada_trucada, int* truco, int*skip);

void error(const char *msg);