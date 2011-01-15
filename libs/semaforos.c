//      semaforos.c
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
 *  @file semaforos.c
 *  @brief Conté les funcions que implementen la funcionalitat d'un semàfor mutex.
 *  @date 11/01/2011
 */

#include "semaforos.h"

/**
 *  Funció que crea un nou semàfor mutex
 *  @param clau
 *  @param num
 *  @return El descriptor del nou semàfor.
 */
int nouSemafor(int clau, int num)
{
    int s = semget(clau, num, IPC_CREAT | 0600); // cream el semafor

    if (s < 0) {
        printf("[semaforos.c] ERROR: Error creant el semafor!\n");
        return -1;
    }

    return s;
}

/**
 *  Funció que decrementa el valor del semàfor mutex
 *  @param s descriptor del semafor a esperar (wait)
 *  @param pos
 *  @param flag
 */
void esperarSemafor(int s, int pos, int flag)
{
    struct sembuf sbuf;

    sbuf.sem_num = pos;
    sbuf.sem_op = -1;
    sbuf.sem_flg = flag;

    semop(s, &sbuf, 1);
}

/**
 *  Funció que incrementa el valor del semàfor mutex
 *  @param s descriptor del semafor a senyalitzar (signal)
 *  @param pos
 */
void senyalitzarSemafor(int s, int pos)
{
    struct sembuf sbuf;

    sbuf.sem_num = pos;
    sbuf.sem_op = 1;
    sbuf.sem_flg = 0;

    semop(s, &sbuf, 1);
}

/**
 *
 *  @param s descriptor del semafor a inicialitzar
 *  @param valor
 */
void inicialitzarSemafor(int s, int valor)
{
    semctl(s, 0, SETVAL, valor);
}

/**
 *
 *  @param s descriptor del semafor a eliminar
 */
void eliminarSemafor(int s)
{
    int rem = semctl(s, 0, IPC_RMID, 0);

    if (rem < 0) {
        printf("[semaforos.c] ERROR: Error eliminant el semafor!\n");
        exit(-1);
    }

    printf("[semaforos.c] DEBUG: Semafor %d eliminat correctament.\n", s);
}
