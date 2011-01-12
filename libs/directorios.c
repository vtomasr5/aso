//      directorios.c
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
 *  @file directorios.c
 *  @brief Conté les funcions sobre directoris de més alt nivell que interactuen amb el sistema de fitxers.
 *  S'implementen les funcions necessàries per a cercar les entrades dels directoris,
 *  crear o eliminar fitxers i llegir i escriure en el sistema de fitxers.
 *  @date 10/12/2010
 */

#include "directorios.h"

// per saber el nombre d'elements que té un array
#define length(x) (sizeof(x) / sizeof(x[0]))

// semàfor (mutex) encarregat d'assegurar exlusió mutua al SB, MB i AI.
int mutex;

/**
 *  Funció que estreu el camí de la ruta que se li passa com a paràmetre i que
 *  permet diferenciar entre fitxers i directoris.
 *  @param cami ruta completa cap a l'arxiu que es vol referenciar
 *  @param inicial primera part de la ruta compresa entre els dos primers '/'
 *  @param final la resta de la ruta
 *  @return tipus el tipus d'arxiu que fa referència la ruta completa
 */
int extreureCami(const char *cami, char *inicial, char *final)
{
    int i = 0;
    int j = 0;
    int tipus = 1; // tipus directori

    if (cami[i] != '/') {
        printf("[directorios.c] ERROR: Ruta incorrecta!\n");
        return -1;
    }

    i++;
    while ((cami[i] != '/') && (cami[i] != '\0')) { // obtenim lo que hi ha entre els dos primers '/'

        inicial[i-1] = cami[i];
        i++;
    }

    inicial[i] = '\0';

    while (cami[i] != '\0') { // obtenim el reste de la ruta
        final[j] = cami[i];
        i++;j++;
        tipus = 0; // tipus fitxer
    }
    final[j] = '\0';

    return tipus;
}

/**
 *  Funció que donat una ruta (cami_parcial) calcula els valor següents:
 *  l'inode del directori que conté el fitxer (p_inode_dir), el número de l'inode del propi fitxers (p_inode)
 *  i el número de l'entrada de directori del fitxer (p_entrada)
 *  @param cami_parcial Es la ruta del fitxer del qual volem trobar l'entrada.
 *  @param p_inode_dir Es el numero de l'inode del directori que conté el fitxer (el directori més proper)
 *  @param p_inode Es el numero de l'inode del fitxer
 *  @param p_entrada Es el número d'entrada de directori del fitxer
 */
