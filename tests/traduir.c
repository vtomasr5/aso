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
    int tam = 4000;
    unsigned char buff[tam];
    uint bfisic = 0;

    if (argc != 2) {
        printf("[traduir.c] ERROR: Arguments incorrectes. Ex: ./traduir disco.imagen\n");
        exit(-1);
    }

    // montam es FS
    if (bmount(argv[1]) == -1) {
        return -1;
    }

    // proves
    int r;
    r = reservarInode(0, 7); // reservam de tipus 'lliure' i amb permis d'escriptura
    if (r == -1) {
        printf("[traduir.c] ERROR: falla reservarInode1()\n");
        return -1;
    }
    printf("[traduir.c] INFO: Inode reservat: %d\n", r);

    //~ memset(buff, '\0', tam);

    int t = traduirBlocInode(r, 0, &bfisic, '1');
    printf("[traduir.c] INFO: bfisic = %d\n", bfisic);

    /*
    int i;
    for (i = 0; i < TB -1; i++) {
        printf("%d = %d\n", i, buff[i]);
    }
    */

    //~ uint offset = 0;
    //~ uint nbytes = tam;

    //~ int s;

    //~ printf("[traudir.c] Se ejecuta mi_write_f ****\n\n");
    //~ s = mi_write_f(r, buff, offset, nbytes);
    //~ if (s == -1) {
        //~ printf("[traduir.c] ERROR: falla mi_write_f()\n");
        //~ return -1;
    //~ }
    //~ printf("\n[traduir.c] INFO: bytes escrits = %d\n", s);
    //~ printf("[traudir.c] ******* FIN mi_write_f *******\n");

/*
    r = reservarInode('0', 'r'); // reservam de tipus 'lliure' i amb permis d'escriptura
    if (r == -1) {
        printf("[traduir.c] ERROR: falla reservarInode2()\n");
        return -1;
    }

    printf("[traduir.c] INFO: Inode reservat: %d\n", r);
*/

    //~ printf("\n[traudir.c] Se ejecuta mi_read_f ****\n");
    //~ s = mi_read_f(r, buff, offset, nbytes);
    //~ if (s == -1) {
        //~ printf("[traduir.c] ERROR: falla mi_read_f()\n");
        //~ return -1;
    //~ }
    //~ printf("\n[traduir.c] INFO: bytes llegits = %d\n", s);
    //~ printf("[traudir.c] ******* FIN mi_read_f *******\n");

    // desmontam es FS
    if (bumount() == -1) {
        return -1;
    }

    return 0;
}
