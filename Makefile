# Makefile per a sa pràctica d'Ampliació de Sistemes Operatius:
# Un sistema de fitxers (AFS)

# variables
OBJ_LIB = libs/bloques.o libs/ficheros_basico.o libs/ficheros.o libs/directorios.o libs/semaforos.o
CC = gcc
CFLAGS = -Wall -O2 -g -ggdb
EXEC = bin/mi_mkfs bin/mi_chmod bin/mi_ls bin/mi_stat bin/mi_cat bin/mi_ln bin/mi_rm bin/mi_creat bin/mi_write bin/simulacion bin/mi_truncar

all: mi_mkfs mi_chmod mi_ls mi_stat mi_cat mi_ln mi_rm mi_creat mi_write simulacion mi_truncar

# biblioteques
directorios.o: libs/directorios.c include/directorios.h
	$(CC) $(CFLAGS) -c libs/directorios.c

ficheros.o: libs/ficheros.c include/ficheros.h
	$(CC) $(CFLAGS) -c libs/ficheros.c

ficheros_basico.o: libs/ficheros_basico.c include/ficheros_basico.h
	$(CC) $(CFLAGS) -c libs/ficheros_basico.c

bloques.o: libs/bloques.c include/bloques.h
	$(CC) $(CFLAGS) -c libs/bloques.c

semaforos.o: libs/semaforos.c include/semaforos.h
	$(CC) $(CFLAGS) -c libs/semaforos.c

# executables
mi_mkfs: src/mi_mkfs.c $(OBJ_LIB)
	$(CC) -o bin/mi_mkfs src/mi_mkfs.c $(OBJ_LIB)

mi_chmod: src/mi_chmod.c $(OBJ_LIB)
	$(CC) -o bin/mi_chmod src/mi_chmod.c $(OBJ_LIB)

mi_ls: src/mi_ls.c $(OBJ_LIB)
	$(CC) -o bin/mi_ls src/mi_ls.c $(OBJ_LIB)

mi_stat: src/mi_stat.c $(OBJ_LIB)
	$(CC) -o bin/mi_stat src/mi_stat.c $(OBJ_LIB)

mi_cat: src/mi_cat.c $(OBJ_LIB)
	$(CC) -o bin/mi_cat src/mi_cat.c $(OBJ_LIB)

mi_ln: src/mi_ln.c $(OBJ_LIB)
	$(CC) -o bin/mi_ln src/mi_ln.c $(OBJ_LIB)

mi_rm: src/mi_rm.c $(OBJ_LIB)
	$(CC) -o bin/mi_rm src/mi_rm.c $(OBJ_LIB)

mi_creat: src/mi_creat.c $(OBJ_LIB)
	$(CC) -o bin/mi_creat src/mi_creat.c $(OBJ_LIB)

mi_write: src/mi_write.c $(OBJ_LIB)
	$(CC) -o bin/mi_write src/mi_write.c $(OBJ_LIB)

mi_truncar: src/mi_truncar.c $(OBJ_LIB)
	$(CC) -o bin/mi_truncar src/mi_truncar.c $(OBJ_LIB)

simulacion: src/simulacion.c $(OBJ_LIB)
	$(CC) $(CFLAGS) -o bin/simulacion src/simulacion.c $(OBJ_LIB)

clean:
	rm -f $(OBJ_LIB)
	rm -f $(EXEC)

.PHONY : clean