int cercarEntrada(const char *cami_parcial, unsigned int *p_inode_dir, unsigned int *p_inode, unsigned int *p_entrada, char reservar)
{
    char cami_inicial[200]; // guardamos la primera cadena de la ruta sin '/'
    char cami_final[200]; // el resto de la ruta hasta el final
    memset(cami_inicial, '\0', 200);
    memset(cami_final, '\0', 200);
    int trobat = 0;
    int i = 0;
    STAT estat;

    if (strcmp(cami_parcial, "/") == 0) { // si buscamos el directorio raiz
        *p_inode_dir = 0;
        *p_inode = 0;
        return 0;
    }

    if (extreureCami(cami_parcial, cami_inicial, cami_final) == -1) {
        printf("[directorios.c] ERROR: Cami incorrecte\n");
        return -1;
    }

    printf("[directorios.c] DEBUG: cercarEntrada - camino inicial: %s, cami final: %s\n", cami_inicial, cami_final);

    mi_stat_f(*p_inode_dir, &estat); // llegim la informació de l'inode i la guardam en sa varible estat
    printf("[directorios.c] DEBUG: p_inode_dir = %d\n", *p_inode_dir);
    printf("[directorios.c] DEBUG: estat.tamany = %d\n", estat.tamany);

    int num_ent = estat.tamany / sizeof(entrada); // leemos todas las entradas del directorio

    entrada ent[num_ent]; // definimos un array de entradas de directorio que contiene el numero de entradas del inodo leido

    if (estat.tamany > 0) {
        int llegits = mi_read_f(*p_inode_dir, &ent, 0, num_ent * sizeof(entrada)); // leemos las entradas del directorio y las guardamos en el "buffer" ent
        printf("[directorios.c] DEBUG: bytes llegits de entrada (ent) = %d\n", llegits);
    }

    int k;
    for (k = 0; k < length(ent); k++) {
        printf("[directorios.c]  DEBUG: ----> Entrada directori: nom = %s || inode = %d\n", ent[k].nom, ent[k].inode);
    }

    printf("\n[directorios.c] DEBUG: cercarEntrada - numero d'entrades: %d\n", num_ent);

    while ((trobat == 0) && (i < num_ent)) { // se busca la entrada
        //printf("[directorios.c] DEBUG: cercarEntrada - dins while - *p_entrada : %d  , *p_inode : %d\n\n", i,*p_inode);
        if (strcmp(ent[i].nom, cami_inicial) == 0) {
            *p_entrada = i; // el número de su entrada dentro del último directorio que lo contiene
            *p_inode = ent[i].inode; // su número de inodo
             trobat = 1;
             printf("[directorios.c] DEBUG: cercarEntrada - dins if - ent[i].nom: %s == cami_inicial: %s\n", ent[i].nom, cami_inicial);
             printf("[directorios.c] DEBUG: cercarEntrada - dins if - *p_entrada: %d  , *p_inode: %d\n", *p_entrada, *p_inode);
        }
        i++;
    }

    if (trobat) {
        if ((strlen(cami_final) == 0) || (strcmp(cami_final, "/") == 0)) { // si solo queda una entrada en el camino (fichero o directorio)
            printf("[directorios.c] DEBUG: TROBAT - p_inode: %d , p_entrada: %d \n", *p_inode, *p_entrada);
            inode inod = llegirInode(*p_inode);

            if (((inod.tipus == 1) && (strcmp(cami_final, "/") == 0)) || ((inod.tipus == 2) && (strlen(cami_final) == 0))) {//es un fichero o directorio
                *p_inode = ent[i-1].inode;
                *p_entrada = i-1;
            }
        } else {
            *p_inode_dir = *p_inode; // el directorio donde hay que buscar la entrada, es el inodo encontrado
            return cercarEntrada(cami_final, p_inode_dir, p_inode, p_entrada, reservar);
        }
    } else { // no encontrado luego lo creamos ahora
       printf("[directorios.c] DEBUG: NO TROBAT entrada: %s\n", cami_inicial);
       if (reservar == '1') {
            printf("[directorios.c] DEBUG: reservar = %c\n", reservar);
            entrada entra;
            STAT estat2;
            uint tipus_inode;

            if (strlen(cami_final) == 0) {
                tipus_inode = 2; // fitxer
            } else {
                tipus_inode = 1; // directori
            }

            int r = 0;
            if ((r = reservarInode(tipus_inode, 7)) == -1) { // reservam els inodes de cada directori/fitxer del cami parcial
                return -1;
            }

            entra.inode = r;
            strcpy(entra.nom, cami_inicial); // copiam el camí a l'entrada de directori
            mi_stat_f(*p_inode_dir, &estat2); // obtenim la informació de l'inode

            int n_ent = estat2.tamany / sizeof(entrada); // calculam el numero d'entrades

            if (mi_write_f(*p_inode_dir, &entra, n_ent * sizeof(entrada), sizeof(entrada)) == -1) { // escrivim els canvis
                printf("[directorios.c] ERROR: No s'ha pogut escriure!\n");
                return -1;
            }

            *p_inode = r; // l'inode reservar es el seu inode (p_inode)
            *p_entrada = (estat2.tamany / sizeof(entrada)) + 1;
            printf("[directorios.c] DEBUG: inode reservat = %d, cami = %s, p_entrada = %u \n", r, cami_inicial, *p_entrada);

            if ((strlen(cami_final) == 0) || (strcmp(cami_final, "/") == 0)) {  // si hemos acabado o lo ultimo es una "/"
                return 0;
            } else {
                *p_inode_dir = *p_inode; // ara p_inode_dir es el directori que conté fitxers
                return cercarEntrada(cami_final, p_inode_dir, p_inode, p_entrada, reservar); // cridada recursiva
            }
        } else {
            return -1;
        }
    }
    return 0;
}

