//      mi_creat.c
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
 *  @file mi_creat.c
 *  @brief Crea un nou fitxer o directori amb uns permisos determinats.
 *  @date 07/01/2011
 */

#include "../include/directorios.h"

int main(int argc, char *argv[])
{
    unsigned short int mode;

    if (argc != 4) {
        printf("[mi_creat.c] ERROR: Arguments incorrectes. Ex: mi_creat <nomFS> <cami> <permissos>\n");
        exit(-1);
    }

    sem_init();

    mode = atoi(argv[3]);
    if (mode < 0 || mode > 7) {
        printf("[mi_creat.c] ERROR: Permissos incorrectes!\n");
        sem_del();
        return -1;
    }

    // montam es FS
    if (bmount(argv[1]) == -1) {
        sem_del();
        return -1;
    }

    // codi
    mi_creat(argv[2], mode);

    // desmontam es FS
    if (bumount() == -1) {
        sem_del();
        return -1;
    }

    sem_del();

    return 0;
}
