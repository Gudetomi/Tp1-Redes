/* Truco UFSJ multiplayer, servidor TCP
    Antônio Pereira de Souza Júnior -  2022103670
    Gustavo Henrique Alves Detomi - gustavodetomi@gmail.com - 172050107
    Wagner Lancetti - wlancetti@gmail.com - 2022103705
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
#define MAX_CARTAS 40 // Quantidade de cartas em um baralho de truco
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



typedef struct{ // Struct para guardar as cartas
    char valor;
    char naipe;
    int forca;
}Carta;


void error(const char *msg){
    perror(msg);
    exit(1);
}

Carta *Define_Forca(Carta *cartas){ // Função que determina qual carta é mais forte, seguindo a regra do truco mineiro
    // 4♣ > 7♥ > A♠ > 7♦ > 3 > 2 > K > J > Q > 7 > 6 > 5 > 4
    int i = 0;
    for (i = 0; i < MAX_CARTAS; i++){
        if(cartas[i].valor == '4' && cartas[i].naipe == 'P'){ // 4♣ 
            cartas[i].forca = 14;
        }else if(cartas[i].valor == '7' && cartas[i].naipe == 'C'){ // 7♥
            cartas[i].forca = 13;
        }else if(cartas[i].valor == 'A' && cartas[i].naipe == 'E'){ // A♠
            cartas[i].forca = 12;
        }else if(cartas[i].valor == '7' && cartas[i].naipe == 'O'){ // 7♦ 
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
            if(num[j] == num[i]){ // Se o numero rand for igual a um numero existente
                confere = 1;
            }
        }
        if (confere == 0){ // Se nao houve repeticao de numeros
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
    // Verifica se houve empate entre as cartas
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

void Envia_Cartas_Mesa(int id, int newsockfd[], Carta *usadas, int qtd_jogadas){ // Envia as cartas da meesa pro cliente
    int i;
    char buffer[3];
    bzero(buffer,3);
    for(i = 0; i < qtd_jogadas; i++){ // Cartas na mesa
        buffer[0] = usadas[i].valor;
        buffer[1] = usadas[i].naipe;
        buffer[2] = usadas[i].forca + '0'; // Transformar int em char
        write(newsockfd[id], buffer, 3); // Manda para o cliente suas cartas
        bzero(buffer,3);
    }
}

int Analisa_Rodada(int *rodadas){ // Verifica se já tem um vencedor para a rodada
    int i, d1 = 0, d2 = 0, queda = -1;
    for(i = 0; i < 3; i++){ // Verifica se um time ja fez 2 rodadas
        if(rodadas[i] == 0){ // Se foi a dupla 1 que ganhou a rodada
            d1++;
        }else if(rodadas[i] == 1){ // Se foi a dupla 2 que ganhou a rodada
            d2++;
        }
    }
    if(d1 == 2){ // Se a dupla 1 ganhou as duas primeiras rodadas
        queda = 0;
    }else if(d2 == 2){ // Se a dupla 2 ganhou as duas primeiras rodadas
        queda = 1;
    }
    return queda;
}

int* Remove_Carta_Cliente(char valor, char naipe, Carta *cartas, int *vet, int id){ // Remove a carta que o cliente usou na rodada
    int i, id_carta;
    for(i = 0; i < MAX_CARTAS; i++){ // Acha o indice da carta usada pelo cliente
        if((cartas[i].valor == valor) && (cartas[i].naipe == naipe)){
            id_carta = i;
            break;
        }
    }
    for(i = 0; i < 3; i++){  // Remove a carta da mão do cliente
        if(vet[id * 3 + i] == id_carta){ // (id*3): mao do cliente no vetor // (+ i): cada carta do mao dele
            vet[id * 3 + i] = -1; // Encontrou a carta utilizada, "remove" ela do cliente
        }
    }
    return vet;
}


void Send_All(int newsockfd[], char *buffer, int tam){ // Avisa para todos os clientes a mensagem (buffer)
    int i, n;
    for(i = 0; i < TOTAL_CONECTIONS; i++){
        n = write(newsockfd[i], buffer, tam);
    }
    n++;
}

Carta Armazena_Carta(Carta carta, char buffer[]){ // Guarda uma carta lida do cliente
    carta.valor = buffer[0];
    carta.naipe = buffer[1];
    carta.forca = buffer[2] - '0'; // Converte char em int
    return carta;
}

int *Atualiza_Vetor(int *vet, int valor, int tam){ // Atualiza o vetor de acordo com o valor passado
    int i;
    for(i = 0; i < tam; i++){
        vet[i] = valor; // Valor do parametro de entrada
    }
    return vet;
}

void Status_Queda(int *newsockfd, char *buffer, char id_dupla){
    Send_All(newsockfd, buffer, 5); // Avisa pros clientes que acabou a rodada
    if (strcmp(buffer, "QUEDA") == 0){ // Se acabou a queda manda pros clientes a dupla ganhadora
        Send_All(newsockfd, &id_dupla, 1);
    }
}


void Libera_Proximo_Cliente(int *newsockfd, int id, Carta *usadas, int qtd_jogadas, Carta *cartas, int *queda, int *qtd, int *vet){
    int n, i, j, indice;
    char buffer[1024];
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
    Envia_Cartas_Mesa(id, newsockfd, usadas, qtd_jogadas); // Envia as cartas na mesa

    bzero(buffer,1024);
    buffer[0] = qtd[id] + '0'; // Transforma inteiro em char
    n = write(newsockfd[id], buffer, 1); // Manda para o cliente quantas cartas ele ainda possui na rodada
    bzero(buffer,1024);  

    j = 0;
    for(i = 0; i < qtd[id]; i++){ // Cartas do cliente
        indice = vet[id * 3 + j]; // Pega a carta aleatória do cliente
        if(indice != -1){
            buffer[0] = cartas[indice].valor;
            buffer[1] = cartas[indice].naipe;
            buffer[2] = cartas[indice].forca + '0'; // Transformar int em char
            n = write(newsockfd[id], buffer, 3); // Manda para o cliente suas cartas
        }else{
            i--;
        }
        j++;
    }
}

int Truco_Recusado(int *newsockfd, int *ids_truco, char *buffer, int num_rodada, int id){
    int i = 0, j, n;
    int *avisados;
    if(num_rodada == 0){ // Ninguem jogou antes do truco
        for (i = 0; i < TOTAL_CONECTIONS; i++){
            if ((i != ids_truco[0]) && (i != ids_truco[1])){
                bzero(buffer, 1024);
                n = write(newsockfd[i], "FUJA", 4); // Avisa que a rodada ja acabou por desistencia
                bzero(buffer, 1024);
                n = write(newsockfd[i], buffer, 2);
            }
        }
    }else{ // Uma pessoa jogou antes do truco
        avisados = (int*) calloc(TOTAL_CONECTIONS, sizeof(int));
        for(j = 0; j < num_rodada; j++){  // ID volta pro primeiro jogador da rodada
            PREVIOUS_PLAYER
            bzero(buffer, 1024);
            n = write(newsockfd[id], "Tru", 3); // Avisa pra todo mundo que houve desistencia no truco
            bzero(buffer, 1024);
            n = write(newsockfd[id], buffer, 2); // Avisa o placar
            avisados[id] = 1;
        }
        for (i = 0; i < TOTAL_CONECTIONS; i++){
            if ((i != ids_truco[0]) && (i != ids_truco[1]) && avisados[i] != 1){
                bzero(buffer, 1024);
                n = write(newsockfd[i], "FUJA", 4);
                bzero(buffer, 1024);
                n = write(newsockfd[i], buffer, 2);
            }
        }
        free(avisados);
    }
    n++;
    return id;
}


int main(int argc, char *argv[]){
    int sockfd, portno, newsockfd[TOTAL_CONECTIONS];
    int id = 0, i, n, verifica, ganhador, valor, id_dupla, id_truco, skip = 0, recusado = 0, qtd_jogadas = 0, multiplicador = 1, num_rodada = 0, empate = 0, truco = 0, rodada_trucada = 0;
    socklen_t clilen[TOTAL_CONECTIONS];
    char buffer[1024];
    struct sockaddr_in serv_addr, cli_addr[TOTAL_CONECTIONS];

    Carta *cartas = Pega_Baralho(); // Lê o arquivo e armazena o baralho
    Carta *rodada_atual, *usadas;
    int *vet, *qtd, *rodadas, *queda, *jogo, *ids_truco; // Rodadas -> 3 rodadas /-/ Quedas -> Resultado das rodadas /-/ Jogo -> Queda == 12, equipa ganha

    if (argc < 2){
        fprintf(stderr,"ERROR, nenhuma porta estabelecida!\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) 
        error("ERROR, nao foi possivel abrir o socket!");

    // Inicializa o servidor
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR no binding!");
    listen(sockfd,TOTAL_CONECTIONS); // Estabelece a quantidade de conexões que o servidor pode aceitar
    
    for(i = 0; i < TOTAL_CONECTIONS; i++){ // Aceita todas as conexões dos clientes
        clilen[i] = sizeof(cli_addr[i]);
        newsockfd[i] = accept(sockfd, (struct sockaddr *) &cli_addr[i], &clilen[i]);
        if (newsockfd[i] < 0){
            error("ERROR ao tentar aceitar o cliente!");
            break;
        }
    }
    
    vet = Distribui_Cartas(); // Da 3 cartas para cada usuário (sem repetição)
    qtd = (int*)calloc(TOTAL_CONECTIONS, sizeof(int)); // Quantidade de cartas restantes para o cliente
    rodada_atual = (Carta*)calloc(4, sizeof(Carta)); // Cartas de cada cliente na rodada
    usadas = (Carta*)calloc(4, sizeof(Carta)); // Cartas usadas na rodada
    rodadas = (int*)calloc(3,sizeof(int)); // 3 rodadas
    queda = (int*)calloc(2, sizeof(int)); // Duas duplas
    ids_truco = (int*)calloc(2, sizeof(int));
    jogo = (int*)calloc(2, sizeof(int)); // Duas duplas
    qtd = Atualiza_Vetor(qtd, 3, TOTAL_CONECTIONS);
    ids_truco = Atualiza_Vetor(ids_truco, -1, 2);

    CLEAR_ROUND  // Zera as rodadas
    
    while(1){
        if (JOGADOR_TRUCADO){ // Se for a rodada do jogador trucado
            n = write(newsockfd[id], "JOGA", 4);
            skip = 1;
            bzero(buffer, 1024);
            n = read(newsockfd[id], buffer, 1); // Cliente avisa que nao pode pedir truco
        }else{ // Se nao for a rodada do jogador trucado / nao houve truco na rodada
            Libera_Proximo_Cliente(newsockfd, id, usadas, qtd_jogadas, cartas, queda, qtd, vet);
            skip = 0;
        }

        if(NAO_JOGOU_TRUCADO){ // Cliente participou de um truco e já realizou todos esses comandos
            if(rodada_trucada == 1){ // Alguem ja pediu truco e mandou descer nessa rodada
                bzero(buffer, 1024);
                buffer[0] = 2 + '0';
                n = write(newsockfd[id], buffer, 1);
            }else{ // Se ninguem pediu truco pra esse cliente
                bzero(buffer, 1024);
                buffer[0] = 0 + '0';
                n = write(newsockfd[id], buffer, 1);
            }
            bzero(buffer, 1024);
            n = read(newsockfd[id], buffer, 1); // Define se o jogador pediu truco ou nao
            truco = buffer[0] - '0';
            
            if (PEDIRAM_TRUCO){ // Se o jogador pediu truco
                id_dupla = id % 2;
                if (qtd_jogadas == 3){ // Ultimo jogador da rodada
                    bzero(buffer, 1024);
                    n = write(newsockfd[id], "ULTIMOJG", 8); // Avisa que aquele cliente é o ultimo a jogar, entao nao pode pedir truco
                    printf("\n\nÉ o ultimo jogador, nao pode pedir truco...\n\n");
                }else{ // Qualquer outro jogador
                    id_truco = id;
                    rodada_trucada = 1;
                    free(ids_truco);
                    ids_truco = (int*)calloc(2, sizeof(int));
                    ids_truco[0] = id;
                    NEXT_PLAYER
                    ids_truco[1] = id;
                    Libera_Proximo_Cliente(newsockfd, id, usadas, qtd_jogadas, cartas, queda, qtd, vet);
                    bzero(buffer, 1024);
                    buffer[0] = truco + '0';
                    n = write(newsockfd[id], buffer, 1); // Avisa que esta trucado
                    bzero(buffer, 1024);
                    n = read(newsockfd[id], buffer, 8); // Verifica a escolha do cliente que foi chamado pro truco
                    PREVIOUS_PLAYER
                    n = write(newsockfd[id], buffer, 8);
                    if (AUMENTAR){ // TRUCADO AUMENTOU PARA SEIS
                        multiplicador= 3; // Seis
                        bzero(buffer, 1024);
                        n = read(newsockfd[id], buffer, 1); // Decisao do Trucando
                        valor = buffer[0] - '0';
                        NEXT_PLAYER
                        n = write(newsockfd[id], buffer, 1);
                        if (AUMENTARAM_APOSTA){ // TRUCANDO AUMENTOU PRA OITO
                            multiplicador++; // Oito
                            bzero(buffer, 1024);
                            n = read(newsockfd[id], buffer, 1); // Decisao do Trucado
                            valor = buffer[0] - '0';
                            PREVIOUS_PLAYER
                            n = write(newsockfd[id], buffer, 1);
                            if (AUMENTARAM_APOSTA){ // TRUCADO AUMENTOU PRA DEZ
                                multiplicador++; // Dez
                                bzero(buffer, 1024);
                                n = read(newsockfd[id], buffer, 1); // Decisao do Trucando
                                valor = buffer[0] - '0';
                                NEXT_PLAYER
                                n = write(newsockfd[id], buffer, 1);
                                if (AUMENTARAM_APOSTA){ // TRUCANDO AUMENTOU PRA QUEDA
                                    multiplicador++;
                                }
                            }
                        }
                        if(id != id_truco){ // Volta pro jogador que pediu truco
                            PREVIOUS_PLAYER
                        }
                        bzero(buffer, 1024);
                        n = write(newsockfd[id], "JOGA", 4);

                    }else if(RECUSADO){ // Se foi recusado o truco
                        recusado = 1;
                    }else{ // Se mandou descer o truco
                        n = write(newsockfd[id], "JOGA", 4);
                        multiplicador++;
                    }
                }
            }
        }

        if (recusado == 0){ // Se nao recusou a partida, continua normalmente
            skip = 0;
            bzero(buffer, 1024);
            n = read(newsockfd[id], buffer, 3); // Lê qual carta o cliente escolheu jogar
            rodada_atual[id] = Armazena_Carta(rodada_atual[id], buffer);
            usadas[qtd_jogadas] = Armazena_Carta(usadas[qtd_jogadas], buffer);
            qtd_jogadas++;
            printf("%i: %c %c %i\n",id, rodada_atual[id].valor, rodada_atual[id].naipe, rodada_atual[id].forca);
            bzero(buffer,1024);

            vet = Remove_Carta_Cliente(rodada_atual[id].valor, rodada_atual[id].naipe, cartas, vet, id); // Remove a carta que o cliente usou na rodada
            if(FIM_RODADA){ // Se a rodada terminou (os 4 jogadores já jogaram cartas)
                qtd = Atualiza_Vetor(qtd, qtd[0]-1, TOTAL_CONECTIONS); // Todos os clientes jogaram uma carta, reduzir uma pra cada
                ganhador = Ganhador(rodada_atual[0],rodada_atual[1],rodada_atual[2],rodada_atual[3]); // Define quem ganhou a rodada, ou se empatou
                if (HOUVE_GANHADOR){ // Se houve um ganhador
                    bzero(buffer,1024);
                    buffer[0] = ganhador + '0'; // ID do ganhador
                    buffer[1] = rodada_atual[ganhador-1].valor; // Valor da carta usada (A,2,3,...,K)
                    buffer[2] = rodada_atual[ganhador-1].naipe; // Naipe da carta usada (♥, ♦, ♣, ♣)
                    Send_All(newsockfd, buffer, 3); // Manda a mensagem para todos os clientes
                    bzero(buffer,1024);
                    rodadas[num_rodada] = (ganhador-1)%2; // Define qual dupla ganhou a rodada
                    num_rodada++;
                    verifica = Analisa_Rodada(rodadas); // 0: dupla 1 /-/ 1: dupla 2s
                    if (verifica != -1 || empate == 1){ // Dupla ganhou a queda, ou empatou a primeira rodada (ganha quem fizer a próxima)
                        printf("Alguem ganhou a queda...\n");
                        if(verifica != -1){ // Se ganhou sem empate
                            queda[verifica] += 2 * multiplicador; // multiplicador: Se foi com Truco, Seis, ...
                        }else{ // Se ganhou com empate
                            verifica = (ganhador-1)%2; // Quem ganhou a ultima rodada
                            queda[verifica] += 2 * multiplicador;
                        }
                        bzero(buffer, 1024);
                        strcpy(buffer, "QUEDA");
                        Status_Queda(newsockfd, "QUEDA", (verifica + 1) + '0');
                        bzero(buffer, 1024);
                        CLEAR_ROUND // Limpa qual dupla ganhou cada rodada
                        RESET_ROUND // Libera o espaço de todos os vetores usados na rodada
                        num_rodada = 0; // Numero da rodada volta para o inicio
                        vet = Distribui_Cartas(); // Da 3 cartas para cada usuário (sem repetição)
                        qtd = (int*)calloc(TOTAL_CONECTIONS, sizeof(int)); // cartas restantes de cada cliente
                        ids_truco = (int*)calloc(2, sizeof(int));
                        rodada_atual = (Carta*)calloc(4, sizeof(Carta)); // Cartas de cada cliente na rodada
                        usadas = (Carta*)calloc(4, sizeof(Carta)); // Cartas usadas na rodada
                        qtd = Atualiza_Vetor(qtd, 3, TOTAL_CONECTIONS);
                        ids_truco = Atualiza_Vetor(ids_truco, -1, 2);
                        empate = 0;
                        recusado = 0;
                        multiplicador = 1;
                        rodada_trucada = 0;
                        truco = 0;
                    }else{ // Se a queda ainda nao foi definida
                        bzero(buffer, 1024);
                        strcpy(buffer, "GOIGN");
                        Status_Queda(newsockfd, buffer, 'D');
                        skip = 0;
                        ids_truco = Atualiza_Vetor(ids_truco, -1, 2);
                        bzero(buffer, 1024);
                    }
                    
                }else{ // Se der empate
                    Send_All(newsockfd, "Emp", 3);
                    bzero(buffer,1024);
                    if(EMPATE_1RODADA){ // Empatou a primeira rodada
                        rodadas[num_rodada] = 2; // Empate na primeira queda (ngm leva)
                        empate = 1; // O proximo cliente a ganhar a rodada vence a queda
                        Send_All(newsockfd, "1Rodada", 7);
                        bzero(buffer,1024);
                        skip = 0;
                        ids_truco = Atualiza_Vetor(ids_truco, -1, 2);
                        printf("\nEmpate na rodada 1!\n\n");
                        num_rodada++;
                    }else{ // Empatou na segunda/terceira rodada
                        Send_All(newsockfd, "2Rodada", 7);
                        bzero(buffer,1024);
                        if(NAO_EMPATOU_PRIMEIRA){ // Se a primeira não deu empate, entao quem fez ela ganha a queda
                            bzero(buffer, 1024);
                            buffer[0] = (rodadas[0] + 1) + '0'; // Dupla ganhadora
                            buffer[1] = (multiplicador * 2) + '0'; // qtd de pontos feitos
                            buffer[2] = 'D';
                            Send_All(newsockfd, buffer, 3);
                            verifica = rodadas[0];
                            queda[verifica] += 2 * multiplicador; // multiplicador: Se foi com Truco, Seis, ...
                            bzero(buffer,1024);
                            strcpy(buffer, "QUEDA");
                            Status_Queda(newsockfd, buffer, (verifica + 1) + '0');
                            CLEAR_ROUND
                            RESET_ROUND // Reseta valores para uma nova rodada
                            num_rodada = 0; // Numero da rodada volta para o inicio
                            vet = Distribui_Cartas(); // Da 3 cartas para cada usuário (sem repetição)
                            qtd = (int*)calloc(TOTAL_CONECTIONS, sizeof(int)); // cartas restantes de cada cliente
                            rodada_atual = (Carta*)calloc(4, sizeof(Carta)); // Cartas de cada cliente na rodada
                            usadas = (Carta*)calloc(4, sizeof(Carta)); // Cartas usadas na rodada
                            qtd = Atualiza_Vetor(qtd, 3, TOTAL_CONECTIONS);
                            ids_truco = (int*)calloc(2, sizeof(int));
                            ids_truco = Atualiza_Vetor(ids_truco, -1, 2);
                            printf("\nHouve ganhador na rodada 1. Logo finaliza!\n\n");
                            empate = 0;
                            skip = 0;
                            truco = 0;
                            recusado = 0;
                            multiplicador = 1;
                            rodada_trucada = 0;
                        }else{
                            Send_All(newsockfd, "Emp", 3);
                            rodadas[num_rodada] = 2;
                            num_rodada++;
                            bzero(buffer,1024);
                            printf("\nHouve empate! Rodada 2 ou 3.\n\n");
                        }
                    }
                }
                qtd_jogadas = 0; // Zera as cartas pra próxima rodada
                NEXT_PLAYER // Pula um cliente (o primeiro que jogou na rodada anterior, passa a ser o ultimo a jogar)
            }
        }else{ // Recusada == 1 (alguem recusou o truco)
            bzero(buffer, 1024);
            buffer[0] = id_dupla + '0';
            buffer[1] = (multiplicador * 2) + '0';
            n = write(newsockfd[id], buffer, 2);
            NEXT_PLAYER
            n = write(newsockfd[id], buffer, 2);
            PREVIOUS_PLAYER
            id = Truco_Recusado(newsockfd, ids_truco, buffer, num_rodada, id);
            queda[id_dupla] += 2 * multiplicador; // multiplicador: Se foi com Truco, Seis, ...
            CLEAR_ROUND
            RESET_ROUND // Reseta valores para uma nova rodada
            num_rodada = 0; // Numero da rodada volta para o inicio
            vet = Distribui_Cartas(); // Da 3 cartas para cada usuário (sem repetição)
            qtd = (int*)calloc(TOTAL_CONECTIONS, sizeof(int)); // cartas restantes de cada cliente
            rodada_atual = (Carta*)calloc(4, sizeof(Carta)); // Cartas de cada cliente na rodada
            usadas = (Carta*)calloc(4, sizeof(Carta)); // Cartas usadas na rodada
            qtd = Atualiza_Vetor(qtd, 3, TOTAL_CONECTIONS);
            ids_truco = (int*)calloc(2, sizeof(int));
            ids_truco = Atualiza_Vetor(ids_truco, -1, 2);
            truco = 0;
            empate = 0;
            skip = 0;
            recusado = 0;
            rodada_trucada = 0;
            multiplicador = 1;
        }
        NEXT_PLAYER // Muda qual cliente irá jogar agora
        n++;
    }

    FREE_ALL // Limpa o espaço de memória
    for (i = 0; i < TOTAL_CONECTIONS; i++){ // Finaliza as conexões dos clientes
        close(newsockfd[i]);
    }
    close(sockfd); // Finaliza o servidor
    return 0; 
}