/**
 *  Funció que allibera memoria
 */
void alliberar(uint *p_inode_dir, uint *p_inode, uint *p_entrada)
{
    free(p_inode_dir);
    free(p_inode);
    free(p_entrada);
}

/**
 *  Funció que crea un fitxer o directori i sa seva respectiva entrada de directori.
 *  A més ho crea amb uns permisos especificats per paràmetre.
 *  @param cami ruta que es vol crei
 *  @param mode permisos amb que es crea el fitxer o directori
 */
int mi_creat(const char *cami, unsigned int mode)
{
    uint *p_inode_dir, *p_inode, *p_entrada;
    int num_inode;

    p_inode_dir = malloc(sizeof(uint));
    p_inode = malloc(sizeof(uint));
    p_entrada = malloc(sizeof(uint));

    *p_inode_dir = 0;
    *p_inode = 0;
    *p_entrada = 0;

    // realment com que no troba l'entrada de directori, la crea
    if (cercarEntrada(cami, p_inode_dir, p_inode, p_entrada, '1') != -1) {
        printf("[directorios.c] DEBUG: mi_creat - Cami %s creat\n", cami);
        num_inode = *p_inode;

        //modificam els permisos del fitxer o directori
        inode in = llegirInode(num_inode);
        in.permisos = mode;

        if (escriureInode(num_inode, in) == -1) {
            return -1;
        }
    } else {
        printf("[directorios.c] DEBUG: No s'ha trobat l'entrada!\n");
        alliberar(p_inode_dir, p_inode, p_entrada);
        return -1;
    }

    alliberar(p_inode_dir, p_inode, p_entrada);
    return 0;
}

/**
 *  Funció que crea un enllaç des de l'inode de cami1 (no ha d'existir) a
 *  l'inode especificat per cami2, de tal manera que al acabar l'execució
 *  els dos camins facin referència al mateix inode.
 *  @param cami1 ruta que enllaçada a cami2
 *  @param cami2 ruta enllaçada a l'inode de cami1
 */
