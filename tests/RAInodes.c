/**
 *  @file RAInodes.c
 *  @brief Realitza les proves necessaries per a verificar el correcte funcionament
 *  de les funcions reservarInode() i alliberarInode()
 *  @date 17/07/2011
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/bloques.h"
#include "../include/definicions.h"
#include "../include/ficheros_basico.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("[RAInodes.c] ERROR: Arguments incorrectes. Ex: RAInodes disco.imagen\n");
        exit(-1);
    }

    // montam es FS
    if (bmount(argv[1]) == -1) {
        return -1;
    }

    superbloc sb;
    if (bread(POSICIO_SB, (char *)&sb) == -1) { // llegim el superbloc
        return -1;
    }

    // proves
    int b;
    if ((b = reservarInode(2, 7)) == -1) { // tipus = fitxer (2), permisos = 7
        return -1;
    }
    printf("Inode reservat: %d\n", b);

    inode d;
    d = llegirInode(b);
    printf("Contingut inode (permisos): %d\n", d.permisos);
    printf("Contingut inode (tamany): %d\n", d.tamany);
    printf("Contingut inode (tipus): %d\n", d.tipus);

    int e;
    if ((e = alliberarInode(b, 1)) == -1) {
        printf("ERROR en alliberarInode()\n");
        return -1;
    }
    printf("Inode alliberat: %d\n", e);

    // desmontam es FS
    if (bumount() == -1) {
        return -1;
    }
    return 0;
}
