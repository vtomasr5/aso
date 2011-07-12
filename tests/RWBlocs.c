/**
 *  @file RWBlocs.c
 *  @brief Realitza les proves necessaries per a verificar el correcte funcionament
 *  de les funcions reservarBloc() i alliberarBloc()
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
        printf("[mi_mkfs.c] ERROR: Arguments incorrectes. Ex: mi_mkfs disco.imagen\n");
        exit(-1);
    }

    // montam es FS
    if (bmount(argv[1]) == -1) {
        return -1;
    }

    // proves
    int b;
    if ((b = reservarBloc()) == -1) {
        return -1;
    }
    printf("Bloc reservat: %d\n", b);

    int bit;
    if ((bit = llegirBit(b)) == -1) {
        return -1;
    }
    printf("Bit llegit: %d\n", bit);

    if (alliberarBloc(b) == -1) {
        return -1;
    }
    printf("Bloc alliberat\n");

    bit = -1;
    if ((bit = llegirBit(b)) == -1) {
        return -1;
    }
    printf("Bit llegit: %d\n", bit);

    // desmontam es FS
    if (bumount() == -1) {
        return -1;
    }
    return 0;
}
