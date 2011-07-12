#include "../include/bloques.h"
#include "../include/ficheros_basico.h"
#include "../include/definicions.h"
#include <stdio.h>
#include <string.h>

/**
 * @file infomb.c
 * @brief Mostra el contingut del Mapa de Bits del disc dur virtual passat per paràmetre
 * @date 21/10/2010
 */
int main (int argc, char *argv[])
{
    int i = 0;
    int cont = 0;
    unsigned char buffer[TB];
    unsigned char byte[8];
    int j = 0;

    if (argc != 3) {
        printf("[ERROR] Paràmtres incorrectes! Prova: ./infomb disco.imagen numBloc \n");
    }

    bmount(argv[1]);
    memset(buffer, 0, TB);
    bread(atoi(argv[2]), buffer);

    // recorrem tot el bloc
    for (i = 0; i < TB; i++) {
        printf("\nbyte %d ", cont);
        cont++;
        //printf("%d ", buffer[i]);

        if (buffer[i] == 255) { // si son tot 1
            if (i < 10) { // per tabular
                printf("=   1 1 1 1 1 1 1 1");
            } else {
                printf("=  1 1 1 1 1 1 1 1");
            }
        } else if (buffer[i] == 0) { // si son tot 0
            if (i >= 100 && i < 1000) { // per tabular
                printf("=  0 0 0 0 0 0 0 0");
            } else if (i >= 1000) {
                printf("=  0 0 0 0 0 0 0 0");
            } else {
                printf("=   0 0 0 0 0 0 0 0");
            }
        } else { // mostrar el valor dels bits de dins del byte
            int aux = buffer[i];

            memset(byte, 0, 8);
            printf("=  ");
            for (j = 7; j >= 0; j--) {
                if ((aux % 2) == 0) {
                    byte[j] = 0;
                } else {
                    byte[j] = 1;
                }
                aux = aux / 2;
            }

            for (j = 0; j < 8; j++) {
                printf("%d ", byte[j]);
            }
        }
    }
    printf("\n");
    bumount();
    return 0;
}
