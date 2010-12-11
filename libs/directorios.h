//      directorios.h
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
 *  @file directorios.h
 *  @brief Conté les estructures i capçaleres de les funcions sobre directoris més alt nivell que
 *  s'implementen en el fitxer directorios.c
 *  @date 10/12/2010
 */

#include "ficheros.h"

typedef struct {
    char nom[60];
    unsigned int inod;
} entrada;

int extreureCami(const char *cami, char *inicial, char *final);
int cercarEntrada(const char *cami_parcial, unsigned int *p_inode_dir, unsigned int *p_inode, unsigned int *p_entrada, char reservar);

int mi_creat(const char *cami, unsigned char mode);
int mi_link(const char *cami1, const char *cami2);
int mi_unlink(const char *cami);
int mi_dir(const char *cami, char *buffer);