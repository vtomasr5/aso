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

#define TAM 1024

typedef struct {
    char nom[TAM];
    int valor;
} estructura;

int main(int argc, char *argv[])
{
    estructura est;

    if (argc != 2) {
        printf("[read_write.c] ERROR: Arguments incorrectes. Ex: ./read_write ../disco.imagen\n");
        exit(-1);
    }

    // montam es FS
    if (bmount(argv[1]) == -1) {
        return -1;
    }

    // proves
    memset(est.nom, 'a', TAM);
    est.valor = 10;

    printf("[read_write.c] DEBUG: est.nom = %s\n", est.nom);

    int r = reservarInode(1, 7);

    if (mi_write_f(r, &est, 0, sizeof(estructura)) == -1) { // escrivim els canvis
        printf("[read_write.c] ERROR: No s'ha pogut escriure!\n");
    }

    printf("[read_write.c] DEBUG: sizeof(est) escritura = %ld\n", sizeof(est));
    printf("[read_write.c] DEBUG: sizeof(est.nom) escritura = %ld\n", sizeof(est.nom));
    printf("[read_write.c] DEBUG: sizeof(est.valor) escritura = %ld\n\n", sizeof(est.valor));

    if (mi_read_f(r, &est, 0, sizeof(estructura)) == -1) { // escrivim els canvis
        printf("[read_write.c] ERROR: No s'ha pogut llegir!\n");
    }

    printf("[read_write.c] DEBUG: sizeof(est) lectura = %ld\n", sizeof(est));
    printf("[read_write.c] DEBUG: sizeof(est.nom) lectura = %ld\n", sizeof(est.nom));
    printf("[read_write.c] DEBUG: sizeof(est.valor) lectura = %ld\n", sizeof(est.valor));

    alliberarInode(r);

    printf("[read_write.c] DEBUG:\nnom = '%s'\nvalor = %d\n", est.nom, est.valor);

    // desmontam es FS
    if (bumount() == -1) {
        return -1;
    }

    return 0;
}
