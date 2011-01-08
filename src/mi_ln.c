//      mi_ln.c
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
 *  @file mi_ln.c
 *  @brief Crea un enllaç entre dos directoris.
 *  @date 07/01/2011
 */

#include "../libs/directorios.h"

int main(int argc, char *argv[])
{
    if (argc != 4) {
        printf("[mi_ln.c] ERROR: Arguments incorrectes. Ex: mi_ln <nomFS> <cami_nou> <cami_existent>\n");
        exit(-1);
    }

    // montam es FS
    if (bmount(argv[1]) == -1) {
        return -1;
    }

    // codi
    if (mi_link(argv[2], argv[3]) == -1) {
        return -1;
    } else {
        printf("[mi_ln.c] INFO: Enllaç realitzat correctament.\n");
    }

    // desmontam es FS
    if (bumount() == -1) {
        return -1;
    }

    return 0;
}
