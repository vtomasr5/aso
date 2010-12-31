/**
 *  @file cercar.c
 *  @brief Prova que realitza la cerca d'entrades al directori
 *  @date 28/12/2010
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../libs/directorios.h"

int main(int argc, char *argv[])
{
    //int tam = 2000;
    //unsigned char buffer[tam];

    if (argc != 2) {
        printf("[mi_mkfs.c] ERROR: Arguments incorrectes. Ex: ./cercar disco.imagen\n");
        exit(-1);
    }

    // montam es FS
    if (bmount(argv[1]) == -1) {
        return -1;
    }

    // proves
    printf("nnnnnnnnnnnnnnnnnnnn creat / nnnnnnnnnnnnnnnnnnnnnnnnnnn\n");
    mi_creat("/", 7);

    printf("nnnnnnnnnnnnnnnnnn creat /dir2/dir3/fichero nnnnnnnnnnnnnnnnnnnnnn\n");
    mi_creat("/dir2/dir3/fichero", 7);

    printf("nnnnnnnnnnnnnnnnnn creat /dir4/ nnnnnnnnnnnnnnnnnnnnnnnnnn\n");
    mi_creat("/dir4/", 7);

    printf("nnnnnnnnnnnnnnnnnnn mi link nnnnnnnnnnnnnnnnnnnnnnnnn\n");
    mi_link("/dir2/dir3/","/dir4/");

    // desmontam es FS
    if (bumount() == -1) {
        return -1;
    }

    return 0;
}
