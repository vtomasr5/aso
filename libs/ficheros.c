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
 *  S'implementes les funcions necessàries per a llegir i escriure en el sistema de fitxers.
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
    int blocFisic;
    int darrer_bloc_logic = (offset + nbytes) - 1;
    darrer_bloc_logic = darrer_bloc_logic / TB; // darrer bloc (logic) on escriurem
    int desplacament_primer_bloc = offset % TB; // Offset del primer bloc. Punt d'inici del primer bloc
    int bytes_lliures_primer_bloc = TB - desplacament_primer_bloc; // Numero de bytes a escriure en el primer bloc
    //int blocs_intermitjos = (nbytes - bytes_lliures_primer_bloc) / TB; // quantitat de bytes a escriure en els blocs intermitjos
    int darrer_bloc_intermitj = (offset+nbytes-1)/TB;//ultimo bloque donde escribi
    //int bytes_darrer_bloc = (nbytes - bytes_lliures_primer_bloc) % TB; // quantitat de bytes a escriure en el darrer bloc
    int bytes_darrer_bloc = (nbytes - (TB - (offset % TB))) % TB; // quantitat de bytes a escriure en el darrer bloc
    //int c = 1;
    //int primer_bloc = blocLogic; // para el memcpy version 2,
    
    in = llegirInode(inod);
	printf("[ficheros.c - mi_write_f] bytes_a_escribir %d\n",nbytes);
    if (in.permisos != 'w') {
        printf("[ficheros.c] ERROR: L'inode no te permisos d'escriptura!\n");
        return -1;
    }
	printf ("[ficheros.c] mi_write_f blogLogic: %d | offset: %d | TB: %d\n",blocLogic, offset, TB);
    blocFisic = traduirBlocInode(inod, blocLogic, '1'); // bloc físic on escriurem
    in = llegirInode(inod);
    
