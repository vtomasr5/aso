//      mi_pwd.c
//
//      Copyright 2011 Vicenç Juan Tomàs Montserrat <vtomasr5@gmail.com>
//      Copyright 2011 Toni Mulet Escobar <t.mulet@gmail.com>
//      Copyright 2011 Eduardo Gasser <edugasser@gmail.com>
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
 *  @file mi_pwd.c
 *  @brief Mostra la ruta on està actualment.
 *  @date 24/06/2011
 */

#include "../include/directorios.h"

int main(int argc, char *argv[])
{
    // montam es FS
    if (bmount(argv[1]) == -1) {
        return -1;
    }

    // codi
    // pwd

    // desmontam es FS
    if (bumount() == -1) {
        return -1;
    }

    return 0;
}
