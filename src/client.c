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
#define KRED  "\x1B[31m"
#define RESET "\x1B[0m"


typedef struct{ // Struct para guardar as cartas
    char valor;
    char naipe;
    int forca;
}Carta;


void Print_Carta(Carta *cartas, int tam){
    char cor[4][11] = {"\033[0;31m", "\033[0m", "\033[0m", "\033[0;31m"};
    char naipe[4][7] = {"\u2665", "\u2663", "\u2660", "\u2666"};
    int map_naipes[3] = {0, 0, 0};

    for (int i=0; i < tam; i++) {
        switch (cartas[i].naipe) {
            case 'C': // Copas
                map_naipes[0] = 0;
                break;
            case 'P': // Paus
                map_naipes[0] = 1;
                break;
            case 'E': // Espadas
                map_naipes[0] = 2;
                break;
            case 'O': // Ouro
                map_naipes[0] = 3;
                break;
        }
    }

    switch(tam){
        case 1: // Copas
            printf("  0\n");
            printf(" ____\n");
            printf("|%s%s\033[0m   |\n", cor[map_naipes[0]], naipe[map_naipes[0]]);
            printf("|  %c |\n", cartas[0].valor);
            printf(" \u203E\u203E\u203E\u203E\n");
            break;
        case 2: // Ouro
            printf("  0      1\n");
            printf(" ____   ____\n");
            printf("|%s%s\033[0m   | |%s%s\033[0m   |\n", cor[map_naipes[0]], naipe[map_naipes[0]], cor[map_naipes[1]], naipe[map_naipes[1]]);
            printf("|  %c | |  %c |\n", cartas[0].valor, cartas[1].valor);
            printf(" \u203E\u203E\u203E\u203E   \u203E\u203E\u203E\u203E\n");
            break;
        case 3: // Espadas
            printf("  0      1      2  \n");
            printf(" ____   ____   ____\n");
            printf("|%s%s\033[0m   | |%s%s\033[0m   | |%s%s\033[0m   |\n", cor[map_naipes[0]], naipe[map_naipes[0]], cor[map_naipes[1]], naipe[map_naipes[1]], cor[map_naipes[2]], naipe[map_naipes[2]]);
            printf("|  %c | |  %c | |  %c |\n", cartas[0].valor, cartas[1].valor, cartas[2].valor);
            printf(" \u203E\u203E\u203E\u203E   \u203E\u203E\u203E\u203E   \u203E\u203E\u203E\u203E\n");
            break;
    }
}

Carta Escolhe_Carta(Carta *cartas, int tam, int *opcao){
    int valor;
    Print_Carta(cartas, tam);
    printf("Escolha o ID da carta que deseja jogar: ");
    scanf("%i", &valor);
    *opcao = valor;
    return cartas[*opcao];
}


void error(const char *msg){
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]){
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[1024];

    Carta *cartas;
    Carta escolha;
    int qtd, i, opcao = 0;

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
    
    while(1){
        bzero(buffer,1024);
        n = read(sockfd, buffer, 4); // Espero (Wait), ou Jogo (Play)?
        if (n < 0){
            error("ERROR reading to socket");
        }
        if (strcmp(buffer, "Play") == 0){ // Deixa o cliente jogar
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
            escolha = Escolhe_Carta(cartas,qtd,&opcao); // Mostra as cartas pro cliente e faz ele decidir qual vai jogar
            buffer[0] = escolha.valor;
            buffer[1] = escolha.naipe;
            buffer[2] = escolha.forca + '0';
            buffer[3] = opcao + '0';
            n = write(sockfd, buffer, 4); // Volta pro servidor qual carta foi escolhida
            n = write(sockfd, "Joguei",6); // Avisa que terminou de jogar
            free(cartas); // Desaloca espaço
            bzero(buffer,1024);
            n = read(sockfd, buffer, 4); // Recebe a mensagem do servidor para ficar aguardando a próxima jogada
        }
    }
    close(sockfd);
    return 0;
}
