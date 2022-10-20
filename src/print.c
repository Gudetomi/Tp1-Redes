/* Truco UFSJ multiplayer, cliente TCP
    Antônio Pereira de Souza Júnior - antonio258p@gmail.com - 2022103670
    Gustavo Henrique Alves Detomi - gustavodetomi@gmail.com - 172050107
    Wagner Lancetti - wlancetti@gmail.com - 2022103705
*/

#include <stdio.h>
#include "print.h"

struct Carta { // Struct para guardar as cartas
    char valor;
    char naipe;
    int forca;
};

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

void PrintaMesa(int placar1, int placar2, int jogos1, int jogos2){
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
    printf(" ______   ______\n");
    printf("| t1   | | t2   |\n");
    printf("|q: %s | |q: %s |\n", p1, p2);
    printf("|j: %d  | |j: %d  |\n", jogos1, jogos2);
    printf(" \u203E\u203E\u203E\u203E\u203E\u203E   \u203E\u203E\u203E\u203E\u203E\u203E\n");
    printf("q: número de quedas ganhas\nj: número de jogos ganhas\n");

    printf("╔╦╗┌─┐┌─┐┌─┐\n");
    printf("║║║├┤ └─┐├─┤\n");
    printf("╩ ╩└─┘└─┘┴ ┴\n");
}

void printVazio(){
    printf("╦  ╦┌─┐┌─┐┬┌─┐\n");
    printf("╚╗╔╝├─┤┌─┘│├─┤\n");
    printf(" ╚╝ ┴ ┴└─┘┴┴ ┴\n\n");
}