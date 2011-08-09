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

#include "../include/directorios.h"
#include "../include/semaforos.h"

// per saber el nombre d'elements que té un array
#define length(x) (sizeof(x) / sizeof(x[0]))

// semafor mutex per controlar l'acces concurrent
static int mutex;

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
 *  @param reservar Si val '1' se crea una nova entrada i, si val '0' només consulta
 *  @param mode Permisos per crear una nova entrada de directori
 */
int cercarEntrada(const char *cami_parcial, unsigned int *p_inode_dir, unsigned int *p_inode, unsigned int *p_entrada, unsigned int reservar, int mode)
{
    char cami_inicial[200]; // guardamos la primera cadena de la ruta sin '/'
    char cami_final[200]; // el resto de la ruta hasta el final
    memset(cami_inicial, '\0', 200);
    memset(cami_final, '\0', 200);
    int i = 0;

    if (strcmp(cami_parcial, "/") == 0) { // si buscamos el directorio raiz
        *p_inode_dir = 0;
        *p_inode = 0;
        return 0;
    }

    if (extreureCami(cami_parcial, cami_inicial, cami_final) == -1) {
        printf("[directorios.c] ERROR: Cami incorrecte!\n");
        return -1;
    }

    STAT estat;
    mi_stat_f(*p_inode_dir, &estat); // llegim la informació de l'inode i la guardam en sa varible estat

    int num_ent = estat.tamany / sizeof(entrada); // leemos todas las entradas del directorio

    entrada ent[num_ent]; // definimos un array de entradas de directorio que contiene el numero de entradas del inodo leido
    if (estat.tamany != 0) { // si hi ha entrades
        mi_read_f(*p_inode_dir, &ent, 0, estat.tamany); // leemos las entradas del directorio y las guardamos en el "buffer" ent

        //~ // inici debug
        //~ int k;
        //~ for (k = 0; k < length(ent); k++) { // mostram les entrades per pantalla
            //~ printf("[directorios.c] DEBUG: ----> Entrada directori: nom = '%s' || inode = '%d'\n", ent[k].nom, ent[k].inode);
        //~ }
        //~ // fi debug

        while ((strcmp(ent[i].nom, cami_inicial) != 0) && (i < num_ent)) { // anam comparant les entrades del directori amb el cami inicial per veure si existeix
            i++;
        }
    } else { // no trobat (no hi ha entrades)
        i = num_ent;
    }

    if (i < num_ent) { // trobat
        *p_entrada = i;
        *p_inode = ent[i].inode;

        if ((strlen(cami_final) == 0) || (strcmp(cami_final, "/") == 0)) { // si solo queda una entrada en el camino (fichero o directorio)
            return 0;
        } else {
            *p_inode_dir = *p_inode;
            return cercarEntrada(cami_final, p_inode_dir, p_inode, p_entrada, reservar, mode);
        }
    } else { // no encontrado luego lo creamos ahora
       if (reservar) {
            entrada entra;
            memset(entra.nom, '\0', 60);
            uint tipus_inode;

            if (strlen(cami_final) == 0) {
                tipus_inode = 2; // fitxer
            } else {
                tipus_inode = 1; // directori
            }

            // reservam els inodes de cada directori/fitxer del cami parcial
            int r = reservarInode(tipus_inode, 7);
            if (r == -1) {
                printf("[directorios.c] ERROR: No s'ha pogut reservat l'inode\n");
                return -1;
            }

            entra.inode = r; // entrada de directori de l'inode reservat
            strcpy(entra.nom, cami_inicial); // copiam el camí a l'entrada de directori

            if (mi_write_f(*p_inode_dir, &entra, num_ent * sizeof(entrada), sizeof(entrada)) == -1) { // escrivim els canvis
                printf("[directorios.c] ERROR: No s'ha pogut escriure!\n");
                return -1;
            }

            *p_inode = r; // l'inode reservar es el seu inode (p_inode)
            *p_entrada = (estat.tamany / sizeof(entrada));

            if ((strlen(cami_final) == 0) || (strcmp(cami_final, "/") == 0)) {  // si hemos acabado o lo ultimo es una "/"
                return 0;
            } else {
                *p_inode_dir = *p_inode; // ara p_inode_dir es el directori que conté fitxers
                return cercarEntrada(cami_final, p_inode_dir, p_inode, p_entrada, reservar, mode); // cridada recursiva
            }
        } else {
            printf("[directorios.c] ERROR: La ruta no existeix!\n");
            //~ *p_entrada = -1;
            //~ *p_inode = -1;
            return -1; // entrada no trobada
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
    p_inode_dir = malloc(sizeof(uint));
    p_inode = malloc(sizeof(uint));
    p_entrada = malloc(sizeof(uint));

    *p_inode_dir = 0;
    *p_inode = 0;
    *p_entrada = 0;

    if (cami[0] != '/') {
        printf("[directorios.c] - mi_creat - ERROR: Cami incorrecte!\n");
        alliberar(p_inode_dir, p_inode, p_entrada);
        return -1;
    }

    sem_wait();

    if (cercarEntrada(cami, p_inode_dir, p_inode, p_entrada, 0, mode) == -1) { // si no hi es...
        *p_inode_dir = 0;
        *p_inode = 0;
        *p_entrada = 0;
        if (cercarEntrada(cami, p_inode_dir, p_inode, p_entrada, 1, mode) == -1) { // ...la crea
            printf("[directorios.c] ERROR: mi_creat - No s'ha pogut crear l'entrada.\n");
            alliberar(p_inode_dir, p_inode, p_entrada);
            sem_signal();
            return 0;
        }
        printf("[directorios.c] INFO: mi_creat - Cami %s creat\n", cami);
    } else {
        printf("[directorios.c] INFO: mi_creat - La ruta ja existeix!\n"); // ja existeix
    }

    alliberar(p_inode_dir, p_inode, p_entrada);
    sem_signal();
    return 0;
}

/**
 *  Funció que crea un enllaç des de l'inode de cami1 (no ha d'existir) a
 *  l'inode especificat per cami2, de tal manera que al acabar l'execució
 *  els dos camins facin referència al mateix inode.
 *  @param cami1 ruta que enllaçada a cami2 (nou)
 *  @param cami2 ruta enllaçada a l'inode de cami1 (existent)
 */
int mi_link(const char *cami1, const char *cami2)
{
    uint *p_inode_dir, *p_inode, *p_entrada;
    uint *p_inode_dir_enllac, *p_inode_enllac, *p_entrada_enllac;
    entrada ent;
    STAT estat;

    // vars enllaçat
    p_inode_dir = malloc(sizeof(uint));
    p_inode = malloc(sizeof(uint));
    p_entrada = malloc(sizeof(uint));

    *p_inode_dir = 0;
    *p_inode = 0;
    *p_entrada = 0;

    // vars enllaç
    p_inode_dir_enllac = malloc(sizeof(uint));
    p_inode_enllac = malloc(sizeof(uint));
    p_entrada_enllac = malloc(sizeof(uint));

    *p_inode_dir_enllac = 0;
    *p_inode_enllac = 0;
    *p_entrada_enllac = 0;

    if (cami1[0] != '/' || cami2[0] != '/') {
        printf("[directorios.c] - mi_link - ERROR: Cami incorrecte\n");
        alliberar(p_inode_dir, p_inode, p_entrada);
        alliberar(p_inode_dir_enllac, p_inode_enllac, p_entrada_enllac);
        return -1;
    }

    char c1 = cami1[strlen(cami1) - 1];
    char c2 = cami2[strlen(cami2) - 1];
    if (((c1 == '/') && (c2 != '/')) || ((c1 != '/') && (c2 == '/'))) {
        printf("[directorios.c] ERROR: No es pot linkar. Camins de tipus diferents\n");
        alliberar(p_inode_dir_enllac, p_inode_enllac, p_entrada_enllac);
        alliberar(p_inode_dir, p_inode, p_entrada);
        return -1;
    }

    sem_wait();

    if (cercarEntrada(cami2, p_inode_dir_enllac, p_inode_enllac, p_entrada_enllac, 0, 7) == -1) { // entrada que ha d'existir
        printf("[directorios.c] ERROR: No s'ha trobat l'entrada, el link no s'ha pogut realitzar!!\n");
        alliberar(p_inode_dir_enllac, p_inode_enllac, p_entrada_enllac);
        alliberar(p_inode_dir, p_inode, p_entrada);
        sem_signal();
        return -1;
    }

    if (cercarEntrada(cami1, p_inode_dir, p_inode, p_entrada, 0, 7) == -1) { // entrada que se crea i enllaça
        sem_signal();

        p_inode_dir = 0;
        p_inode = 0;
        p_entrada = 0;

        cercarEntrada(cami1, p_inode_dir, p_inode, p_entrada, 1, 7); // cream

        // llegim el contingut del directori que conté el nou inode i les entrades de directori.
        mi_stat_f(*p_inode_dir, &estat);

        alliberarInode(*p_inode);

        mi_truncar_f(*p_inode_dir, estat.tamany - sizeof(entrada));

        int darrer = strlen(cami1) - 1;
        if (cami1[darrer] == '/') {
            darrer--;
        }

        int d = darrer;
        while (cami1[d] != '/') {
            d--;
        }

        memset(ent.nom, '\0', 60);
        memcpy(ent.nom, cami1 + d + 1, darrer - d);  // guardam el nom
        ent.inode = *p_inode_enllac;
        mi_write_f(*p_inode_dir, &ent, estat.tamany - sizeof(entrada), sizeof(entrada));

        inode in = llegirInode(*p_inode_enllac);
        in.links_directoris++;
        escriureInode(*p_inode_enllac, in);
        printf("[directorios.c] INFO: Enllaç realitzat correctament.\n");
    } else {
        printf("[directorios.c] INFO: El camí %s ja existeix!\n", cami1);
    }

    alliberar(p_inode_dir_enllac, p_inode_enllac, p_entrada_enllac);
    alliberar(p_inode_dir, p_inode, p_entrada); // alliberam memoria
    sem_signal();
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
    int res = 0;
    inode in;
    STAT estat;
    p_inode_dir = malloc(sizeof(uint));
    p_inode = malloc(sizeof(uint));
    p_entrada = malloc(sizeof(uint));

    *p_inode_dir = 0;
    *p_inode = 0;
    *p_entrada = 0;

    if (strcmp(cami, "/") == 0) { // caso de eliminar "/"
        printf("[directorios.c] ERROR: No pots borrar l'arrel '/'\n");
        alliberar(p_inode_dir, p_inode, p_entrada);
        sem_signal();
        return -1;
    }

    sem_wait();

    res = cercarEntrada(cami, p_inode_dir, p_inode, p_entrada, 0, 7); // cercam si hi es
    if (res == -1) { // no existeix
        printf("[directorios.c] ERROR: No s'ha pogut trobar l'entrada!\n");
        alliberar(p_inode_dir, p_inode, p_entrada);
        sem_signal();
        return -1;
    }

    mi_stat_f(*p_inode_dir, &estat);
    if (estat.tamany == ((*p_entrada+1) * sizeof(entrada))) { // només una entrada
        if (mi_truncar_f(*p_inode_dir, estat.tamany - sizeof(entrada)) == -1) { // la borram
            printf("[directorios.c] ERROR: No s'ha truncat l'inode!\n");
            alliberar(p_inode_dir, p_inode, p_entrada);
            sem_signal();
            return -1;
        }
    }

    // hi ha més d'una entrada
    if (mi_read_f(*p_inode_dir, &ent, estat.tamany - sizeof(entrada), sizeof(entrada)) == -1) { // in.tam - sizeof(struct entrada) = numero de bytes de offset
        printf("[directorios.c] ERROR: No s'ha pogut llegir\n");
        alliberar(p_inode_dir, p_inode, p_entrada);
        sem_signal();
        return -1;
    }

    if (mi_write_f(*p_inode_dir, &ent, *p_entrada * sizeof(entrada), sizeof(entrada)) == -1) { // la trepitjam
        printf("[directorios.c] ERROR: No s'ha pogut escriure\n");
        alliberar(p_inode_dir, p_inode, p_entrada);
        sem_signal();
        return -1;
    }

    if (mi_truncar_f(*p_inode_dir, estat.tamany - sizeof(entrada)) == -1) { // borram sa darrera entrada
        printf("[directorios.c] ERROR: No s'ha truncat l'inode\n");
        alliberar(p_inode_dir, p_inode, p_entrada);
        sem_signal();
        return -1;
    }

    in = llegirInode(*p_inode);
    in.links_directoris--;

    if (in.links_directoris == 0) {
        if (alliberarInode(*p_inode) == -1) {
            printf("[directorios.c] ERROR: L'inode no s'ha alliberat\n");
            alliberar(p_inode_dir, p_inode, p_entrada);
            sem_signal();
        }
    } else {
        if (escriureInode(*p_inode, in)  == -1) {
            printf("[directorios.c] ERROR: No s'ha pogut escriure a l'inode\n");
            alliberar(p_inode_dir, p_inode, p_entrada);
            sem_signal();
        }
    }

    printf("[directorios.c] INFO: El cami '%s' s'ha borrat correctament.\n", cami);
    alliberar(p_inode_dir, p_inode, p_entrada);
    sem_signal();
    return 0;
}

/**
 *  Funció que escriu el contingut del directori 'cami' dins el buffer.
 *  @param cami Ruta que ha de llegir
 *  @param buffer Buffer on ha de guardar la informació.
 *  @return El nombre de fitxers que hi ha dins el directori
 */
int mi_dir(const char *cami, char *buff)
{
    uint p_inode_dir, p_inode, p_entrada;
    int longitut = strlen(cami);
    int bytes_llegits = 0;
    char num_bytes[40];

    entrada ent;
    inode in, aux;

    memset(buff, '\0', BUFFER_DIR); // resetejam
    p_inode_dir = p_inode = p_entrada = 0;

    sem_wait();

    if (cercarEntrada(cami, &p_inode_dir, &p_inode, &p_entrada, 0, 7) == -1) {
        printf("[directorios.c] ERROR: Entrada no trobada!!\n");
        sem_signal();
        return -1;
    }

    if (cami[longitut-1] == '/') { // si es un directori
        in = llegirInode(p_inode);

        while (bytes_llegits < in.tamany) {
            int res = mi_read_f(p_inode, &ent, bytes_llegits, sizeof(entrada));
            if (res == -1) {
                printf("[directorios.c] ERROR: Error de lectura a mi_read_f()\n");
                sem_signal();
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
                        sprintf(num_bytes, ", tamany = %d bytes", aux.tamany); // escrivim dins num_bytes
                        strcat(buff, num_bytes);
                    }

                    strcat(buff, ":");
                }
                bytes_llegits += sizeof(entrada);
            } else {
                sem_signal();
                return 0;
            }
        }
    } else {
        printf("[directorios.c] ERROR: Això no es un directori!\n");
        sem_signal();
        return -1;
    }

    sem_signal();
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
    sem_wait();
    uint p_inode, p_entrada, p_inode_dir = 0;

    if (cercarEntrada(cami, &p_inode_dir, &p_inode, &p_entrada, 0, mode) == -1) { // busca el inodo de la ultima entrada de la ruta y la deposita en p_inode
        printf("[directorios.c] ERROR: No s'ha trobat el cami!!\n");
        sem_signal();
        return -1;
    }

    if (mi_chmod_f(p_inode, mode) == -1) { // camviam els permisos a l'inode
        sem_signal();
        return -1;
    }

    sem_signal();
    return 0;
}

