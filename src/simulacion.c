//      simulacion.c
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
 *  @file simulacion.c
 *  @brief Programa simulador que crea 100 procesos que escriuen de forma simultanea en el
 *  sistema de fitxers que utilitza les llibreries implementades anteriorment.
 *  @date 10/01/2011
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include "../include/directorios.h"
#include "../include/semaforos.h"

#define PROCESOS 10
#define N_VEGADES 50
#define TAM 100 // llargaria nom cami

typedef struct {
    int data;
    int pid;
    int escriptura;
    int pos_registre;
} registre; // sizeof = 16 bytes

int acabados = 0;

/**
 *  Funció que s'encarrega de que no quedin processos zombies.
 *  @param s
 */
void enterrador(int s)
{
    while (wait3(NULL, WNOHANG, NULL) > 0) {
        acabados++;
    }
    printf("[simulacion.c] INFO: Processos acabats: %d de %d\n", acabados, PROCESOS);
}

/**
 *  Funció que crea 100 procesos que escriuen de forma concurrent en el sistema de fitxers.
 *  @param num_proces PID del procés que arranca
 *  @param fitxer Fitxer que es crea
 */
int proces(int num_proces, char *fitxer)
{
    char nom_carpeta[TAM];
    //memcpy(nom_carpeta, '\0', TAM);
    int i = 0;
    registre reg;
    int rand; // numero del random

    sprintf(nom_carpeta, "%s", fitxer);
    sprintf(nom_carpeta, "%sproceso_%d/", nom_carpeta, getpid());

    srandom(getpid());
    //printf("[simulacion.c] DEBUG: Carpeta: %s\n", &nom_carpeta[0]);

    if (mi_creat(nom_carpeta, 7) != -1) { // cream els directori 'proceso_n'
        sprintf(nom_carpeta, "%sprueba.dat", nom_carpeta); // afegim a la ruta el nom del fitxer

        if (mi_creat(nom_carpeta, 7) != -1) { // cream els fitxers 'prueba.dat'
            for (i = 0; i < N_VEGADES; i++) {
                rand = (random() % sizeof(registre)) * sizeof(registre);
                reg.data = time(NULL);
                reg.pid = getpid();
                reg.escriptura = i + 1;
                reg.pos_registre = rand;

                if (mi_write(nom_carpeta, &reg, rand, sizeof(registre)) == -1) {
                    printf("[simulacion.c] ERROR: Hi ha hagut un error d'escriptura a '%s'\n", nom_carpeta);
                    return -1;
                }

                printf("[simulacion.c] INFO: Proces: %d, escric el registre: %d, a la posicio: %d.\n", reg.pid, i + 1, rand);
                usleep(50000); // 0.05s
            }
        } else {
            printf("[simulacion.c] ERROR: No s'ha pogut crear el fitxer '%s'\n", nom_carpeta);
            return -1;
        }
    } else {
        printf("[simulacion.c] ERROR: No s'ha pogut crear el directori '%s'\n", nom_carpeta);
        return -1;
    }

    return 0;
}

/**
 *  Funció que verifica que les escriptures de forma concurrent s'han realitzar
 *  de la forma correcta.
 */
