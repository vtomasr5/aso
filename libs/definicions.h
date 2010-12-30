// tamany de bloc
#define TB 1024

// nombre de inodes que s'usaran a l'array de inodes
#define N_INODES 4

// nombre màxim de punters directes per a cada i-node
#define MAX_PUNTERS_DIRECTES 12

// nombre màxim de punters indirectes per a cada i-node
#define MAX_PUNTERS_INDIRECTES 3

// posició del superbloc
#define POSICIO_SB 0

// nombre de punters que caben dins un bloc (256)
#define N_PUNTERS_BLOC (TB / 4)

// tamany del buffer del directori
#define BUFFER_DIR (TB * 6)
