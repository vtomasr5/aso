/**
 *  @file read_write.c
 *  @brief Prova que comprova que les funcions mi_write_f() i mi_read_f() funcionen bé.
 *  @date 14/08/2011
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/directorios.h"
#include "../include/ficheros.h"

typedef struct {
    char nom[60];
    int valor;
} estructura;

int main(int argc, char *argv[])
{
    estructura est;
    //~ char buff[60];
    memset(est.nom, 'a', 60);

    if (argc != 2) {
        printf("[read_write.c] ERROR: Arguments incorrectes. Ex: ./read_write ../disco.imagen\n");
        exit(-1);
    }

    // montam es FS
    if (bmount(argv[1]) == -1) {
        return -1;
    }

    // proves
    //~ est.nom = "prova";
    est.valor = 10;

    int r = reservarInode(2, 7);

    if (mi_write_f(r, &est, 0 * sizeof(estructura), sizeof(estructura)) == -1) { // escrivim els canvis
        printf("[read_write.c] ERROR: No s'ha pogut escriure!\n");
    }

    if (mi_read_f(r, &est, 0 * sizeof(estructura), sizeof(estructura)) == -1) { // escrivim els canvis
        printf("[read_write.c] ERROR: No s'ha pogut llegir!\n");
    }

    printf("read_write.c] DEBUG: nom = '%s', valor = %d\n", est.nom, est.valor);

    // desmontam es FS
    if (bumount() == -1) {
        return -1;
    }

    return 0;
}
