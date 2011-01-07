# Makefile per a sa pràctica d'Ampliació de Sistemes Operatius
# Un sistema de fitxers

# variables
OBJ_LIB = libs/bloques.o libs/ficheros_basico.o libs/ficheros.o libs/directorios.o
OBJ_EXE = src/mi_mkfs.o src/mi_chmod.o src/mi_ls.o src/mi_stat.o src/mi_cat.o src/mi_ln.o src/mi_rm.o src/mi_creat.o
OBJ = $(OBJ_EXE) $(OBJ_LIB)
CC = gcc
CFLAGS = -Wall -O1 -ggdb

all: mi_mkfs mi_chmod mi_ls mi_stat mi_cat mi_ln mi_rm mi_creat

# biblioteques
directorios.o: libs/directorios.c libs/directorios.h
	$(CC) $(CFLAGS) -c libs/directorios.c

ficheros.o: libs/ficheros.c libs/ficheros.h
	$(CC) $(CFLAGS) -c libs/ficheros.c

ficheros_basico.o: libs/ficheros_basico.c libs/ficheros_basico.h
	$(CC) $(CFLAGS) -c libs/ficheros_basico.c

bloques.o: libs/bloques.c libs/bloques.h
	$(CC) $(CFLAGS) -c libs/bloques.c

# executables
mi_mkfs: src/mi_mkfs.c $(OBJ_LIB)
	$(CC) -o src/mi_mkfs src/mi_mkfs.c $(OBJ_LIB)

mi_chmod: src/mi_chmod.c $(OBJ_LIB)
	$(CC) -o src/mi_chmod src/mi_chmod.c $(OBJ_LIB)

mi_ls: src/mi_ls.c $(OBJ_LIB)
	$(CC) -o src/mi_ls src/mi_ls.c $(OBJ_LIB)

mi_stat: src/mi_stat.c $(OBJ_LIB)
	$(CC) -o src/mi_stat src/mi_stat.c $(OBJ_LIB)

mi_cat: src/mi_cat.c $(OBJ_LIB)
	$(CC) -o src/mi_cat src/mi_cat.c $(OBJ_LIB)

mi_ln: src/mi_ln.c $(OBJ_LIB)
	$(CC) -o src/mi_ln src/mi_ln.c $(OBJ_LIB)

mi_rm: src/mi_rm.c $(OBJ_LIB)
	$(CC) -o src/mi_rm src/mi_rm.c $(OBJ_LIB)

mi_creat: src/mi_creat.c $(OBJ_LIB)
	$(CC) -o src/mi_creat src/mi_creat.c $(OBJ_LIB)

clean:
	rm -f $(OBJ)

.PHONY : clean
