//      mi_write.c
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
 *  @file mi_write.c
 *  @brief Escriu a un fitxer.
 *  @date 07/01/2011
 */

#include "../include/directorios.h"

int main(int argc, char *argv[])
{
    char *buff;
    int offset;
    int nbytes;

    if (argc != 6) {
        printf("[mi_write.c] ERROR: Arguments incorrectes. Ex: mi_write <nomFS> <cami> <buff> <offset> <nbytes>\n");
        exit(-1);
    }

    sem_init();

    // montam es FS
    if (bmount(argv[1]) == -1) {
        sem_del();
        return -1;
    }

    // codi
    offset = atoi(argv[4]);
    nbytes = atoi(argv[5]);
    buff = argv[3];

    if (mi_write(argv[2], buff, offset, nbytes) == -1) {
        printf("[mi_write.c] ERROR: No s'ha pogut escriure!\n");
    } else {
        printf("[mi_write.c] INFO: S'ha escrit correctament.\n");
    }

    // desmontam es FS
    if (bumount() == -1) {
        sem_del();
        return -1;
    }

    sem_del();

    return 0;
}
