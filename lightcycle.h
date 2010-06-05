#include "tron.h"

class Lightcycle {
	public:
	field **grid;
	FILE *output;
	int *player_posX;
	int *player_posY;
	int *dimX;
	int *dimY;
	int *count_player;
	int *number_player;
	int *player_count_steps;
	pthread_mutex_t *file_mutex;

	/*Lightcycle(field **grid, FILE *output, int *player_posX, int *player_posY, int *dimX, int *dimY, int *count_player, int *number_player, int *player_count_steps, pthread_mutex_t *file_mutex) : 
		grid(grid), output(output), player_posX(player_posX), player_posY(player_posY), dimX(dimX), dimY(dimY), count_player(count_player),
		number_player(number_player), player_count_steps(player_count_steps), file_mutex(file_mutex) {
	}*/	

	virtual void *compute(void *t) = 0;
};

class Lightcycle1 : public Lightcycle {
	public:
	Lightcycle1(field **grid, FILE *output, int *player_posX, int *player_posY, int *dimX, int *dimY, int *count_player, int *number_player, int *player_count_steps, pthread_mutex_t *file_mutex) {

		//Lightcycle(grid, output, player_posX, player_posY, dimX, dimY, count_player, number_player, player_count_steps, file_mutex);
		this->grid = grid;
		this->output = output;
		this->player_posX = player_posX;
		this->player_posY = player_posY;
		this->dimX = dimX;
		this->dimY = dimY;
		this->count_player = count_player;
		this->number_player = number_player;
		this->player_count_steps = player_count_steps;
		this->file_mutex = file_mutex;
	}
	void *compute(void *t) {
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
			} else if (player_posX[player_id]!=*dimX-1 && grid[player_posY[player_id]][player_posX[player_id] +1 ].value == '.'){
				status = pthread_mutex_lock(&grid[player_posY[player_id]][player_posX[player_id] +1 ].mutex);
				player_posX[player_id] += 1;
				isSet = 1;
			// down
			} else if (player_posY[player_id]!=*dimY-1 && grid[player_posY[player_id] +1 ][player_posX[player_id]].value == '.'){
				status = pthread_mutex_lock(&grid[player_posY[player_id] +1 ][player_posX[player_id]].mutex);
				player_posY[player_id] += 1;
				isSet = 1;
			}
			// comment if-statement for excercise 6
			if(*count_player<=1){
				grid[player_posY[player_id]][player_posX[player_id]].value=(player_id+65);
				count_player -= 1;
			
				player_count_steps[player_id]++;
				int i;
				while(1) {
					for(i = 0; i < *number_player; i++) {
						if(player_count_steps[i]>player_count_steps[player_id]) {
							// verlierer
							pthread_mutex_lock(file_mutex);
							fprintf(output, "Lichtrenner %c terminiert auf Position %i,%i (Thread %i).\n", player_id+65, player_posX[player_id], player_posY[player_id], player_id+1);
							fflush(output);
							pthread_mutex_unlock(file_mutex);
							return 0;	
						} else if(*count_player<=1 && i==*number_player-1){
							// sieger
							pthread_mutex_lock(file_mutex);
							fprintf(output, "Lichtrenner %c verbleibt auf Position %i,%i (Thread %i).\n", player_id+65, player_posX[player_id], player_posY[player_id], player_id+1);
							fflush(output);
							pthread_mutex_unlock(file_mutex);
							return 0;
						} else if(*count_player<0 && i==*number_player-1){
							// kein sieger
							pthread_mutex_lock(file_mutex);
							fprintf(output, "Kein Lichtrenner verbleibt am Grid.\n");
							fflush(output);
							pthread_mutex_unlock(file_mutex);
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
					for(i = 0; i < *number_player; i++) {
						if(player_count_steps[i]>player_count_steps[player_id]) {
							// verlierer
							pthread_mutex_lock(file_mutex);
							fprintf(output, "Lichtrenner %c terminiert auf Position %i,%i (Thread %i).\n", player_id+65, player_posX[player_id], player_posY[player_id], player_id+1);
							fflush(output);
							pthread_mutex_unlock(file_mutex);
							return 0;	
						} else if(*count_player<=1 && i==*number_player-1){
							// sieger
							pthread_mutex_lock(file_mutex);
							fprintf(output, "Lichtrenner %c terminiert auf Position %i,%i (Thread %i).\n", player_id+65, player_posX[player_id], player_posY[player_id], player_id+1);
							fflush(output);
							pthread_mutex_unlock(file_mutex);
							return 0;
						} else if(*count_player<1 && i==*number_player-1){
							// kein sieger
							pthread_mutex_lock(file_mutex);
							fprintf(output, "Lichtrenner %c terminiert auf Position %i,%i (Thread %i).\n", player_id+65, player_posX[player_id], player_posY[player_id], player_id+1);
							fprintf(output, "Kein Lichtrenner verbleibt am Grid.\n");
							fflush(output);
							pthread_mutex_unlock(file_mutex);
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
};
