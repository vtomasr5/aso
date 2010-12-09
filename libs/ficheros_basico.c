//      ficheros_basico.c
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
 *  @file ficheros_basico.c
 *  @brief Conté els funcions de baix nivell que tracten directament amb el sistema de fitxers.\n
 *  En aquest nivell es poden realitzar operacions que permenten inicialitzar l'esctructura
 *  del sistema de fixers, tal com la creació i posterior inicialitzacio del superbloc utilitzant
 *  l'estructura amb la informació adequada, creació i inicialització del mapa de bits otorgant
 *  els valors corresponents d'acord amb els blocs ocupats en el sistema de fitxers (0 si està lliure
 *  i 1 si està ocupat) i finalment l'array de inodes que emmagatzemarà la informació dels inodes
 *  de tal manera que sigui fàcilment accessible en els blocs de dades.\n
 *  Aquesta biblioteca també s'encarrega de escriure un bit o llegir un bit determinat
 *  en el mapa de bits així de les operacions equivalents en l'array de Inodes.\n
 *  A més, gestiona els blocs del disc virtual, reservant o alliberant blocs segons convengui, i
 *  s'encarrega de la relació entre els blocs i els inodes utilitzant la funció traduirBlocInode().
 *  @date 11/10/2010
 */

#include "ficheros_basico.h"

/**
 *  Funció encarregada de calcular en blocs el tamany de la zona de mapa de bits.
 *  Sabem que hi ha un bit per cada bloc. Cada bloc te TB (1024) posicions per 8 bits
 *  que ocupa cada posició, llavors hi haurà 8192 bits per bloc. Dividim el nombre
 *  de blocs entre 8192 bits/bloc per saber el nombre de blocs que ocupa el mapa de bits.
 *  @param nblocs nombre de blocs
 *  @return tamany del mapa de bits, en blocs
 */
int tamMB(unsigned int nblocs)
{
    int tam_mb;
    int modul = nblocs % (TB * 8);

    // diferenciam el cas de que sigui múltiple o no
    if (modul == 0) {
        tam_mb = nblocs / (TB * 8);
    } else {
        tam_mb = nblocs / (TB * 8);
        tam_mb = tam_mb + 1; // completam afegint un més, encara que malgastem espai
    }
    return tam_mb;
}

/**
 *  Funció que calcula el tamany de l'array de inodes.
 *  Calculam el nombre de inodes que tindrà cada bloc com a màxim (TB/tamany(inode)).
 *  Després calculam la quantitat de inodes que utilitzarà el sistema de fitxers com a màxim (nblocs/N_INODES).
 *  Després dividirem el nombre maxim de inodes per la quantitat màxima de inodes que pot tenir un bloc obtenint
 *  el nombre de blocs que ocupa l'array de inodes.
 *  @param nblocs nombre de inodes
 *  @return el tamany, en blocs, que ocupa l'array de inodes
 */
int tamAI(unsigned int nblocs)
{
    // calculam el nombre de inodes que tindrà cada bloc com a màxim. (TB/tamany_inode)
    int inodesbloc = TB / sizeof(inode); // tamany de l'inode

    // calculam la quantitat màxima de inodes que s'emprarà en el FS
    int tam_ai = nblocs / N_INODES;

    // diferenciam el cas que sigui múltiple o no
    if (tam_ai == 0) {
        tam_ai = tam_ai / inodesbloc;
    } else {
        tam_ai = (tam_ai / inodesbloc) + 1; // completam afegint un més, encara que malgastem espai
    }
    return tam_ai;
}

/**
 *  Funció encarregada de inicialitzar el Superbloc amb uns paràmetres inicials. Finalment
 *  escriu en el superbloc, el bloc 0, el contingut modificat.
 *  @param nblocs nombre de blocs a inicialitzar
 */
int initSB(unsigned int nblocs)
{
    superbloc sb;

    sb.primerbloc_mb = POSICIO_SB + 1; // nombre del primer bloc del mapa de bits
    sb.darrerbloc_mb = tamMB(nblocs); // nombre del darrer bloc del mapa de bits
    sb.primerbloc_ai = sb.darrerbloc_mb + 1; // nombre del primer bloc de l' array de inodes
    sb.darrerbloc_ai = sb.primerbloc_ai + tamAI(nblocs); //nombre del darrer bloc de l' array d' inodes
    sb.primerbloc_dades = sb.darrerbloc_ai + 1; // nombre del primer bloc de la zona de dades
    sb.darrerbloc_dades = nblocs - 1; // nombre del darrer bloc de la zona de dades
    sb.inode_arrel = 0; // nombre de l'inode del directori arrel
    sb.inode_lliure = 0; // nombre del primer inode lliure
    sb.blocs_lliures = nblocs - (sb.darrerbloc_ai + 1); // quantitat de blocs de dades lliures
    sb.inodes_lliures = nblocs/N_INODES; // quantitat de inodes lliures
    sb.total_blocs = nblocs; // quantitat total de blocs
    sb.total_inodes = nblocs/N_INODES; // quantitat total de inodes

    if (bwrite(POSICIO_SB, (char *)&sb) == -1) { // escrivim el SB inicialitzat a la seva posicio
        return -1;
    }
    printf("[ficheros_basico.c] INFO: Superbloc inicialitzat\n");
    return 0;
}

/**
 *  Funció encarregada d'inicialitzar el Mapa de Bits.
 *  S'inicialitzen tots els bits a zero i s'escriuen els canvis en els blocs corresponents.
 *  Llavors es posen a 1 els bits que ja han estat ocupats (SB, MB i AI).
 */
