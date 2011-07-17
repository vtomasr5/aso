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
 *  @brief Conté les funcions de baix nivell que tracten directament amb el sistema de fitxers.\n
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

#include "../include/ficheros_basico.h"

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
        tam_mb = (nblocs / (TB * 8)) + 1;
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
    int inodesbloc = TB / sizeof(inode);

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
    int i = 0;

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
int initAI()
{
    superbloc sb;
    inode inod;
    int i, j;
    unsigned char buff[TB];
    int inodesbloc = TB / sizeof(inode); // inodes per bloc
    int inode_actual = 0; // variable que fa referència a l'inode que s'està tractant actualment

    // no hi ha inodes directes emprats
    for (i = 0; i < MAX_PUNTERS_DIRECTES; i++) {
        inod.pdirectes[i] = -1;
    }

    // no hi ha inodes indirectes emprats
    for (i = 0; i < MAX_PUNTERS_INDIRECTES; i++) {
        inod.pindirectes[i] = -1;
    }

    if (bread(POSICIO_SB, (char *)&sb) == -1) { // llegim el superbloc
        return -1;
    }

    // Per a cada bloc de l'AI, anam inicialitzant la informació corresponent
    for (i = sb.primerbloc_ai; i <= sb.darrerbloc_ai; i++) {
        memset(buff, 0, TB); // inicialitzam a zero tots els blocs de l'array de inodes

        // per a cada inode inicialitzam els camps corresponents
        for (j = 0; j < inodesbloc; j++) {
            inod.tipus = 0; // 0 lliure, 1 directori, 2 fitxer
            inod.tamany = 0;
            inod.blocs_assignats_dades = 0;
            inod.data_creacio = time(NULL);
            inod.data_modificacio = time(NULL);
            inod.data_acces = time(NULL);
            inod.links_directoris = 0;

            // llista de inodes
            if (inode_actual < sb.total_inodes - 1) {
                inod.pdirectes[0] = inode_actual + 1; // apuntam al següent
            } else if (inode_actual == sb.total_inodes - 1) {
                inod.pdirectes[0] = -1; // cas del darrer inode
            }
            memcpy(&buff[j * sizeof(inode)], &inod, sizeof(inode)); // guardam els canvis
            inode_actual++; // passam al següent inode
        }

        if (bwrite(i, buff) == -1) { // guardam els canvis realitzats en el superbloc
            return -1;
        }
    }

    printf("[ficheros_basico.c] INFO: Array de inodes inicialitzat\n");
    return 0;
}

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
    unsigned char buff[TB];
    unsigned char valor = 128; // mascara 10000000
    int pos_byte = bloc / 8;
    int pos_bit = bloc % 8;

    if (bread(POSICIO_SB, (char *)&sb) == -1) { // llegim el superbloc
        return -1;
    }

    int bloc_mb = sb.primerbloc_mb + (bloc / (TB*8)); // calculam el bloc on es realitzaran les operacions

    if (bread(bloc_mb, buff) == -1) { // llegim el bloc calculat
        return -1;
    }

    if (pos_bit > 0) {
        valor >>= pos_bit; // desplaçam cap a la dreta "pos_bit posicions"
    }

    if (bit == 0) {
        buff[pos_byte] &= ~valor; // AND (&) i NOT (~) en bits
        //sb.blocs_lliures++; // augmentam el contador de blocs lliures del SB
    } else if (bit == 1) {
        buff[pos_byte] |= valor; // OR (|) en bits
        //sb.blocs_lliures--; // disminuim el contador de blocs lliures del SB
    }

    if (bwrite(bloc_mb, buff) == -1) { // escrivim els canvis
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
    unsigned char buff[TB];
    unsigned char valor = 128; // mascara 10000000
    int pos_byte = bloc / 8;
    int pos_bit = bloc % 8;

    if (bread(POSICIO_SB, (char *)&sb) == -1) { // llegim el superbloc
        return -1;
    }

    memset(buff, 0, TB); // netejam el buffer

    bloc_mb = sb.primerbloc_mb + (bloc / (TB*8));

    if (bread(bloc_mb, buff) == -1) { // llegim el bloc calculat
        return -1;
    }

    if (pos_bit > 0) {
        valor >>= pos_bit; // desplaçam cap a la dreta "pos_bit posicions"
    }

    valor &= buff[pos_byte]; // operador AND para bits
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

            sb.blocs_lliures++; // augmentam els la quantitat de blocs lliures
        }

        if (bwrite(POSICIO_SB, (char *)&sb) == -1) { // guardam els canvis al superbloc
            return -1;
        }
    } else {
        printf("[ficheros_basico.c] ERROR: Aquest bloc no pertany a la zona de dades!\n");
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
int reservarInode(uint tipusInode, unsigned int permisosInode)
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
        if (escriureInode(inode_lliure, inod) == -1) {
            return -1;
        }

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
 *  @return nombre de blocs que pertanyen a l'inode i que estaven ocupats
 */
int alliberarBlocInode(inode inod)
{
    int buff[N_PUNTERS_BLOC];
    int buff1[N_PUNTERS_BLOC];
    int buff2[N_PUNTERS_BLOC];
    int blocs = 0;
    int i = 0;
    int j = 0;
    int k = 0;
    int ret = 0;
    int ino = 0;

    // punters directes
    for (i = 0; i < MAX_PUNTERS_DIRECTES; i++) {
        ino = inod.pdirectes[i];

        //printf("[ficheros_basico.c] DEBUG: bloc (inod.pdirectes[%d]) = %d\n", i, ino);

        if (ino > 0) {
            ret = alliberarBloc(ino);
            // alliberam el bloc on esta el punter directe
            if (ret != -1) {
                blocs++;
            }
        }
        inod.pdirectes[i] = -1; // resetejam la posicio del inode
    }

    // punters indirectes nivell 0
    ino = inod.pindirectes[0];
    //printf("[ficheros_basico.c] DEBUG: bloc (inod.pindirectes[0]) = %d\n", ino);
    if (ino > 0) {
        if (bread(ino, buff) == -1) {
            return -1;
        }

        for (i = 0; i < N_PUNTERS_BLOC; i++) {
            if (buff[i] > 0) {
                ret = alliberarBloc(buff[i]);
                if (ret != -1) {
                    blocs++;
                    buff[i] = -1;
                }
            }
        }

        if (bwrite(ino, buff) == -1) {
            return -1;
        }
    }

    // punters indirectes nivell 1
    ino = inod.pindirectes[1];
    //printf("[ficheros_basico.c] DEBUG: bloc (inod.pindirectes[1]) = %d\n", ino);
    if (ino > 0) {
        if (bread(ino, buff) == -1) {
            return -1;
        }

        for (i = 0; i < N_PUNTERS_BLOC; i++) {
            if (buff[i] > 0) {
                if (bread(buff[i], buff1) == -1) {
                    return -1;
                }

                for (j = 0; j < N_PUNTERS_BLOC; j++) {
                    if (buff1[j] > 0) {
                        ret = alliberarBloc(buff1[j]);
                        if (ret != -1) {
                            blocs++;
                            buff1[j] = -1;
                        }
                    }
                }

                if (bwrite(buff[i], buff1) == -1) {
                    return -1;
                }

                ret = alliberarBloc(buff[i]);
                if (ret != -1) {
                    blocs++;
                    buff[i] = -1;
                }
            }
        }

        if (bwrite(ino, buff) == -1) {
            return -1;
        }
    }

    // punters indirectes nivell 2
    ino = inod.pindirectes[2];
    //printf("[ficheros_basico.c] DEBUG: bloc (inod.pindirectes[2]) = %d\n", ino);
    if (ino > 0) {
        if (bread(ino, buff) == -1) {
            return -1;
        }

        for (i = 0; i < N_PUNTERS_BLOC; i++) {
            if (buff[i] > 0) {
                if (bread(buff[i], buff1) == -1) {
                    return -1;
                }

                for (j = 0; j < N_PUNTERS_BLOC; j++) {
                    if (buff1[j] > 0) {
                        if (bread(buff1[j], buff2) == -1) {
                            return -1;
                        }

                        for (k = 0; k < N_PUNTERS_BLOC; k++) {
                            if (buff2[k] > 0) {
                                ret = alliberarBloc(buff2[k]);
                                if (ret != -1) {
                                    blocs++;
                                    buff2[k] = -1;
                                }
                            }
                        }

                        ret = alliberarBloc(buff1[j]);
                        if (ret != -1) {
                            blocs++;
                        }
                    }
                }

                ret = alliberarBloc(buff[i]);
                if (ret != -1) {
                    blocs++;
                }
            }
        }

        if (bwrite(ino, buff) == -1) {
            return -1;
        }
    }

    // retornam els numero de blocs alliberats
    return blocs;
}

/**
 *  Funció encarregada d'alliberar tots els blocs que pertanyen a un inode concret
 *  @param inod posició de l'inode a alliberar
 *  @param eliminar paràmetre que si esta a 1 s'ha de eliminar l'inode, i si, està a 0, no.
 */
int alliberarInode(int inod, int eliminar)
{
    superbloc sb;
    inode in;
    int blocs_ocupats = 0;
    int i = 0;
    int tam = 0;
    //inode AI[(TB / sizeof(inode))];

    if (bread(POSICIO_SB, (char *)&sb) == -1) { // llegim el superbloc
        return -1;
    }

    in = llegirInode(inod); // llegim l'inode a alliberar
    printf("[ficheros_basico.c] DEBUG: Inod = %d\n", inod);
    if (inod == 0) { // comprovam que l'inode no sigui l'arrel
        printf("[ficheros_basico.c] ERROR: No se pot eliminar l'inode arrel!!\n");
    } else if (in.tipus == 0) { // comprovam que l'inode no sigui lliure
        printf("[ficheros_basico.c] ERROR: L'inode que vols alliberar ja és lliure (GPL xD)!\n");
    } else if ((in.tipus == 1) || (in.tipus == 2)) {

        // calculam els blocs que ocupa, mirant els punters directes i indirectes de l'inode
        blocs_ocupats = alliberarBlocInode(in);

        printf("[ficheros_basico.c] DEBUG: blocs ocupats per l'inode = %d\n", blocs_ocupats);

        if (eliminar == 0) { // només volem alliberar l'inode
            tam = in.tamany - (blocs_ocupats * TB); // nou tamany en bytes
            if (tam < 0) {
                tam = 0;
            }

            in.tamany = tam;
            in.blocs_assignats_dades -= blocs_ocupats;
        } else if (eliminar == 1) { // definitivament volen eliminar l'inode, borram els blocs i resetejam els camps de l'inode
            in.tipus = 0;
            in.permisos = 0;
            in.data_modificacio = 0;
            in.data_acces = 0;
            in.tamany = 0;
            in.blocs_assignats_dades = 0;
            in.pdirectes[0] = sb.inode_lliure;

            for (i = 0; i < MAX_PUNTERS_DIRECTES; i++) {
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
    unsigned char buff[N_PUNTERS_BLOC];
    unsigned char buff2[N_PUNTERS_BLOC];
    unsigned char buff3[N_PUNTERS_BLOC];
    memset(buff, 0, N_PUNTERS_BLOC);
    memset(buff2, 0, N_PUNTERS_BLOC);
    memset(buff3, 0, N_PUNTERS_BLOC);
    int bfisic = 0;
    int pd = MAX_PUNTERS_DIRECTES; // pd = punters directes
    int pin0 = MAX_PUNTERS_DIRECTES + N_PUNTERS_BLOC; // 268
    int pin1 = MAX_PUNTERS_DIRECTES + N_PUNTERS_BLOC + (N_PUNTERS_BLOC * N_PUNTERS_BLOC); // 65.804
    int pin2 = MAX_PUNTERS_DIRECTES + N_PUNTERS_BLOC +
                (N_PUNTERS_BLOC * N_PUNTERS_BLOC) +
                (N_PUNTERS_BLOC * N_PUNTERS_BLOC * N_PUNTERS_BLOC); // 16.843.020
    int bloc_res = 0; // bloc reservat
    int temp = 0;
    int temp2 = 0;

    in = llegirInode(inod); // llegim l'inode

    if (blocLogic < pin2) { // Comprovam que no sobrepassi el maxim
        if (reservar == '0') { // Consulta
            if (blocLogic >= 0 && blocLogic <= pd - 1) { // punters directes de 0 - 11
                bfisic  =  in.pdirectes[blocLogic]; // retornam directament la posició del bloc físic
                //printf("----->[ficheros_basico.c - traduirBlocInode (consulta)] DEBUG: blocLogic: %d | bfisic:%d\n", blocLogic, bfisic);
                printf("[ficheros_basico.c] DEBUG: pdirectes bfisic: %d\n", bfisic);
                return bfisic;
            } else if (blocLogic >= pd &&  blocLogic <= pin0 - 1) {  // punters indirectes de nivell0 // 12 - 267
                temp2 = in.pindirectes[0];
                printf("[ficheros_basico.c] DEBUG: pdinirectes0 (temp2): %d\n", temp2);
                if (temp2 > 0) { // comprovam que hi hagui direccions
                    if (bread(in.pindirectes[0], buff) == -1) { // carregam a memoria (a buff) els punters indirectes de nivell 0
                        return -1;
                    }

                    bfisic = blocLogic - pd; // calculam la localització del bloc físic
                    return buff[bfisic]; // retornam el "punter" del bloc físic que es troba al buff (dins memòria) i que apunta la zona de dades (el bloc de dades)
                    printf("[ficheros_basico.c] DEBUG: buff[bsific] = %d\n", buff[bfisic]);
                } else {
                    printf("[ficheros_basico.c] ERROR: No hi ha direccions disponibles a pin0!\n");
                    return -1;
                }
            } else if (blocLogic >= pin0 && blocLogic <= pin1 - 1) { // punters indirectes nivell 1 //268 - 65.803
                temp2 = in.pindirectes[1];
                printf("[ficheros_basico.c] DEBUG: pdinirectes1 (temp2): %d\n", temp2);
                if (temp2 > 0) { // comprovam que hi hagui direccions
                    if (bread(in.pindirectes[1], buff) == -1) { // carregam a memoria (a buff) els punters indirectes de nivell 1
                        return -1;
                    }

                    bfisic= blocLogic - pin0; // calculam la posició del bloc físic
                    temp = bfisic / N_PUNTERS_BLOC;

                    if (bread(buff[temp], buff2) == -1) { // copiam el resultat de la divisió dins un altra buffer
                        return -1;
                    }

                    return buff2[bfisic % N_PUNTERS_BLOC]; // retornam el "punter" del bloc físic que es troba al buff (dins memòria) i que apunta la zona de dades (el bloc de dades)
                    printf("[ficheros_basico.c] DEBUG: buff2[bfisic mod N_PUNTERS_BLOC] = %d\n", buff2[bfisic % N_PUNTERS_BLOC]);
                } else {
                    printf("[ficheros_basico.c] ERROR: No hi ha direccions disponibles a pin1!\n");
                    return -1;
                }
            } else if (blocLogic >= pin1 && blocLogic <= pin2 - 1) { // punters indirectes de nivell 2 // 65.804 - 16.843.019
                temp2 = in.pindirectes[2];
                printf("[ficheros_basico.c] DEBUG: pdinirectes2 (temp2): %d\n", temp2);
                if (temp2 > 0) { // comprovam que hi hagui direccions
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

                } else {
                    printf("[ficheros_basico.c] ERROR: No hi ha direccions disponibles a pin2!\n");
                    return -1;
                }
            }
        } else if (reservar == '1') { // Escriptura
            if (blocLogic >= 0 && blocLogic <= pd - 1) { // punters directes de 0 - 11
                temp2 = in.pdirectes[blocLogic];
                printf("[ficheros_basico.c] DEBUG: pdirectes (temp2): %d\n", temp2);
                if (temp2 > 0) { // comprovam que existeix el bloc físic
                    bfisic = in.pdirectes[blocLogic]; // retornam directament la posició del bloc físic
                    printf("[traduirBlocInode] DEBUG: PUNTERS DIRECTOS blocFisic: %d\n", bfisic);
                    return bfisic;
                } else {
                    in.pdirectes[blocLogic] = reservarBloc(); // reservam el primer bloc lliure que trobam
                    in.blocs_assignats_dades++; // modificam els blocs assignats de l'inode
                    in.data_modificacio = time(NULL);

                    if (escriureInode(inod, in) == -1) {
                        return -1;
                    }

                    bfisic = in.pdirectes[blocLogic]; // retornam directament la posició del bloc físic
                    printf("--->[traduirBlocInode] DEBUG: PUNTEROS DIRECTOS blocLogic: %d\n",blocLogic);
                    printf("--->[traduirBlocInode] DEBUG: PUNTEROS DIRECTOS blocFisic: %d\n", bfisic);
                    return bfisic;
                }
            } else if (blocLogic >= pd &&  blocLogic <= pin0 - 1) {  // punters indirectes de nivell0 // 12 - 267
                temp2 = in.pindirectes[0];
                printf("[ficheros_basico.c] DEBUG: pdirectes (temp2): %d\n", temp2);
                if (temp2 > 0) { // comprovam que hi hagui direccions
                    if (bread(in.pindirectes[0], buff) == -1) { // carregam a memoria (a buff) els punters indirectes de nivell 0
                        return -1;
                    }
                    bfisic = blocLogic - pd; // calculam la localització del bloc físic
                    printf("[traduirBlocInode] DEBUG: PUNTEROS INDIRECTOS  bfisic: %d\n",buff[bfisic]);
                    return buff[bfisic]; // retornam el "punter" del bloc físic que es troba al buff (dins memòria) i que apunta la zona de dades (el bloc de dades)
                } else { // inicialitzam els punters indirectes de nivell 0
                    bloc_res = reservarBloc(); // reservam un bloc
                    in.pindirectes[0] = bloc_res; // assignam el bloc reservat al punter indirecte de nivell 0

                    if (bread(in.pindirectes[0], buff) == -1) {
                        return -1;
                    }

                    bfisic = blocLogic - pd;
                    bloc_res = reservarBloc();
                    buff[bfisic] = bloc_res;

                    if (bwrite(in.pindirectes[0], buff) == -1) {
                        return -1;
                    }

                    in.blocs_assignats_dades++;
                    in.data_modificacio = time(NULL);

                    if (escriureInode(inod, in) == -1) {
                        return -1;
                    }
                    printf("[traduirBlocInode] DEBUG: PUNTERS INDIRECTOS bfisic: %d\n",buff[bfisic]);
                    return buff[bfisic]; // retornam el "punter" del bloc físic que es troba al buff (dins memòria) i que apunta la zona de dades (el bloc de dades)
                }
            } else if (blocLogic >= pin0 && blocLogic <= pin1 - 1) { // punters indirectes nivell 1 //268 - 65.803
                temp2 = in.pindirectes[1];

                if (temp2 > 0) { // comprovam que hi hagui direccions
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

                    bfisic = blocLogic - pin0;
                    bloc_res = reservarBloc();
                    buff[bfisic / N_PUNTERS_BLOC] = bloc_res;

                    if (bwrite(buff[bfisic / N_PUNTERS_BLOC], buff) == -1) {
                        return -1;
                    }
                    if (bread(buff[bfisic / N_PUNTERS_BLOC], buff2) == -1) { // carregam a memoria (a buff) els punters indirectes de nivell 2
                        return -1;
                    }

                    bloc_res = reservarBloc();
                    buff2[bfisic / N_PUNTERS_BLOC] = bloc_res;

                    if (bwrite(buff[bfisic / N_PUNTERS_BLOC], buff2) == -1) {
                        return -1;
                    }

                    in.blocs_assignats_dades++;
                    in.data_modificacio = time(NULL);
                    if (escriureInode(inod, in) == -1) {
                        return -1;
                    }

                    return buff2[bfisic % N_PUNTERS_BLOC];
                }
            } else if (blocLogic >= pin1 && blocLogic <= pin2 - 1) { // punters indirectes de nivell 2 // 65.804 - 16.843.019
                temp2 = in.pindirectes[2];
                if (temp2 > 0) { // comprovam que hi hagui direccions
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

                    if (bwrite(buff[bfisic/(N_PUNTERS_BLOC * N_PUNTERS_BLOC)], buff) == -1) {
                        return -1;
                    }

                    if (bread(buff[bfisic/(N_PUNTERS_BLOC * N_PUNTERS_BLOC)], buff2) == -1) {
                        return -1;
                    }

                    int b2 = bfisic % (N_PUNTERS_BLOC * N_PUNTERS_BLOC);

                    bloc_res = reservarBloc(); // nivell 1
                    buff2[b2 / N_PUNTERS_BLOC] = bloc_res;

                    if (bwrite(buff2[b2 / N_PUNTERS_BLOC], buff2) == -1) {
                        return -1;
                    }

                    if (bread(buff2[b2 / N_PUNTERS_BLOC], buff3) == -1) {
                        return -1;
                    }

                    bloc_res = reservarBloc(); // nivell 2
                    buff3[b2 % N_PUNTERS_BLOC] = bloc_res;

                    if (bwrite(buff3[b2 % N_PUNTERS_BLOC], buff3) == -1) {
                        return -1;
                    }

                    in.blocs_assignats_dades++;
                    in.data_modificacio = time(NULL);
                    if (escriureInode(inod, in) == -1) {
                        return -1;
                    }

                    return (buff3[b2 % N_PUNTERS_BLOC]); // devolvemos el bloque fisico
                }
            }
        } else {
            printf("[ficheros_basico.c] ERROR: Valor de 'reservar' erroni!\n");
            return -1;
        }
    } else {
        printf("[ficheros_basico.c] ERROR: Numero de bloc superior al màxim permès!\n");
        return -1;
    }
    return 0;
}

/**
 *  Funció que mostra per pantalla els atributs d'un inode.
 *  @param inod posició del l'inode
 */
int contingutInode(int inod) {
    inode in;
    superbloc sb;
    unsigned char buffer[TB];
    memset(buffer, 0, TB);

    if (bread(POSICIO_SB, (char *)&sb) == -1) {
        return -1;
    }

    // Calculamos en que bloque determinado esta el Inodo a leer y la posicion que tiene dentro del bloque
    int blocAI = sb.primerbloc_ai + (inod / (TB / sizeof(inode)));
    int pos_inode_bloc = (inod % (TB / sizeof(inode)));

    if (inod >= 0) {
        //Leemos el bloque donde se encuentra el Inodo y lo insertamos en un buffer
        if (bread(blocAI, buffer) == -1) {
            return -1;
        }

        //Insertamos en in toda la información asociada al Inodo que queremos leer
        memcpy(&in, &buffer[pos_inode_bloc * sizeof(inode)], sizeof(inode));

        printf("\n[ficheros_basico.c] INFO: ---------INICI-----------\n");
        printf("[ficheros_basico.c] INFO: Numero de l'inode = %d\n", inod);

        if (in.tipus == 0) {
            printf("[ficheros_basico.c] INFO: L'inode es de tipus %d (LLIURE) \n", in.tipus);
        } else if(in.tipus == 1) {
            printf("[ficheros_basico.c] INFO: L'inode es de tipus %d (DIRECTORI) \n", in.tipus);
        } else if(in.tipus == 2) {
            printf("[ficheros_basico.c] INFO: L'inode es de tipus %d (FITXER) \n", in.tipus);
        } else {
            printf("[ficheros_basico.c] INFO: Tipus desconegut!");
        }

        printf("[ficheros_basico.c] INFO: L'inode te uns permisos = %d\n", in.permisos);
        printf("[ficheros_basico.c] INFO: L'inode te un tamany de %d bytes\n", in.tamany);
        printf("[ficheros_basico.c] INFO: L'inode te %d blocs assignats\n", in.blocs_assignats_dades);
        printf("[ficheros_basico.c] INFO: L'inode te %d enllacos a directoris\n", in.links_directoris);

        struct tm *creacio = localtime(&in.data_creacio);
        struct tm *modificacio = localtime(&in.data_modificacio);
        struct tm *acces = localtime(&in.data_acces);
        char *c = asctime(creacio);
        char *m = asctime(modificacio);
        char *a = asctime(acces);

        printf("[ficheros_basico.c] INFO: Data de creació de l'inode = %s", c);
        printf("[ficheros_basico.c] INFO: Data de modificació de l'inode = %s", m);
        printf("[ficheros_basico.c] INFO: Data d'acces de l'inode = %s", a);

        printf("[ficheros_basico.c] INFO: pdirectes[0] apunta a: %d \n", in.pdirectes[0]);
        printf("[ficheros_basico.c] INFO: pindirectes[0] apunta a: %d \n", in.pindirectes[0]);
        printf("[ficheros_basico.c] INFO: pindirectes[1] apunta a: %d \n", in.pindirectes[1]);
        printf("[ficheros_basico.c] INFO: pindirectes[2] apunta a: %d \n", in.pindirectes[2]);

        printf("[ficheros_basico.c] INFO: ---------FINAL------------\n\n");
    } else {
        printf("[ficheros_basico.c] ERROR: Numero d'inode incorrecte!\n");
        return -1;
    }
    return 0;
}
