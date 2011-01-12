//      mi_cat.c
//
//      Copyright 2010 Vicenç Juan Tomàs Montserrat <vtomasr5@gmail.com>
//      Copyright 2010 Toni Mulet Escobar <t.mulet@gmail.com>
//      Copyright 2010 Eduardo Gasser <edugasser@gmail.com>
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
 *  @file mi_cat.c
 *  @brief Mostra el contingut d'un fitxer.
 *  @date 07/01/2011
 */

#include "../libs/directorios.h"

int main(int argc, char *argv[])
{
    STAT estat;
    unsigned char buff[TB];
    memset(buff, '\0', TB);
    FILE *file;

    if (argc != 3) {
        printf("[mi_cat.c] ERROR: Arguments incorrectes. Ex: mi_cat <nomFS> <cami>\n");
        exit(-1);
    }

    // montam es FS
    if (bmount(argv[1]) == -1) {
        return -1;
    }

    // codi
    if (mi_stat(argv[2], &estat) == -1) {
        return -1;
    }

    if (estat.tipus != 1) { // si no es un directori
        if (estat.tamany > 0) { // si no esta buit
            printf("\n");

            int i;
            for (i = 0; (i * TB) < estat.tamany; i++) {
                if (mi_read(argv[2], buff, (i * TB), TB) != -1) { // BUG
                    //printf("[mi_cat.c] ERROR: No s'ha pogut llegir! (mi_read(%s, buff, %d, %d)\n", argv[2], (i*TB), TB);
                    return -1;
                }
                write(1, buff, TB);
            }

            printf("\n\n");
        } else {
            printf("[mi_cat.c] INFO: Aquest fitxer esta buit.\n");
        }
    } else {
        printf("[mi_cat.c] ERROR: No se pot fet un 'mi_cat' sobre un directori!\n");
    }

    // desmontam es FS
    if (bumount() == -1) {
        return -1;
    }

    return 0;
}
