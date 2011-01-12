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
    char buff[100];
    char buffer[BUFFER_DIR];
    int m = -3; // inicialitzacio diferent per a provar que s'actualitza el valor

    memset(buffer, '\0', BUFFER_DIR);
    memset(buff, 'a', 100);

    if (argc != 2) {
        printf("[cercar.c] ERROR: Arguments incorrectes. Ex: ./cercar ../disco.imagen\n");
        exit(-1);
    }

    // montam es FS
    if (bmount(argv[1]) == -1) {
        return -1;
    }

    // proves
    printf("nnnnnnnnnnnnnnnnnn creat /dir1/fichero nnnnnnnnnnnnnnnnnnnnnn\n");
    mi_creat("/dir1/fichero", 7);

    memset(buffer, '\0', BUFFER_DIR); // reset buffer

    printf("nnnnnnnnnnnnnnnnnnn mi dir /dir1 nnnnnnnnnnnnnnnnnnnnnnnnn\n");
    m = mi_dir("/dir1/", buffer);
    printf("[cercar.c] DEBUG: num fitxers dins el directori = %d\n", m);

    printf("nnnnnnnnnnnnnnnnnnn mi lsdir /dir1 nnnnnnnnnnnnnnnnnnnnnnnnn\n");
    mi_lsdir("/dir1/", buffer);

    printf("nnnnnnnnnnnnnnnnnnn mi write /dir1/fichero nnnnnnnnnnnnnnnnnnnnnnnnn\n");
    mi_write("/dir1/fichero", buff, 10, 100);

    memset(buffer, '\0', BUFFER_DIR); // reset buffer

    printf("nnnnnnnnnnnnnnnnnnn mi dir /dir1 nnnnnnnnnnnnnnnnnnnnnnnnn\n");
    m = mi_dir("/dir1/", buffer);
    printf("[cercar.c] DEBUG: num fitxers dins el directori = %d\n", m);

    printf("nnnnnnnnnnnnnnnnnnn mi lsdir /dir1 nnnnnnnnnnnnnnnnnnnnnnnnn\n");
    mi_lsdir("/dir1/", buffer);

    printf("nnnnnnnnnnnnnnnnnnn mi read /dir1/fichero nnnnnnnnnnnnnnnnnnnnnnnnn\n");
    mi_read("/dir1/fichero", buff, 10, 100);

    printf("[cercar.c] INFO: Contingut buff: %s\n", buff);

    // desmontam es FS
    if (bumount() == -1) {
        return -1;
    }

    return 0;
}