int mi_link(const char *cami1, const char *cami2)
{
    uint *p_inode_dir, *p_inode, *p_entrada;
    uint num_inode = 0;
    uint aux = 0;
    entrada ent;
    STAT estat;

    p_inode_dir = malloc(sizeof(uint));
    p_inode = malloc(sizeof(uint));
    p_entrada = malloc(sizeof(uint));

    *p_inode_dir = 0;
    *p_inode = 0;
    *p_entrada = 0;

    if (cercarEntrada(cami2, p_inode_dir, p_inode, p_entrada, '1') == -1) {
        printf("[directorios.c] ERROR: No s'ha trobat l'entrada!\n");
        alliberar(p_inode_dir, p_inode, p_entrada);
        return -1;
    }
    num_inode = *p_inode; // num_inode conté l'inode que apunta al directori desitjat

    printf("[directorios.c] mi_link DEBUG: *p_inode_dir = %d, *p_inode = %d, *p_entrada = %d\n", *p_inode_dir, *p_inode, *p_entrada);

    inode inod = llegirInode(num_inode);
    inod.links_directoris++;
    if (escriureInode(num_inode, inod) == -1) {
        return -1;
    }

    *p_inode_dir = 0;
    *p_inode = 0;
    *p_entrada = 0;

    if (cercarEntrada(cami1, p_inode_dir, p_inode, p_entrada, '1') == -1) {
        printf("[directorios.c] ERROR: El link no s'ha pogut realitzar!!\n");
        alliberar(p_inode_dir, p_inode, p_entrada);
        return -1;
    }
    aux = *p_inode; // inode crear que s'ha d'eliminar després d'haver fet el link a l'altra inode

    printf("[directorios.c] mi_link DEBUG: *p_inode_dir = %d, *p_inode = %d, *p_entrada = %d, num_inode = %d\n", *p_inode_dir, *p_inode, *p_entrada, num_inode);

    mi_stat_f(*p_inode_dir, &estat);

    printf("[directorios.c] mi_link DEBUG Estat.tamany = %d\n", estat.tamany);
    printf("[directorios.c] mi_link DEBUG antes de leer: ent.inode = %d\n", ent.inode);

    int bllegits = mi_read_f(*p_inode_dir, &ent, (*p_entrada) * sizeof(entrada), sizeof(entrada));
    if (bllegits == -1) {
        printf("[directorios.c] ERROR: Error de lectura a mi_read_f()\n");
        return -1;
    }
    ent.inode = num_inode; // cambiamos su numero de inodo por la entrada del cami2

    printf("[directorios.c] DEBUG: bytes llegits = %d\n", bllegits);

    printf("[directorios.c] mi_link DEBUG Estat.tamany = %d\n", estat.tamany);
    printf("[directorios.c] mi_link DEBUG: despres de llegir/abans d'escriure: ent.nom = %s, ent.inode = %d\n", ent.nom, ent.inode);

    int bescrits = mi_write_f(*p_inode_dir, &ent, (*p_entrada) * sizeof(entrada), sizeof(entrada));
    if  (bescrits == -1) {
        printf("[directorios.c] ERROR: Error d'escriptura a mi_write_f()\n");
        return -1;
    }

    printf("[directorios.c] DEBUG: bytes escrits = %d\n", bescrits);
    printf("[directorios.c] mi_link DEBUG despues de escribir: ent.nom = %s, ent.inode = %d\n", ent.nom, ent.inode);
    alliberar(p_inode_dir, p_inode, p_entrada); // alliberam memoria

    alliberarInode(aux, 0); // alliberam l'inode ja que ara apunta a un altra inode

    printf("[directorios.c] DEBUG: mi_link realitzat correctament.\n");
    return 0;
}

/**
 *  Funció que elimina l'entrada de directori. Si només hi ha un enllaç elimina
 *  el propi fitxer o directori.
 *  @param cami Ruta que ha de borrar.
 */
int mi_unlink(const char *cami)
{
    uint *p_inode_dir, *p_inode, *p_entrada;
    entrada ent;
    int res_tipus = 0;
    inode in;

    p_inode_dir = malloc(sizeof(uint));
    p_inode = malloc(sizeof(uint));
    p_entrada = malloc(sizeof(uint));

    *p_inode_dir = 0;
    *p_inode = 0;
    *p_entrada = 0;

    if (strcmp(cami, "/") == 0) { // caso de eliminar "/"
        printf("[directorios.c] ERROR: No pots borrar l'arrel '/'\n");
        alliberar(p_inode_dir, p_inode, p_entrada);
        return -1;
    }

    res_tipus = cercarEntrada(cami, p_inode_dir, p_inode, p_entrada, '0');
    if (res_tipus == -1) {
        printf("[directorios.c] ERROR: No s'ha pogut trobar l'entrada!\n");
        alliberar(p_inode_dir, p_inode, p_entrada);
        return -1;
    }

    // leemos el directorio
    in = llegirInode(*p_inode_dir);

    // si no tiene enlaces, o bien es un fichero o bien es un directorio sin links
    if (in.links_directoris == 1) {
        mi_read_f(*p_inode_dir, &ent, in.tamany - sizeof(entrada), sizeof(entrada)); // in.tam - sizeof(struct entrada) = numero de bytes de offset
        mi_write_f(*p_inode_dir, &ent, *p_entrada * sizeof(entrada), sizeof(entrada));
        mi_truncar_f(*p_inode_dir, in.tamany - sizeof(entrada));

        alliberarInode(*p_inode, 1); // lo eliminamos completamente

        printf("[directorios.c] INFO: El cami '%s' s'ha borrat correctament.\n", cami);
        alliberar(p_inode_dir, p_inode, p_entrada);
        return 0;
    } else if (in.links_directoris > 1) { // si tiene mas de 1 enlace de directorio (es un directorio)
        inode in2;
        char inicial[60];
        memset(inicial, '\0', 60);
        int llegits = 0;

        // llegim l'inode
        in2 = llegirInode(*p_inode);

        in2.links_directoris--;
        if (escriureInode(*p_inode, in2) == -1) {
            return -1;
        }
        printf("[directorios.c] DEBUG: in2.tamany = %d\n", in2.tamany);

        // mientras queden directorios y ficheros dentro del directorio que tratamos, los eliminamos
        while (in2.tamany > 0) {
            strcpy(inicial, cami);
            llegits = mi_read_f(*p_inode, &ent, 0 * sizeof(entrada), sizeof(entrada));
            strcat(inicial, ent.nom);
            in2 = llegirInode(ent.inode);

            if (in2.tipus == 1) {  // si es directorio
                strcat(inicial, "/");
            }

            // borramos las entradas contenidas en el directorio
            if (mi_unlink(inicial) == -1) {
                alliberar(p_inode_dir, p_inode, p_entrada);
                return -1;
            }

            in2 = llegirInode(*p_inode);
        }

        mi_read_f(*p_inode_dir, &ent, in.tamany - sizeof(entrada), sizeof(entrada));
        mi_write_f(*p_inode_dir, &ent, *p_entrada * sizeof(entrada), sizeof(entrada));
        mi_truncar_f(*p_inode_dir, in.tamany - sizeof(entrada));

        alliberarInode(*p_inode, 1);

        printf("[directorios.c] INFO: El cami '%s' s'ha borrat correctament.\n", cami);

        //alliberar(p_inode_dir, p_inode, p_entrada);
        //return 0;
    }

    printf("[directorios.c] INFO: El cami '%s' s'ha borrat correctament.\n", cami);
    alliberar(p_inode_dir, p_inode, p_entrada);
    return 0;
}

