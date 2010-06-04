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

//field data = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0};

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

int main(int argc, char *argv[]) {
	int c;
	int isSetDimX = 0;
	int isSetDimY = 0;
	int isRightRange = 1;
	int error = 0;
	
	program_name = argv[0];
	
	// program parameter
	while ((c = getopt (argc, argv, "x:y:")) != EOF) {
		switch (c) {
			case 'x':
				if(!isSetDimX) {
					if ((dimX = atoi(optarg)) < 0) {
						isRightRange = 0;
					}
					isSetDimX = 1;
				} else {
					error = 1;
				}
				break;
			case 'y':
				if(!isSetDimY) {
					if ((dimY = atoi(optarg)) < 0) {
						isRightRange = 0;
					}
					isSetDimY = 1;
				} else {
					error = 1;
				}
				break;
			case '?':
				error = 1;
				break;
			default:
				fprintf(stdout, "Error in getopt\n");
				error = 1;
				break;
		}
	}
	// for each player three arguments: posx,posy,typ
	if(((argc-optind)%3)==0) {
		count_player = (argc-optind)/3;
		number_player = count_player;
		if(count_player > 26) {
			fprintf(stderr, "Wrong number of player, only 26 players are able to play!\n");
			exit(EXIT_FAILURE);
		}
		player_posX = malloc(count_player * sizeof(int));
		player_posY = malloc(count_player * sizeof(int));
		player_typ_nr = malloc(count_player * sizeof(int));
		player_typ = malloc(count_player * sizeof(char *));
		player_count_steps = malloc(count_player * sizeof(int));
		int index;
		int player = 0;
		// increment 3
		for(index = optind; index < argc; ) {
			player_posX[player] = atoi(argv[index]);
			player_posY[player] = atoi(argv[index+1]);
			if(player_posX[player]>=dimX || player_posY[player]>=dimY) {
				fprintf(stderr, "Player position out of field: player %i x:%i y:%i\n", player+1,player_posX[player],player_posY[player]);
				exit(EXIT_FAILURE); 	
			}
			int i;
			for(i = 0; i <= player-1; i++) {
				if(player_posX[i]==player_posX[player] && player_posY[i]==player_posY[player]) {
					fprintf(stderr, "Player aren't allowed to have the same start position.\n");
					exit(EXIT_FAILURE);
				}
			}
			player_typ[player] = malloc(TYP_SIZE * sizeof(char));
			if(strncmp("typ",argv[index+2],3)==0){
				char *temp = malloc(TYP_SIZE * sizeof(char));
				strcpy(player_typ[player],argv[index+2]);
				strncpy(temp,argv[index+2]+3,strlen(argv[index+2]));
				player_typ_nr[player] = atoi(temp);
				free(temp);
				// check type number
				if(player_typ_nr[player]==0 || player_typ_nr[player]>TYPES) {
					fprintf(stderr, "Wrong player type: %s\n", argv[index+2]);
					exit(EXIT_FAILURE);	
				}
			} else {
				fprintf(stderr, "Wrong player type: %s\n", argv[index+2]);
				exit(EXIT_FAILURE);	
			}
			index += 3;
			player++;
		}
	} else {
		error = 1;
	}
	
	if(error | !isSetDimX | !isSetDimY) {
		fprintf(stderr, "usage:	%s -x <DimX> -y <DimY> (<posX> <posY> <typ>)+\n", program_name);
		exit(EXIT_FAILURE);
	}
	if(!isRightRange){
		fprintf(stderr, "<DimX>,<DimY>,<posX> and <posY> should be greater or equals 0!\n"); 	
	}
	
	// allocate grid 2-dimensional
	grid = malloc( dimY * sizeof(field *));
	if(grid == NULL) {
		fprintf(stderr, "out of memory\n");
		exit(EXIT_FAILURE);
	}
	int i;
	for(i = 0; i < dimY; i++) {
		grid[i] = malloc( (dimX) * sizeof(field));
		if(grid[i] == NULL) {
			fprintf(stderr, "out of memory\n");
			exit(EXIT_FAILURE);
		}
	}
	// set point for every field
	for(i = 0; i < dimY; i++) {
		int j;
		for(j = 0; j < dimX; j++) {
			pthread_mutex_init(&grid[i][j].mutex, NULL);
			pthread_cond_init (&grid[i][j].cond, NULL);
			grid[i][j].value = '.';
		}
	}
	
	// initialize threads
	// plus one for view
	pthread_t threads[count_player+1];
	pthread_attr_t attr;
	/* Initialisiere mutex und Condition-Variablen */
	pthread_mutex_init(&count_mutex, NULL);
	pthread_cond_init (&count_threshold_cv, NULL);
	/* Erzeuge Threads in einem Zustand, sodass join anwendbar ist */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
	// open logfile
	output = fopen(FILENAME, "w");
	
	for(i = 0; i < count_player; i++) {
		switch(player_typ_nr[i]){
			case 1:
				pthread_create(&threads[i+1], &attr, cycle, (void *)(i+1));
				break;
			/*case 2:
				pthread_create(&threads[i+1], &attr, cycle2, (void *)(i+1));
				break;*/
		}
	}
	// show view after cycles set position
	pthread_create(&threads[0], &attr, print_grid, (void *)0);
	/* Wait for all threads to complete */
	for (i = 0; i < count_player+1; i++) {
		pthread_join(threads[i], NULL);
	}
	fclose(output);
	return EXIT_SUCCESS;
}