int verificar() {
    int proces, escriptures, p_escriptura, pos_p_escriptura, d_escriptura, pos_d_escriptura, posicio_menor, pos_menor_posicio, posicio_major, pos_major_posicio;
    registre reg;
    entrada ent;
    STAT estat, estat2;
    char aux[TAM];
    char dir1[TAM]; // simul_ddmmaahhmmss
    char dir2[TAM];
    int i, j;

    memset(aux, '\0', TAM);
    memset(dir1, '\0', TAM);
    memset(dir2, '\0', TAM);

    if (mi_read("/", &ent, 0, sizeof(entrada)) == -1) { // llegim '/'
        printf("[simulacion.c] ERROR: Error de lectura1!\n");
        return -1;
    }

    sprintf(dir1, "/%s/", ent.nom);
    mi_stat(dir1, &estat);
    printf("[simulacion.c] DEBUG: dir1 = '%s'\n", dir1);
    for (i = 0; i < (estat.tamany / sizeof(entrada)); i++) { // recorrem totes les entrades
        printf("algo\n");
        if (mi_read(dir1, &ent, i * sizeof(entrada), sizeof(entrada)) == -1) { // llegim totes les entrades una a una
            printf("[simulacion.c] ERROR: Error de lectura2!\n");
            return -1;
        }
        sprintf(dir2, "%s%s/prueba.dat", dir1, ent.nom); // direccio completa
        mi_stat(dir2, &estat2); // lectura de la informacio de l'arcxiu

        proces = escriptures = p_escriptura = pos_p_escriptura = d_escriptura = pos_d_escriptura = posicio_menor = pos_menor_posicio = posicio_major = pos_major_posicio = 0;
        p_escriptura = time(NULL);
        posicio_menor = estat2.tamany;

        int k = 8;
        while (k < strlen(ent.nom)) { // llegim el PID
            aux[k-8] = ent.nom[k];
            k++;
        }
        proces = atoi(aux);

        for (j = 0; j < estat2.tamany / sizeof(registre); j++) { // recorrem els registres
            reg.data = reg.pid = reg.escriptura = reg.pos_registre = 0;

            if (mi_read(dir2, &reg, j * sizeof(registre), sizeof(registre)) == -1) {
                printf("[simulacion.c] ERROR: Error de lectura3!\n");
                return -1;
            }

            struct tm *t = localtime((time_t*)reg.data);
            char *d = asctime(t);

            if (reg.pid != 0) {
                if (reg.pid == proces) { // validam
                    escriptures++;
                }
                if (reg.data < p_escriptura) {
                    p_escriptura = atoi(d);
                    pos_p_escriptura = reg.escriptura;
                }
                if (reg.data > d_escriptura) {
                    d_escriptura = atoi(d);
                    pos_d_escriptura = reg.escriptura;
                }
                if (reg.pos_registre < posicio_menor) {
                    posicio_menor = reg.pos_registre;
                    pos_menor_posicio = reg.escriptura;
                }
                if (reg.pos_registre > posicio_major) {
                    posicio_major = reg.pos_registre;
                    pos_major_posicio = reg.escriptura;
                }

            printf("[simulacion.c] DEBUG: data: %d, pid: %d, escriptura: %d, pos_registre: %d\n", reg.data, reg.pid, reg.escriptura, reg.pos_registre);
            }
        }
        printf("[simulacion.c] ############################# VERIFICACIO ###################################\n");
        printf("[simulacion.c] INFO: Proces: %d\n", proces);
        printf("[simulacion.c] INFO: Escriptures: %d\n", escriptures);
        printf("[simulacion.c] INFO: Primera escriptura a les: %d, en la posicio de l'escriptura: %d\n", p_escriptura, pos_p_escriptura);
        printf("[simulacion.c] INFO: Darrera escriptura a les: %d, en la posicio de l'escriptura: %d\n", d_escriptura, pos_d_escriptura);
        printf("[simulacion.c] INFO: Menor posicio de l'escriptura: %d, en la posicio de l'escriptura: %d\n", posicio_menor, pos_menor_posicio);
        printf("[simulacion.c] INFO: Major posicio de l'escritura: %d, en la posicio de l'escriptura: %d\n\n", posicio_major, pos_major_posicio);
    }

    return 0;
}

/**
 *  Funció que crea 100 processos que escriuen de forma simultanea en el sistema
 *  de fitxers emprant les llibreries implementades anteriorment.
 */
int main(int argc, char **argv)
{
    char nom_carpeta[TAM];
    struct tm *t;
    time_t hora;
    int i = 0;

    if (argc != 2) {
        printf("[simulacion.c] ERROR: Paràmetres incorrectes. Ex: simulacion <nomFS>\n");
        return -1;
    }

    memset(nom_carpeta, '\0', TAM);

    signal(SIGCHLD, enterrador); // senyal per als zombies

    acabados = 0;
    hora = time(NULL);
    t = localtime(&hora);

    if (bmount(argv[1]) == -1) {
        return -1;
    }

    sem_init();

    sprintf(nom_carpeta, "/simul_%d%d%d%d%d%d/", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

    if (mi_creat(nom_carpeta, 7) != -1) { // cream sa carpeta inicial /simul...
        for (i = 0; i < PROCESOS; i++) {
            printf("[simulacion.c] ############################# Proces: %d #############################\n", i);
            if (fork() == 0) { // es creen els 100 processos fills
                proces(i, nom_carpeta);
                exit(0);
            }
            usleep(200000); // 0.2s
        }

        while (acabados < PROCESOS) {
            pause();
        }
    } else {
        printf("[simulacion.c] ERROR: No s'ha pogut crear el directori '%s'\n", nom_carpeta);
        return -1;
    }

    verificar(); // verificam les escriptures

    sem_del();

    if (bumount() == -1) {
        return -1;
    }

    return 0;
}
