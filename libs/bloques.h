/**
 *  @file bloques.h
 *  @brief Conté les capçaleres de les funcions necessàries pel correcte funcionament del programa
 *  @date 04/10/2010
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

int bmount(const char *);
int bumount();
int bread(uint, void *);
int bwrite(uint, const void *);
