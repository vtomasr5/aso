//      semaforos.h
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
 *  @file semaforos.h
 *  @brief Conté les capçaleres de les funcions que implementen la funcionalitat d'un semàfor mutex.
 *  @date 11/01/2011
 */

#if !defined(_SEMAFOROS_H)
#define _SEMAFOROS_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <stdlib.h>

int nouSemafor(int clau, int num);
void esperarSemafor(int s);
void senyalitzarSemafor(int s);
void inicialitzarSemafor(int s, int valor);
void eliminarSemafor(int s);

#endif // _SEMAFOROS_H
