/**
 *  @file truncar.c
 *  @brief Prova que realitza un truncament a un fitxer a 'n' bytes
 *  @date 11/01/2011
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../libs/directorios.h"

int main(int argc, char *argv[])
{
    STAT estat;
    char buff[11] = {'a','b','c','d','e','f','g','h','i','j'};

    if (argc != 4) {
        printf("[truncar.c] ERROR: Arguments incorrectes. Ex: ./truncar <nomFS> <inode> <nbytes>\n");
        exit(-1);
    }

    // montam es FS
    if (bmount(argv[1]) == -1) {
        return -1;
    }

    // proves
    printf("nnnnnnnnnnnnnnnnnn creat /dir1 /dir2/ /fichero nnnnnnnnnnnnnnnnnnnnnn\n");
    mi_creat("/dir1/dir2/fichero", 7);

    printf("nnnnnnnnnnnnnnnnnn write /dir1 /dir2/ /fichero nnnnnnnnnnnnnnnnnnnnnn\n");
    mi_write("/dir1/dir2/fichero", buff, 0, 11);

    printf("nnnnnnnnnnnnnnnnnn stat /dir1 /dir2/ /fichero nnnnnnnnnnnnnnnnnnnnnn\n");
    mi_stat("/dir1/dir2/fichero", &estat);
    veure_estat(&estat);

    printf("nnnnnnnnnnnnnnnnnn truncar /dir1 /dir2/ /fichero nnnnnnnnnnnnnnnnnnnnnn\n");
    mi_truncar_f(atoi(argv[2]), atoi(argv[3])); // truncam fichero a 0 bytes

    printf("nnnnnnnnnnnnnnnnnn stat /dir1 /dir2/ /fichero nnnnnnnnnnnnnnnnnnnnnn\n");
    mi_stat("/dir1/dir2/fichero", &estat);
    veure_estat(&estat);

    // desmontam es FS
    if (bumount() == -1) {
        return -1;
    }

    return 0;
}
