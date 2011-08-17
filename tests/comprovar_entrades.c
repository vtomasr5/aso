/**
 *  @file comprovar_entrades.c
 *  @brief Prova que mostra les entrades de directori d'un directori per pantalla.
 *  @date 14/08/2011
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/directorios.h"

#define length(x) (sizeof(x) / sizeof(x[0]))

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("[comprovar_entrades.c] ERROR: Arguments incorrectes. Ex: ./comprovar_entrades disco.imagen <directori>\n");
        exit(-1);
    }

    // montam es FS
    if (bmount(argv[1]) == -1) {
        return -1;
    }

    // proves
    STAT estat;
    uint p_inode_dir, p_inode, p_entrada = 0;
    //~ char nom[50];
    //~ int i = 0;
    //~ for (i = 1; i <= 16; i++) {
        //~ sprintf(nom, "/dir%d/", i);
        //~ if (mi_creat(nom, 7) == -1) {
            //~ printf("error\n");
            //~ return -1;
        //~ }
    //~ }

    if (cercarEntrada(argv[2], &p_inode_dir, &p_inode, &p_entrada, 0, 7) == -1) { // entrada que ha d'existir
        printf("[comprovar_entrades.c] ERROR: No s'ha trobat el cami o no es un directori!\n");
        bumount();
        return -1;
    }

    mi_stat_f(p_inode_dir, &estat); // llegim la informació de l'inode i la guardam en sa varible estat

    int num_ent = estat.tamany / sizeof(entrada); // leemos todas las entradas del directorio

    entrada ent[num_ent]; // definimos un array de entradas de directorio que contiene el numero de entradas del inodo leido
    if (estat.tamany != 0) { // si hi ha entrades
        if (mi_read_f(p_inode_dir, &ent, 0, estat.tamany) == -1) { // leemos las entradas del directorio y las guardamos en el "buffer" ent
            printf("[comprovar_entrades.c] ERROR: No s'ha pogut llegir l'entrada!\n");
            bumount();
            return -1;
        }

        // inici debug
        int k;
        for (k = 0; k < length(ent); k++) { // mostram les entrades per pantalla
            printf("[comprovar_entrades.c] DEBUG: ----> Entrada directori: nom = '%s' || inode = '%d'\n", ent[k].nom, ent[k].inode);
        }
        printf("[comprovar_entrades.c] DEBUG: ----> n_entrades = %ld\n", length(ent));
        // fi debug
    } else {
        printf("[comprovar_entrades.c] ERROR: No s'han trobat entrades!\n");
    }


    // desmontam es FS
    if (bumount() == -1) {
        return -1;
    }

    return 0;
}
