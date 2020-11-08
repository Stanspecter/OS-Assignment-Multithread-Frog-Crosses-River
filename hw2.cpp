#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <curses.h>
#include <termios.h>
#include <fcntl.h>
//#include <random>

#define ROW 10
#define COLUMN 50 
#define LENGTH 17 //Log Length

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
pthread_cond_t thrhold;
int gamestatus; // Flag of gamestatus

struct Node{
	int x , y; 
	Node( int _x , int _y ) : x( _x ) , y( _y ) {}; 
	Node(){} ; 
} frog ; 

char map[ROW+1][COLUMN] ; 

// Determine a keyboard is hit or not. If yes, return 1. If not, return 0. 
int kbhit(void){
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);

	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);

	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);

	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if(ch != EOF)
	{
		ungetc(ch, stdin);
		char mv = getchar();
		if (mv == 'q' || mv == 'Q'){
			return 9;
		}
		if (mv == 'w' || mv == 'W'){
			return 1;
		}
		if (mv == 'a' || mv == 'A'){
			return 2;
		}
		if (mv == 's' || mv == 'S'){
			return 3;
		}
		if (mv == 'd' || mv == 'D'){
			return 4;
		}
	}
	return 0;
}

void sleeper(int row, int slpr[ROW]){
	usleep(slpr[row]);	
}


//Recieve the keyboard movement, denote signals by integers

void *logs_move( void *t ){
	int tid = (long)t;
	int row =  tid+1; // Horizontal Position
	pthread_mutex_lock(&mutex1); // Lock Map

	for (int l = 0; l < LENGTH; l++){
		map[row][l] = '=';
	}
	for (int c = LENGTH; c < COLUMN; c++){
		map[row][c] = ' ';
	}

	
	int temp_row[COLUMN];
	int addv=(row % 2 == 1)?1:49;
	pthread_mutex_unlock(&mutex1);

	int sleept[10];
	for (int i = 0; i < 10; i++){
		sleept[i] = 100000+(rand()%50)*1000;
		//printf("sleeper %d", sleeper[i]);
	}


	while(gamestatus==1){
		
		pthread_mutex_lock(&mutex1);
		pthread_mutex_lock(&mutex2);
	/*  Move the logs  */
		/*Check through the logs, 
		*change where the frog locates to "0",
		*and the old location of frag back to "="
		*/
		for (int col = 0; col < COLUMN; col++){
			if (row != frog.x) {
				if(map[row][col] == '0') {
					temp_row[(col+addv)%COLUMN] = '=';
				}
				else {
					temp_row[(col+addv)%COLUMN] = map[row][col];
				}
			}
			else {
				/*Check if the frog is on the log.
				*Update the map to the latest version
				*/
				if (col != frog.y && map[row][col] == '0') {
					temp_row[(col+addv)%COLUMN] = '=';
				}
				else {
					if(col == frog.y && map[row][col] != '0') {
					temp_row[(col+addv)%COLUMN] = '0';
					}
				//move the log
					else {
						temp_row[(col+addv)%COLUMN] = map[row][col];
					}
				}
			}
			
		}
		for (int col = 0; col < COLUMN; col++){
			
			map[row][col] = temp_row[col];	
		}
	/*  Check keyboard hits, to change frog's position or quit the game. */
		int mv = kbhit();
		if (mv){
			switch(mv){
			case 9://Quit
				gamestatus = 0;
				break;
			case 1://Go up
				if (map[frog.x - 1][frog.y] == '=' || map[frog.x - 1][frog.y] == '|'){
					if (frog.x==ROW){
						map[frog.x][frog.y] = '|';
					}
					else{
						map[frog.x][frog.y] = '=';
					}
					frog.x-=1;
					map[frog.x][frog.y] = '0';
				}
				else{
					gamestatus = 3;
				}
				break;
			case 2://Go left
				if (map[frog.x][frog.y-1] == '=' || map[frog.x][frog.y-1] == '|'){
					if (frog.x==ROW){
						map[frog.x][frog.y] = '|';
					}
					else{
						map[frog.x][frog.y] = '=';
					}
					frog.y-=1;
					map[frog.x][frog.y] = '0';
				}
				else{
					gamestatus = 3;
				}
				break;
			case 3://Go down
				if (map[frog.x + 1][frog.y] == '=' || map[frog.x + 1][frog.y] == '|'){
					if (frog.x==ROW){
						map[frog.x][frog.y] = '|';
					}
					else{
						map[frog.x][frog.y] = '=';
					}
					frog.x+=1;
					map[frog.x][frog.y] = '0';
				}
				else{
					gamestatus = 3;
				}
				break;
			case 4://Go right
				if (map[frog.x][frog.y + 1] == '=' || map[frog.x][frog.y + 1] == '|'){
					if (frog.x==ROW){
						map[frog.x][frog.y] = '|';
					}
					else{
						map[frog.x][frog.y] = '=';
					}
					frog.y+=1;
					map[frog.x][frog.y] = '0';
				}
				else{
					gamestatus = 3;
				}
				break;
			default:
				break;

			}
		}
	
	/*  Check game's status  */
		if (frog.x == 0) {
				gamestatus = 2;			//win the game
				
			}
		
		


	/*  Print the map on the screen  */
		if(row == frog.x){
			frog.y = (frog.y+addv)%COLUMN;
		}

		if (frog.y == 0 || frog.y == COLUMN-1){
			gamestatus = 4; 	//touch the boundary
		}
		
		printf("\033[H");//Relocates the cursor
		for (int r = 0; r <= ROW; r++){
			for (int c = 0; c < COLUMN; c++){
				printf("%c", map[r][c]);
			}
			printf("\n");
		}

		pthread_mutex_unlock(&mutex2);
		pthread_mutex_unlock(&mutex1);
		//default_random_engine e;
		//uniform_int_distribution<unsigned> u(70000, 300000); //Used for C++11
		sleeper(row,sleept);
	}
	pthread_exit(NULL);
}

