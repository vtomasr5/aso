/**
 *  @file crear_entrades.c
 *  @brief Crear 16 entrades de directori.
 *  @date 14/08/2011
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/directorios.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("[crear_entrades.c] ERROR: Arguments incorrectes. Ex: ./crear_entrades disco.imagen\n");
        exit(-1);
    }

    // montam es FS
    if (bmount(argv[1]) == -1) {
        return -1;
    }

    // proves
    char nom[50];

    int i = 0;
    for (i = 1; i <= 16; i++) {
        sprintf(nom, "/dir%d/", i);
        if (mi_creat(nom, 7) == -1) {
            printf("[crear_entrades.c] ERROR: No s'han creat les entrades.\n");
            return -1;
        }
    }

    // desmontam es FS
    if (bumount() == -1) {
        return -1;
    }

    return 0;
}
