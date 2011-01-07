//      bloques.c
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
 *  @file bloques.c
 *  @brief Conté les funcions necessàries per a montar i desmontar el sistema de fitxers, llegir i
 *  escriure els blocs del sistema de fitxers virtual.
 *  bloques.c té com a objectiu bàsic l'escriptura i lectura de blocs en el sistema de fitxers. Per
 *  això s'implementen les funcións de bread() i bwrite() que permeten aquestes operacions sobre
 *  el disc dur virtual.
 *  @date 04/10/2010
 */

#include "bloques.h"
#include "definicions.h"

static int descriptor;

/**
 *  Monta el sistema de fitxers a un arxiu del sistema. Realment deixa obert un arxiu
 *  amb un descriptor que apunta a ell per tal de poder accedir tal arxiu per realizar
 *  les operacions necessàries.
 *  @param nom_fs Nom del fitxer a on es realizaran les operacions del sistema de fitxers
 *  @return int Un descriptor de l'arxiu que s'ha obert
 */
int bmount(const char *nom_fs)
{
    descriptor = open(nom_fs, O_CREAT | O_RDWR, 0666);
    if (descriptor < 0) {
        printf("[bloques.c] ERROR: No s'ha pogut montar\n");
        return -1;
    } else {
        printf("\n[bloques.c] INFO: FS montat\n");
        return descriptor;
    }
}

/**
 *  Desmonta el sistema de fitxers. Tanca el fitxer obert apuntat pel descriptor.
 */
int bumount()
{
    int res = close(descriptor);
    if (res < 0) {
        printf("[bloques.c] ERROR: No s'ha pogut desmontar\n");
        return -1; // error
    } else {
        printf("[bloques.c] INFO: FS desmontat\n\n");
        return 0; // tot ok
    }
}

/**
 *  Llegeix els bloc expecificat pel primer argument i el posa fins el buffer,
 *  el segon argument
 *  @param bloc El bloc del qual volem llegir
 *  @param buff El buffer on volem posar la informació
 */
int bread(uint bloc, void *buff)
{
    int res = lseek(descriptor, bloc * TB, SEEK_SET);
    // printf ("[bloques.c] DEBUG: Bloque recibido: %d\n", bloc);
    if (res < 0) {
        printf("[bloques.c] ERROR: No s'ha pogut posicionar (bread)\n");
        return -1;
    } else {
        // llegeix el bloc de descriptor i l'inserta al buffer
        res = read(descriptor, buff, TB);
        if (res < 0) {
            printf("[bloques.c] ERROR: No s'ha pogut llegir el bloc %d \n", bloc);
            return -1;
        } else {
            return 0;
        }
    }
}

/**
 *  Escriu el contingut del buffer en el bloc especificat pel primer argument
 *  @param bloc El bloc a on volem escriure
 *  @param buff El buffer d'es d'on s'escriu al bloc
 */
int bwrite(uint bloc, const void *buff)
{
    int res = lseek(descriptor, bloc * TB, SEEK_SET);
    if (res < 0) {
        printf("[bloques.c] ERROR: No s'ha pogut posicionar (bwrite)\n");
        return -1;
    } else {
        // escriu en el bloc el contingut de buff
        res = write(descriptor, buff, TB);
        if (res < 0) {
            printf("[bloques.c] ERROR: No s'ha pogut escriure el bloc %d \n", bloc);
            return -1;
        } else {
            return 0;
        }
    }
}
