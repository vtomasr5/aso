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
 *  @brief Simulació de 100 procesos que escriuen de forma concurrent en el
 *  sistema de fitxers amb les llibreries implementades anteriorment.
 *  @date 10/01/2011
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include "../libs/directorios.h"

#define PROCESOS 100
#define N_VEGADES 50
#define TAM 1024

typedef struct {
    int data;
    int pid;
    int escriptura;
    int pos_registre;
} registre; // sizeof = 16 bytes

int acabados = 0;

/**
 *  Funció que s'encarrega de que no quedin processos zombies
 *  @param s
 */
void reaper(int s)
{
    while(wait3(NULL, WNOHANG, NULL) > 0) {
        acabados++;
    }
    printf("[simulacion.c] INFO: Procesos %d de %d acabats\n", acabados, PROCESOS);
}

/**
 *  Funció que crea 100 procesos que
 *
 *  @param num_proces PID del procés que arranca
 *  @param fitxer Fitxer que es crea
 */
int proces(int num_proces, char *fitxer)
{
    char nom_carpeta[TAM];
    int i = 0;
    registre reg;
    int rand; // numero del random

    sprintf(nom_carpeta, "%s", fitxer);
    sprintf(nom_carpeta, "%sproceso_%d/", nom_carpeta, getpid());

    srandom(getpid());

    if (mi_creat(nom_carpeta, 7) != -1) {
        sprintf(nom_carpeta, "%sprueba.dat", nom_carpeta);

        for (i = 0; i < N_VEGADES; i++) {
            rand = (random() % sizeof(registre)) * sizeof(registre);
            reg.data = time(NULL);
            reg.pid = getpid();
            reg.escriptura = i + 1;
            reg.pos_registre = rand;

            if (mi_write(nom_carpeta, &reg, rand, sizeof(registre)) == -1) {
                printf("[simulacion.c] ERROR: Hi ha hagut un error d'escriptura\n");
                return -1;
            }

            printf("[simulacion.c] INFO: Proces: %d, escric el registre: %d, a la posició: %d.\n", reg.pid, i + 1, reg.pos_registre);
            usleep(50000); // 0.05s
        }
    } else {
        printf("[simulacion.c] ERROR: No s'ha pogut crear el cami2 '%s'\n", nom_carpeta);
        return -1;
    }
    return 0;
}

/**
 *
 *
 */
int verificar() {
    int proces, escriptures, p_escriptura, pos_p_escriptura, d_escriptura, pos_d_escriptura, posicio_menor, pos_menor_posicio, posicio_major, pos_major_posicio;
    registre reg;
    entrada ent;
    STAT estat, estat2;
    char aux[30];
    char dir1[100]; // simul_ddmmaahhmmss
    char dir2[100];
    int i, j;

    memset(aux, '\0', 30);
    memset(dir1, '\0', 100);
    memset(dir2, '\0', 100);

    printf("\n");
    if (mi_read("/", &ent, 0, sizeof(entrada)) == -1) { // llegim '/'
        printf("[simulacion.c] ERROR: Error de lectura!\n");
        return -1;
    }

    sprintf(dir1, "/%s/", ent.nom);
    mi_stat(dir1, &estat);

    for (i = 0; i < (estat.tamany / sizeof(entrada)); i++) { // recorrem totes les entrades
        if (mi_read(dir1, &ent, i * sizeof(entrada), sizeof(entrada)) == -1) {
            printf("[simulacion.c] ERROR: Error de lectura!\n");
            return -1;
        }
        sprintf(dir2, "%s%s/prueba.dat", dir1, ent.nom); // direccio completa
        mi_stat(dir2, &estat2); // lectura de la informacio de l'arcxiu

        proces = escriptures = p_escriptura = pos_p_escriptura = d_escriptura = pos_d_escriptura = posicio_menor = pos_menor_posicio = posicio_major = pos_major_posicio = 0;

        p_escriptura = time(NULL);
        posicio_menor = estat2.tamany;

        int k = 8;
        while (k < strlen(ent.nom)) {
            aux[k-8] = ent.nom[k];
            k++;
        }

        proces = atoi(aux);
        for (j = 0; j < estat2.tamany / sizeof(registre); j++) { // recorrem els registres
            reg.data = reg.pid = reg.escriptura = reg.pos_registre = 0;

            if (mi_read(dir2, &reg, j * sizeof(registre), sizeof(registre)) == -1) {
                printf("[simulacion.c] ERROR: Error de lectura!\n");
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
        printf("####################################################################\n");
        printf("[simulacion.c] INFO: Proces: %d\n", i + 1);
        printf("[simulacion.c] INFO: PID: %d\n", proces);
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
    int pid_proces;
    char nom_carpeta[TAM];
    struct tm *t;
    time_t hora;
    int i = 0;

    if (argc != 2) {
        printf("[simulacion.c] ERROR: Paràmetres incorrectes. Ex: simulacion <nomFS>\n");
        return -1;
    }

    memset(nom_carpeta, '\0', TAM);

    signal(SIGCHLD,reaper); // senyal per als zombies
    acabados = 0;
    hora = time(NULL);
    t = localtime(&hora);

    if (bmount(argv[1]) == -1) {
        return -1;
    }

    // cream sa carpeta inicial /simul...
    sprintf(nom_carpeta, "/simul_%d%d%d%d%d%d/", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

    if (mi_creat(nom_carpeta, 7) != -1) {
        for (i = 0; i < PROCESOS; i++) {
            if (fork() == 0) {
                proces(i, nom_carpeta);
                exit(0);
            }
            usleep(200000); // 0.2s
        }

        while (acabados < PROCESOS) {
            pause();
        }
    } else {
        printf("[simulacion.c] ERROR: No s'ha pogut crear el cami1 '%s'\n", nom_carpeta);
        return -1;
    }

    verificar(); // verificam les escriptures

    if (bumount() == -1) {
        return -1;
    }
    return 0;
}
