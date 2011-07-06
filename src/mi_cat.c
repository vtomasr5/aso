//      mi_cat.c
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
 *  @file mi_cat.c
 *  @brief Mostra el contingut d'un fitxer.
 *  @date 07/01/2011
 */

#include "../include/directorios.h"

int main(int argc, char *argv[])
{
    STAT estat;
    unsigned char buff[TB];
    memset(buff, '\0', TB);
    FILE *file;
    int lectures = 0;
    uint p_inode_dir, p_inode, p_entrada;
    p_inode_dir = p_inode = p_entrada = 0;

    if (argc != 3) {
        printf("[mi_cat.c] ERROR: Arguments incorrectes. Ex: mi_cat <nomFS> <cami>\n");
        exit(-1);
    }

    sem_init();

    // montam es FS
    if (bmount(argv[1]) == -1) {
        sem_del();
        return -1;
    }

    // codi
    if (cercarEntrada(argv[2], &p_inode_dir, &p_inode, &p_entrada, '0') == -1) {
        printf("[mi_cat.c] ERROR: No s'ha trobat l'entrada!\n");
        sem_del();
        return -1;
    }

    if (mi_stat(argv[2], &estat) == -1) {
        sem_del();
        return -1;
    }

    if (estat.tipus != 1) { // si no es un directori
        if (estat.tamany > 0) { // si no esta buit
            printf("\n");

            int i = 0;
            int bf = 0;
            while (lectures < estat.blocs_assignats_dades) {
                bf = traduirBlocInode(p_inode, i, '0');
                if (bf > 0) {
                    if (mi_read(argv[2], buff, (i * TB), TB) == -1) {
                        printf("[mi_cat.c] ERROR: No s'ha pogut llegir!\n");
                        sem_del();
                        return -1;
                    }
                    lectures++; // quantitat de blocs llegits
                    //file = fopen("/dev/stdout", "w");
                    //fwrite(buff, TB, 1, file);
                    write(1, buff, estat.tamany);
                    i++;
                }
            }

            printf("\n\n");
        } else {
            printf("[mi_cat.c] INFO: Aquest fitxer esta buit.\n");
        }
    } else {
        printf("[mi_cat.c] ERROR: No se pot fet un 'mi_cat' sobre un directori!\n");
    }

    if (infoSB() == -1) { // mostram el contingut del superbloc
        return -1;
    }

    // desmontam es FS
    if (bumount() == -1) {
        sem_del();
        return -1;
    }

    sem_del();

    return 0;
}
