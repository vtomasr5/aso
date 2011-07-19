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
        printf("[RWBlocs.c] ERROR: Arguments incorrectes. Ex: RWBlocs disco.imagen\n");
        exit(-1);
    }

    // montam es FS
    if (bmount(argv[1]) == -1) {
        return -1;
    }

    // proves
    int b = -1;
    if ((b = reservarBloc()) == -1) {
        if (bumount() == -1) {
            return -1;
        }
        return -1;
    }
    printf("Bloc reservat: %d\n", b);

    int bit;
    if ((bit = llegirBit(b)) == -1) {
        return -1;
    }
    printf("Bit llegit: %d\n", bit);

    int d = -1;
    if ((d = alliberarBloc(b)) == -1) {
        return -1;
    }
    printf("Bloc alliberat: %d\n", d);

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
