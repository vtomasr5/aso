/**
 *  @file traduir.c
 *  @brief Prova que realitza la tradució de bloc lógic a bloc físic mitjançant inodes
 *  @date 09/11/2010
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../libs/bloques.h"
#include "../libs/definicions.h"
#include "../libs/ficheros.h"

int main(int argc, char *argv[])
{
    unsigned char buff[TB];

    if (argc != 2) {
        printf("[mi_mkfs.c] ERROR: Arguments incorrectes. Ex: ./traduir disco.imagen\n");
        exit(-1);
    }

    // montam es FS
    if (bmount(argv[1]) == -1) {
        return -1;
    }

    // proves
    int r;
    r = reservarInode('0', 'w'); // reservam de tipus 'lliure' i amb permis d'escriptura
    if (r == -1) {
        printf("[traduir.c] ERROR: falla reservarInode1()\n");
        return -1;
    }
    printf("[traduir.c] INFO: Inode reservat: %d\n", r);

    memset(buff, 2, TB);

    /*
    int i;
    for (i = 0; i < TB -1; i++) {
        printf("%d = %d\n", i, buff[i]);
    }
    */

    const int offset = 0;
    const int nbytes = 7000;

    int s;

    printf("[traudir.c] Se ejecuta mi_write_f ****\n");
    s = mi_write_f(r, buff, offset, nbytes);
    if (s == -1) {
        printf("[traduir.c] ERROR: falla mi_write_f()\n");
        return -1;
    }
    printf("[traduir.c] INFO: bytes escrits = %d\n", s);
    printf("[traudir.c] ******* FIN mi_write_f *******\n");

/*
    r = reservarInode('0', 'r'); // reservam de tipus 'lliure' i amb permis d'escriptura
    if (r == -1) {
        printf("[traduir.c] ERROR: falla reservarInode2()\n");
        return -1;
    }

    printf("[traduir.c] INFO: Inode reservat: %d\n", r);
*/
/*
    printf("[traudir.c] Se ejecuta mi_read_f ****\n");
    s = mi_read_f(r, buff, offset, nbytes);
    if (s == -1) {
        printf("[traduir.c] ERROR: falla mi_read_f()\n");
        return -1;
    }
    printf("[traduir.c] INFO: bytes llegits = %d\n", s);
    printf("[traudir.c] ******* FIN mi_read_f *******\n");
*/

    // desmontam es FS
    if (bumount() == -1) {
        return -1;
    }

    return 0;
}
