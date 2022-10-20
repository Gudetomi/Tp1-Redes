/* Truco UFSJ multiplayer, cliente TCP
    Antônio Pereira de Souza Júnior - antonio258p@gmail.com - 2022103670
    Gustavo Henrique Alves Detomi - gustavodetomi@gmail.com - 172050107
    Wagner Lancetti - wlancetti@gmail.com - 2022103705
*/
#include "truco.h"
#include "print.h"

#define KRED  "\x1B[31m"
#define RESET "\x1B[0m"
#define RECUSADO (strcmp(buffer, "RECUSADO") == 0)
#define AUMENTAR (strcmp(buffer, "AUMENTAR") == 0)
#define CONTINUA (strcmp(buffer, "CONTINUA") == 0)
#define ULITMO_ROUND (strcmp(buffer, "ULTIMOJG") == 0)

struct Carta { // Struct para guardar as cartas
    char valor;
    char naipe;
    int forca;
};

void play(int sockfd) {
    char buffer[1024];
    Carta *cartas, *mesa, *vencedora;
    Carta escolha;
    int qtd, qtd_mesa, i, truco = 0, valor, recusado, opcao, n;
    truco = 0;
    recusado = 0;
    bzero(buffer, 1024);
    // Pega a pontuação
    n = read(sockfd, buffer, 4); // Placar
    // Mostrar a mesa //
    PrintaMesa(buffer[0] - '0', buffer[1] - '0', buffer[2] - '0', buffer[3] - '0');
    bzero(buffer, 1024);
    n = read(sockfd, buffer, 1); // Qtd de cartas na mesa
    qtd_mesa = buffer[0] - '0'; // Quantidade de cartas que estão na mesa
    bzero(buffer, 1024);
    if (qtd_mesa != 0) { // Se houver cartas na mesa
        mesa = (Carta *) calloc(qtd_mesa, sizeof(Carta));
        for (i = 0; i < qtd_mesa; i++) {
            n = read(sockfd, buffer, 3);
            mesa[i].valor = buffer[0];
            mesa[i].naipe = buffer[1];
            mesa[i].forca = buffer[2] - '0'; // transformar char em int
            bzero(buffer, 1024);
        }
        Print_Carta(mesa, qtd_mesa, 1); // Mostrar as cartas pro cliente
        free(mesa);
        printf("\n\n\n");
    } else {
        printVazio();
    }
    printf("________________________________________________________________\n");

    n = read(sockfd, buffer, 1);  // Lê quantas cartas o cliente ainda tem na rodada
    qtd = atoi(buffer);
    bzero(buffer, 1024); // Zera o buffer
    cartas = (Carta *) calloc(qtd, sizeof(Carta));

    for (i = 0; i < qtd; i++) { // Lê cada uma das cartas do cliente (só o servidor conhece elas)
        n = read(sockfd, buffer, 3);
        cartas[i].valor = buffer[0];
        cartas[i].naipe = buffer[1];
        cartas[i].forca = buffer[2] - '0'; // transformar char em int
        bzero(buffer, 1024);
    }
    Print_Carta(cartas, qtd, 0); // Mostrar as cartas pro cliente

    bzero(buffer, 1024);
    n = read(sockfd, buffer, 1); // Verifica se o jogador vai jogar trucado ou nao
    valor = buffer[0] - '0';

    if (valor == 1) { // Esta jogando trucado
        truco = 1;
        do {
            printf("Digite 0 caso for RECUSAR o truco.\nDigite 1 caso deseja AUMENTAR a aposta do truco para %s.\nDigite 2 caso deseja ACEITAR o truco.\n",
                   Define_Valor(truco));
            printf("\nOpcao: ");
            scanf("%i", &valor);
        } while (valor >= 3 && valor < 0);
        bzero(buffer, 1024);
        switch (valor) {
            case 0: // Recusou
                strcpy(buffer, "RECUSADO");
                break;
            case 1: // Aumentou o truco
                strcpy(buffer, "AUMENTAR");
                break;
            case 2: // Mandou descer a carta
                strcpy(buffer, "CONTINUA");
                break;
        }
        n = write(sockfd, buffer, 8); // Mandou pro servidor a escolha
        if (AUMENTAR) { // Se aceitou, define até onde esta disposto a ir
            bzero(buffer, 1024);
            n = read(sockfd, buffer, 1);
            valor = buffer[0] - '0';
            if (valor == 1) { // TRUCANDO AUMENTOU PRA OITO
                truco = 2;
                printf("\n\nA aposta foi aumentada para %s!\n\n", Define_Valor(truco));
                truco = 3;
                printf("Aumentar aposta para %s?\n0 - Não\n1 - Sim\nOpção: ", Define_Valor(truco));
                scanf("%i", &opcao);
                bzero(buffer, 1024);
                buffer[0] = opcao + '0';
                n = write(sockfd, buffer, 1);
                if (opcao == 1) { // TRUCADO AUMENTOU PRA DEZ
                    truco++; // Truco = 4
                    bzero(buffer, 1024);
                    n = read(sockfd, buffer, 1);
                    valor = buffer[0] - '0';
                    if (valor == 1) {
                        printf("\nTRUCANDO PEDIU APOSTA DA QUEDA!\n\n");
                    }
                }
            }
            truco = 1;
            printf("\nAguardando para jogar....\n\n");
            bzero(buffer, 1024);
            n = read(sockfd, buffer, 4); // Avisando que pode voltar a jogar
        } else if (RECUSADO) { // Se recusar
            recusado = 1;
        } else if (CONTINUA) { // Se mandou descer a carta
            printf("\nAguardando para jogar....\n\n");
            bzero(buffer, 1024);
            n = read(sockfd, buffer, 4); // Esoera a ordem para jogar
            truco = 1;
        }
    }

    if (truco == 0 && recusado == 0 &&
        valor != 2) { // Jogador pode pedir truco (Nao esta trucado, nem recusou um truco)
        valor = Define_Truco(sockfd); // Ja mando pro servidor a escolha
        if (valor == 1) { // Se jogador optou por chamar no truco
            truco = 1;
            bzero(buffer, 1024);
            n = read(sockfd, buffer, 8);
            if (ULITMO_ROUND) { // Ultimo jogador nao pode pedir truco
                printf("\n\nServidor: Ultimo cliente nao pode pedir truco.\n\n");
            } else if (AUMENTAR) {
                printf("\n\nTrucado aumentou a aposta para %s!\n\n", Define_Valor(truco));
                truco = 2;
                printf("Aumentar aposta para %s?\n0 - Não\n1 - Sim\nOpção: ", Define_Valor(truco));
                scanf("%i", &opcao);
                bzero(buffer, 1024);
                buffer[0] = opcao + '0';
                n = write(sockfd, buffer, 1);
                bzero(buffer, 1024);
                if (opcao == 1) { // TRUCANDO AUMENTOU PRA OITO
                    truco++; // Truco = 3
                    bzero(buffer, 1024);
                    n = read(sockfd, buffer, 1); // Opcao do TRUCADO
                    valor = buffer[0] - '0';
                    if (valor == 1) { // TRUCADO AUMENTOU PRA DEZ
                        printf("\n\nTrucado aumentou a aposta para %s!\n\n", Define_Valor(truco));
                        truco++; // Truco = 4
                        printf("Aumentar aposta para %s?\n0 - Não\n1 - Sim\nOpção: ", Define_Valor(truco));
                        scanf("%i", &opcao);
                        bzero(buffer, 1024);
                        buffer[0] = opcao + '0';
                        n = write(sockfd, buffer, 1);
                    }
                }
                bzero(buffer, 1024);
                n = read(sockfd, buffer, 4); // Avisando que pode voltar a jogar
            } else if (RECUSADO) {
                recusado = 1;
            } else if (CONTINUA) { // Mandou continuar
                bzero(buffer, 1024);
                n = read(sockfd, buffer, 4);
                truco = 1;
            }
        }
    } else { // Avisa que nao pode pedir truco
        bzero(buffer, 1024);
        buffer[0] = '2'; // Nao posso pedir truco
        n = write(sockfd, buffer, 1);
    }

    if (recusado == 0) { // Se o jogador nao recusou o truco // Só continuou jogando
        bzero(buffer, 1024);
        escolha = Escolhe_Carta(cartas, qtd); // Mostra as cartas pro cliente e faz ele decidir qual vai jogar
        buffer[0] = escolha.valor;
        buffer[1] = escolha.naipe;
        buffer[2] = escolha.forca + '0';
        n = write(sockfd, buffer, 3); // Volta pro servidor qual carta foi escolhida
        free(cartas); // Desaloca espaço

        bzero(buffer, 1024);
        n = read(sockfd, buffer, 3); // Recebe a mensagem de quem ganhou a rodada

        if (strcmp(buffer, "Emp") == 0) { // Se empatar
            system("clear");
            bzero(buffer, 1024);
            n = read(sockfd, buffer, 7); // Qual rodada empatou
            if (strcmp(buffer, "1Rodada") == 0) { // Empate na primeira rodada
                printf("\nEmpatou na primeira rodada! Quem ganhar qualquer uma das proximas rodadas vence!\n\n");
            } else { // Empate na 2 ou 3 rodada
                bzero(buffer, 1024);
                n = read(sockfd, buffer,
                         3); // Recebe qual dupla ganhou e quantos pontos fez, ou avisando que empatou dnv

                if (strcmp(buffer, "Emp") == 0) {
                    bzero(buffer, 1024);
                    n = read(sockfd, buffer, 1); // Verifica se houveram 3 empates seguidos
                    valor = buffer[0] - '0';
                    printf("\nEmpatou novamente...\n\n");
                    if (valor == 1) {
                        printf("\nHouveram 3 empates seguidos... Uma nova rodada se inicia! Nenhuma dupla ganhou pontos.\n");
                    }
                    printf("\n\nAguardando para jogar...\n\n");
                } else {
                    printf("\n\nQuem ganhou a queda, e %c pontos, foi a dupla %c por ter ganhado a primeira rodada.\n\n",
                           buffer[1], buffer[0]);
                    Verifica_Queda(sockfd, buffer);
                    printf("\n\nAguardando para jogar...\n\n");
                }
            }
        } else if (strcmp(buffer, "Tru") == 0) { // Se alguem pedir truco lá pra frente e a dupla correr
            system("clear");
            bzero(buffer, 1024);
            n = read(sockfd, buffer, 2); // buffer[0] a dupla que ganhou, buffer[1] quantos pontos fez
            printf("\n\nA dupla %c ganhou %c pontos e a queda por desistência do adversário.\n\n", buffer[0],
                   buffer[1]);
            printf("\n\nAguardando para jogar...\n\n");
        } else { // Se houver um ganhador da rodada
            vencedora = (Carta *) malloc(1 * sizeof(Carta));
            vencedora[0].valor = buffer[1];
            vencedora[0].naipe = buffer[2];
            system("clear");
            printf("\n\nQuem ganhou a rodada foi o jogador %c, usando a carta:\n", buffer[0]);
            Print_Carta(vencedora, 1, 0); // Mostra a carta vencedora
            free(vencedora);
            bzero(buffer, 1024);
            Verifica_Queda(sockfd, buffer); // Verifica se a queda acabou
            printf("\nRodada finalizou...\n");
            printf("\n\nAguardando para jogar...\n\n");
        }
    } else if (recusado == 1) { // Se o jogador recusou o truco
        bzero(buffer, 1024);
        n = read(sockfd, buffer, 2); // buffer[0] a dupla que ganhou, buffer[1] quantos pontos fez
        printf("\n\nA dupla %c ganhou %c pontos e a queda por desistência do adversário.\n\n", buffer[0], buffer[1]);
        printf("\n\nAguardando para jogar...\n\n");
    }
    bzero(buffer, 1024);
    strcpy(buffer, "Wait");
    printf("_________________________________________________________\n\n");
    n++;
}

void runCliente(int sockfd) {
    char buffer[1024];
    while (1) {
        bzero(buffer, 1024);
        int n = read(sockfd, buffer, 4); // Espera (Wait), Joga (Play), Recusaram Truco (FUJA), Acabou o jogo(JOGO)
        if (n < 0) {
            error("ERROR reading to socket");
        }
        if (strcmp(buffer, "JOGO") == 0) {
            bzero(buffer, 1024);
            n = read(sockfd, buffer, 1);
            printf("\nO jogo acabou, a dupla %c venceu dois jogos...\n\n", buffer[0]);
            break;
        }
        if (strcmp(buffer, "FUJA") == 0) {
            n = read(sockfd, buffer, 2);
            printf("\n\nA dupla %c ganhou %c pontos e a queda por desistência do adversário.\n\n", buffer[0],
                   buffer[1]);
            printf("\n\nAguardando para jogar...\n\n");
            printf("_________________________________________________________\n\n");
        }
        if (strcmp(buffer, "Play") == 0) { // Deixa o cliente jogar
            play(sockfd);
        }

    }
}

int main(int argc, char *argv[]){
    system("clear");
    int sockfd, portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

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
    runCliente(sockfd);
    close(sockfd);
    return 0;
}
