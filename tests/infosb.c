#include "../libs/bloques.h"
#include "../libs/ficheros_basico.h"
#include "../libs/definicions.h"

/**
 * @file infosb.c
 * @brief Mostra el contingut del superbloc del disc dur virtual passat per paràmetre
 * @date 21/10/2010
 */
int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("[ERROR] Paràmetres incorrectes! Prova: ./infosb ../disco.imagen\n");
        exit(-1);
    }

    bmount(argv[1]);

    infoSB();

    bumount();

    return 0;
}