void *print_grid(void *t){
	//int status;
	int i;
	int j;
	while(1) {
		system("clear");
		for(i = 0; i < dimY; i++) {
			for(j = 0; j < dimX; j++) {
				printf("%c", grid[i][j].value);
			}
			printf("\n");
		}
		// if game ends
		if(count_player == 0){
			return 0;
		}
		usleep(REFRESH);
	}	
}

void *cycle(void *t){
	int player_id = (int)t-1;
	int status;
	// check if no field is empty
	int isSet;
	status = pthread_mutex_lock(&grid[player_posY[player_id]][player_posX[player_id]].mutex);
	grid[player_posY[player_id]][player_posX[player_id]].value=(player_id+65);
	status = pthread_cond_signal(&grid[player_posY[player_id]][player_posX[player_id]].cond);
	status = pthread_mutex_unlock(&grid[player_posY[player_id]][player_posX[player_id]].mutex);
	player_count_steps[player_id] = 1;
	usleep(SPEED);
	while(1){
		// left
		if(player_posX[player_id]!=0 && grid[player_posY[player_id]][player_posX[player_id] -1 ].value == '.'){
			status = pthread_mutex_lock(&grid[player_posY[player_id]][player_posX[player_id] -1 ].mutex);
			player_posX[player_id] -= 1;
			isSet = 1;
		// up
		} else if (player_posY[player_id]!=0 && grid[player_posY[player_id] -1 ][player_posX[player_id]].value == '.'){
			status = pthread_mutex_lock(&grid[player_posY[player_id] -1 ][player_posX[player_id]].mutex);
			player_posY[player_id] -= 1;
			isSet = 1;
		// right
		} else if (player_posX[player_id]!=dimX-1 && grid[player_posY[player_id]][player_posX[player_id] +1 ].value == '.'){
			status = pthread_mutex_lock(&grid[player_posY[player_id]][player_posX[player_id] +1 ].mutex);
			player_posX[player_id] += 1;
			isSet = 1;
		// down
		} else if (player_posY[player_id]!=dimY-1 && grid[player_posY[player_id] +1 ][player_posX[player_id]].value == '.'){
			status = pthread_mutex_lock(&grid[player_posY[player_id] +1 ][player_posX[player_id]].mutex);
			player_posY[player_id] += 1;
			isSet = 1;
		}
		// comment if-statement for excercise 6
		if(count_player<=1){
			grid[player_posY[player_id]][player_posX[player_id]].value=(player_id+65);
			count_player -= 1;
			
			player_count_steps[player_id]++;
			int i;
			while(1) {
				for(i = 0; i < number_player; i++) {
					if(player_count_steps[i]>player_count_steps[player_id]) {
						// verlierer
						pthread_mutex_lock(&file_mutex);
						fprintf(output, "Lichtrenner %c terminiert auf Position %i,%i (Thread %i).\n", player_id+65, player_posX[player_id], player_posY[player_id], player_id+1);
						fflush(output);
						pthread_mutex_unlock(&file_mutex);
						return 0;	
					} else if(count_player<=1 && i==number_player-1){
						// sieger
						pthread_mutex_lock(&file_mutex);
						fprintf(output, "Lichtrenner %c verbleibt auf Position %i,%i (Thread %i).\n", player_id+65, player_posX[player_id], player_posY[player_id], player_id+1);
						fflush(output);
						pthread_mutex_unlock(&file_mutex);
						return 0;
					} else if(count_player<0 && i==number_player-1){
						// kein sieger
						pthread_mutex_lock(&file_mutex);
						fprintf(output, "Kein Lichtrenner verbleibt am Grid.\n");
						fflush(output);
						pthread_mutex_unlock(&file_mutex);
						return 0;	
					}
				}
				usleep(SPEED);
			}
			status = pthread_cond_signal(&grid[player_posY[player_id]][player_posX[player_id]].cond);
			status = pthread_mutex_unlock(&grid[player_posY[player_id]][player_posX[player_id]].mutex);
		}
		if(!isSet){
			grid[player_posY[player_id]][player_posX[player_id]].value=(player_id+97);
			count_player -= 1;
			
			player_count_steps[player_id]++;
			int i;
			while(1) {
				for(i = 0; i < number_player; i++) {
					if(player_count_steps[i]>player_count_steps[player_id]) {
						// verlierer
						pthread_mutex_lock(&file_mutex);
						fprintf(output, "Lichtrenner %c terminiert auf Position %i,%i (Thread %i).\n", player_id+65, player_posX[player_id], player_posY[player_id], player_id+1);
						fflush(output);
						pthread_mutex_unlock(&file_mutex);
						return 0;	
					} else if(count_player<=1 && i==number_player-1){
						// sieger
						pthread_mutex_lock(&file_mutex);
						fprintf(output, "Lichtrenner %c terminiert auf Position %i,%i (Thread %i).\n", player_id+65, player_posX[player_id], player_posY[player_id], player_id+1);
						fflush(output);
						pthread_mutex_unlock(&file_mutex);
						return 0;
					} else if(count_player<1 && i==number_player-1){
						// kein sieger
						pthread_mutex_lock(&file_mutex);
						fprintf(output, "Lichtrenner %c terminiert auf Position %i,%i (Thread %i).\n", player_id+65, player_posX[player_id], player_posY[player_id], player_id+1);
						fprintf(output, "Kein Lichtrenner verbleibt am Grid.\n");
						fflush(output);
						pthread_mutex_unlock(&file_mutex);
						//fprintf(output, "
						return 0;	
					}
				}
				usleep(SPEED);
			}
			status = pthread_cond_signal(&grid[player_posY[player_id]][player_posX[player_id]].cond);
			status = pthread_mutex_unlock(&grid[player_posY[player_id]][player_posX[player_id]].mutex);
		}
		grid[player_posY[player_id]][player_posX[player_id]].value=(player_id+65);
		isSet = 0;
		status = pthread_cond_signal(&grid[player_posY[player_id]][player_posX[player_id]].cond);
		status = pthread_mutex_unlock(&grid[player_posY[player_id]][player_posX[player_id]].mutex);
		player_count_steps[player_id]++;
		usleep(SPEED);
	}
	return NULL;
}

