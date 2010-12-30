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
 *  @brief Conté els funcions sobre directoris de més alt nivell que interactuen amb el sistema de fitxers.
 *  S'implementen les funcions necessàries per a cercar les entrades dels directoris,
 *  crear o eliminar fitxers i llegir i escriure en el sistema de fitxers.
 *  @date 10/12/2010
 */

#include "directorios.h"

// per saber el nombre d'elements que té un array
#define length(x) (sizeof(x)/sizeof(x[0]))

/*
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

/*
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
    int trobat = 0;
    int i = 0;

    if (strcmp(cami_parcial, "/") == 0) { // si buscamos el directorio raiz
        *p_inode_dir = 0;
        *p_inode = 0;
        return 0;
    }

    if (extreureCami(cami_parcial, cami_inicial, cami_final) == -1) {
        printf("[directorios.c] ERROR: Camí incorrecte\n");
        return -1;
    }

    printf("-------------  [Directorios.c] cercarEntrada - camino parcial:%s   -------------\n", cami_parcial);
    printf("[directorios.c] cercarEntrada - camino inicial: %s, cami final: %s\n", cami_inicial, cami_final);
    STAT estat;

    mi_stat_f(*p_inode_dir, &estat); // llegim la informació de l'inode i la guardam en sa varible estat

    int num_ent = estat.tamany/sizeof(entrada);     //leemos todas las entradas del directorio

    entrada ent[num_ent]; // definimos un array de entradas de directorio que contiene el numero de entradas del inodo leido

    if (mi_read_f(*p_inode_dir, &ent, 0, num_ent * sizeof(entrada)) == -1) { // leemos las entradas del directorio y las guardamos en el "buffer" ent
        return -1;
    }

    int k;
    for (k = 0; k < length(ent); k++) {
        printf("---->entrada directorio: %s || %d\n", ent[k].nom, ent[k].inode);
    }
    printf("\n[Directorios.c] cercarEntrada - num_ent :%d\n", num_ent);

    while ((trobat == 0) && (i < num_ent)) { //se busca la entrada
        //printf("[Directorios.c] DEBUG: cercarEntrada - dins while - *p_entrada : %d  , *p_inode : %d\n\n", i,*p_inode);
        if (strcmp(ent[i].nom, cami_inicial) == 0) {
            *p_entrada = i; // El número de su entrada dentro del último directorio que lo contiene
            *p_inode = ent[i].inode; // Su número de inodo
             trobat = 1;
             printf("[Directorios.c] DEBUG: cercarEntrada - dins if - ent[i] %s, cami_inicial %s\n", ent[i].nom, cami_inicial);
             printf("[Directorios.c] DEBUG: cercarEntrada - dins if - *p_entrada : %d  , *p_inode : %d\n\n", i,*p_inode);
        }
        i++;
    }

    if (trobat) {
        if ((strlen(cami_final) == 0) || (strcmp(cami_final, "/") == 0)) { // si solo queda una entrada en el camino (fichero o directorio)
            printf("[Directorios.c] Trobat - p_inode : %d | p_entrada: %d \n", *p_inode, *p_entrada);
            inode inod=llegirInode(*p_inode);
            if(((inod.tipus==1) && (strcmp(cami_final,"/")==0)) || ((inod.tipus==2) &&(strlen(cami_final)==0))){//es un fichero o directorio
                *p_inode=ent[i-1].inode;
                *p_entrada = i-1;
            }
            return 0;
        } else {
            *p_inode_dir = *p_inode; // el directorio donde hay que buscar la entrada, es el inodo encontrado
            return cercarEntrada(cami_final, p_inode_dir, p_inode, p_entrada, reservar);
        }
    } else { // no encontrado luego lo creamos ahora
       printf("----------------------------------------------------------------\n");
       printf("[Directorios.c] No Trobat\n");
       if (reservar) {
            entrada entra;
            STAT estat2;
            int tipus_inode;

            if (strlen(cami_final) == 0) {
                tipus_inode = 2;
            } else {
                tipus_inode = 1;
            }

            int r = 0;
            if ((r = reservarInode(tipus_inode, 7)) == -1) { // Aqui esta el error
                return -1;
            }

            entra.inode = r;
            strcpy(entra.nom, cami_inicial); // copiam el camí a l'entrada de directori
            mi_stat_f(*p_inode_dir, &estat2); // obtenim la informació de l'inode
            int n_ent = estat2.tamany / sizeof(entrada); // calculam el numero d'entrades
            mi_write_f(*p_inode_dir, &entra, n_ent * sizeof(entrada), sizeof(entrada)); // escrivim els canvis



            *p_inode = entra.inode;
            *p_entrada = estat2.tamany / sizeof(entrada);
             printf("[directorios.c] DEBUG: num de linode que cream:  cami: %s num : %d\n", cami_inicial, r);
            if ((strlen(cami_final) == 0) || (strcmp(cami_final, "/") == 0)) {  // si hemos acabado o lo ultimo es una "/"
                return 0;
            } else {
                *p_inode_dir = *p_inode;
                return cercarEntrada(cami_final, p_inode_dir, p_inode, p_entrada, reservar);
            }
        } else {
            return -1;
        }
    }
    return 0;
}

int mi_creat(const char *cami, unsigned int mode)
{
    uint *p_inode_dir, *p_inode, *p_entrada;

    p_inode_dir = malloc(sizeof(int));
    *p_inode_dir = 0;
    p_inode = malloc(sizeof(int));
    *p_inode=0;
    p_entrada = malloc(sizeof(int));
    *p_entrada=0;

    // realment com que no troba l'entrada de directori, la crea
    if (cercarEntrada(cami, p_inode_dir, p_inode, p_entrada, 1) != -1) {
        printf("[Directorios.c] DEBUG: mi_creat - Cami %s creat\n", cami);
/*
        //modificam els permisos del fitxer o directori
        inode in = llegirInode(p_inode);
        in.permisos = mode;
        escriureInode(p_inode, in);
        */
    }
    return 0;
}

