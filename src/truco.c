/* Truco UFSJ multiplayer, cliente TCP
    Antônio Pereira de Souza Júnior - antonio258p@gmail.com - 2022103670
    Gustavo Henrique Alves Detomi - gustavodetomi@gmail.com - 172050107
    Wagner Lancetti - wlancetti@gmail.com - 2022103705
*/

#include "truco.h"

struct Carta { // Struct para guardar as cartas
    char valor;
    char naipe;
    int forca;
};

Carta Escolhe_Carta(Carta *cartas, int tam){ // Qual carta o cliente vai usar
    int opcao;
    do{
        printf("\nEscolha o ID da carta que deseja jogar: ");
        scanf("%i", &opcao);
    }while(opcao > tam);

    return cartas[opcao];
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

Carta Armazena_Carta(Carta carta, char buffer[]){ // Guarda uma carta lida do cliente
    carta.valor = buffer[0];
    carta.naipe = buffer[1];
    carta.forca = buffer[2] - '0'; // Converte char em int
    return carta;
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

int Define_Truco(int sockfd){ // Define se o cliente vai pedir truco ou nao
    int opcao, n;
    char buffer[1024];
    do{
        printf("\n\nSe deseja pedir Truco digite 1. Caso deseja descer a carta digite 0.\n");
        printf("\nOpcao: ");
        scanf("%i",&opcao);
    }while(opcao > 2);
    bzero(buffer, 1024);
    buffer[0] = opcao + '0';
    n = write(sockfd, buffer, 1);
    n++;
    return opcao;
}

char *Define_Valor(int valor){
    char *str;
    switch(valor){
        case 1:
            str = (char*)calloc(5, sizeof(char));
            strcpy(str, "Seis");
            break;
        case 2:
            str = (char*)calloc(5, sizeof(char));
            strcpy(str, "Oito");
            break;
        case 3:
            str = (char*)calloc(4, sizeof(char));
            strcpy(str, "Dez");
            break;
        case 4:
            str = (char*)calloc(6, sizeof(char));
            strcpy(str, "Queda");
            break;
    }
    return str;
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
    // Verifica se houve empate entre as cartas dos adversarios
    if ((maior == 1 || maior == 3) && (forca == carta2.forca || forca == carta4.forca)){
        maior = 5;
    }else if((maior == 2 || maior == 4) && (forca == carta1.forca || forca == carta3.forca)){
        maior = 5;
    }
    // Retorna o id do cliente que usou a maior carta (1,2,3 ou 4), ou empate (5)
    return maior;
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

int *Atualiza_Vetor(int *vet, int valor, int tam){ // Atualiza o vetor de acordo com o valor passado
    int i;
    for(i = 0; i < tam; i++){
        vet[i] = valor; // Valor do parametro de entrada
    }
    return vet;
}

int Verifica_Jogo(int *vet, int qtd){ // Verifica se alguma dupla já fez 12 pontos
    int valor = -1;
    if (vet[0] >= qtd){
        printf("\n\nEntro aqui 1...\n\n");
        valor = 0;
    }else if(vet[1] >= qtd){
        printf("\n\nEntro aqui 2...\n\n");
        valor = 1;
    }
    return valor;

}

void Zera_Variaveis_Queda(int *empate, int *recusado, int*multiplicador, int*rodada_trucada, int* truco, int*skip){
    *empate = 0;
    *recusado = 0;
    *multiplicador = 1;
    *rodada_trucada = 0;
    *truco = 0;
    *skip = 0;
}

void error(const char *msg){
    perror(msg);
    exit(0);
}