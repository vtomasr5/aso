//      mi_stat.c
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
 *  @file mi_stat.c
 *  @brief Mostra el contingut d'un inode.
 *  @date 07/01/2011
 */

#include "../libs/directorios.h"

int main(int argc, char *argv[])
{
    STAT estat;

    if (argc != 3) {
        printf("[mi_stat.c] ERROR: Arguments incorrectes. Ex: mi_stat <nomFS> <cami>\n");
        exit(-1);
    }

    // montam es FS
    if (bmount(argv[1]) == -1) {
        return -1;
    }

    // codi
    if (infoSB() == -1) { // mostram el contingut del superbloc
        return -1;
    }

    if (mi_stat(argv[2], &estat) == -1) {
        return -1;
    } else {
        printf("\nEstat del camí '%s':\n", argv[2]);
        printf("\n");
        veure_estat(&estat);
        printf("\n");
    }

    if (infoSB() == -1) { // mostram el contingut del superbloc
        return -1;
    }

    // desmontam es FS
    if (bumount() == -1) {
        return -1;
    }

    return 0;
}