int initMB()
{
    superbloc sb;
    unsigned char mapaBits[TB];
    int i;

    if (bread(POSICIO_SB, (char *)&sb) == -1) { // llegim el superbloc
        return -1;
    }

    // inicialitzam tots els bits del Mapa de Bits a zero
    memset(mapaBits, 0, TB);

    // escrivim les modificacions a tots els blocs
    for (i = sb.primerbloc_mb; i <= sb.darrerbloc_mb; i++) {
        if (bwrite(i, mapaBits) == -1) {
            return -1;
        }
    }

    // posam a 1 els bits ocupats que corresponen al Superbloc, MB i AI
    for (i = POSICIO_SB; i <= sb.darrerbloc_ai; i++) {
        if (escriureBit(i, 1) == -1) {
            return -1;
        }
        sb.blocs_lliures--;
    }

    printf("[ficheros_basico.c] INFO: Mapa de Bits inicialitzat\n");
    return 0;
}

/**
 *  Inicialitza, a la zona de metainformació, l'array de inodes amb la informació
 *  de cada camp actualitzada segons convengui. Els punters directes i indirectes
 *  també s'inicialitzen a "-1" (encara no hi ha punters).
 */
int initAI(uint nblocs)
{
    superbloc sb;
    int inodesbloc = TB / sizeof(inode);
    inode AI[inodesbloc];
    int i, j = 0;
    int tam = tamAI(nblocs);

    if (bread(POSICIO_SB, (char *)&sb) == -1) { // llegim el superbloc
        return -1;
    }

    for (i = 0; i < tam; i++) { // per a cada bloc
        for (j = 0; j < inodesbloc; j++) { // per a cada inode del bloc
            AI[j].tipus = 0;
            AI[j].tamany = 0;
            AI[j].data_creacio = time (NULL);
            AI[j].data_modificacio = time (NULL);
            AI[j].data_acces = time(NULL);
            AI[j].blocs_assignats_dades = 0;
            AI[j].links_directoris = 0;
            AI[j].pdirectes[0] = inodesbloc * i + j + 1;

            int k;
            for (k = 1; k < MAX_PUNTERS_DIRECTES; k++) {
                AI[j].pdirectes[k] = -1;
            }
            for (k = 0; k < MAX_PUNTERS_INDIRECTES; k++) {
                AI[j].pindirectes[k] = -1;
            }
        }
        if (bwrite ((sb.primerbloc_ai + i), AI) == -1) {
            return -1;
        }
    }
    inode arrel = llegirInode(0);
    arrel.tipus = 1;

    return 0;
}

//~ int initAI()
//~ {
    //~ superbloc sb;
    //~ inode inod;
    //~ int i, j;
    //~ unsigned char ArrayInodes[TB];
    //~ int inodesbloc = TB / sizeof(inode); // inodes per bloc
    //~ int inode_actual = 1; // variable que fa referència a l'inode que s'està tractant actualment
    //~ // comença per 1 perque el 0 es el directori arrel i no necessita tenir següent.
//~
    //~ // no hi ha inodes directes emprats
    //~ for (i = 0; i < MAX_PUNTERS_DIRECTES; i++) {
        //~ inod.pdirectes[i] = -1;
    //~ }
//~
    //~ // no hi ha inodes indirectes emprats
    //~ for (i = 0; i < MAX_PUNTERS_INDIRECTES; i++) {
        //~ inod.pindirectes[i] = -1;
    //~ }
//~
    //~ if (bread(POSICIO_SB, (char *)&sb) == -1) { // llegim el superbloc
        //~ return -1;
    //~ }
//~
    //~ // Per a cada bloc de l'AI, anam inicialitzant la informació corresponent
    //~ for (i = sb.primerbloc_ai; i <= sb.darrerbloc_ai; i++) {
        //~ memset(ArrayInodes, 0, TB); // inicialitzam a zero tots els blocs de l'array de inodes
//~
        //~ // per a cada inode inicialitzam els camps corresponents
        //~ for (j = 0; j < inodesbloc; j++) {
            //~ inod.tipus = '0'; // 0 lliure, 1 directori, 2 fitxer
            //~ inod.tamany = 0;
            //~ inod.blocs_assignats_dades = 0;
            //~ inod.data_creacio = time(NULL);
            //~ inod.data_modificacio = time(NULL);
            //~ inod.data_acces = time(NULL);
//~
            //~ // llista de inodes
            //~ if (inode_actual < sb.total_inodes - 1) {
                //~ inod.pdirectes[0] = inode_actual + 1; // apuntam al següent
            //~ } else if (inode_actual == sb.total_inodes - 1) {
                //~ inod.pdirectes[0] = -1; // cas del darrer inode
            //~ }
            //~ memcpy(&ArrayInodes[j * sizeof(inode)], &inod, sizeof(inode)); // guardam els canvis
            //~ inode_actual++; // passam al següent inode
        //~ }
//~
        //~ if (bwrite(i, ArrayInodes) == -1) { // guardam els canvis realitzats en el superbloc
            //~ return -1;
        //~ }
    //~ }
    //~ printf("[ficheros_basico.c] INFO: Array de inodes inicialitzat\n");
    //~ return 0;
//~ }

/**
 *  Mostra la informació que conte el superbloc, per pantalla
 */
