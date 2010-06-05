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
