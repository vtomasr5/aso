//      ficheros.c
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
 *  @file ficheros.c
 *  @brief Conté les funcions de més alt nivell que interactuen amb el sistema de fitxers.
 *  S'implementen les funcions necessàries per a llegir i escriure en el sistema de fitxers.
 *  @date 15/11/2010
 */

#include "../include/ficheros.h"

/**
 *  Funció que s'encarrega d'escriure x bytes en els blocs corresponents en el sistema de fitxers.
 *  @param  inod posició de l'inode on s'escriuran el blocs corresponents
 *  @param  buff_original buffer que conté els bytes a escriure
 *  @param  offset desplaçament de x bytes per a escriure en la posició correcta dins el bloc
 *  @param  nbytes nombre de bytes a escriure
 *  @return nombre de bytes escrits al bloc
 */
int mi_write_f (unsigned int inod, const void *buff_original, unsigned int offset, unsigned int nbytes)
{
    inode in;
    unsigned char buff_bloc[TB];
    uint bytes_escrits = 0;
    uint primer_bloc_logic = offset / TB; // primer bloc (logic) on escriurem
    uint darrer_bloc_logic = (offset + nbytes - 1) / TB; // ultim bloc on escriurem primer_byte_logic

    uint primer_byte_logic = offset % TB; // offset del primer bloc. Desplaçament dins el bloc
    uint darrer_byte_logic = (offset + nbytes - 1) % TB; // numero de bytes a escriure en el primer bloc
    memset(buff_bloc, '\0', TB);
    uint bfisic = 0;
    int ret = 0;

    in = llegirInode(inod);

    if ((in.permisos != 6) && (in.permisos != 7) && (in.permisos != 2) && (in.permisos != 3)) {
        printf("[ficheros.c] ERROR: L'inode no te permisos d'escriptura!\n");
        return -1;
    }

    ret = traduirBlocInode(inod, primer_bloc_logic, &bfisic, 1); // bloc físic
    if (ret == -1) {
        printf("[ficheros.c] ERROR: traduirBlocInode()1\n");
        return -1;
    }

    // únicament tenim que escriure en un bloc
    if ((primer_byte_logic + nbytes) < TB) {
        if (bread(bfisic, buff_bloc) == -1) {
            return -1;
        }

        memcpy(buff_bloc + primer_byte_logic, buff_original, nbytes);

        if (bwrite(bfisic, buff_bloc) == -1) {
            return -1;
        }

        bytes_escrits += nbytes;

    } else { // cas en que escrivim en més d'un bloc
        if (bread(bfisic, buff_bloc) == -1) {
            return -1;
        }

        memcpy(buff_bloc + primer_byte_logic, buff_original, TB - primer_byte_logic);

        if (bwrite(bfisic, buff_bloc) == -1) {
            return -1;
        }

        bytes_escrits += (TB - primer_byte_logic);
    }

    if ((darrer_bloc_logic - primer_bloc_logic) > 1) {
        int i;
        for (i = primer_bloc_logic+1; i < darrer_bloc_logic; i++) {
            ret = traduirBlocInode(inod, i, &bfisic, 1); // bloc físic
            if (ret == -1) {
                printf("[ficheros.c] ERROR: traduirBlocInode()2\n");
                return -1;
            }

            if (bread(bfisic, buff_bloc) == -1) {
                return -1;
            }

            memcpy(buff_bloc, buff_original + (TB - primer_byte_logic) + (i - primer_bloc_logic - 1) * TB, TB);

            // Escrivim directament TB bytes al bloc
            if (bwrite(bfisic, buff_bloc) == -1) {
                return -1;
            }

           bytes_escrits += TB;
        }
    }

    if ((darrer_bloc_logic - primer_bloc_logic) >= 1) {
        ret = traduirBlocInode(inod, darrer_bloc_logic, &bfisic, 1); // bloc físic
        if (ret == -1) {
            printf("[ficheros.c] ERROR: traduirBlocInode()3\n");
            return -1;
        }

        if (bread(bfisic, buff_bloc) == -1) {
            return -1;
        }

        memcpy(buff_bloc, buff_original + (TB - primer_byte_logic) + (darrer_bloc_logic - primer_bloc_logic - 1) * TB, darrer_byte_logic + 1);

        if (bwrite(bfisic, buff_bloc) == -1) {
            return -1;
        }

        bytes_escrits += darrer_byte_logic + 1;
    }

    in = llegirInode(inod);

    if ((offset + nbytes) > in.tamany) { // controlam el tamany
        in.tamany = offset + nbytes;
    }
    in.data_acces = time(NULL);
    in.data_modificacio = time(NULL);
    if (escriureInode(inod, in) == -1) {
        return -1;
    }

    // retornam els bytes escrits al bloc
    return bytes_escrits;
}