int main( int argc, char *argv[] ){

	// Initialize the river map and frog's starting position
	printf("\033[H\033[2J");
	int i , j ; 
	for( i = 1; i < ROW; ++i ){	
		for( j = 0; j < COLUMN - 1; ++j )	
			map[i][j] = ' ' ;  
	}	

	for( j = 0; j < COLUMN - 1; ++j )	
		map[ROW][j] = map[0][j] = '|' ;

	for( j = 0; j < COLUMN - 1; ++j )	
		map[0][j] = map[0][j] = '|' ;

	frog = Node( ROW, (COLUMN-1) / 2 ) ; 
	map[frog.x][frog.y] = '0' ; 

	//Print the map
	printf("\033[H");//Relocates the cursor
		for (int r = 0; r <= ROW; r++){
			for (int c = 0; c < COLUMN; c++){
				printf("%c", map[r][c]);
			}
			printf("\n");
		}

	gamestatus=1; //Game exits with status other than 1


	/*  Create pthreads for wood move and frog control.  */
	pthread_mutex_init(&mutex1, NULL);
	pthread_mutex_init(&mutex2, NULL);
	pthread_cond_init(&thrhold, NULL);
	pthread_attr_t attr;
	pthread_t threads[ROW-1];
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    int rc1, rc2;
	
	for(long i = 0; i<ROW-1; i++){
        rc1 = pthread_create(&threads[i], &attr, logs_move, (void*)i);
        if(rc1){
            printf("ERROR: return code from pthread_create() is %d", rc1);
            exit(1);
        }
	}

	for(long i = 0; i<ROW-1; i++){
        rc2 = pthread_join(threads[i], NULL);;
        if(rc2){
            printf("ERROR: return code from pthread_join() is %d", rc2);
            exit(1);
        }
    }
    
	/*  Display the output for user: win, lose or quit.  */
	if (!gamestatus){
		printf("\n--YOU QUIT THE GAME--\n");
	}
	else{
		switch (gamestatus){
		case 2: 
			printf("\n--YOU WIN THE GAME--\n");
			break;
		case 3:
			printf("\n--YOU LOSE THE GAME--\nYou jumped into the river.\n");
			break;
		case 4:
			printf("\n--YOU LOSE THE GAME--\nYou touched the boundary.\n");
			break;
		default:
			break;
		}
	} 

	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&mutex1);
	pthread_cond_destroy(&thrhold);
    
    pthread_exit(NULL);
	
	
	return 0;

}
