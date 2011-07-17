/**
 *  @file RInode.c
 *  @brief Llegeix el contingut d'un inode que pertany a un bloc.
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
    superbloc sb;

    if (argc != 3) {
        printf("[RInode.c] ERROR: Arguments incorrectes. Ex: RInode disco.imagen n_bloc\n");
        exit(-1);
    }

    if (argv[2] < 0) {
        printf("[RWInodes.c] ERROR: El n_bloc no pot ser negatiu!\n");
        exit(-1);
    }

    // montam es FS
    if (bmount(argv[1]) == -1) {
        return -1;
    }

    // llegim el superbloc
    if (bread(POSICIO_SB, (char *)&sb) == -1) {
        return -1;
    }

    // proves
    inode d;
    d = llegirInode(atoi(argv[2]));
    printf("Contingut inode (permisos): %d\n", d.permisos);
    printf("Contingut inode (tamany): %d\n", d.tamany);

    // desmontam es FS
    if (bumount() == -1) {
        return -1;
    }
    return 0;
}
