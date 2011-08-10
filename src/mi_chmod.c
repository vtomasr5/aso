//      mi_chmod.c
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
 *  @file mi_chmod.c
 *  @brief Canvia els permisos del fitxer o directori.
 *  @date 07/01/2011
 */

#include "../include/directorios.h"

int main(int argc, char *argv[])
{
    unsigned short int mode;

    if (argc != 4) {
        printf("[mi_chmod.c] ERROR: Arguments incorrectes. Ex: mi_chmod <nomFS> <cami> <permissos_nous>\n");
        exit(-1);
    }

    sem_init();

    // montam es FS
    if (bmount(argv[1]) == -1) {
        sem_del();
        return -1;
    }

    // codi
    mode = atoi(argv[3]);

    if (mi_chmod(argv[2], mode) != -1) {
        printf("\n[mi_chmod.c] INFO: Permisos canviats correctament\n\n");
    }

    // desmontam es FS
    if (bumount() == -1) {
        sem_del();
        return -1;
    }

    sem_del();

    return 0;
}