/**
 *  Funció que s'encarrega de llegir x bytes dels blocs corresponents en el sistema de fitxers.
 *  @param  inod posició de l'inode on s'escriuran el blocs corresponents
 *  @param  buff_original buffer que contendrà els bytes llegits
 *  @param  offset desplaçament de x bytes per a escriure en la posició correcta dins el bloc
 *  @param  nbytes nombre de bytes a escriure
 *  @return nombre de bytes llegits al bloc
 */
int mi_read_f (unsigned int inod, void *buff_original, unsigned int offset, unsigned int nbytes)
{
    unsigned char buff_bloc[TB];
    inode in;
    uint bytes_llegits = 0;
    uint primer_bloc_logic = offset / TB; // primer bloc logic
    uint darrer_bloc_logic = (offset + nbytes - 1) / TB; // darrer bloc logic

    uint primer_byte_logic = offset % TB; // desplaçament del primer bloc. On es comença a llegir
    uint darrer_byte_logic = (offset + nbytes - 1) % TB; // nombre de bytes a escriure al primer bloc
    memset(buff_bloc, '\0', TB);
    int ret = 0;
    uint bfisic = 0;

    in = llegirInode(inod);

    if ((in.permisos != 6) && (in.permisos != 7) && (in.permisos != 4) && (in.permisos != 5)) { // si no tiene permisos de lectura
        printf("[ficheros.c] ERROR: L'inode no te permisos de lectura!\n");
        return -1;
    }

    ret = traduirBlocInode(inod, primer_bloc_logic, &bfisic, 0); // bloc físic
    if (ret == -1) {
        printf("[ficheros.c] ERROR: traduirBlocInode()4\n");
        return -1;
    }

    if ((primer_byte_logic + nbytes) < TB) {
        if (bread(bfisic, buff_bloc) == -1) {
            return -1;
        }

        memcpy(buff_original, buff_bloc + primer_byte_logic, nbytes);

        bytes_llegits += nbytes;
    } else {
        if (bread(bfisic, buff_bloc) == -1) {
            return -1;
        }

        memcpy(buff_original, buff_bloc + primer_byte_logic, TB - primer_byte_logic);

        bytes_llegits += (TB - primer_byte_logic);
    }

    if ((darrer_bloc_logic - primer_bloc_logic) > 1) {
        int i;
        for (i = primer_bloc_logic+1; i < darrer_bloc_logic; i++) {
            ret = traduirBlocInode(inod, i, &bfisic, 0); // bloc físic
            if (ret == -1) {
                printf("[ficheros.c] ERROR: traduirBlocInode()5\n");
                return -1;
            }

            if (bread(bfisic, buff_bloc) == -1) {
                return -1;
            }

            memcpy(buff_original + (TB - primer_byte_logic) + (i - primer_bloc_logic - 1) * TB, buff_bloc, TB);
            bytes_llegits += TB;
        }
    }

    // el darrer bloc
    if ((darrer_bloc_logic - primer_bloc_logic) >= 1) {
        ret = traduirBlocInode(inod, darrer_bloc_logic, &bfisic, 0); // bloc físic
        if (ret == -1) {
            printf("[ficheros.c] ERROR: traduirBlocInode()6\n");
            return -1;
        }

        if (bread(bfisic, buff_bloc) == -1) {
            return -1;
        }

        memcpy(buff_original + (TB - primer_byte_logic) + (darrer_bloc_logic - primer_bloc_logic - 1) * TB, buff_bloc, darrer_byte_logic + 1);

        bytes_llegits += (darrer_byte_logic + 1);
    }

    // actualitzam la metainformació de l'inode
    in = llegirInode(inod);
    in.data_acces = time(NULL);
    if (escriureInode(inod, in) == -1) {
        return -1;
    }

    return bytes_llegits;
}

/**
 *  Canvia els permisos d'un fitxer o directori.
 *  @param inod posició del l'inode que s'ha de canviar els permisos.
 *  @param mode els nous permisos que s'han d'establir a l'inode.
 */