/**
 *  Funció que recull la metainformació del l'inode del camí que se li passa per paràmetre.
 *  @param cami ruta que ha de consultar
 *  @param p_stat esctructura on es guarda la informació del l'inode
 */
int mi_stat(const char *cami, STAT *p_stat)
{
    sem_wait();

    uint p_inode, p_entrada, p_inode_dir = 0;

    if (cercarEntrada(cami, &p_inode_dir, &p_inode, &p_entrada, 0, 7) == -1) {
        printf("[directorios.c] ERROR: No s'ha trobat el cami!!\n");
        sem_signal();
        return -1;
    }

    mi_stat_f(p_inode, p_stat);

    sem_signal();
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
    sem_wait();

    uint p_inode, p_entrada, p_inode_dir = 0;

    if (cercarEntrada(cami, &p_inode_dir, &p_inode, &p_entrada, 0, 7) == -1) {
        printf("[directorios.c] ERROR: No s'ha trobat el cami!!\n");
        sem_signal();
        return -1;
    }

    int llegits = mi_read_f(p_inode, buff, offset, nbytes);
    if (llegits == -1) {
        sem_signal();
        return -1;
    }

    sem_signal();
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

    sem_wait();

    uint p_inode, p_entrada, p_inode_dir = 0;

    if (cercarEntrada(cami, &p_inode_dir, &p_inode, &p_entrada, 0, 7) == -1) {
        printf("[directorios.c] ERROR: No s'ha trobat el cami!!\n");
        sem_signal();
        return -1;
    }

    int escrits = mi_write_f(p_inode, buff, offset, nbytes);
    if (escrits == -1) {
        sem_signal();
        return -1;
    }

    sem_signal();
    return escrits;
}

/**
 *  Funció que mostra per pantalla el contingut d'un directori
 *  @param cami ruta que es vol mostrar el contingut
 *  @param buff buffer on hi ha la informació.
 */
int mi_lsdir(const char *cami, char *buff)
{
    sem_wait();

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

    sem_signal();
    return 0;
}

/**
 *  Wait del mutex
 */
void sem_wait()
{
    esperarSemafor(mutex);
}

/**
 *  Signal del mutex
 */
void sem_signal()
{
    senyalitzarSemafor(mutex);
}

/**
 *  Inicialitzam el semàfor mutex
 */
void sem_init()
{
    key_t key;
    key = ftok("/bin/ls", 'a');
    if (key == -1) {
        printf("[directorios.c] ERROR: Error al crear la clau del semafor.\n");
        exit(-1);
    }

    mutex = nouSemafor(key, 1);
    inicialitzarSemafor(mutex, 1);
}

/**
 *  Elimina el semafor mutex
 */
void sem_del()
{
    eliminarSemafor(mutex);
}
