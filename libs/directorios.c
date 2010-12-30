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
    //printf("\n[Directorios.c] i = %d\n",i);
    /*
    camino = "/dir1/dir2/fichero"
    inicial = "dir1" (devuelve DIRECTORIO)
    final = "/dir2/fichero"

    camino = "/dir/"
    inicial = "dir" (devuelve DIRECTORIO) =  tipus 1
    final = "/"

    camino = "/fichero"
    inicial = "fichero" (devuelve FICHERO) = tipus 2
    final = ""
    */

    if (trobat) {
        if ((strlen(cami_final) == 0) || (strcmp(cami_final, "/") == 0)) { // si solo queda una entrada en el camino (fichero o directorio)
            printf("[Directorios.c] Trobat - p_inode : %d | p_entrada: %d \n", *p_inode, *p_entrada);
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
    return 0;
}

int mi_unlink(const char *cami)
{
    return 0;
}

int mi_dir(const char *cami, char *buffer)
{
    return 0;
}

int mi_chmod(const char *cami, unsigned char mode)
{
	unsigned int p_inode, p_entrada, p_inode_dir = 0;
	
	if (cercarEntrada(cami, &p_inode_dir, &p_inode, &p_entrada, 0) != -1) { // busca el inodo de la ultima entrada de la ruta y la deposita en p_inode
		if (mi_chmod_f(p_inode, mode) == -1) { // cambiamos los permisos al inodo
		    return -1;
		}
		return 0;
	} else { // no troba s'entrada
	    return -1;
	}
}

/*
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

    printf("camino parcial:%s\n", cami_parcial);

    STAT estat;
    mi_stat_f(*p_inode_dir, &estat); // llegim la informació de l'inode i la guardam en sa varible estat
    int num_ent = estat.tamany/sizeof(entrada);     //leemos todas las entradas del directorio
    entrada ent[num_ent]; // definimos un array de entradas de directorio que contiene el numero de entradas del inodo leido
    mi_read_f(*p_inode_dir, &ent, 0, num_ent * sizeof(entrada)); // leemos las entradas del directorio y las guardamos en el "buffer" ent

    while ((trobat == 0) && (i < num_ent)) { //se busca la entrada
        printf("[directorios.c] DEBUG: cercarEntrada - dins while - ent[i] %s, cami_inicial %s\n", ent[i].nom, cami_inicial);
        if (strcmp(ent[i].nom, cami_inicial) == 0) {
            *p_entrada = i; // El número de su entrada dentro del último directorio que lo contiene
            *p_inode = ent[i].inode; // Su número de inodo
            trobat = 1;
        }
        i++;
    }

    camino = "/dir1/dir2/fichero"
    inicial = "dir1" (devuelve DIRECTORIO)
    final = "/dir2/fichero"

    camino = "/dir/"
    inicial = "dir" (devuelve DIRECTORIO) =  tipus 1
    final = "/"

    camino = "/fichero"
    inicial = "fichero" (devuelve FICHERO) = tipus 2
    final = ""

    if (trobat) {
        if ((strlen(cami_final) == 0) || (strcmp(cami_final, "/") == 0)) { // si solo queda una entrada en el camino (fichero o directorio)
            inode inod = llegirInode(*p_inode); // llegim s'inode del fitxer o del directori que hem trobat

            if (((inod.tipus == 1) && (strcmp(cami_final, "/") == 0)) || ((inod.tipus == 2) && (strlen(cami_final) == 0))) { //es un fichero o directorio
                i = i - 1; // corregimos la variable i, ya que al salir del while trobat=true, se incrementa una vez mas de la que toca.
                *p_inode = ent[i].inode; //  numero del inodo del fichero
                *p_entrada = i; // numero de su entrada dentro del último directorio que lo contiene
                return 0;
            }
        } else {
            *p_inode_dir = *p_inode;
            return cercarEntrada(cami_final, p_inode_dir, p_inode, p_entrada, reservar);
        }
    } else { // no encontrado luego lo creamos ahora
        if (reservar) {
            entrada entra;
            STAT estat2;
            int tipus_inode;

            if (strlen(cami_final) == 0) {
                tipus_inode = 2;
            } else {
                tipus_inode = 1;
            }

            if ((entra.inode == reservarInode(tipus_inode, 7)) == -1) {
                return -1;
            }

            strcpy(entra.nom, cami_inicial); // copiam el camí a l'entrada de directori
            mi_stat_f(*p_inode_dir, &estat2); // obtenim la informació de l'inode
            int n_ent = estat2.tamany / sizeof(entrada); // calculam el numero d'entrades
            mi_write_f(*p_inode_dir, &entra, n_ent * sizeof(entrada), sizeof(entrada)); // escrivim els canvis

            printf("[directorios.c] DEBUG: Tamany de l'offset: %lu\n", sizeof(n_ent * sizeof(entrada)));

            *p_inode = reservar;
            *p_entrada = estat2.tamany / sizeof(entrada);

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
*/