int infoSB()
{
    superbloc sb;
    if (bread(POSICIO_SB, (char *)&sb) == -1) { // llegim el superbloc
        return -1;
    }
    printf("\n[ficheros_basico.c] ----- CONTINGUT DEL SUPERBLOC -----\n");
    printf("[ficheros_basico.c] INFO: primer bloc MB = %d\n", sb.primerbloc_mb);
    printf("[ficheros_basico.c] INFO: darrer bloc MB = %d\n", sb.darrerbloc_mb);
    printf("[ficheros_basico.c] INFO: primer bloc AI = %d\n", sb.primerbloc_ai);
    printf("[ficheros_basico.c] INFO: darrer bloc AI = %d\n", sb.darrerbloc_ai);
    printf("[ficheros_basico.c] INFO: primer bloc de dades = %d\n", sb.primerbloc_dades);
    printf("[ficheros_basico.c] INFO: darrer bloc de dades = %d\n", sb.darrerbloc_dades);
    printf("[ficheros_basico.c] INFO: inode arrel = %d\n", sb.inode_arrel);
    printf("[ficheros_basico.c] INFO: primer inode lliure = %d\n", sb.inode_lliure);
    printf("[ficheros_basico.c] INFO: quantitat de inodes lliures = %d\n", sb.inodes_lliures);
    printf("[ficheros_basico.c] INFO: quantitat de blocs lliures = %d\n", sb.blocs_lliures);
    printf("[ficheros_basico.c] INFO: total de blocs =  %d\n", sb.total_blocs);
    printf("[ficheros_basico.c] INFO: total de inodes =  %d\n\n", sb.total_inodes);
    return 0;
}

/**
 *  Funció que escriu un bit a 0 ò a 1 en el Mapa de Bits segons el bloc estigui lliure o ocupat respectivament.
 *  @param bloc bloc on es realitzaran les operacions.
 *  @param bit bit a modificar (0: bloc buit, 1: bloc emprat)
 */
int escriureBit(int bloc, int bit)
{
    superbloc sb;
    unsigned char mapaBits[TB];
    unsigned char valor = 128; // mascara 10000000
    int pos_byte = bloc / 8;
    int pos_bit = bloc % 8;

    if (bread(POSICIO_SB, (char *)&sb) == -1) { // llegim el superbloc
        return -1;
    }

    int bloc_mb = sb.primerbloc_mb + (bloc / (TB*8)); // calculam el bloc on es realitzaran les operacions

    if (bread(bloc_mb, mapaBits) == -1) { // llegim el bloc calculat
        return -1;
    }

    if (pos_bit > 0) {
        valor >>= pos_bit; // desplaçam cap a la dreta "pos_bit posicions"
    }

    if (bit == 0) {
        mapaBits[pos_byte] &= ~valor; // AND (&) i NOT (~) en bits
        //sb.blocs_lliures++; // augmentam el contador de blocs lliures del SB
    } else if (bit == 1) {
        mapaBits[pos_byte] |= valor; // OR (|) en bits
        //sb.blocs_lliures--; // disminuim el contador de blocs lliures del SB
    }

    if (bwrite(bloc_mb, mapaBits) == -1) { // escrivim els canvis
        return -1;
    }

    if (bwrite(POSICIO_SB, (char *)&sb) == -1) {
        return -1;
    }
    return 0;
}

/**
 *  Funció que llegeix un bit en el Mapa de Bits d'un determinat bloc
 *  @param bloc bloc des d'on es vol llegir el bit
 *  @return valor bit llegit
 */
int llegirBit(int bloc)
{
    superbloc sb;
    int bloc_mb; // calculam el bloc on es realitzaran les operacions
    unsigned char mapaBits[TB];
    unsigned char valor = 128; // mascara 10000000
    int pos_byte = bloc / 8;
    int pos_bit = bloc % 8;

    if (bread(POSICIO_SB, (char *)&sb) == -1) { // llegim el superbloc
        return -1;
    }

    memset(mapaBits, 0, TB); // netejam el buffer

    bloc_mb = sb.primerbloc_mb + (bloc / (TB*8));

    if (bread(bloc_mb, mapaBits) == -1) { // llegim el bloc calculat
        return -1;
    }

    if (pos_bit > 0) {
        valor >>= pos_bit; // desplaçam cap a la dreta "pos_bit posicions"
    }

    valor &= mapaBits[pos_byte]; // operador AND para bits
    valor >>= (7-pos_bit); // desplazamiento de bits a la derecha
    //printf("valor = %d\n",valor);

    return valor;
}

/**
 *  Funció reserva el primer bloc lliure que troba.
 *  @return bloc bloc reservat
 */
int reservarBloc()
{
    superbloc sb;
    unsigned int bloc_mb = 0; // bloc on operarem
    int pos_byte = 0;
    unsigned char buff[TB];
    unsigned char mascara = 128; // 1000000
    int i = 0;

    if (bread(POSICIO_SB, (char *)&sb) == -1) { // llegim el superbloc
        return -1;
    }

    if (sb.blocs_lliures == 0) {
        printf("[ficheros_basico.c] ERROR: No hi ha blocs lliures!\n");
        return -1;
    }

    // Calculam el bloc que se situa en el primer bloc de dades del MB, el llegim i l'insertam al buffer
    bloc_mb = (sb.primerbloc_dades / (TB*8)) + 1;
    if (bread(bloc_mb, buff) == -1) {
        return -1;
    }

    pos_byte = bloc_mb / 8;

    // Anam descartant els bytes que siguin tot uns (11111111)
    while (buff[pos_byte] == 255) {
        pos_byte++; // miram el següent
        if ((pos_byte % TB) == 0) { // s'han comprovat tots els bytes del bloc
            bloc_mb++;
            if (bread(bloc_mb, buff) == -1) {
                return -1;
            }
            pos_byte++; // miram el següent
        }
    }
    // Tenim el primer bloc del MB que te un bit a 0 (lliure)
    // Amb sa màscara identificam el primer bit lliure (a 0) i ho reservam
    while (buff[pos_byte] & mascara) {
        buff[pos_byte] <<= 1; // desplaçament a l'esquerra una posició
        i++;
    }

    int bloc = (pos_byte * 8) + i; // numero de bloc a reservar
    if (escriureBit(bloc, 1) == -1) { // escrivim el bit a 1 (el reservam)
        return -1;
    }
    sb.blocs_lliures--; // disminuim els la quantitat de blocs lliures

    if (bwrite(POSICIO_SB, (char *)&sb) == -1) { // guardam els canvis al superbloc
        return -1;
    }
    return bloc;
}