/*
    int k;
	for (k = 0; k < MAX_PUNTERS_DIRECTES; k++) {
		printf("[ficheros.c - mi_write_f] DEBUG: punters directes = %d\n", in.pdirectes[k]);
	}
*/
    /*
    if (bread(blocFisic, buff_bloc) == -1) {
       return -1;
    }

    int j;
    for (j = 0; j < TB; j++) {
        buff_bloc[j] = 9;

    }

    if (bwrite(blocFisic, buff_bloc) == -1) {
       return -1;
    }
*/
    printf("[ficheros.c] DEBUG: blocFisic 0 = %d\n", blocFisic);
	printf("!!![ficheros.c] bytes_darrer_bloc %d  | darrer_bloc_intermitj: %d | bytes_lliures_primer_bloc: %d | darrer_bloc_logic: %d\n", bytes_darrer_bloc,darrer_bloc_intermitj,bytes_lliures_primer_bloc,darrer_bloc_logic);
  
    if (blocFisic <= 0) {
        printf("[ficheros.c] ERROR: Bloc físic incorrecte\n");
        return -1;
    }
  // únicament tenim que escriure en un bloc
    if (nbytes <= bytes_lliures_primer_bloc) {
        if (bread(blocFisic, buff_bloc) == -1) {
            return -1;
        }

        memcpy(&buff_bloc[desplacament_primer_bloc], buff_original, bytes_lliures_primer_bloc);
        //memcpy(buff_bloc + desplacament_primer_bloc, buff_original, nbytes);

        if (bwrite(blocFisic, buff_bloc) == -1) {
            return -1;
        }

        bytes_escrits = nbytes;
        printf("[ficheros.c] DEBUG: bytes_escrits [primer_bloc_unicament] %d\n",bytes_escrits);
        
        // actualitzam les dades de l'inode
        in.tamany = in.tamany + bytes_escrits;
        in.data_acces = time(NULL);
        escriureInode(inod, in);

        // return bytes_escrits;
    } else if (nbytes > bytes_lliures_primer_bloc) { // cas en que escrivim en més d'un bloc
        blocLogic++;
        if (bread(blocFisic, buff_bloc) == -1) {
            return -1;
        }
        bytes_escrits = TB - desplacament_primer_bloc;
        //memcpy(buff_bloc + desplacament_primer_bloc, &buff_original, bytes_escrits);
        memcpy(&buff_bloc[desplacament_primer_bloc], buff_original, bytes_lliures_primer_bloc);
        //memcpy(&buff_bloc[desplacament_primer_bloc], buff_original, TB);

        printf ("[ficheros.c] cas directe - DEBUG: bytes_escritos: %d\n", bytes_escrits);
        if (bwrite(blocFisic, buff_bloc) == -1) {
            return -1;
        }

        // cas de que hi hagi blocs intermitjos
        int i;
      	printf ("[ficheros.c] blocLogic %d darrer_bloc_intermitj: %d \n", blocLogic,darrer_bloc_intermitj);
        for (i = blocLogic; i < darrer_bloc_intermitj; i++) { //el problema era <= , debia ser <, sino escribia un bloque intermedio mas
            printf ("[ficheros.c] cas intermitjo - darrer_bloc_intermitj %d  | i: %d\n", darrer_bloc_intermitj,i);
            //memcpy(&buff_bloc, buff_original + ( TB - ( offset % TB ) ) + ( i - primer_bloc - 1 ) * TB, TB);
            
            
            printf("[ficheros.c] antes del memcpy de my_write_f \n");
            // memcpy(buff_bloc, buff_original + ( TB - ( offset % TB ) ) + ( i - primer_bloc - 1 ) * TB, TB);
            memcpy(buff_bloc, buff_original + bytes_escrits, TB);
            printf("[ficheros.c] despues del memcpy de my_write_f \n");
            
            
            blocFisic = traduirBlocInode(inod, i, '1');
     
            if (bwrite(blocFisic, buff_bloc) == -1) {
                return -1;
            }
            
            bytes_escrits += TB;
            printf ("[ficheros.c] cas intermitjo - DEBUG: bytes_escritos: %d\n", bytes_escrits);
        }
        blocLogic = i;
		printf("[ficheros.c] DEBUG: salimos de los casos intermedios | blocLogic : %d \n", blocLogic);
        // cas darrer bloc
        if (bytes_darrer_bloc > 0) {
        	printf("[ficheros.c]  bytes_darrer_bloc: %d\n",  bytes_darrer_bloc);
            blocFisic = traduirBlocInode(inod, blocLogic, '1');
            
            if (bread(blocFisic, buff_bloc) == -1) {
                return -1;
            }

            //memcpy(&buff_bloc, buff_original + desplacament_primer_bloc + (blocLogic - primer_bloc - 1) * TB, bytes_darrer_bloc);
            memcpy(buff_bloc, buff_original + bytes_escrits, bytes_darrer_bloc);

            if (bwrite(blocFisic, buff_bloc) == -1) {
                return -1;
            }
            
            bytes_escrits += bytes_darrer_bloc;
            printf ("[ficheros.c] cas darrer - DEBUG: bytes_escritos: %d\n", bytes_escrits);
        }

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
    int darrer_bloc_logic = (offset + nbytes) - 1; // darrer bloc logic
    darrer_bloc_logic = darrer_bloc_logic / TB;
    int desplacament_primer_bloc = offset % TB; // desplaçament del primer bloc. On es comença a llegir
    int bytes_lliures_primer_bloc = TB - desplacament_primer_bloc; // nombre de bytes a escriure al primer bloc
    int blocs_intermitjos = (nbytes - bytes_lliures_primer_bloc) / TB;
    int bytes_darrer_bloc = (nbytes - bytes_lliures_primer_bloc) % TB;
    
    in = llegirInode(inod);
/*
   if (in.permisos != 'r') {
       printf("[ficheros.c] ERROR: L'inode no te permisos de lectura!\n");
        return -1;
    }
*/
    blocFisic = traduirBlocInode(inod, blocLogic, '0'); //Aqui esta el error

    if (blocFisic <= 0) {

        printf("[ficheros.c] ERROR: Bloc físic incorrecte\n");
        return -1;
    }

    if (nbytes <= bytes_lliures_primer_bloc) { // només llegim 1 bloc
        if (bread(blocFisic, buff_bloc) == -1) {
            return -1;
        }

        memcpy(buff_original, &buff_bloc[desplacament_primer_bloc], nbytes);

        bytes_llegits = nbytes;
        printf("[ficheros.c] DEBUG: blocFisic = %d\n", blocFisic);
        printf("[ficheros.c] DEBUG: tamany buff_original = %lu\n", sizeof(buff_original)); // en bytes
        printf ("[ficheros.c]  cas directe - DEBUG: bytes_leidos: %d\n", bytes_llegits);

        // return bytes_llegits;
    } else if (nbytes > bytes_lliures_primer_bloc) { // tenim més d'un bloc a llegir
        if (bread(blocFisic, buff_bloc) == -1) {
            return -1;
        }

        memcpy(buff_original, &buff_bloc[desplacament_primer_bloc], bytes_lliures_primer_bloc);
        bytes_llegits = bytes_lliures_primer_bloc;
        printf("[ficheros.c] DEBUG: bytes llegits = %d\n", bytes_llegits);

        // hi ha blocs intermitjos
        int i;
        for (i = 0; i < blocs_intermitjos; i++) {
            blocLogic++;
            printf("[ficheros.c - mi_read_f ] cas intermitjos - se ejecuta traduirBlocInod ****\n");
            blocFisic = traduirBlocInode(inod, blocLogic, '0');
			
            if (blocFisic <= 0) {
                printf("[ficheros.c] ERROR: Bloc físic incorrecte\n");
                return -1;
            }

            if (bread(blocFisic, buff_bloc) == -1) {
                return -1;
            }

            //memcpy(buff_original + ( TB - ( offset % TB ) ) + ( i - primer_bloc - 1 ) * TB, &buff_bloc, TB);
            memcpy(buff_original + bytes_llegits, &buff_bloc, TB);
            bytes_llegits += TB;
            printf ("[ficheros.c]  cas intermitjo - DEBUG: bytes_leidos: %d\n", bytes_llegits);
        }
		printf("[ficheros.c] salimos de los casos intermedios \n");
        // el darrer bloc
        if (bytes_darrer_bloc > 0) {
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

            //memcpy(buff_original + desplacament_primer_bloc + (blocLogic - primer_bloc - 1) * TB, &buff_bloc, bytes_darrer_bloc);
            memcpy(buff_original + bytes_llegits, &buff_bloc, bytes_darrer_bloc);
            bytes_llegits += bytes_darrer_bloc;
            printf("[ficheros.c] DEBUG: tamany buff_original = %lu\n", sizeof(buff_original)); // en bytes
            printf ("[ficheros.c]  cas darrer - DEBUG: bytes_leidos: %d\n", bytes_llegits);
        }
    }
    return bytes_llegits;
}

/**
 *  Canvia els permisos d'un fitxer o directori.
 *  @param inod posició del l'inode que s'ha de canviar els permisos.
 *  @param mode els nous permisos que s'han d'establir a l'inode.
 */
int mi_chmod_f (unsigned int inod, unsigned short int mode)
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
    inode in;
    
    in = llegirInode(inod);
    
    if (nbytes > in.tamany) {
        printf("[ficheros.c] ERROR: No se pot truncar un fitxer més gran que el seu tamany\n");
        return -1;
    }
    
    if (nbytes == 0) {
        alliberarInode(inod, 0, 0);
        
        in = llegirInode(inod);
        printf("[ficheros.c] DEBUG: tamany inode abans %d\n", in.tamany);
        in.tamany = nbytes;
        printf("[ficheros.c] DEBUG: tamany inode despres %d\n", in.tamany);
        
        escriureInode(inod, in);
    } else {
        
    }
    
    return 0;
}

/**
 *  Retorna la metainformació d'un fitxer o directori. STAT es l'estructura que 
 *  conté la informació adient.
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
