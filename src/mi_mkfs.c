//      mi_mkfs.c
//
//      Copyright 2010 Vicenç Juan Tomàs Montserrat <vtomasr5@gmail.com>
//      Copyright 2010 Toni Mulet Escobar <t.mulet@gmail.com>
//      Copyright 2010 Eduardo Gasser <uderessag@gmail.com>
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.

/**
 *  @file mi_mkfs.c
 *  @brief S'encarrega de crear el sistema de fitxers sobre el fitxer que emula
 *  un disc dur virtual. Per realitzar tal tasca fa ús de les funcions implementades
 *  en la biblioteca ficheros_basico.c.
 *  @date 04/10/2010
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/bloques.h"
#include "../include/definicions.h"
#include "../include/ficheros_basico.h"
#include "../include/semaforos.h"

/**
 *  Punt d'inici del programa mi_mkfs. Es comproven els paràmetres, es monta l'arxiu virtual,
 *  s'escriuen els blocs al fitxer i s'inicialitzen les estructures de la part de metadades del
 *  disc dur virtual. Finalment es desmonta l'arxiu virtual, deixant el disc dur formatat i a punt
 *  per executar sobre ell operacions de lectura, escritura d'informació.
 *  @param argc Nombre d'arguments del programa
 *  @param argv Array dels arguments del programa
 */
int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("[mi_mkfs.c] ERROR: Arguments incorrectes. Ex: mi_chmod <nomFS> <num_blocs>\n");
        exit(-1);
    }

    if (atoi(argv[2]) < 0) {
        printf("[mi_mkfs.c] ERROR: Nombre de bloc incorrectes\n");
        exit(-1);
    }

    // montam es FS
    if (bmount(argv[1]) == -1) {
        return -1;
    }

    int i;
    unsigned char buff[TB];
    // escrivim els blocs passats per paràmetre
    for(i = 0; i < atoi(argv[2]); i++) {
        if (bwrite(i, buff) == -1) {
            return -1;
        }
    }

    // inicialitzam el SB
    if (initSB(atoi(argv[2])) == -1) {
        printf("[mi_mkfs.c] ERROR: Error cridant a initSB");
        return -1;
    }

    // inicialitzam el MB
    if (initMB() == -1) {
        printf("[mi_mkfs.c] ERROR: Error cridant a initMB");
        return -1;
    }

    // inicialitzam l'AI
    if (initAI(atoi(argv[2])) == -1) {
        printf("[mi_mkfs.c] ERROR: Error cridant a initAI");
        return -1;
    }

    // reservam l'inode arrel del tipus directori (1)
    int r;
    if ((r = reservarInode(1, 7)) == -1) {
        printf("[mi_mkfs.c] ERROR: Error cridant a reservarInode");
        return -1;
    }

    if (infoSB() == -1) { // mostram el contingut del superbloc
        printf("[mi_mkfs.c] ERROR: Error cridant a infoSB");
        return -1;
    }

    // desmontam es FS
    if (bumount() == -1) {
        return -1;
    }
    return 0;
}