/**
 *  Funció que allibera un bloc de la zona de dades. Posa a 0 la informació del bloc i escriu el bit
 *  amb el valor corresponent (un 0) en el mapa de bits.
 *  @param bloc bloc a alliberar
 */
int alliberarBloc(int bloc)
{
    superbloc sb;
    unsigned char buff[TB];

    if (bread(POSICIO_SB, (char *)&sb) == -1) { // llegim el superbloc
        return -1;
    }

    // El bloc a alliberar ha d'estar a la zona de dades
    if ((bloc >= sb.primerbloc_dades) && (bloc <= sb.darrerbloc_dades)) {
        if (llegirBit(bloc) == 0) { // Comprovam que ja no sigui lliure
            printf("[ficheros_basico.c] ERROR: Aquest bloc ja es lliure!\n");
            return -1;
        } else if (llegirBit(bloc) == 1) {
            memset(buff, 0, TB); // rellenam el bloc amb zeros

            if (bwrite(bloc, buff) == -1) {
                return -1;
            }

            if (escriureBit(bloc, 0) == -1) { // l'alliberam en el mapa de bits
                return -1;
            }
            return 0;
        }
        sb.blocs_lliures++; // augmentam els la quantitat de blocs lliures

        if (bwrite(POSICIO_SB, (char *)&sb) == -1) { // guardam els canvis al superbloc
            return -1;
        }
    } else {
        printf("[ficheros_basixo.c] ERROR: Aquest bloc no pertany a la zona de dades!\n");
        return -1;
    }
    return 0;
}

/**
 *  Escriu en una determinada posició d'un inode, la informació corresponent del inode
 *  @param pos_in posicio a on s'ha d'escriure l'inode
 *  @param inod informació de l'inode que s'ha d'escriure a la posicio 'pos_in'
 */
int escriureInode(int pos_in, inode inod)
{
    superbloc sb;
    unsigned char buff[TB];

    if (bread(POSICIO_SB, (char *)&sb) == -1) { // llegim el superbloc
        return -1;
    }

    int blocAI = sb.primerbloc_ai + (pos_in / (TB/sizeof(inode))); // on esta el bloc que conté l'inode on hem d'escriure
    int desp = (pos_in % (TB / sizeof(inode))); // posicio de l'inode dins el bloc

    memset(buff, 0, TB);

    if (bread(blocAI, buff) == -1) { // llegim el bloc on esta l'inode i el posam al buffer
        return -1;
    }

    inod.data_modificacio = time(NULL); // actualitzam sa data de modificació

    memcpy(&buff[desp * sizeof(inode)], &inod, sizeof(inode)); // guardam els canvis a l'AI

    if (bwrite(blocAI, buff) == -1) {
        return -1;
    }
    return 0;
}

/**
 *  Llegeix d'una determinada posició d'un inode, la informació corresponent de l'inode
 *  @param pos_in posicio d'on s'ha de llegir l'inode
 *  @return inode que s'ha llegit
 */
inode llegirInode(int pos_in)
{
    superbloc sb;
    inode inod;
    unsigned char buff[TB];

    bread(POSICIO_SB, (char *)&sb); // llegim el superbloc

    int blocAI = sb.primerbloc_ai + (pos_in / (TB / sizeof(inode))); // on esta l'inode que hem de llegir
    int desp = pos_in % (TB / sizeof(inode)); // posició que te l'inode dins el bloc (desplaçament dins el bloc)

    memset(buff, 0, TB);
    bread(blocAI, buff); // llegim el bloc on esta l'inode i el posam al buffer

    memcpy(&inod, &buff[desp * sizeof(inode)], sizeof(inode)); // escrivim la informació que hem llegit
    return inod;
}

/**
 *  S'encarrega de trobar el primer inode lliure en el sistema de fitxers i retorna el seu número
 *  @param tipusInode Tipus de l'inode que volem reservar
 *  @param permisosInode Tipus de permisos de l'inode (rwx) amb els que es vol reservar
 *  @return número de l'inode
 */
int reservarInode(int tipusInode, char permisosInode)
{
    superbloc sb;
    inode inod;
    int seg_inode_lliure;
    int inode_lliure;

    if (bread(POSICIO_SB, (char *)&sb) == -1) { // llegim el superbloc
        return -1;
    }

    if (sb.inodes_lliures > 0) { // comprovam que hi ha inodes lliures
        inode_lliure = sb.inode_lliure; // reservam el primer lliure

        inod = llegirInode(inode_lliure); // llegim l'inode

        inod.data_modificacio = time(NULL); // actualitzam la data
        inod.data_acces = time(NULL);
        inod.tipus = tipusInode; // actualitzam el tipus de inode
        inod.permisos = permisosInode; // actualitzam els permisos
        inod.tamany = 0; // actualitzam el tamany
        inod.links_directoris = 1; // actualitzam les entrades de directori
        inod.blocs_assignats_dades = 0; // actualitzam els blocs per a dades

        // obtenim el seguent inode lliure i el posam a null (-1)
        seg_inode_lliure = inod.pdirectes[0];

        int i;
        for (i = 0; i < MAX_PUNTERS_DIRECTES; i++) {
            inod.pdirectes[i] = -1;
        }

        for (i = 0; i < MAX_PUNTERS_INDIRECTES; i++) {
            inod.pindirectes[i] = -1;
        }

        // guardam els canvis
        escriureInode(inode_lliure, inod);

        // actualitzam els camps corresponents en el superbloc
        sb.inode_lliure = seg_inode_lliure;
        sb.inodes_lliures--;

        // escrivim els canvis del superbloc
        if (bwrite(POSICIO_SB, (char *)&sb) == -1) {
            return -1;
        }
        return inode_lliure;
    } else {
        printf("[ficheros_basico.c] ERROR: No hi ha inodes lliures!\n");
        return -1;
    }
}

