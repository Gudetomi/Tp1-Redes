/* Truco UFSJ multiplayer, cliente e servidor TCP

Antônio - 
Gustavo - 
Wagner Lancetti - wlancetti@gmail.com

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define TOTAL_CONECTIONS 4 // Quantas conexões o servidor vai aceitar
#define MAX_CARTAS 40
#define CHANGE_PLAYER (id = (id + 1) % TOTAL_CONECTIONS); // Alterar o jogador que vai jogar


typedef struct{ // Struct para guardar as cartas
    char valor;
    char naipe;
    int forca;
}Carta;



void error(const char *msg)
{
    perror(msg);
    exit(1);
}


void Escolhe_Naipe(Carta carta){
    switch(carta.naipe){
        case 'C': // Copas
            printf("♥");
            break;
        case 'O': // Ouro
            printf("♦");
            break;
        case 'E': // Espadas
            printf("♠");
            break;
        case 'P': // Paus
            printf("♣");
            break;
    }
}

void Print_Baralho(Carta *cartas){
    int i;
    for(i = 0; i < MAX_CARTAS; i++){
        printf("%c  ", cartas[i].valor);
        Escolhe_Naipe(cartas[i]);
        printf("  %i\n",cartas[i].forca);
    }
}

Carta *Define_Forca(Carta *cartas){ // Função que determina qual carta é mais forte, seguindo a regra do truco mineiro
    // 4♣ > 7♥ > A♠ > 7♦ > 3 > 2 > K > J > Q > 7 > 6 > 5 > 4
    int i = 0;
    for (i = 0; i < MAX_CARTAS; i++){
        if(cartas[i].valor == '4' && cartas[i].naipe == 'P'){
            cartas[i].forca = 14;
        }else if(cartas[i].valor == '7' && cartas[i].naipe == 'C'){
            cartas[i].forca = 13;
        }else if(cartas[i].valor == 'A' && cartas[i].naipe == 'E'){
            cartas[i].forca = 12;
        }else if(cartas[i].valor == '7' && cartas[i].naipe == 'O'){
            cartas[i].forca = 11;
        }else if (cartas[i].valor == '3'){
            cartas[i].forca = 10;
        }else if (cartas[i].valor == '2'){
            cartas[i].forca = 9;
        }else if (cartas[i].valor == 'A'){
            cartas[i].forca = 8;
        }else if (cartas[i].valor == 'K'){
            cartas[i].forca = 7;
        }else if (cartas[i].valor == 'J'){
            cartas[i].forca = 6;
        }else if (cartas[i].valor == 'Q'){
            cartas[i].forca = 5;
        }else if (cartas[i].valor == '7'){
            cartas[i].forca = 4;
        }else if (cartas[i].valor == '6'){
            cartas[i].forca = 3;
        }else if (cartas[i].valor == '5'){
            cartas[i].forca = 2;
        }else if (cartas[i].valor == '4'){
            cartas[i].forca = 1;
        }else{
            printf("Oi?\n\n");
        }
    }
    return cartas;
}

Carta* Pega_Baralho(){ // Lê o arquivo para guardar o baralho
    int i = 0;
    char n[2];
    Carta *cartas = (Carta*)calloc(MAX_CARTAS, sizeof(Carta));
    FILE *fp = fopen("cartas.txt","r");
    while(!feof(fp)){
        fscanf(fp, "%s", n);
        cartas[i].valor = n[0];
        cartas[i].naipe = n[1];
        i+=1;
    }
    fclose(fp);
    cartas = Define_Forca(cartas);
    return cartas;
}

int* Distribui_Cartas(){ // Escolhe as cartas que cada cliente vai receber (3 primeiras do primeiro cliente, as 3 próximas do segundo, etc)
    int *num = (int*)calloc(TOTAL_CONECTIONS * 3, sizeof(int));
    int i = 0, j, confere;
    srand(time(NULL));
    do{
        num[i] = rand()%(MAX_CARTAS-1); // Um número de 0 até a qtd de cartas do baralho no truco (40)
        confere = 0;
        for(j = 0; j < i; j++){ // Garante que não haverá repetição de cartas
            if(num[j] == num[i]){
                confere = 1;
            }
        }
        if (confere == 0){
            i++;
        }
    }while(i < TOTAL_CONECTIONS * 3);
    return num;
}

int Ganhador(Carta carta1, Carta carta2, Carta carta3, Carta carta4){ // Função que define qual carta ganha, ou se da empate
    int maior = 0, forca;

    // Define qual cliente usou a maior carta
    if (carta1.forca >= carta2.forca){ // Se a carta 1 for mais forte que a 2
        maior = 1;
        forca = carta1.forca;
    }else{ // Se a carta 2 for mais forte que a 1
        maior = 2;
        forca = carta2.forca;
    }
    if (forca < carta3.forca){ // Se a carta 3 for mais forte que a 1 e a 2
        maior = 3;
        forca = carta3.forca;
    }
    if (forca < carta4.forca){ // Se a carta 4 for mais forte que a 1, 2 e 3
        maior = 4;
        forca = carta4.forca;
    }

    // Verifica se houve empate
    if (maior == 1 && (forca == carta2.forca || forca == carta3.forca || forca == carta4.forca)){
        maior = 5;
    }else if(maior == 2 && (forca == carta1.forca || forca == carta3.forca || forca == carta4.forca)){
        maior = 5;
    }else if(maior == 3 && (forca == carta1.forca || forca == carta2.forca || forca == carta4.forca)){
        maior = 5;
    }else if(maior == 4 && (forca == carta1.forca || forca == carta2.forca || forca == carta3.forca)){
        maior = 5;
    }

    // Retorna o id do cliente que usou a maior carta (1,2,3 ou 4), ou empate (5)
    return maior;
}

int main(int argc, char *argv[]){
    int sockfd, portno, newsockfd[TOTAL_CONECTIONS];
    int id = 0, i, indice;
    socklen_t clilen[TOTAL_CONECTIONS];
    char buffer[1024];
    struct sockaddr_in serv_addr, cli_addr[TOTAL_CONECTIONS];
    int n, ganhador;

    Carta *cartas = Pega_Baralho(); // Lê o arquivo e armazena o baralho
    Carta *rodada_atual;
    Print_Baralho(cartas);
    int *vet;
    int *qtd;


    if (argc < 2){
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) 
        error("ERROR opening socket");

    // Inicializa o servidor
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd,TOTAL_CONECTIONS); // Estabelece a quantidade de conexões que o servidor pode aceitar


    for(i = 0; i < TOTAL_CONECTIONS; i++){ // Aceita todas as conexões dos clientes
        clilen[i] = sizeof(cli_addr[i]);
        
        newsockfd[i] = accept(sockfd, (struct sockaddr *) &cli_addr[i], &clilen[i]);
        if (newsockfd[i] < 0){
            error("ERROR on accept client");
            break;
        }
    }

    bzero(buffer,1024);
    vet = (int*)calloc(TOTAL_CONECTIONS*3, sizeof(int)); // Cada cliente: 3 posições de cartas
    vet = Distribui_Cartas(); // Da 3 cartas para cada usuário (sem repetição)
    qtd = (int*)calloc(TOTAL_CONECTIONS, sizeof(int));
    for(i = 0; i < TOTAL_CONECTIONS; i++){ // Cada cliente começa com 3 cartas
        qtd[i] = 3;
    }
    rodada_atual = (Carta*)calloc(4, sizeof(Carta));
    while(1) {
        n = write(newsockfd[id], "Play", 4); // Coloca um Cliente para jogar
        if (n < 0){
            error("ERROR writing to client");
        }
        // Envia as cartas que o cliente tem
        bzero(buffer,1024);
        buffer[0] = qtd[id] + '0'; // Transforma inteiro em char
        n = write(newsockfd[id], buffer, 1); // Manda para o cliente quantas cartas ele ainda possui na rodada
        indice = id*3; // Pega o ID do usuário que vai jogar (serve para pegar as 3 cartas aleatórias que caíram para o jogador)
        bzero(buffer,1024);
        for(i = 0; i < qtd[id]; i++){
            indice = vet[id * 3 + i]; // Pega a carta aleatória do cliente 
            buffer[0] = cartas[indice].valor;
            buffer[1] = cartas[indice].naipe;
            buffer[2] = cartas[indice].forca + '0'; // Transformar int em char
            n = write(newsockfd[id], buffer, 3); // Manda para o cliente suas cartas
        }
        bzero(buffer,1024);
        n = read(newsockfd[id], buffer, 3); // Lê qual carta o cliente escolheu jogar
        rodada_atual[id].valor = buffer[0];
        rodada_atual[id].naipe = buffer[1];
        rodada_atual[id].forca = buffer[2] - '0'; // Converte char em int
        printf("%c %c %i\n",rodada_atual[id].valor, rodada_atual[id].naipe, rodada_atual[id].forca);
        n = read(newsockfd[id], buffer, 6); // Cliente avisando que terminou de Jogar
        if(id == TOTAL_CONECTIONS-1){ // Se a rodada terminou (os 4 jogadores já jogaram cartas)
            ganhador = Ganhador(rodada_atual[0],rodada_atual[1],rodada_atual[2],rodada_atual[3]);
            if (ganhador != 5){ // Se não der empate
                printf("Quem ganhou a rodada foi o jogador %i, usando a carta %c, naipe ",ganhador, rodada_atual[ganhador-1].valor);
                Escolhe_Naipe(rodada_atual[ganhador-1]);
                printf(" com força %i.\n\n",rodada_atual[ganhador-1].forca);
            }else{ // Se der empate
            // TODO: Verificar o que fazer nesse caso, fala que deu empate e é isso, ou respeita o truco (maior na frente, etc...)
                printf("Houve empate!\n\n");
            }
        }
        if (strcmp(buffer,"Joguei") == 0){ // Cliente avisa que já fez sua jogada
            n = write(newsockfd[id], "Wait", 4);
            CHANGE_PLAYER // Muda qual cliente irá jogar agora
        }

    }
    free(cartas);
    for (i = 0; i < TOTAL_CONECTIONS; i++){ // Finaliza as conexões dos clientes
        close(newsockfd[i]);
    }
    close(sockfd); // Finaliza o servidor
    return 0; 
}
