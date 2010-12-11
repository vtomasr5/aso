//      directorios.c
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
 *  @file directorios.c
 *  @brief Conté els funcions sobre directoris de més alt nivell que interactuen amb el sistema de fitxers.
 *  S'implementen les funcions necessàries per a llegir i escriure en el sistema de fitxers.
 *  @date 10/12/2010
 */

#include "directorios.h"


int extreureCami(const char *cami, char *inicial, char *final)
{
    return 0;
}

int cercarEntrada(const char *cami_parcial, unsigned int *p_inode_dir, unsigned int *p_inode, unsigned int *p_entrada, char reservar)
{
    return 0;
}

int mi_creat(const char *cami, unsigned char mode)
{
    return 0;
}

int mi_link(const char *cami1, const char *cami2)
{
    return 0;
}

int mi_unlink(const char *cami)
{
    return 0;
}

int mi_dir(const char *cami, char *buffer)
{
    return 0;
}