/**
 *  Allibera els blocs que pertanyen a l'inode i que estaven ocupats per l'inode.
 *  Recórrer els punters directes i indirectes i localitza els blocs de l'inode en concret
 *  i allibera els blocs que els hi pertoca.
 *  @param inod inode en el qual volem alliberar els blocs
 *  @param bloc bloc on està situat l'inode
 *  @return nombre de blocs que pertanyen a l'inode i que estaven ocupats
 */
int alliberarBlocInode(inode inod, int bloc)
{
    int buff[N_PUNTERS_BLOC];
    int buff1[N_PUNTERS_BLOC];
    int buff2[N_PUNTERS_BLOC];
    int blocs = 0;
    int i = 0;
    int j = 0;
    int k = 0;
    int ret = -1;

    if ((bloc >= 0) && (bloc < MAX_PUNTERS_DIRECTES - 1)) { // punters directes
            for (i = bloc; i < MAX_PUNTERS_DIRECTES; i++) {
                if (inod.pdirectes[i] > 0) {
                    ret = alliberarBloc(inod.pdirectes[i]);
                    // alliberam el bloc on esta el pdirecte
                    if (ret == 0) {
                        blocs++;
                    }
                }
                inod.pdirectes[i] = -1;
            }
    } else if (bloc <= (MAX_PUNTERS_DIRECTES + N_PUNTERS_BLOC - 1)) { // punters indirectes de nivell 0
        if (bloc >= MAX_PUNTERS_DIRECTES) {
            bloc = bloc - MAX_PUNTERS_DIRECTES;
        } else if (bloc < MAX_PUNTERS_DIRECTES) {
            bloc = 0;
        }

        if (inod.pindirectes[0] > 0) {
            if (bread(inod.pindirectes[0], buff) == -1) {
                return -1;
            }
            for (i = bloc; i < N_PUNTERS_BLOC; i++) {
                if (buff[i] > 0) {
                    ret = alliberarBloc(buff[i]);
                    if (ret == 0) {
                        blocs++;
                        buff[i] = -1;
                    }
                }
            }
            if (bwrite(inod.pindirectes[0], buff) == -1) {
                return -1;
            }

            if (bloc == 0) {
                // alliberam el bloc on esta el pindirecte[0]
                ret = alliberarBloc(inod.pindirectes[0]);
                if (ret == 0) {
                    blocs++;
                    inod.pindirectes[0] = -1;
                }
            }
        }
    } else if (bloc <= (MAX_PUNTERS_DIRECTES + N_PUNTERS_BLOC + (N_PUNTERS_BLOC * N_PUNTERS_BLOC) - 1)) { // punters indirectes de nivell 1
        if (inod.pindirectes[1] > 0) {

            if (bloc >= (MAX_PUNTERS_DIRECTES + N_PUNTERS_BLOC)) {
                bloc = bloc - (MAX_PUNTERS_DIRECTES + N_PUNTERS_BLOC);
            } else if (bloc < (MAX_PUNTERS_DIRECTES + N_PUNTERS_BLOC)) {
                bloc = 0;
            }
                if (bread(inod.pindirectes[1], buff) == -1) {
                return -1;
            }

            for (i = (bloc / N_PUNTERS_BLOC); i < N_PUNTERS_BLOC; i++) {
                if (buff[i] > 0) {
                    if (bread(buff[i], buff1) == -1) {
                        return -1;
                    }

                    for (j = (bloc % N_PUNTERS_BLOC); j < N_PUNTERS_BLOC; j++) {
                        if (buff1[j] > 0) {
                            ret = alliberarBloc(buff1[j]);
                            if (ret == 0) {
                                blocs++;
                                buff1[j] = -1;
                            }
                        }
                    }

                    if (bwrite(buff[i], buff1) == -1) {
                        return -1;
                    }

                    ret = alliberarBloc(buff[i]);
                    if (ret == 0) {
                        blocs++;
                        buff[i] = -1;
                    }
                }
            }

            if (bwrite(inod.pindirectes[1], buff) == -1) {
                return -1;
            }

            if (bloc == 0) {
                // alliberam el bloc on esta el pindirecte[1]
                ret = alliberarBloc(inod.pindirectes[1]);
                if (ret == 0) {
                    blocs++;
                    inod.pindirectes[1] = -1;
                }
            }
        }
    } else if (bloc <= (MAX_PUNTERS_DIRECTES + N_PUNTERS_BLOC + (N_PUNTERS_BLOC * N_PUNTERS_BLOC) + (N_PUNTERS_BLOC * N_PUNTERS_BLOC * N_PUNTERS_BLOC) - 1)) { // punters indirectes de nivell 2
        bloc = bloc - (MAX_PUNTERS_DIRECTES + N_PUNTERS_BLOC + (N_PUNTERS_BLOC * N_PUNTERS_BLOC));
        if (inod.pindirectes[2] > 0) {
            if (bread(inod.pindirectes[2], buff) == -1) {
                return -1;
            }

            for (i = bloc / (N_PUNTERS_BLOC * N_PUNTERS_BLOC); i < N_PUNTERS_BLOC; i++) {
                if (buff[i] > 0) {
                    if (bread(buff[i], buff1) == -1) {
                        return -1;
                    }
                    bloc = bloc % (N_PUNTERS_BLOC * N_PUNTERS_BLOC);

                    for (j = bloc / N_PUNTERS_BLOC; j < N_PUNTERS_BLOC; j++) {
                        if (buff1[j] > 0) {
                            if (bread(buff1[j], buff2) == -1) {
                                return -1;
                            }

                            for (k = 0; k < N_PUNTERS_BLOC; k++) {
                                if (buff2[k] > 0) {
                                    ret = alliberarBloc(buff2[k]);
                                    if (ret == 0) {
                                        blocs++;
                                        buff2[k] = -1;
                                    }
                                }
                            }
                            ret = alliberarBloc(buff1[j]);
                            if (ret == 0) {
                                blocs++;
                            }
                        }
                    }
                    ret = alliberarBloc(buff[i]);
                    if (ret == 0) {
                        blocs++;
                    }
                }
            }

            if (bloc == 0) {
                // alliberam el bloc on esta el pindirecte[2]
                ret = alliberarBloc(inod.pindirectes[2]);
                if (ret == 0) {
                    blocs++;
                    inod.pindirectes[2] = -1;
                }
            }
        }
    }
    return blocs;
}

