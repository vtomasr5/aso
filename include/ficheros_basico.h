//      ficheros_basico.h
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
 *  @file ficheros_basico.h
 *  @brief Conté les capçaleres necessàries per al correcte funcionament del programa
 *  sobre el tractament bàsic dels fitxers.
 *  @date 11/10/2010
 */

#if !defined(_FICHEROS_BASICO_H)
#define _FICHEROS_BASICO_H

#include <time.h>
#include <stdio.h>
#include <string.h>

#include "definicions.h"
#include "bloques.h"

/**
 * Estructura que contempla tots els camps necessaris per a la contrucció d'un superbloc
 * en el sistema de fitxers.
 * Ocupa 48 bytes
 */
typedef struct {
    unsigned int primerbloc_mb;    // Nombre del primer bloc del mapa de bits
    unsigned int darrerbloc_mb;    // Nombre del darrer bloc del mapa de bits
    unsigned int primerbloc_ai;    // Nombre del primer bloc de l'array de inodes
    unsigned int darrerbloc_ai;    // Nombre del darrer bloc de l'array de inodes
    unsigned int primerbloc_dades; // Nombre del primer bloc de la zona de dades
    unsigned int darrerbloc_dades; // Nombre del darrer bloc de la zona de dades
    unsigned int inode_arrel;      // Nombre de l'inode del directori arrel
    unsigned int inode_lliure;     // Nombre del primer inode lliure
    unsigned int blocs_lliures;    // Nombre de blocs lliures
    unsigned int inodes_lliures;   // Nombre de inodes lliures
    unsigned int total_blocs;      // Nombre de blocs totals
    unsigned int total_inodes;     // Nombre de inodes totals
    char resta_superbloc[TB-(4*12)];     // Resta del superbloc per a que ocupi 1024 bytes (padding)
} superbloc;

/**
 * Estructura que conté els camps necessaris per a contruir un inode del sistema de fitxers.
 * Ocupa 128 bytes
 */
typedef struct {
    unsigned int tipus;            // tipus inode (0 = Lliure, 1 = Directori, 2 = Fitxer)
    unsigned int permisos;         // permisos (lectura(r), escriptura(w) i execucio(x))
    unsigned int tamany;            // tamany de l'inode en Bytes lògics
    time_t data_creacio;            // data de creació
    time_t data_modificacio;        // data de modificació
    time_t data_acces;              // data del darrer accés
    unsigned int blocs_assignats_dades;   // nombre de blocs físics assignats a la zona de dades
    unsigned int links_directoris;
    unsigned int pdirectes[MAX_PUNTERS_DIRECTES];       // punters a blocs directes
    unsigned int pindirectes[MAX_PUNTERS_INDIRECTES];   // punters a blocs indirectes
    char resta_inode[32]; // padding
} inode;

int tamMB(unsigned int);   // tamany del mapa de bits
int tamAI(unsigned int);   // tamany del l'array de i-nodes
int initSB(unsigned int);  // inicialització del superbloc
int initMB();  // inicialització mapa de bits
int initAI();  // inicialització array d'inodes
int infoSB();
int escriureBit(int, int);
int llegirBit(int);
int reservarBloc();
int alliberarBloc(int);
int escriureInode(int, inode);
inode llegirInode(int);
int reservarInode(unsigned int, unsigned int);
int alliberarBlocInode(inode);
int alliberarInode(int, int);
int traduirBlocInode(unsigned int inod, unsigned int blocLogic, char reservar);
int contingutInode(int inod);

#endif // _FICHEROS_BASICO_H