int mi_link(const char *cami1, const char *cami2)
{
    uint *p_inode_dir, *p_inode, *p_entrada;
    uint num_inode;
    entrada ent;
    STAT estat;

    p_inode_dir = malloc(sizeof(int));
    *p_inode_dir = 0;
    p_inode = malloc(sizeof(int));
    *p_inode = 0;
    p_entrada = malloc(sizeof(int));
    *p_entrada = 0;

    if (cercarEntrada(cami2, p_inode_dir, p_inode, p_entrada, 0) == -1) {
        printf("[directorios.c] ERROR: El link no s'ha pogut realitzar!\n");
        return -1;
    }

    num_inode = *p_inode; // copia puntero del inodo encontrado
    inode inod = llegirInode(num_inode);
    inod.links_directoris++;
    escriureInode(num_inode, inod);

    *p_inode_dir = 0;
    *p_inode = 0;
    *p_entrada = 0;

    if (cercarEntrada(cami1, p_inode_dir, p_inode, p_entrada, 1) == -1) {
        printf("[directorios.c] ERROR: El link no s'ha pogut realitzar!!\n");
        return -1;
    }
    alliberarInode(*p_inode, 0, 0); // MAL

    mi_stat_f(*p_inode_dir, &estat);

    mi_read_f(*p_inode_dir, &ent, (*p_entrada) * sizeof(entrada), sizeof(entrada));

    ent.inode = num_inode;

    mi_write_f(*p_inode_dir, &ent, (*p_entrada) * sizeof(entrada), sizeof(entrada));

    printf("[directorios.c] DEBUG: mi_link realitzat correctament.\n");
    return 0;
}

int mi_unlink(const char *cami)
{
    uint *p_inode_dir, *p_inode, *p_entrada;
    STAT estat;
    entrada ent;

    p_inode_dir = malloc(sizeof(int));
    *p_inode_dir = 0;
    p_inode = malloc(sizeof(int));
    *p_inode = 0;
    p_entrada = malloc(sizeof(int));
    *p_entrada = 0;

    if (cercarEntrada(cami, p_inode_dir, p_inode, p_entrada, 0) == -1) {
        return -1;
    }

    inode inod = llegirInode(*p_inode);
    if (inod.links_directoris == 1) {
        printf("[directorioc.c] INFO: Només queda un enllaç, borram el fitxer.\n");
        alliberarInode(*p_inode, 0, 0);
        //printf("Camino\n*p_inodo_dir=%d\n*p_inodo=%d\n*p_entrada=%d\n",*p_inodo_dir,*p_inodo,*p_entrada);
        mi_stat_f(*p_inode_dir, &estat);

        if ((*p_entrada + 1) * sizeof(entrada) == estat.tamany) {
            mi_truncar_f(*p_inode_dir, estat.tamany - sizeof(entrada));
        } else {
            mi_read_f(*p_inode_dir, &ent, estat.tamany - sizeof(entrada), sizeof(entrada));
            mi_truncar_f(*p_inode_dir, estat.tamany - sizeof(entrada));
            mi_write_f(*p_inode_dir, &ent, sizeof(entrada) * (*p_entrada), sizeof(entrada));
        }
    } else {
        inod.links_directoris--;
        escriureInode(*p_inode, inod);
        //printf("Camino\n*p_inodo_dir=%d\n*p_inodo=%d\n*p_entrada=%d\n",*p_inodo_dir,*p_inodo,*p_entrada);
        mi_stat_f(*p_inode_dir, &estat);

        if ((*p_entrada + 1) * sizeof(entrada) == estat.tamany) {
            mi_truncar_f(*p_inode_dir, estat.tamany - sizeof(entrada));
        } else {
            mi_read_f(*p_inode_dir, &ent, estat.tamany - sizeof(entrada), sizeof(entrada));
            mi_truncar_f(*p_inode_dir, estat.tamany - sizeof(entrada));
            mi_write_f(*p_inode_dir, &ent, sizeof(entrada) * (*p_entrada), sizeof(entrada));
        }
    }
    printf("[directorios.c] DEBUG: mi_unlink realitzat correctament.\n");
    return 0;
}