/**
 *  Funció encarregada d'alliberar tots els blocs que pertanyen a un inode concret
 *  @param inod posició de l'inode a alliberar
 *  @param bloc bloc on està l'inode que es vol eliminar
 *  @param eliminar paràmetre que si esta a 1 s'ha de eliminar l'inode, i si, està a 0, no.
 */
int alliberarInode(int inod, int bloc, int eliminar)
{
    superbloc sb;
    inode in;
    int blocs_ocupats = 0;
    int blocs_assignats = 0;
    int i = 0;

    if (bread(POSICIO_SB, (char *)&sb) == -1) { // llegim el superbloc
        return -1;
    }

    in = llegirInode(inod); // llegim l'inode a alliberar
    if (inod == 0) { // comprovam que l'inode no sigui l'arrel
        printf("[ficheros_basico.c] ERROR: No se pot eliminar l'inode arrel!!\n");
    } else if (in.tipus == 0) { // comprovam que l'inode no sigui lliure
        printf("[ficheros_basico.c] ERROR: L'inode que vols alliberar ja és lliure (GPL xD)!\n");
    } else if ((in.tipus == 1) || (in.tipus == 2)) {
        blocs_assignats = in.blocs_assignats_dades;

        // calculam els blocs que ocupa, mirant els punters directes i indirectes de l'inode
        blocs_ocupats = alliberarBlocInode(in, bloc);

        if (eliminar == 0) { // només volem alliberar l'inode
            in.tamany = in.tamany - blocs_ocupats * TB; // tamany en bytes
            if (in.tamany < 0) {
                in.tamany = 0;
            }
            in.blocs_assignats_dades = in.blocs_assignats_dades - blocs_ocupats;
        } else if (eliminar == 1) { // definitivament volen eliminar l'inode, borram els blocs i els camps
            in.tipus = 0;
            in.data_modificacio = 0;
            in.tamany = 0;
            in.blocs_assignats_dades = 0;

            in.pdirectes[0] = sb.inode_lliure;
            for (i = 1; i < MAX_PUNTERS_DIRECTES; i++) {
                in.pdirectes[i] = -1;
            }

            for (i = 0; i < MAX_PUNTERS_INDIRECTES; i++) {
                in.pindirectes[i] = -1;
            }
            sb.inode_lliure = inod;
            sb.inodes_lliures++;
        }

        if (escriureInode(inod, in) == -1) {
            return -1;
        }

        sb.blocs_lliures += blocs_ocupats;
        if (bwrite(POSICIO_SB, (char *)&sb) == -1) { // guardam els canvis en el superbloc
            return -1;
        }
    }
    return 0;
}

/**
 *  Funció que s'encarrega de calcular el número de bloc físic el quan fa referència al número de bloc lògic d'un determinat inode.
 *  @param inod inode que volem traduir
 *  @param blocLogic bloc lògic
 *  @param reservar si val 0 llavors només consulta. Si val 1, llavors consunta, i si, no existeix cap bloc físic, també reserva.
 *  @return El bloc físic al qual fa referència el bloc lògic
 */
