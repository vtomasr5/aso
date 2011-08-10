//      mi_truncar.c
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
 *  @file mi_truncar.c
 *  @brief Elimina el contingut del fitxer sense eliminar-ho del disc. Ho buida.
 *  @date 07/01/2011
 */

#include "../include/directorios.h"

int main(int argc, char *argv[])
{
    STAT estat;
    uint p_inode, p_entrada, p_inode_dir = 0;
    uint num_inode = 0;
    uint bytes = 0;

    if (argc != 4) {
        printf("[mi_truncar.c] ERROR: Arguments incorrectes. Ex: mi_truncar <nomFS> <cami> <nbytes>\n");
        exit(-1);
    }

    sem_init();

    // montam es FS
    if (bmount(argv[1]) == -1) {
        sem_del();
        return -1;
    }

    // codi
    if (cercarEntrada(argv[2], &p_inode_dir, &p_inode, &p_entrada, 0, 7) == -1) {
        printf("[mi_truncar.c] ERROR: No s'ha trobat el cami!!\n");
        sem_del();
        return -1;
    }

    if (mi_stat(argv[2], &estat) == -1) {
        sem_del();
        return -1;
    }

    num_inode = p_inode;
    bytes = atoi(argv[3]);

    if (estat.tipus != 1) { // si no es un directori
        if (estat.tamany > 0) { // si no esta buit
            if (mi_truncar_f(num_inode, bytes) == -1) {
                printf("[mi_truncar.c] ERROR: No s'ha pogut truncar!\n");
            }
        } else {
            printf("[mi_truncar.c] INFO: Aquest fitxer esta buit!\n");
        }
    } else {
        printf("[mi_truncar.c] INFO: Només se poden truncar fitxers!\n");
    }

    // desmontam es FS
    if (bumount() == -1) {
        sem_del();
        return -1;
    }

    sem_del();

    return 0;
}
