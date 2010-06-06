#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define TYP_SIZE 5
#define TYPES 1
#define REFRESH 300000
#define SPEED 300000
#define FILENAME "tron.log"
#define LOGSTRING_SIZE 256


typedef struct grid_field {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	char value;
} field;

char *program_name = NULL;
field **grid = NULL;

FILE *output;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

int dimX;
int dimY;
int count_player;
int number_player;
int *player_posX;
int *player_posY;
char **player_typ;
int *player_typ_nr;
int *player_count_steps;

pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t count_threshold_cv;

void *print_grid(void *t);
void *cycle(void *t);
void *cycle2(void *t);
