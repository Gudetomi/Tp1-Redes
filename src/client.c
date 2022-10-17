/* Truco UFSJ multiplayer, cliente e servidor TCP
    Antônio - 
    Gustavo - 
    Wagner Lancetti - wlancetti@gmail.com
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netdb.h>
#define KRED  "\x1B[31m"
#define RESET "\x1B[0m"


typedef struct{ // Struct para guardar as cartas
    char valor;
    char naipe;
    int forca;
}Carta;

void Print_Init(){
    printf(" _______  _______  _______  _______  _______\n");
    printf("|\\     /||\\     /||\\     /||\\     /||\\     /|\n");
    printf("| +---+ || +---+ || +---+ || +---+ || +---+ |   _   _   ___   ___      _ \n");
    printf("| |   | || |   | || |   | || |   | || |   | |  | | | | | __| / __|  _ | |\n");
    printf("| |T  | || |r  | || |u  | || |c  | || |o  | |  | |_| | | _|  \\__ \\ | || |\n");
    printf("| +---+ || +---+ || +---+ || +---+ || +---+ |   \\___/  |_|   |___/  \\__/ \n");
    printf("|/_____\\||/_____\\||/_____\\||/_____\\||/_____\\|\n\n\n");

}

void Print_Carta(Carta *cartas, int tam, int mesa){
    char cor[4][11] = {"\033[0;31m", "\033[0m", "\033[0m", "\033[0;31m"};
    char naipe[4][7] = {"\u2665", "\u2663", "\u2660", "\u2666"};
    int map_naipes[3] = {0, 0, 0};

    for (int i=0; i < tam; i++) {
        switch (cartas[i].naipe) {
            case 'C': // Copas
                map_naipes[i] = 0;
                break;
            case 'P': // Paus
                map_naipes[i] = 1;
                break;
            case 'E': // Espadas
                map_naipes[i] = 2;
                break;
            case 'O': // Ouro
                map_naipes[i] = 3;
                break;
        }
    }

    switch(tam){
        case 1: // Copas
            if (!mesa)
                printf("  0\n");
            printf(" ____\n");
            printf("|%s%s\033[0m   |\n", cor[map_naipes[0]], naipe[map_naipes[0]]);
            printf("|  %c |\n", cartas[0].valor);
            printf(" \u203E\u203E\u203E\u203E\n");
            break;
        case 2: // Ouro
            if (!mesa)
                printf("  0      1\n");
            printf(" ____   ____\n");
            printf("|%s%s\033[0m   | |%s%s\033[0m   |\n", cor[map_naipes[0]], naipe[map_naipes[0]], cor[map_naipes[1]], naipe[map_naipes[1]]);
            printf("|  %c | |  %c |\n", cartas[0].valor, cartas[1].valor);
            printf(" \u203E\u203E\u203E\u203E   \u203E\u203E\u203E\u203E\n");
            break;
        case 3: // Espadas
            if (!mesa)
                printf("  0      1      2  \n");
            printf(" ____   ____   ____\n");
            printf("|%s%s\033[0m   | |%s%s\033[0m   | |%s%s\033[0m   |\n", cor[map_naipes[0]], naipe[map_naipes[0]], cor[map_naipes[1]], naipe[map_naipes[1]], cor[map_naipes[2]], naipe[map_naipes[2]]);
            printf("|  %c | |  %c | |  %c |\n", cartas[0].valor, cartas[1].valor, cartas[2].valor);
            printf(" \u203E\u203E\u203E\u203E   \u203E\u203E\u203E\u203E   \u203E\u203E\u203E\u203E\n");
            break;
    }
}

void PrintaMesa(int placar1, int placar2){
    char p1[3], p2[3];
    sprintf(p1, "%d", placar1);
    sprintf(p2, "%d", placar2);

    if (placar1 <= 9){
        p1[1] = ' ';
        p1[2] = '\0';
    }
    if (placar2 <= 9){
        p2[1] = ' ';
        p2[2] = '\0';
    }
    printf("________________________________________________________________\n");
    printf("╔═╗┬  ┌─┐┌─┐┌─┐┬─┐\n");
    printf("╠═╝│  ├─┤│  ├─┤├┬┘\n");
    printf("╩  ┴─┘┴ ┴└─┘┴ ┴┴└─\n");
    printf(" ____   ____\n");
    printf("| %s | | %s |\n", p1, p2);
    printf("| t1 | | t2 |\n");
    printf(" \u203E\u203E\u203E\u203E   \u203E\u203E\u203E\u203E\n");

    printf("╔╦╗┌─┐┌─┐┌─┐\n");
    printf("║║║├┤ └─┐├─┤\n");
    printf("╩ ╩└─┘└─┘┴ ┴\n");
}

Carta Escolhe_Carta(Carta *cartas, int tam){ // Qual carta o cliente vai usar
    int opcao;
    Print_Carta(cartas, tam, 0);
    printf("\n");
    printf("Escolha o ID da carta que deseja jogar: ");
    scanf("%i", &opcao);
    return cartas[opcao];
}


void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void Verifica_Queda(int sockfd, char *buffer){
    int n;
    n = read(sockfd,buffer,5); // "QUEDA" ou "GOING"
    if(strcmp(buffer, "QUEDA") == 0){ // Verifica se a queda acabou
        bzero(buffer, 1024);
        n = read(sockfd,buffer, 1);
        printf("\n\nA dupla %c ganhou a queda!\n\n", buffer[0]);
        bzero(buffer, 1024);
    }
    n++;
}

int main(int argc, char *argv[]){
    system("clear");
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[1024];

    Carta *cartas, *mesa, *vencedora;
    Carta escolha;
    int qtd, qtd_mesa, i;

    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    Print_Init();
    
    bzero(buffer,1024); 
    while(1){
        n = read(sockfd, buffer, 4); // Espero (Wait), ou Jogo (Play)?
        if (n < 0){
            error("ERROR reading to socket");
        }
        if (strcmp(buffer, "Play") == 0){ // Deixa o cliente jogar
            bzero(buffer,1024);
            // Pega a pontuação
            n = read(sockfd, buffer, 2); // Placar
            // Mostrar a mesa //
            PrintaMesa(buffer[0] - '0', buffer[1] - '0');
            bzero(buffer,1024);
            n = read(sockfd, buffer, 1); // Qtd de cartas na mesa
            qtd_mesa = buffer[0] - '0'; // Quantidade de cartas que estão na mesa
            bzero(buffer, 1024);
            if(qtd_mesa != 0){ // Se houver cartas na mesa
                mesa = (Carta*)calloc(qtd_mesa, sizeof(Carta));
                for(i = 0; i < qtd_mesa; i++){
                    n = read(sockfd, buffer, 3);
                    mesa[i].valor = buffer[0];
                    mesa[i].naipe = buffer[1];
                    mesa[i].forca = buffer[2] - '0'; // transformar char em int
                    bzero(buffer,1024);
                }
                Print_Carta(mesa, qtd_mesa, 1); // Mostrar as cartas pro cliente
                free(mesa);
                printf("\n\n\n");
            }else{
                printf("╦  ╦┌─┐┌─┐┬┌─┐\n");
                printf("╚╗╔╝├─┤┌─┘│├─┤\n");
                printf(" ╚╝ ┴ ┴└─┘┴┴ ┴\n\n");

            }
            printf("________________________________________________________________\n");

            n = read(sockfd, buffer,1);  // Lê quantas cartas o cliente ainda tem na rodada
            qtd = atoi(buffer);
            bzero(buffer,1024); // Zera o buffer
            cartas = (Carta*)calloc(qtd, sizeof(Carta));

            for(i = 0; i < qtd; i++){ // Lê cada uma das cartas do cliente (só o servidor conhece elas)
                n = read(sockfd, buffer, 3);
                cartas[i].valor = buffer[0];
                cartas[i].naipe = buffer[1];
                cartas[i].forca = buffer[2] - '0'; // transformar char em int
                bzero(buffer,1024);
            }           

            escolha = Escolhe_Carta(cartas,qtd); // Mostra as cartas pro cliente e faz ele decidir qual vai jogar
            buffer[0] = escolha.valor;
            buffer[1] = escolha.naipe;
            buffer[2] = escolha.forca + '0';
            n = write(sockfd, buffer, 3); // Volta pro servidor qual carta foi escolhida

            free(cartas); // Desaloca espaço
            
            bzero(buffer,1024);
            n = read(sockfd, buffer, 3); // Recebe a mensagem de quem ganhou a rodada
            if (strcmp(buffer, "Emp") != 0){ // Se houver um ganhador da rodada
                vencedora = (Carta *) malloc(1 * sizeof(Carta));
                vencedora[0].valor = buffer[1];
                vencedora[0].naipe = buffer[2];
                system("clear");
                printf("\n\nQuem ganhou a rodada foi o jogador %c, usando a carta:\n", buffer[0]);
                Print_Carta(vencedora, 1, 0); // Mostra a carta vencedora
                free(vencedora);
                bzero(buffer, 1024);
                Verifica_Queda(sockfd, buffer); // Verifica se a queda acabou
            }else{ // Se empatar
                system("clear");
                bzero(buffer, 1024);
                n = read(sockfd, buffer, 7); // Qual rodada empatou
                if (strcmp(buffer, "1Rodada") == 0){ // Empate na primeira rodada
                    printf("\nEmpatou na primeira rodada! Quem ganhar qualquer uma das proximas rodadas vence!\n\n");
                }else{ // Empate na 2 ou 3 rodada
                    bzero(buffer, 1024);
                    n = read(sockfd, buffer, 3); // Recebe a carta que ganhou ou avisando que empatou dnv
                    if (strcmp(buffer, "Emp") == 0){
                        printf("\nEmpatou novamente...\n\n");
                    }else{
                        printf("\n\nQuem ganhou a queda foi a dupla %c, por ter ganhado a primeira rodada.\n\n", buffer[0]);
                        Verifica_Queda(sockfd, buffer);
                    }
                }
            }
            bzero(buffer, 1024);
            strcpy(buffer, "Wait");
            printf("_________________________________________________________\n\n");
            printf("\n\nAguardando para jogar...\n\n");
        }
    }
    close(sockfd);
    return 0;
}
