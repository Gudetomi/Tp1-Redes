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
#define NEXT_PLAYER (id = (id + 1) % TOTAL_CONECTIONS); // Passar para o próximo jogador
#define PREVIOUS_PLAYER (id = (id % TOTAL_CONECTIONS) == 0 ? TOTAL_CONECTIONS - 1 : (id - 1) % TOTAL_CONECTIONS);  // Voltar pro jogador anteior
#define CLEAR_ROUND rodadas[0] = -1; rodadas[1] = -1; rodadas[2] = -1; // Limpa os ganhadores dos rounds
#define RESET_ROUND free(vet); free(qtd); free(rodada_atual); free(usadas); free(ids_truco); // Limpa os vetores de cartas dos clientes e quais já foram usadas no round
#define FREE_ALL free(cartas); free(vet); free(qtd); free(rodada_atual); free(usadas); free(rodadas); free(queda); free(jogo); // Libera todo espaco de memoria
#define RECUSADO (strcmp(buffer, "RECUSADO") == 0)
#define AUMENTAR (strcmp(buffer, "AUMENTAR") == 0)
#define CONTINUA (strcmp(buffer, "CONTINUA") == 0)
#define PEDIRAM_TRUCO (truco == 1)
#define NAO_JOGOU_TRUCADO (skip == 0)
#define JOGADOR_TRUCADO (ids_truco[1] == id)
#define AUMENTARAM_APOSTA (valor == 1)
#define FIM_RODADA (qtd_jogadas == 4)
#define HOUVE_GANHADOR (ganhador != 5)
#define EMPATE_1RODADA (num_rodada == 0)
#define NAO_EMPATOU_PRIMEIRA (rodadas[0] != 2)
#define JOGO_ACABOU (verifica != -1)


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