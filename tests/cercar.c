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
    char buffer[BUFFER_DIR];
    int m = -3; // inicialitzacio diferent per a provar que s'actualitza el valor

    memset(buffer, '\0', BUFFER_DIR);

    if (argc != 2) {
        printf("[mi_mkfs.c] ERROR: Arguments incorrectes. Ex: ./cercar ../disco.imagen\n");
        exit(-1);
    }

    // montam es FS
    if (bmount(argv[1]) == -1) {
        return -1;
    }

    // proves
    printf("nnnnnnnnnnnnnnnnnn creat /dir2/dir3/fichero nnnnnnnnnnnnnnnnnnnnnn\n");
    mi_creat("/dir2/dir3/fichero", 7);

    printf("nnnnnnnnnnnnnnnnnn creat /dir3/ nnnnnnnnnnnnnnnnnnnnnn\n");
    mi_creat("/dir3/", 7);

    printf("nnnnnnnnnnnnnnnnnnn mi link nnnnnnnnnnnnnnnnnnnnnnnnn\n");
    mi_link("/dir4/","/dir3/"); // dir4 no tiene que existir

    printf("nnnnnnnnnnnnnnnnnn creat /dir4/fichero nnnnnnnnnnnnnnnnnnnnnn\n");
    mi_creat("/dir4/fichero", 7);

    printf("nnnnnnnnnnnnnnnnnnn mi dir nnnnnnnnnnnnnnnnnnnnnnnnn\n");
    m = mi_dir("/dir4/", buffer);
    printf("[cercar.c] DEBUG: num fitxers dins el directori = %d\n", m);

    printf("nnnnnnnnnnnnnnnnnnn mi lsdir nnnnnnnnnnnnnnnnnnnnnnnnn\n");
    mi_lsdir("/dir4/", buffer);

    printf("nnnnnnnnnnnnnnnnnn creat /dir4/ nnnnnnnnnnnnnnnnnnnnnnnnnn\n");
    mi_creat("/dir4/", 7);

    printf("nnnnnnnnnnnnnnnnnn creat /dir3/ nnnnnnnnnnnnnnnnnnnnnnnnnn\n");
    mi_creat("/dir3/", 7); // per veure info

    printf("nnnnnnnnnnnnnnnnnnn mi unlink nnnnnnnnnnnnnnnnnnnnnnnnn\n");
    mi_unlink("/dir4/"); // borrar entrada

    printf("nnnnnnnnnnnnnnnnnnn mi dir nnnnnnnnnnnnnnnnnnnnnnnnn\n");
    m = mi_dir("/dir2/dir3/", buffer);
    printf("[cercar.c] DEBUG: num fitxers dins el directori = %d\n", m);

    printf("nnnnnnnnnnnnnnnnnnn mi lsdir nnnnnnnnnnnnnnnnnnnnnnnnn\n");
    mi_lsdir("/dir2/dir3/", buffer);

    // desmontam es FS
    if (bumount() == -1) {
        return -1;
    }

    return 0;
}