int mi_dir(const char *cami, char *buffer)
{
    int longitut = strlen(cami);
    STAT estat;
    int pinode;

    if (longitut > 0) {
        if (cami[longitut-1] == '/') { // es un directori
            uint *p_inode_dir, *p_inode, *p_entrada;

            p_inode_dir = malloc(sizeof(int));
            *p_inode_dir = 0;
            p_inode = malloc(sizeof(int));
            *p_inode = 0;
            p_entrada = malloc(sizeof(int));
            *p_entrada = 0;

            if (cercarEntrada(cami, p_inode_dir, p_inode, p_entrada, 0) == -1) {   //no ha encontrado el directorio
                return -1;
            }

            if (strcmp(cami, "/") == 0) { //si es un directorio que no sea el raiz
                pinode = *p_inode_dir;
            } else {
                pinode = *p_inode;  //si el directorio a listar es el raiz
            }

            if (mi_stat_f(pinode, &estat) == -1) {
                return -1;
            }

            int n_entrades = (estat.tamany / sizeof(entrada));
            if (n_entrades > 0) {   //si hay entradas de directorio
                entrada ent[n_entrades];
                if (mi_read_f(pinode, &ent, 0, n_entrades * sizeof(entrada)) == -1) {    //leemos todas las entradas
                    return -1;
                }

                int i;
                for (i = 0; i < n_entrades; i++) {
                    strcat(buffer, ent[i].nom);
                    strcat(buffer, ":");
                }
                return i;
            } else {
                return 0;
            }
        } else {
            printf("[directorios.c] ERROR: Això no es un directori!\n");
            return -1;
        }
    }
    return 0;
}

int mi_chmod(const char *cami, unsigned char mode)
{
    uint p_inode, p_entrada, p_inode_dir = 0;

    if (cercarEntrada(cami, &p_inode_dir, &p_inode, &p_entrada, 0) != -1) { // busca el inodo de la ultima entrada de la ruta y la deposita en p_inode
        if (mi_chmod_f(p_inode, mode) == -1) { // cambiamos los permisos al inodo
            return -1;
        }
    } else { // no troba s'entrada
        printf("[directorios.c] ERROR: Entrada no trobada.");
        return -1;
    }
    return 0;
}

int mi_stat(const char *cami, STAT *p_stat)
{
    uint p_inode, p_entrada, p_inode_dir = 0;

    if (cercarEntrada(cami, &p_inode_dir, &p_inode, &p_entrada, 0) == -1) {
        printf("[directorios.c] ERROR: No s'ha trobat el camí!!\n");
        return -1;
    }

    mi_stat_f(p_inode, p_stat);

    return 0;
}

int mi_read(const char *cami, void *buff, unsigned int offset, unsigned int nbytes)
{
    uint p_inode, p_entrada, p_inode_dir = 0;

    if (cercarEntrada(cami, &p_inode_dir, &p_inode, &p_entrada, 0) == -1) {
        printf("[directorios.c] ERROR: No s'ha trobat el camí!!\n");
        return -1;
    }

    mi_read_f(p_inode, buff, offset, nbytes);

    return 0;
}

int mi_write(const char *cami, const void *buff, unsigned int offset, unsigned int nbytes)
{
    uint p_inode, p_entrada, p_inode_dir = 0;

    if (cercarEntrada(cami, &p_inode_dir, &p_inode, &p_entrada, 0) == -1) {
        printf("[directorios.c] ERROR: No s'ha trobat el camí!!\n");
        return -1;
    } else {
        int escrito = mi_write_f(p_inode, buff, offset, nbytes);
        return escrito;
    }
    return 0;
}

int mi_lsdir(const char *cami, char *buff)
{
    int cont = 0;

    for(cont = 0; cont < 14; cont++) {
        printf("\n");
    }

    printf("Contingut de: %s\n", cami);
    printf("----------------------------------------\n");

    // imprimimos el contenido del directorio
    for (cont = 0; cont < BUFFER_DIR; cont++) {
        if (buff[cont] == ':') {
            printf("\n");
        } else {
            printf("%c", buff[cont]);
        }
    }

    for(cont = 0; cont < 5; cont++) {
        printf("\n");
    }

    return 0;
}
