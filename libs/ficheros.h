//      ficheros.h
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
 *  @file ficheros.h
 *  @brief Conté les estructures i capçaleres de les funcions de més alt nivell que
 *  s'implementen en el fitxer ficheros.c
 *  @date 11/10/2010
 */

#include "definicions.h"
#include "ficheros_basico.h"
#include "bloques.h"
#include <time.h>

/**
 *  Estructura que conté la informació d'estat sobre els inodes dels fitxers i directoris
 */
typedef struct {
    unsigned char tipus;            // tipus inode (0 = Lliure, 1 = Directori, 2 = Fitxer)
    unsigned int permisos;         // permisos (lectura(r), escriptura(w) i execucio(x))
    unsigned int tamany;            // tamany de l'inode en Bytes lògics
    time_t data_creacio;            // data de creació
    time_t data_modificacio;        // data de modificació
    time_t data_acces;              // data del darrer accés
    //unsigned int blocs_lliures;     // nombre de blocs lliures
    unsigned int blocs_assignats_dades;   // nombre de blocs físics assignats a la zona de dades
    unsigned int links_directoris;
} STAT;

int mi_write_f (unsigned int inod, const void *buff_original, unsigned int offset, unsigned int nbytes);
int mi_read_f (unsigned int inod, void *buff_original, unsigned int offset, unsigned int nbytes);
int mi_chmod_f (unsigned int inod, unsigned int mode);
int mi_truncar_f (unsigned int inod, unsigned int nbytes);
int mi_stat_f (unsigned int inod, STAT *p_stat);
int veure_stat (STAT *p_stat);
