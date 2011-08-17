/**
 *  @file traduir.c
 *  @brief Prova que realitza la tradució de bloc lógic a bloc físic mitjançant inodes
 *  @date 09/11/2010
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/bloques.h"
#include "../include/definicions.h"
#include "../include/ficheros.h"

int main(int argc, char *argv[])
{
    uint bfisic = 0;

    if (argc != 3) {
        printf("[traduir.c] ERROR: Arguments incorrectes. Ex: ./traduir disco.imagen <num_bloc>\n");
        exit(-1);
    }

    // montam es FS
    if (bmount(argv[1]) == -1) {
        return -1;
    }

    // proves
    int r;
    r = reservarInode(2, 7); // reservam de tipus 'dir' i amb permis d'escriptura
    if (r == -1) {
        printf("[traduir.c] ERROR: falla reservarInode()\n");
        bumount();
        return -1;
    }
    printf("[traduir.c] INFO: Inode reservat: %d\n", r);

    if (traduirBlocInode(r, atoi(argv[2]), &bfisic, 1) == -1) {
        printf("[traduir.c] ERROR: falla traduir_bloc_inode\n");
        bumount();
        return -1;
    }
    printf("[traduir.c] INFO: bloc fisic = %d\n", bfisic);

    if (alliberarInode(r) == -1) {
        printf("[traduir.c] ERROR: falla alliberarInode()\n");
        bumount();
        return -1;
    }

    // desmontam es FS
    if (bumount() == -1) {
        return -1;
    }

    return 0;
}
