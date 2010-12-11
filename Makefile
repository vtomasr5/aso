# Makefile per a sa pràctica d'Ampliació de Sistemes Operatius
# Un sistema de fitxers

OBJ = src/mi_mkfs.o libs/bloques.o libs/ficheros_basico.o libs/ficheros.o libs/directorios.o
SRC = src/mi_mkfs.c libs/bloques.c libs/ficheros_basico.c libs/ficheros.c libs/firectorios.c
EXE = src/mi_mkfs
CC = gcc
CFLAGS = -Wall -O1 -ggdb

all: mi_mkfs
	ln -f -s src/mi_mkfs mi_mkfs

mi_mkfs: $(OBJ)
	$(CC) $(OBJ) -o $(EXE)

mi_mkfs.o: src/mi_mkfs.c libs/bloques.h
	$(CC) $(CFLAGS) -c $(EXE)

directorios.o: libs/directorios.c libs/directorios.h
	$(CC) $(CFLAGS) -c libs/directorios.c

ficheros.o: libs/ficheros.c libs/ficheros.h
	$(CC) $(CFLAGS) -c libs/ficheros.c

ficheros_basico.o: libs/ficheros_basico.c libs/ficheros_basico.h
	$(CC) $(CFLAGS) -c libs/ficheros_basico.c

bloques.o: libs/bloques.c libs/bloques.h
	$(CC) $(CFLAGS) -c libs/bloques.c

clean:
	rm $(EXE)
	rm $(OBJ)
	rm mi_mkfs

.PHONY : clean