/**
 *  Funció que escriu el contingut del directori 'cami' dins el buffer.
 *  @param cami Ruta que ha de llegir
 *  @param buffer Buffer on ha de guardar la informació.
 *  @return El nombre de fitxers que hi ha dins el directori
 */
int mi_dir(const char *cami, char *buff) {
    uint p_inode_dir, p_inode, p_entrada;
    int longitut = strlen(cami);
    int bytes_llegits = 0;
    char num_bytes[40];

    entrada ent;
    inode in, aux;

    memset(buff, '\0', BUFFER_DIR); // resetejam
    p_inode_dir = p_inode = p_entrada = 0;

    if (cercarEntrada(cami, &p_inode_dir, &p_inode, &p_entrada, '0') == -1) {
        printf("[directorios.c] ERROR: Entrada no trobada!!\n");
        return -1;
    }

    if (cami[longitut-1] == '/') { // si es un directori
        in = llegirInode(p_inode);

        printf("[directorios.c] DEBUG: in.tamany = %d \n", in.tamany);

        while (bytes_llegits < in.tamany) {
            int res = mi_read_f(p_inode, &ent, bytes_llegits, sizeof(entrada));
            if (res == -1) {
                printf("[directorios.c] ERROR: Error de lectura a mi_read_f()\n");
                return -1;
            }

            if (ent.inode > 0) {
                aux = llegirInode(ent.inode);

                if (aux.tipus != 0) {
                    strcat(buff,"    ");
                    strcat(buff, ent.nom);

                    if (aux.tipus == 1) {
                        strcat(buff, "/ (d)");
                    } else if (aux.tipus == 2) {
                        strcat(buff, " (f)");
                        memset(num_bytes, '\0', 40);
                        sprintf(num_bytes, ", Tamany = %d bytes", aux.tamany); // escrivim dins num_bytes
                        strcat(buff, num_bytes);
                    }

                    strcat(buff, ":");
                }
                bytes_llegits += sizeof(entrada);
            } else {
                return 0;
            }
        }
    } else {
        printf("[directorios.c] ERROR: Això no es un directori!\n");
        return -1;
    }
    // retornam el numero d'entrades que conté el directori
    return bytes_llegits / sizeof(entrada);
}

