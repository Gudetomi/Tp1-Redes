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


typedef struct{ // Struct para guardar as cartas
    char valor;
    char naipe;
    int forca;
}Carta;


void Print_Carta(Carta cartas){
    printf("%c  ", cartas.valor);
    switch(cartas.naipe){
        case 'C': // Copas
            printf("♥  ");
            break;
        case 'O': // Ouro
            printf("♦  ");
            break;
        case 'E': // Espadas
            printf("♠  ");
            break;
        case 'P': // Paus
            printf("♣  ");
            break;
    printf("%i\n\n",cartas.forca);
    }
}

Carta Escolhe_Carta(Carta *cartas, int tam){ // Qual carta o cliente vai usar
    int i, opcao;
    for(i = 0; i < tam; i++){
        printf("Opção %i: ",i);
        Print_Carta(cartas[i]);
        printf("\n");
    }
    printf("Escolha o ID da carta que deseja jogar: ");
    scanf("%i", &opcao);
    return cartas[opcao];
}


void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[1024];

    Carta *cartas, *mesa;
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
    
    while(1){
        bzero(buffer,1024);
        n = read(sockfd, buffer, 4); // Espero (Wait), ou Jogo (Play)?
        if (n < 0){
            error("ERROR reading to socket");
        }
        if (strcmp(buffer, "Play") == 0){ // Deixa o cliente jogar
            bzero(buffer,1024);
            // Mostrar a mesa //
            n = read(sockfd, buffer, 4); // "Mesa"
            printf("\n%s\n", buffer);
            /* Apaga a linha de cima (108)
            if (strcmp(buffer, "Mesa") == 0){
                // Printf do Antonio da palavra Mesa
            }
            */
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
                    Print_Carta(mesa[i]); // Mostrar as cartas pro cliente
                    printf("\n");
                    bzero(buffer,1024);
                }
                // Printf do antonio das cartas (Apaga a chamada da função Print_Carta e do printf tambem dentro do for)
                free(mesa);
                printf("\n\n");
            }else{
                printf("Mesa vazia!\n\n");
            }

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
            n = write(sockfd, "Joguei",6); // Avisa que terminou de jogar
            
            free(cartas); // Desaloca espaço
            bzero(buffer,1024);
            printf("_________________________________________________________\n\n");
            n = read(sockfd, buffer, 4); // Recebe a mensagem do servidor para ficar aguardando a próxima jogada
        }
    }
    close(sockfd);
    return 0;
}
