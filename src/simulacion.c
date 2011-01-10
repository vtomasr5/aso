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
 *
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
    sprintf(nom_carpeta, "%s proceso_%d/", nom_carpeta, getpid());

    srandom(getpid());

    if (mi_creat(nom_carpeta, 7) != -1) {
        sprintf(nom_carpeta, "%sprueba.dat", nom_carpeta);

        for (i = 0; i < N_VEGADES; i++) {
            rand = (random() % 1024) * sizeof(registre);
            reg.data = time(NULL);
            reg.pid = getpid();
            reg.escriptura = i + 1;
            reg.pos_registre = rand;

            if (mi_write(nom_carpeta, &reg, rand, sizeof(registre)) == -1) {
                printf("[simulacion.c] ERROR: Hi ha hagut un error d'escriptura\n");
                return -1;
            }

            printf("[simulacion.c] INFO: Proces: %d, escric el regirtre: %d, a la posició: %d.\n", reg.pid, i + 1, rand);
            usleep(50000); // 0.05s
        }
    } else {
        printf("[simulacion.c] ERROR: No s'ha pogut crear el cami '%s'\n", nom_carpeta);
        return -1;
    }
    return 0;
}

/**
 *
 *
 */
void verificar()
{

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
    }

    verificar();

    if (bumount() == -1) {
        return -1;
    }
    return 0;
}

















