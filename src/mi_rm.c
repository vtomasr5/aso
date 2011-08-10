//      mi_rm.c
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
 *  @file mi_rm.c
 *  @brief Elimina un directori o fitxer si només hi ha enllaç. Si n'hi ha més d'ún elimina un enllaç del mateix.
 *  @date 07/01/2011
 */

#include "../include/directorios.h"

int main(int argc, char *argv[])
{

    if (argc != 3) {
        printf("[mi_rm.c] ERROR: Arguments incorrectes. Ex: mi_rm <nomFS> <cami>\n");
        exit(-1);
    }

    sem_init();

    // montam es FS
    if (bmount(argv[1]) == -1) {
        sem_del();
        return -1;
    }

    // codi

    if (mi_unlink(argv[2]) == -1) {
        printf("[mi_rm.c] ERROR: No s'ha pogut eliminar!\n");
    }

    // desmontam es FS
    if (bumount() == -1) {
        sem_del();
        return -1;
    }

    sem_del();

    return 0;
}