/**
 *  Funció que canvia els permisos a una ruta especificada per paràmetre.
 *  @param cami ruta que es vol modificar
 *  @param mode permisos que es volen assignar a la ruta
 */
int mi_chmod(const char *cami, unsigned char mode)
{
    uint p_inode, p_entrada, p_inode_dir = 0;

    if (cercarEntrada(cami, &p_inode_dir, &p_inode, &p_entrada, '0') == -1) { // busca el inodo de la ultima entrada de la ruta y la deposita en p_inode
        printf("[directorios.c] ERROR: No s'ha trobat el cami!!\n");
        return -1;
    }

    if (mi_chmod_f(p_inode, mode) == -1) { // camviam els permisos a l'inode
        return -1;
    }

    return 0;
}

/**
 *  Funció que recull la metainformació del l'inode del camí que se li passa per paràmetre.
 *  @param cami ruta que ha de consultar
 *  @param p_stat esctructura on es guarda la informació del l'inode
 */
int mi_stat(const char *cami, STAT *p_stat)
{
    uint p_inode, p_entrada, p_inode_dir = 0;

    if (cercarEntrada(cami, &p_inode_dir, &p_inode, &p_entrada, '0') == -1) {
        printf("[directorios.c] ERROR: No s'ha trobat el cami!!\n");
        return -1;
    }

    mi_stat_f(p_inode, p_stat);

    return 0;
}

/**
 *  Funció que llegeix la informació d'un fitxer o directori del sistema de fitxers i l'escriu en el buffer.
 *  @param cami ruta que d'on llegeix la informació
 *  @param buff buffer on guarda la informació
 *  @param offset nombre de bytes de desplaçament
 *  @param nbytes nombre de bytes que ha de llegir
 *  @return nombre de bytes que ha llegit
 */
int mi_read(const char *cami, void *buff, unsigned int offset, unsigned int nbytes)
{
    uint p_inode, p_entrada, p_inode_dir = 0;

    if (cercarEntrada(cami, &p_inode_dir, &p_inode, &p_entrada, '0') == -1) {
        printf("[directorios.c] ERROR: No s'ha trobat el cami!!\n");
        return -1;
    }

    int llegits = mi_read_f(p_inode, buff, offset, nbytes);
    if (llegits == -1) {
        return -1;
    }

    return llegits;
}

/**
 *  Funció que llegeix la informació d'un fitxer o directori que hi ha dins el buffer
 *  i l'escriu en el sistema de fitxers.
 *  @param cami ruta que d'on llegeix la informació
 *  @param buff buffer on guarda la informació
 *  @param offset nombre de bytes de desplaçament
 *  @param nbytes nombre de bytes que ha d'escriure
 *  @return nombre de bytes que ha escrits
 */
int mi_write(const char *cami, const void *buff, unsigned int offset, unsigned int nbytes)
{
    uint p_inode, p_entrada, p_inode_dir = 0;

    if (cercarEntrada(cami, &p_inode_dir, &p_inode, &p_entrada, '0') == -1) {
        printf("[directorios.c] ERROR: No s'ha trobat el cami!!\n");
        return -1;
    }

    int escrits = mi_write_f(p_inode, buff, offset, nbytes);
    if (escrits == -1) {
        return -1;
    }

    return escrits;
}

/**
 *  Funció que mostra per pantalla el contingut d'un directori
 *  @param cami ruta que es vol mostrar el contingut
 *  @param buff buffer on hi ha la informació.
 */
int mi_lsdir(const char *cami, char *buff)
{
    int cont = 0;

    for(cont = 0; cont < 2; cont++) {
        printf("\n");
    }

    printf("##### Contingut de '%s' #####\n", cami);

    // imprimim el contingut del directori per pantalla
    for (cont = 0; cont < BUFFER_DIR; cont++) {
        if (buff[cont] == ':') {
            printf("\n");
        } else {
            printf("%c", buff[cont]);
        }
    }

    for(cont = 0; cont < 2; cont++) {
        printf("\n");
    }

    return 0;
}