// Test typ2 
/*void *cycle2(void *t){
	int player_id = (int)t-1;
	int status;
	// check if no field is empty
	int isSet;
	status = pthread_mutex_lock(&grid[player_posY[player_id]][player_posX[player_id]].mutex);
	grid[player_posY[player_id]][player_posX[player_id]].value=(player_id+65);
	status = pthread_mutex_unlock(&grid[player_posY[player_id]][player_posX[player_id]].mutex);
	if (status != 0)
	while(1){
		status = pthread_mutex_lock(&grid[player_posY[player_id]][player_posX[player_id] -1 ].mutex);
		status = pthread_mutex_lock(&grid[player_posY[player_id] -1 ][player_posX[player_id]].mutex);
		status = pthread_mutex_lock(&grid[player_posY[player_id]][player_posX[player_id] +1 ].mutex);
		status = pthread_mutex_lock(&grid[player_posY[player_id] +1 ][player_posX[player_id]].mutex);
		// down
		 if (player_posY[player_id]!=dimY-1 && grid[player_posY[player_id] +1 ][player_posX[player_id]].value == '.'){
			player_posY[player_id] += 1;
			isSet = 1;
		// right
		} else if (player_posX[player_id]!=dimX-1 && grid[player_posY[player_id]][player_posX[player_id] +1 ].value == '.'){
			player_posX[player_id] += 1;
			isSet = 1;
		// up
		} else if (player_posY[player_id]!=0 && grid[player_posY[player_id] -1 ][player_posX[player_id]].value == '.'){
			player_posY[player_id] -= 1;
			isSet = 1;
		// left
		} else if(player_posX[player_id]!=0 && grid[player_posY[player_id]][player_posX[player_id] -1 ].value == '.'){
			player_posX[player_id] -= 1;
			isSet = 1;
		}
		if(count_player==1){
			grid[player_posY[player_id]][player_posX[player_id]].value=(player_id+65);
			count_player -= 1;
			return 0;
		}
		if(!isSet){
			grid[player_posY[player_id]][player_posX[player_id]].value=(player_id+97);
			count_player -= 1;
			return 0;	
		}
		grid[player_posY[player_id]][player_posX[player_id]].value=(player_id+65);
		isSet = 0;
		status = pthread_mutex_unlock(&grid[player_posY[player_id]][player_posX[player_id] -1 ].mutex);
		status = pthread_mutex_unlock(&grid[player_posY[player_id] -1 ][player_posX[player_id]].mutex);
		status = pthread_mutex_unlock(&grid[player_posY[player_id]][player_posX[player_id] +1 ].mutex);
		status = pthread_mutex_unlock(&grid[player_posY[player_id] +1 ][player_posX[player_id]].mutex);
		//if (status != 0)
		usleep(SPEED);
	}
	return NULL;
}*/