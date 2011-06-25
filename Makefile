# Makefile per a sa pràctica d'Ampliació de Sistemes Operatius:
# Un sistema de fitxers (AFS)

# variables
OBJ_LIB = libs/bloques.o libs/ficheros_basico.o libs/ficheros.o libs/directorios.o libs/semaforos.o
OBJ_EXE = src/mi_mkfs.o src/mi_chmod.o src/mi_ls.o src/mi_stat.o src/mi_cat.o src/mi_ln.o src/mi_rm.o src/mi_creat.o src/mi_write.o src/simulacion.o src/mi_truncar.o mi_pwd.o mi_tree.o
OBJ = $(OBJ_EXE) $(OBJ_LIB)
CC = clang
CFLAGS = -Wall -O1 -ggdb
EXEC = src/mi_mkfs src/mi_chmod src/mi_ls src/mi_stat src/mi_cat src/mi_ln src/mi_rm src/mi_creat src/mi_write src/simulacion src/mi_truncar src/mi_pwd src/mi_tree

all: mi_mkfs mi_chmod mi_ls mi_stat mi_cat mi_ln mi_rm mi_creat mi_write simulacion mi_truncar mi_tree mi_pwd

# biblioteques
directorios.o: libs/directorios.c libs/directorios.h
	$(CC) $(CFLAGS) -c libs/directorios.c

ficheros.o: libs/ficheros.c libs/ficheros.h
	$(CC) $(CFLAGS) -c libs/ficheros.c

ficheros_basico.o: libs/ficheros_basico.c libs/ficheros_basico.h
	$(CC) $(CFLAGS) -c libs/ficheros_basico.c

bloques.o: libs/bloques.c libs/bloques.h
	$(CC) $(CFLAGS) -c libs/bloques.c

semaforos.o: libs/semaforos.c libs/semaforos.h
	$(CC) $(CFLAGS) -c libs/semaforos.c

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

mi_write: src/mi_write.c $(OBJ_LIB)
	$(CC) -o src/mi_write src/mi_write.c $(OBJ_LIB)

mi_pwd: src/mi_pwd.c $(OBJ_LIB)
	$(CC) -o src/mi_pwd src/mi_pwd.c $(OBJ_LIB)

mi_tree: src/mi_tree.c $(OBJ_LIB)
	$(CC) -o src/mi_tree src/mi_tree.c $(OBJ_LIB)

mi_truncar: src/mi_truncar.c $(OBJ_LIB)
	$(CC) -o src/mi_truncar src/mi_truncar.c $(OBJ_LIB)

simulacion: src/simulacion.c $(OBJ_LIB)
	$(CC) $(CFLAGS) -o src/simulacion src/simulacion.c $(OBJ_LIB)

clean:
	rm -f $(OBJ)
	rm -f $(EXEC)

.PHONY : clean