int mi_chmod_f (unsigned int inod, unsigned int mode)
{
    inode in;

    in = llegirInode(inod);

    in.permisos = mode;
    in.data_modificacio = time(NULL);
    in.data_acces = time(NULL);

    if (escriureInode(inod, in) == -1) {
        return -1;
    }

    return 0;
}

/**
 *  Trunca o allibera un fitxer o directori els bytes indicats, alliberant els
 *  blocs necessaris. Si se trunca a 0 bytes s'han d'alliberar tots els blocs.
 *  @param inod posicio del inode que s'ha de truncar
 *  @param nbytes nombre de bytes que s'han de truncar
 */
int mi_truncar_f(unsigned int inod, unsigned int nbytes)
{
    int ret;
    uint bfisic;
    inode in = llegirInode(inod);

    if (nbytes > in.tamany) {
        printf("[ficheros.c] ERROR: No se pot truncar un fitxer més gran que el seu tamany!\n");
        return -1;
    }

    if (nbytes == 0) { // "vaciar" el fichero a 0 bytes
        alliberarInode(inod);

        in.tamany = nbytes;

        if (escriureInode(inod, in) == -1) {
            return -1;
        }
    } else if (nbytes > 0) { // eliminar los bloques correspondientes
        int darrer_bloc = in.tamany / TB; // ultimo bloque a truncar
        if ((in.tamany % TB) == 0) {
            darrer_bloc -= 1;
        }

        int bloc_conservar = nbytes / TB; // ultimo bloque a conservar
        if (nbytes % TB == 0) {
            bloc_conservar -= 1; // si es exacto
        }

        int i = 0;
        for (i = bloc_conservar + 1; i <= darrer_bloc; i++) { // recorremos los bloques que queremos liberar
            ret = traduirBlocInode(inod, i, &bfisic, 0); // bloc físic
            if (ret == -1) {
                printf("[ficheros.c] ERROR: traduirBlocInode()7\n");
                return -1;
            }
            if (bfisic > 0) {
                if (alliberarBloc(bfisic) == -1) { // alliberam el bloc
                    printf("[ficheros.c] ERROR: No s'ha pogut alliberar el bloc!\n");
                    return -1;
                }

                in.blocs_assignats_dades--;
            }

            if ((i == darrer_bloc) && (in.tamany % TB != 0)) { // si es la ultima vez se trunca un trozo de bloque
                in.tamany -= in.tamany % TB;
            } else {
                in.tamany -= TB;
            }
        }

        in.tamany = nbytes;
        if (escriureInode(inod, in) == -1) {
            return -1;
        }
    }
    return 0;
}

/**
 *  Retorna la metainformació d'un fitxer o directori. STAT es l'estructura que
 *  conté la informació de l'inode.
 *  @param inod posició del inode
 *  @param p_stat punter a l'estructura STAT que conté informació sobre l'inode
 */
int mi_stat_f (unsigned int inod, STAT *p_stat)
{
    inode in;

    in = llegirInode(inod);

    p_stat->tipus = in.tipus;
    p_stat->permisos = in.permisos;
    p_stat->tamany = in.tamany;
    p_stat->data_creacio = in.data_creacio;
    p_stat->data_modificacio = in.data_modificacio;
    p_stat->data_acces = in.data_acces;
    p_stat->blocs_assignats_dades = in.blocs_assignats_dades;
    p_stat->links_directoris = in.links_directoris;

    return 0;
}

/**
* Mostra l'estat de l'inode per pantalla
* @param p_stat punter a l'estructura STAT que conté informació sobre l'inode
*/
void veure_estat (STAT *p_stat)
{
    char *tipus = NULL;

    if (p_stat->tipus == 1) {
        tipus = "(d)";
    } else if (p_stat->tipus == 2) {
        tipus = "(f)";
    }
    printf("Tipus: %d %s\n", p_stat->tipus, tipus);
    printf("Permisos: %d\n", p_stat->permisos);
    printf("Tamany: %d\n", p_stat->tamany);

    struct tm *creacio = localtime(&p_stat->data_creacio);
    struct tm *modificacio = localtime(&p_stat->data_modificacio);
    struct tm *acces = localtime(&p_stat->data_acces);

    char *c = asctime(creacio);
    char *m = asctime(modificacio);
    char *a = asctime(acces);

    printf("Data creació: %s", c);
    printf("Data modificació: %s", m);
    printf("Data accés: %s", a);
    printf("Blocs assignats a dades: %d\n", p_stat->blocs_assignats_dades);
    printf("Enllaços a directoris: %d\n", p_stat->links_directoris);
}
