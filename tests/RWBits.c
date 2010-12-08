/**
 *  @file RWBits.c
 *  @brief Realitza les proves necessaries per a verificar el correcte funcionament
 *  de les funcions escriureBit() i llegirBit()
 *  @date 28/10/2010
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../libs/bloques.h"
#include "../libs/definicions.h"
#include "../libs/ficheros_basico.h"

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

    int c;
    if ((c = llegirBit(b)) == -1) {
        return -1;
    }
    printf("Bit llegit: %d\n", c);

    // desmontam es FS
    if (bumount() == -1) {
        return -1;
    }
    return 0;
}
