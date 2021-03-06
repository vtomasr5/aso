/**
 *  @file RWInodes.c
 *  @brief Realitza les proves necessaries per a verificar el correcte funcionament
 *  de les funcions reservarInode() i alliberarInode()
 *  @date 28/10/2010
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
        printf("[RWInodes.c] ERROR: Arguments incorrectes. Ex: RWInodes disco.imagen\n");
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
    if ((b = reservarBloc()) == -1) {
        return -1;
    }
    printf("Bloc reservat: %d\n", b);

    // canviam el contingut de l'inode
    inode inod;
    inod.permisos = 3;
    inod.tamany = 48;

    int i;
    if ((i = escriureInode(b, inod)) == -1) {
        return -1;
    }
    printf("Inode escrit al bloc %d\n", b);

    inode d;
    d = llegirInode(b);
    printf("Contingut inode (permisos): %d\n", d.permisos);
    printf("Contingut inode (tamany): %d\n", d.tamany);

    int e;
    if ((e = alliberarBloc(b)) == -1) {
        return -1;
    }
    printf("Bloc alliberat: %d\n", e);

    // desmontam es FS
    if (bumount() == -1) {
        return -1;
    }
    return 0;
}
