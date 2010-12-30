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
 *  @brief Conté els funcions de més alt nivell que interactuen amb el sistema de fitxers.
 *  S'implementen les funcions necessàries per a llegir i escriure en el sistema de fitxers.
 *  @date 15/11/2010
 */

#include "ficheros.h"

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
    int bytes_escrits = 0;
    int blocLogic = offset / TB; // primer bloc (logic) on escriurem
                        printf("!!!!!!!!!!!!!!! blocLogic %d\n",blocLogic);
    int blocFisic;
    int darrer_bloc_logic = ((offset + nbytes) - 1) / TB; //ultim bloc on escriuremdesplacament_primer_bloc
    //int darrer_byte = (offset + nbytes - 1); // darrer byte on escriurem
    int desplacament_primer_bloc = offset % TB; // Offset del primer bloc. Punt d'inici del primer bloc
    int bytes_lliures_primer_bloc = TB - desplacament_primer_bloc; // Numero de bytes a escriure en el primer bloc
    //int primer_bloc = blocLogic;
    int bytes_darrer_bloc = (nbytes - bytes_lliures_primer_bloc) % TB; // Numero de bytes a escriure en el darrer bloc
    int bytes_per_escriure = nbytes; //Bytes que faltan per escriure
    in = llegirInode(inod);
    //printf("[ficheros.c - mi_write_f] DEBUG: bytes_a_escribir %d\n",nbytes);

    if ((in.permisos != 6) && (in.permisos != 7)) {
        printf("[ficheros.c] ERROR: L'inode no te permisos d'escriptura!\n");
        return -1;
    }
    //printf ("[ficheros.c] DEBUG: mi_write_f blogLogic: %d | offset: %d | TB: %d\n",blocLogic, offset, TB);

    blocFisic = traduirBlocInode(inod, blocLogic, '1'); // bloc físic on escriurem

    //printf("[ficheros.c] DEBUG: blocFisic 0 = %d\n", blocFisic);

   // printf("!!![ficheros.c] DEBUG: bytes_darrer_bloc %d | bytes_lliures_primer_bloc: %d | darrer_bloc_logic: %d\n", bytes_darrer_bloc, bytes_lliures_primer_bloc, darrer_bloc_logic);
    memset(buff_bloc, '\0', TB);
    if (blocFisic <= 0) {
        printf("[ficheros.c] ERROR: Bloc físic incorrecte\n");
        return -1;
    }

    in = llegirInode(inod);

    if (bread(blocFisic, buff_bloc) == -1) {
        return -1;
    }

    // únicament tenim que escriure en un bloc
    if (nbytes <= bytes_lliures_primer_bloc) {
        //if (bread(blocFisic, buff_bloc) == -1) {
        //    return -1;
        //}
        //memcpy(buff_bloc + desplacament_primer_bloc, buff_original, TB - desplacament_primer_bloc);
        memcpy(&buff_bloc[desplacament_primer_bloc], buff_original, nbytes);
        //memcpy(buff_bloc + desplacament_primer_bloc, buff_original, TB - nbytes);

        if (bwrite(blocFisic, buff_bloc) == -1) {
            return -1;
        }

        bytes_escrits = nbytes;

        printf("[ficheros.c] DEBUG: bytes_escrits [primer_bloc_unicament] %d\n",bytes_escrits);

        // actualitzam les dades de l'inode
        in.tamany = in.tamany + bytes_escrits;
        in.data_acces = time(NULL);

        escriureInode(inod, in);

    } else { // cas en que escrivim en més d'un bloc
        //if (bread(blocFisic, buff_bloc) == -1) {
        //    return -1;
        //}

        //memcpy(buff_bloc + desplacament_primer_bloc, buff_original, bytes_lliures_primer_bloc);
        memcpy(&buff_bloc[desplacament_primer_bloc], buff_original, TB - desplacament_primer_bloc);
        //printf("ficheros.c] DEBUG: cas intermitjo - despues del 1º memcpy\n");


        if (bwrite(blocFisic, buff_bloc) == -1) {
            return -1;
        }
        bytes_escrits = bytes_lliures_primer_bloc;
        bytes_per_escriure -= bytes_escrits;
        printf ("\n[ficheros.c] CAS DIRECTE - DEBUG: ESCRIBIM bytes_escritos: %d en el blogLogic %d blocFisic %d| faltan: %d bytes por escribir\n", bytes_escrits,blocLogic,blocFisic,bytes_per_escriure);
        blocLogic++;
        int i;

        for (i = blocLogic; i < darrer_bloc_logic; i++) { //el problema era <= , debia ser <, sino escribia un bloque intermedio mas

            //printf("ficheros.c] DEBUG: cas intermitjo - antes de traduir inode\n");
            blocFisic = traduirBlocInode(inod, i, '1');
            printf("ficheros.c] DEBUG: cas intermitjo - despues de traduir inode\n");

            if (blocFisic <= 0) {
                printf("[ficheros.c] ERROR: Bloc físic incorrecte\n");
                return -1;
            }
            //printf("ficheros.c] DEBUG: cas intermitjo - antes del memcpy \n");
            //memcpy(buff_bloc, buff_original + (TB - desplacament_primer_bloc) + (i - primer_bloc - 1) * TB, TB); //AQUI FALLO SEGMENTACION
            memcpy(&buff_bloc, buff_original + bytes_escrits, TB);
            //memcpy(buff_bloc, &buff_original[(TB - desplacament_primer_bloc) + (i - primer_bloc - 1) * TB], TB); //AQUI FALLO SEGMENTACION
            //printf("ficheros.c] DEBUG: cas intermitjo - despues del memcpy\n");

            // Escrivim directament TB bytes al bloc
            if (bwrite(blocFisic, buff_bloc) == -1) {
                return -1;
            }
           bytes_escrits += TB;
           bytes_per_escriure -= TB; //bytes que faltan por escribir
           printf ("\n[ficheros.c] CAS INTERMITJ - DEBUG: ESCRIBIM bytes_escritos: %d en el blogLogic %d blocFisic %d | faltan: %d bytes por escribir\n", bytes_escrits, blocLogic,blocFisic,bytes_per_escriure);
        }
        blocLogic = i;
        printf("[ficheros.c] DEBUG: salimos de los casos intermedios | blocLogic : %d \n", blocLogic);

        // cas darrer bloc
        if (bytes_per_escriure > 0) {

            blocFisic = traduirBlocInode(inod, blocLogic, '1');

            if (blocFisic <= 0) {
                printf("[ficheros.c] ERROR: Bloc físic incorrecte\n");
                return -1;
            }

            if (bread(blocFisic, buff_bloc) == -1) {
                return -1;
            }

            memcpy(&buff_bloc, buff_original + bytes_escrits, bytes_per_escriure);
            //memcpy(buff_bloc, buff_original + (TB - desplacament_primer_bloc) + (darrer_bloc_logic - primer_bloc - 1) * TB, bytes_per_escriure);
            //memcpy(buff_bloc, &buff_original[(TB - desplacament_primer_bloc) + (darrer_bloc_logic - primer_bloc - 1) * TB], bytes_per_escriure);
            // memcpy(buff_bloc, buff_original + (TB - desplacament_primer_bloc) + (darrer_bloc_logic - primer_bloc - 1) * TB, (darrer_byte % TB) + 1);
            //memcpy(buff_bloc, buff_original + bytes_escrits, bytes_darrer_bloc);

            if (bwrite(blocFisic, buff_bloc) == -1) {
                return -1;
            }

            bytes_escrits += bytes_per_escriure; //sumamos los bytes que faltaban por escribir al total hasta ahora.

            printf ("\n[ficheros.c] DARRER CAS - DEBUG: ESCRIBIM bytes_escritos: %d en el blogLogic %d \n", bytes_per_escriure, blocLogic);

        }
        in = llegirInode(inod);

        in.tamany = in.tamany + bytes_escrits;
        in.data_acces = time(NULL);

        escriureInode(inod, in);
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
    int bytes_llegits = 0;
    int blocLogic = offset / TB; // primer bloc logic
    int blocFisic;
    int darrer_bloc_logic = ((offset + nbytes) - 1 )/ TB; // darrer bloc logic
   // int darrer_byte = (offset + nbytes) - 1;
    int desplacament_primer_bloc = offset % TB; // desplaçament del primer bloc. On es comença a llegir
    int bytes_lliures_primer_bloc = TB - desplacament_primer_bloc; // nombre de bytes a escriure al primer bloc
    //int blocs_intermitjos = (nbytes - bytes_lliures_primer_bloc) / TB;
    //int primer_bloc = blocLogic;
    int bytes_darrer_bloc = (nbytes - bytes_lliures_primer_bloc) % TB;
    int bytes_per_llegits = nbytes; //Bytes que faltan per escriure
    in = llegirInode(inod);
    
    printf("[ficheros.c] - mi_read_f - inod : %d - in.permisos : %d \n\n", inod, in.permisos);

    if ((in.permisos != 6) && (in.permisos != 7)) { // si no tiene permisos de lectura o escritura 
        printf("[ficheros.c] ERROR: L'inode no te permisos de lectura!\n");
        return -1;
    }

    blocFisic = traduirBlocInode(inod, blocLogic, '0');
	 printf("[ficheros.c] blocFisic:%d\n",blocFisic);
    if (blocFisic <= 0) {
        printf("[ficheros.c] ERROR: Bloc físic incorrecte\n");
        return -1;
    }

    if (nbytes <= bytes_lliures_primer_bloc) { // només llegim un bloc
        if (bread(blocFisic, buff_bloc) == -1) {
            return -1;
        }

        memcpy(buff_original, &buff_bloc[desplacament_primer_bloc], nbytes);

        bytes_llegits = nbytes;
        bytes_per_llegits -= bytes_llegits; //Aqui no fa falta

        //printf("[ficheros.c] DEBUG: blocFisic = %d\n", blocFisic);
        //printf("[ficheros.c] DEBUG: tamany buff_original = %lu\n", sizeof(buff_original)); // en bytes
       // printf ("[ficheros.c]  cas directe - DEBUG: bytes_leidos: %d\n", bytes_llegits);

    } else { // tenim més d'un bloc a llegir
        if (bread(blocFisic, buff_bloc) == -1) {
            return -1;
        }

        memcpy(buff_original, &buff_bloc[desplacament_primer_bloc], bytes_lliures_primer_bloc);
        //memcpy(buff_original, &buff_bloc[desplacament_primer_bloc], TB - desplacament_primer_bloc);
        bytes_llegits = bytes_lliures_primer_bloc;
        bytes_per_llegits -= bytes_llegits;
        printf("\n[ficheros.c] CAS INTERMITJ - DEBUG: LLEGIM bytes_llegits: %d en el bloque logico %d| faltan: %d bytes por llegir\n", bytes_llegits,blocLogic,bytes_per_llegits);

        blocLogic++;
        int i;
        for (i = blocLogic; i < darrer_bloc_logic; i++) {

            printf ("\n[ficheros.c] CAS INTERMITJ - DEBUG blocLogic %d dentro del for\n",i);

            blocFisic = traduirBlocInode(inod, i, '0');

            if (blocFisic <= 0) {
                printf("[ficheros.c] ERROR: Bloc físic incorrecte\n");
                return -1;
            }
            
            if (bread(blocFisic, buff_bloc) == -1) {
                return -1;
            }

            //memcpy(buff_original + (TB - darrer_byte) + (i - primer_bloc - 1) * TB, buff_bloc, TB);
            memcpy(buff_original + bytes_per_llegits, &buff_bloc, TB);

            //memcpy(buff_original + (TB - (offset % TB)) + (i - primer_bloc - 1) * TB, &buff_bloc, TB);
            //memcpy(buff_original + bytes_llegits, &buff_bloc, TB);

            bytes_llegits += TB;
            bytes_per_llegits -= TB; //bytes que faltan por escribir
            printf ("\n[ficheros.c] CAS INTERMITJ - DEBUG: LLEGIM bytes_llegits: %d en el bloque logico %d| faltan: %d bytes por llegir\n", bytes_llegits,i,bytes_per_llegits);

        }

        // el darrer bloc
        if (bytes_per_llegits > 0) {
            printf("[ficheros.c]  bytes_darrer_bloc: %d",  bytes_darrer_bloc);
            blocLogic++;

            blocFisic = traduirBlocInode(inod, blocLogic, '0');

            if (blocFisic <= 0) {
                printf("[ficheros.c] ERROR: Bloc físic incorrecte\n");
                return -1;
            }

            if (bread(blocFisic, buff_bloc) == -1) {
                return -1;
            }

            //memcpy(buff_original + desplacament_primer_bloc + (blocLogic - primer_bloc - 1) * TB, buff_bloc, bytes_darrer_bloc);
            memcpy(buff_original + bytes_per_llegits, &buff_bloc, bytes_per_llegits);
            //memcpy(buff_original + bytes_llegits, &buff_bloc, bytes_darrer_bloc);

            bytes_llegits += bytes_per_llegits;
            printf ("\n[ficheros.c] DARRER CAS - DEBUG: LLEIM bytes_llegits: %d \n", bytes_per_llegits);

            //printf("[ficheros.c] DEBUG: tamany buff_original = %lu\n", sizeof(buff_original)); // en bytes
        }
    }
    // actualitzam la metainformació de l'inode
    in = llegirInode(inod);

    in.data_acces = time(NULL);
    escriureInode(inod, in);

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
int mi_truncar_f (unsigned int inod, unsigned int nbytes)
{
    inode in = llegirInode(inod);
    int max_punters = MAX_PUNTERS_DIRECTES + N_PUNTERS_BLOC + (N_PUNTERS_BLOC * N_PUNTERS_BLOC) +
                (N_PUNTERS_BLOC * N_PUNTERS_BLOC * N_PUNTERS_BLOC); // 16.843.020
    int blocs_conservar = 0;
    int blocs_llegits = 0;

    if (nbytes > in.tamany) {
        printf("[ficheros.c] ERROR: No se pot truncar un fitxer més gran que el seu tamany\n");
        return -1;
    }

    if (nbytes == 0) {
        alliberarInode(inod, 0, 0);

        in = llegirInode(inod);
        //printf("[ficheros.c] DEBUG: tamany inode abans %d\n", in.tamany);
        in.tamany = nbytes;
        //printf("[ficheros.c] DEBUG: tamany inode despres %d\n", in.tamany);

        escriureInode(inod, in);
    } else {
        blocs_conservar = (nbytes / TB) + 1;

        int i, bf;
        for (i = 0; i < max_punters; i++) {
            bf = traduirBlocInode(inod, i, '0');

            if (bf > 0) {
                blocs_llegits++;
            }

            if (blocs_llegits == blocs_conservar) {
                alliberarInode(inod, blocs_conservar, 0);
                //in = llegirInode(inod);
                //printf("[ficheros.c] DEBUG: tamany inode abans %d\n", in.tamany);
                in.tamany = nbytes;
                //printf("[ficheros.c] DEBUG: tamany inode despres %d\n", in.tamany);
                escriureInode(inod, in);
            }
        }
    }
    return 0;
}

/**
 *  Retorna la metainformació d'un fitxer o directori. STAT es l'estructura que
 *  conté la informació de l'inode.
 *  @param inod posició del inode que s'ha de truncar
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