int traduirBlocInode(unsigned int inod, unsigned int blocLogic, char reservar)
{
    inode in;
    char buff[N_PUNTERS_BLOC];
    char buff2[N_PUNTERS_BLOC];
    char buff3[N_PUNTERS_BLOC];
    int bfisic = 0;
    int pd = MAX_PUNTERS_DIRECTES; // pd = punters directes
    int pin0 = MAX_PUNTERS_DIRECTES + N_PUNTERS_BLOC; // 268
    int pin1 = MAX_PUNTERS_DIRECTES + N_PUNTERS_BLOC + (N_PUNTERS_BLOC * N_PUNTERS_BLOC); // 65804
    int pin2 = MAX_PUNTERS_DIRECTES + N_PUNTERS_BLOC +
                (N_PUNTERS_BLOC * N_PUNTERS_BLOC) +
                (N_PUNTERS_BLOC * N_PUNTERS_BLOC * N_PUNTERS_BLOC); // 16843020
    int bloc_res = 0; // bloc reservat
    int temp;

    in = llegirInode(inod); // llegim l'inode

    if (blocLogic < pin2) { // Comprovam que no sobrepassi el maxim
        if (reservar == '0') { // Consulta
            if (blocLogic >= 0 && blocLogic <= pd - 1) { // punters directes de 0 - 11
                if (reservar == '1') { // Escritura
                    temp = in.pdirectes[blocLogic];
                    if (temp == -1) { // No te cap bloc assignat
                        in.pdirectes[blocLogic] = reservarBloc(); // reservam el primer bloc lliure que trobam
                        in.blocs_assignats_dades++; // modificam els blocs assignats de l'inode
                        in.data_modificacio = time(NULL);
                        escriureInode(inod, in); // escrivim els canvis de l'inode
                    }
                }
                if (in.pdirectes[blocLogic] == -1) {
                    return -1;
                }
                bfisic  =  in.pdirectes[blocLogic]; // retornam directament la posició del bloc físic

                printf("[ficheros_basico.c - traduirBlocInode (consulta)] DEBUG: blocLogic: %d | bfisic:%d\n", blocLogic, bfisic);
                return bfisic;

            } else if (blocLogic >= pd &&  blocLogic <= pin0 - 1) {  // punters indirectes de nivell0 // 12 - 267
                temp = in.pindirectes[0];

                if (temp > 0) { // comprovam que hi hagui direccions
                    if (bread(in.pindirectes[0], buff) == -1) { // carregam a memoria (a buff) els punters indirectes de nivell 0
                        return -1;
                    }

                    bfisic = blocLogic - pd; // calculam la localització del bloc físic
                    return buff[bfisic]; // retornam el "punter" del bloc físic que es troba al buff (dins memòria) i que apunta la zona de dades (el bloc de dades)
                } else {
                    printf("[ficheros_basico.c] ERROR: No hi ha direccions disponibles a pin0!\n");
                    return -1;
                }
            } else if (blocLogic >= pin0 && blocLogic <= pin1 - 1) { // punters indirectes nivell 1 //268 - 65.803
                temp = in.pindirectes[1];

                if (temp > 0) { // comprovam que hi hagui direccions
                    if (bread(in.pindirectes[1], buff) == -1) { // carregam a memoria (a buff) els punters indirectes de nivell 1
                        return -1;
                    }

                    bfisic= blocLogic - pin0; // calculam la posició del bloc físic
                    temp = bfisic / N_PUNTERS_BLOC;

                    if (bread(buff[temp], buff2) == -1) { // copiam el resultat de la divisió dins un altra buffer
                        return -1;
                    }

                    return buff2[bfisic % N_PUNTERS_BLOC]; // retornam el "punter" del bloc físic que es troba al buff (dins memòria) i que apunta la zona de dades (el bloc de dades)
                } else {
                    printf("[ficheros_basico.c] ERROR: No hi ha direccions disponibles a pin1!\n");
                    return -1;
                }
            } else if (blocLogic >= pin1 && blocLogic <= pin2 - 1) { // punters indirectes de nivell 2 // 65.804 - 16.843.019
                temp = in.pindirectes[2];

                if (temp > 0) { // comprovam que hi hagui direccions
                    if (bread(in.pindirectes[2], buff) == -1) { // carregam a memoria (a buff) els punters indirectes de nivell 2
                        return -1;
                    }
                } else { // Consulta

                    bfisic = blocLogic - pin1; // calculam la posició del bloc físic
                    int b1 = N_PUNTERS_BLOC * N_PUNTERS_BLOC; // 256*256 = 65536
                    temp = bfisic / b1;

                    if (bread(buff[temp], buff2) == -1) { // copiam el resultat de la divisió
                        return -1;
                    }

                    bfisic = bfisic % b1; // avançam un nivell
                    temp = bfisic / N_PUNTERS_BLOC;

                    if (bread(buff2[temp], buff3) == -1) { // copiam el resultat de la divisió
                        return -1;
                    }
                    return buff3[bfisic % N_PUNTERS_BLOC]; // retornam el "punter" del bloc físic que es troba al buff (dins memòria) i que apunta la zona de dades (el bloc de dades)

                } else {
                    printf("[ficheros_basico.c] ERROR: No hi ha direccions disponibles a pin2!\n");
                    return -1;
                }
            }
        } else if (reservar == '1') { // Escriptura

            if (blocLogic >= 0 && blocLogic <= pd - 1) { // punters directes de 0 - 11
                temp = in.pdirectes[blocLogic];
                printf("[ficheros_basico.c] DEBUG: in.pdirectes : %d\n",temp);
                if (temp > 0) { // comprovam que existeix el bloc físic
                    bfisic = in.pdirectes[blocLogic]; // retornam directament la posició del bloc físic
                    return bfisic;
                } else  {
                    in.pdirectes[blocLogic] = reservarBloc(); // reservam el primer bloc lliure que trobam
                    in.blocs_assignats_dades++; // modificam els blocs assignats de l'inode
                    in.data_modificacio = time(NULL);

                    escriureInode(inod, in); // escrivim els canvis de l'inode
                    bfisic = in.pdirectes[blocLogic]; // retornam directament la posició del bloc físic

                    return bfisic;
                }
            } else if (blocLogic >= pd &&  blocLogic <= pin0 - 1) {  // punters indirectes de nivell0 // 12 - 267
                temp = in.pindirectes[0];

                if (temp > 0) { // comprovam que hi hagui direccions
                    if (bread(in.pindirectes[0], buff) == -1) { // carregam a memoria (a buff) els punters indirectes de nivell 0
                        return -1;
                    }

                    bfisic = blocLogic - pd; // calculam la localització del bloc físic
                    return buff[bfisic]; // retornam el "punter" del bloc físic que es troba al buff (dins memòria) i que apunta la zona de dades (el bloc de dades)
                } else { // inicialitzam els punters indirectes de nivell 0
                    bloc_res = reservarBloc(); // reservam un bloc
                    in.pindirectes[0] = bloc_res; // assignam el bloc reservat al punter indirecte de nivell 0

                    if (bread(temp, buff) == -1) {
                        return -1;
                    }
                            if (buff2[bfisic % N_PUNTERS_BLOC] == 0) {
                                bloc_res = reservarBloc();
                                buff2[blocLogic % N_PUNTERS_BLOC] = bloc_res;

                                if (bwrite(buff[bfisic / N_PUNTERS_BLOC], buff2) == -1) {
                                    return -1;
                                }

                    in.blocs_assignats_dades++;
                    in.data_modificacio = time(NULL);

                    escriureInode(inod, in); // escrivim els canvis a l'inode
                    return buff[bfisic]; // retornam el "punter" del bloc físic que es troba al buff (dins memòria) i que apunta la zona de dades (el bloc de dades)
                }
            } else if (blocLogic >= pin0 && blocLogic <= pin1 - 1) { // punters indirectes nivell 1 //268 - 65.803
                temp = in.pindirectes[1];

                if (temp > 0) { // comprovam que hi hagui direccions
                    if (bread(in.pindirectes[1], buff) == -1) { // carregam a memoria (a buff) els punters indirectes de nivell 1
                        return -1;
                    }

                    bfisic= blocLogic - pin0; // calculam la posició del bloc físic
                    temp = bfisic / N_PUNTERS_BLOC;

                    if (bread(buff[temp], buff2) == -1) { // copiam el resultat de la divisió dins un altra buffer
                        return -1;
                    }

                    return buff2[bfisic % N_PUNTERS_BLOC]; // retornam el "punter" del bloc físic que es troba al buff (dins memòria) i que apunta la zona de dades (el bloc de dades)
                } else { // ara reservam
                    bloc_res = reservarBloc();
                    in.pindirectes[1] = bloc_res;

                    if (bread(in.pindirectes[1], buff) == -1) { // carregam a memoria (a buff) els punters indirectes de nivell 2
                        return -1;
                    }
                }//FIN reservar = 1

                return buff2[bfisic % N_PUNTERS_BLOC]; // retornam el "punter" del bloc físic que es troba al buff (dins memòria) i que apunta la zona de dades (el bloc de dades)

                    if (bwrite(in.pindirectes[1], buff) == -1) {
                        return -1;
                    }
                    if (bread(buff[bfisic / N_PUNTERS_BLOC], buff2) == -1) { // carregam a memoria (a buff) els punters indirectes de nivell 2
                        return -1;
                    }

                    bloc_res = reservarBloc();
                    buff2[bfisic / N_PUNTERS_BLOC] = bloc_res;
                    in.blocs_assignats_dades++;
                    in.data_modificacio = time(NULL);

                    if (bwrite(buff[bfisic / N_PUNTERS_BLOC], buff2) == -1) {
                        return -1;
                    }
                    escriureInode(inod, in);

                    return buff2[bfisic % N_PUNTERS_BLOC];
                }
            } else if (blocLogic >= pin1 && blocLogic <= pin2 - 1) { // punters indirectes de nivell 2 // 65.804 - 16.843.019
                temp = in.pindirectes[2];
                if (temp > 0) { // comprovam que hi hagui direccions
                    if (bread(in.pindirectes[2], buff) == -1) { // carregam a memoria (a buff) els punters indirectes de nivell 2
                        return -1;
                    }

                    bfisic = blocLogic - pin1; // calculam la posició del bloc físic
                    int b1 = N_PUNTERS_BLOC * N_PUNTERS_BLOC; // 256*256 = 65536
                    temp = bfisic / b1;

                    if (bread(buff[temp], buff2) == -1) { // copiam el resultat de la divisió
                        return -1;
                    }

                    bfisic = bfisic % b1; // avançam un nivell
                    temp = bfisic / N_PUNTERS_BLOC;

                    if (bread(buff2[temp], buff3) == -1) { // copiam el resultat de la divisió
                        return -1;
                    }
                    return buff3[bfisic % N_PUNTERS_BLOC]; // retornam el "punter" del bloc físic que es troba al buff (dins memòria) i que apunta la zona de dades (el bloc de dades)
                } else { // ara reservam
                    bloc_res = reservarBloc(); // nivell 0
                    in.pindirectes[2] = bloc_res;

                    if (bread(in.pindirectes[2], buff) == -1) {
                        return -1;
                    }
                    bfisic = blocLogic - pin1;

                    bloc_res = reservarBloc();
                    buff[bfisic/(N_PUNTERS_BLOC * N_PUNTERS_BLOC)] = bloc_res;

                    if (bwrite(in.pindirectes[2], buff) == -1) {
                        return -1;
                    }

                    if (bread(buff[bfisic/(N_PUNTERS_BLOC * N_PUNTERS_BLOC)], buff2) == -1) {
                        return -1;
                    }

                    int b2 = bfisic % (N_PUNTERS_BLOC * N_PUNTERS_BLOC);

                    bloc_res = reservarBloc(); // nivell 1
                    buff2[b2 / N_PUNTERS_BLOC] = bloc_res;

                    if (bwrite(buff[bfisic / (N_PUNTERS_BLOC * N_PUNTERS_BLOC)], buff2) == -1) {
                        return -1;
                    }
                } else { // Consulta
                    temp = in.pindirectes[2];
                    if (temp > 0) { // comprovam que hi hagui direccions
                        if (bread(in.pindirectes[2], buff) == -1) { // carregam a memoria (a buff) els punters indirectes de nivell 2
                            return -1;
                        }

                        bfisic = blocLogic - pin1; // calculam la posició del bloc físic
                        int b1 = N_PUNTERS_BLOC * N_PUNTERS_BLOC; // 256*256 = 65536
                        temp = bfisic / b1;
                        if (buff[temp] > 0) {
                            if (bread(buff[temp], buff2) == -1) { // copiam el resultat de la divisió
                                return -1;
                            }
                    bloc_res = reservarBloc(); // nivell 2
                    buff[bfisic % N_PUNTERS_BLOC] = bloc_res;

                    in.blocs_assignats_dades++;
                    in.data_modificacio = time(NULL);

                            } else {
                                return -1;
                            }
                        } else {
                            return -1;
                        }
                    } else {
                        printf("[ficheros_basico.c] ERROR: No hi ha direccions disponibles a pin2!\n");
                        return -1;
                    }
                }//Fin reservar = 1
             return (buff[bfisic % N_PUNTERS_BLOC]); //Devolvemos bloque fisico
            //---------------------
            }

    } else {
        printf("[ficheros_basico.c] ERROR: Numero de bloc superior al màxim permès!\n");
        return -1;
    }
    return 0;
}
