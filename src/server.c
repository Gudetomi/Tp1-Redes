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
#define CLEAR_ROUND rodadas[0] = -1; rodadas[1] = -1; rodadas[2] = -1; // Limpa os ganhadores dos rounds
#define RESET_ROUND free(vet); free(qtd); free(rodada_atual); free(usadas); // Limpa os vetores de cartas dos clientes e quais já foram usadas no round
#define FREE_ALL free(cartas); free(vet); free(qtd); free(rodada_atual); free(usadas); free(rodadas); free(queda); free(jogo); // Libera todo espaco de memoria

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

char* Escolhe_Naipe(Carta carta){
    char *valor;
    valor = (char*)calloc(4,sizeof(char));
    switch(carta.naipe){
        case 'C': // Copas
            strcpy(valor, "♥");
            break;
        case 'O': // Ouro
            strcpy(valor, "♦");
            break;
        case 'E': // Espadas
            strcpy(valor, "♠");
            break;
        case 'P': // Paus
            strcpy(valor, "♣");
            break;
    }
    return valor;
}

void Print_Baralho(Carta *cartas){
    int i;
    for(i = 0; i < MAX_CARTAS; i++){
        printf("%c  ", cartas[i].valor);
        printf("%s  ", Escolhe_Naipe(cartas[i]));
        printf("%i\n",cartas[i].forca);
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


int Analisa_Rodada(int *rodadas){
    int i, d1 = 0, d2 = 0, queda = -1;
    for(i = 0; i < 3; i++){ // Verifica se um time ja fez 2 rodadas
        if(rodadas[i] == 0){
            d1++;
        }else if(rodadas[i] == 1){
            d2++;
        }
    }
    if (d1 == 2){
        queda = 0;
    }else if(d2 == 2){
        queda = 1;
    }
    return queda;
}

int main(int argc, char *argv[]){
    int sockfd, portno, newsockfd[TOTAL_CONECTIONS];
    int id = 0, i, j, n, verifica, indice, id_carta = 0, ganhador, qtd_jogadas = 0, multiplicador = 1, num_rodada = 0;
    socklen_t clilen[TOTAL_CONECTIONS];
    char buffer[1024];
    struct sockaddr_in serv_addr, cli_addr[TOTAL_CONECTIONS];

    Carta *cartas = Pega_Baralho(); // Lê o arquivo e armazena o baralho
    Carta *rodada_atual, *usadas;
    int *vet, *qtd, *rodadas, *queda, *jogo;
    // Rodadas -> Cada rotação de cartas é uma rodada
    // Quedas -> Cada 12 pontos acumulados é uma queda
    // Jogos -> 2 Quedas é um Jogo


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
    vet = (int*)calloc(TOTAL_CONECTIONS*3, sizeof(int)); // Cada cliente: 3 posições de cartas
    vet = Distribui_Cartas(); // Da 3 cartas para cada usuário (sem repetição)
    qtd = (int*)calloc(TOTAL_CONECTIONS, sizeof(int));
    for(i = 0; i < TOTAL_CONECTIONS; i++){ // Cada cliente começa com 3 cartas
        qtd[i] = 3;
    }
    rodada_atual = (Carta*)calloc(4, sizeof(Carta)); // Cartas de cada cliente na rodada
    usadas = (Carta*)calloc(4, sizeof(Carta)); // Cartas usadas na rodada
    rodadas = (int*)calloc(3,sizeof(int)); // 3 rodadas
    queda = (int*)calloc(2, sizeof(int)); // Duas duplas
    jogo = (int*)calloc(2, sizeof(int)); // Duas duplas
    CLEAR_ROUND

    while(1) {
        bzero(buffer,1024);
        n = write(newsockfd[id], "Play", 4); // Coloca um Cliente para jogar
        if (n < 0){
            error("ERROR writing to client");
        }
        // Mostra o placar para o cliente:
        bzero(buffer,1024);
        buffer[0] = queda[0] + '0';
        buffer[1] = queda[1] + '0';
        n = write(newsockfd[id], buffer, 2);
        
        // Mostrar a mesa pro cliente
        bzero(buffer,1024);
        buffer[0] = qtd_jogadas + '0'; // Quantas cartas ja foram usadas na mesa
        n = write(newsockfd[id], buffer, 1); // Qtd de cartas na mesa
        for(i = 0; i < qtd_jogadas; i++){ // Cartas na mesa
            buffer[0] = usadas[i].valor;
            buffer[1] = usadas[i].naipe;
            buffer[2] = usadas[i].forca + '0'; // Transformar int em char
            n = write(newsockfd[id], buffer, 3); // Manda para o cliente suas cartas
        }
        // Envia as cartas que o cliente tem
        bzero(buffer,1024);
        buffer[0] = qtd[id] + '0'; // Transforma inteiro em char
        n = write(newsockfd[id], buffer, 1); // Manda para o cliente quantas cartas ele ainda possui na rodada
        indice = id*3; // Pega o ID do usuário que vai jogar (serve para pegar as 3 cartas aleatórias que caíram para o jogador)
        bzero(buffer,1024);  
        j = 0;
        for(i = 0; i < qtd[id]; i++){ // Cartas do cliente
            indice = vet[id * 3 + j]; // Pega a carta aleatória do cliente
            j++;
            if(indice != -1){
                buffer[0] = cartas[indice].valor;
                buffer[1] = cartas[indice].naipe;
                buffer[2] = cartas[indice].forca + '0'; // Transformar int em char
                n = write(newsockfd[id], buffer, 3); // Manda para o cliente suas cartas
            }else{
                i--;
            }
        }

        bzero(buffer,1024);
        n = read(newsockfd[id], buffer, 3); // Lê qual carta o cliente escolheu jogar
        rodada_atual[id].valor = buffer[0];
        rodada_atual[id].naipe = buffer[1];
        rodada_atual[id].forca = buffer[2] - '0'; // Converte char em int
        // Cartas que já foram usadas
        usadas[qtd_jogadas].valor = buffer[0];
        usadas[qtd_jogadas].naipe = buffer[1];
        usadas[qtd_jogadas].forca = buffer[2] - '0'; // Converte char em int
        qtd_jogadas++;

        printf("%c %c %i\n",rodada_atual[id].valor, rodada_atual[id].naipe, rodada_atual[id].forca);
        bzero(buffer,1024);
        n = read(newsockfd[id], buffer, 6); // Cliente avisando que terminou de Jogar

        if (strcmp(buffer,"Joguei") == 0){ // Cliente avisa que já fez sua jogada
            n = write(newsockfd[id], "Wait", 4);
            indice = vet[id * 3];
            for(i = 0; i < MAX_CARTAS; i++){ // Acha o indice da carta usada pelo cliente
                if((cartas[i].valor == rodada_atual[id].valor) && (cartas[i].naipe == rodada_atual[id].naipe)){
                    id_carta = i;
                    break;
                }
            }
            for(i = 0; i < 3; i++){  // Remove ela da mão do cliente
                if(vet[id * 3 + i] == id_carta){
                    vet[id * 3 + i] = -1;
                }
            }
        }


        if(qtd_jogadas == 4){ // Se a rodada terminou (os 4 jogadores já jogaram cartas)
            for(i = 0; i < TOTAL_CONECTIONS; i++){ // Todos os clientes jogaram uma carta, reduzir uma pra cada
                qtd[i] -= 1;
            }
            ganhador = Ganhador(rodada_atual[0],rodada_atual[1],rodada_atual[2],rodada_atual[3]);
            qtd_jogadas = 0;
            if (ganhador != 5){ // Se não der empate
                bzero(buffer,1024);

                buffer[0] = ganhador + '0';
                buffer[1] = rodada_atual[ganhador-1].valor;
                buffer[2] = rodada_atual[ganhador-1].naipe;
                for(i = 0; i < TOTAL_CONECTIONS; i++){ // Manda pra todos os clientes quem fez a rodada
                    n = write(newsockfd[i], buffer, 3);
                }
                rodadas[num_rodada] = (ganhador-1)%2;
                num_rodada++;
                verifica = Analisa_Rodada(rodadas); // 0 dupla 1, 1 dupla 2
                if (verifica != -1){ // Dupla ganhou a queda
                    queda[verifica] += 2 * multiplicador; // multiplicador: Se foi com Truco, Seis, ...
                    CLEAR_ROUND
                    RESET_ROUND
                    num_rodada = 0;
                    vet = (int*)calloc(TOTAL_CONECTIONS*3, sizeof(int)); // Cada cliente: 3 posições de cartas
                    vet = Distribui_Cartas(); // Da 3 cartas para cada usuário (sem repetição)
                    qtd = (int*)calloc(TOTAL_CONECTIONS, sizeof(int));
                    for(i = 0; i < TOTAL_CONECTIONS; i++){ // Cada cliente começa com 3 cartas
                        qtd[i] = 3;
                    }
                    rodada_atual = (Carta*)calloc(4, sizeof(Carta)); // Cartas de cada cliente na rodada
                    usadas = (Carta*)calloc(4, sizeof(Carta)); // Cartas usadas na rodada
                    verifica = -1;

                }

            }else{ // Se der empate
                for(i = 0; i < TOTAL_CONECTIONS; i++){
                    n = write(newsockfd[i], "Emp", 3);
                }
                printf("Houve empate!\n\n");
            }
            free(rodada_atual);
            free(usadas);
            rodada_atual = (Carta*)calloc(4, sizeof(Carta));
            usadas = (Carta*)calloc(4, sizeof(Carta));
            multiplicador = 1;
            CHANGE_PLAYER
        }
        CHANGE_PLAYER // Muda qual cliente irá jogar agora
    }
    // Limpa o espaço de memória
    FREE_ALL
    for (i = 0; i < TOTAL_CONECTIONS; i++){ // Finaliza as conexões dos clientes
        close(newsockfd[i]);
    }
    close(sockfd); // Finaliza o servidor
    return 0; 
}
