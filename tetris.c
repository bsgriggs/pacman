#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>

typedef struct level{
	int score;
	int nsec;
}level;

typedef struct game{
	//2d array of the game UI
	char **game;
	int width;
	int height;
	int level;
	int gameover;
	int score;
	//struct for new blocks
	struct block{
		char data[5][5];
		int width;
		int height;
	}block;
	int x;
	int y;
}game;

//array of tetris pieces
struct block blocks[] ={
	{{"##", 
	  "##"}, 2, 2},

	{{" X ",
	  "XXX"}, 3, 2},

	{{"@@@@"},4, 1},
	
	{{"OO",
	  "O ",
	  "O "}, 2, 3},

	{{"&&",
	  " &",
	  " &"}, 2, 3},

	{{"ZZ ",
	  " ZZ"},3, 2}
};

//array of levels with required scores, timespec tv_nsec values
//where nsec is nanoseconds of elapsed time
struct level levels[]={
	{0, 1200000},
	{1500, 900000},
	{8000, 700000},
	{20000, 500000},
	{40000, 400000},
	{75000, 300000},
	{100000, 200000}
};

#define BLOCKS_SIZE (sizeof(blocks)/sizeof(struct block))
#define LEVELS_SIZE (sizeof(levels)/sizeof(struct level))

struct termios save;

//global prototype definitions
void initialize(int, int, game*);
void printGame(game*);
int collisionTest(game*);

int main(){
	

	return 1;
}// main end

//set defaults and allocated memory for the game UI
void initialize(int width, int height, game *g) {
	//set defaults
	int x, y;
	g->level = 1;
	g->score = 0;
	g->gameover = 0;
	g->width = width;
	g->height = height;
	
	//allocated memory for the game UI
	g->game = malloc(sizeof(char *)*width);
	for (x=0; x<width; x++) {
		g->game[x] = malloc(sizeof(char)*height);
		for (y=0; y<height; y++)
			g->game[x][y] = ' ';
	}
}//initialize end

//printf the game to the terminal
void printGame(game *g) {
	int x,y;
	//seperate new UI from last
	for (x=0; x<30; x++)
		printf("\n");
	//display score info
	printf("{Current Score: %d | Current Level: %d]\n", g->score, g->level);
	//print top border
	for (x=0; x<2*g->width+2; x++)
		printf("=");
	printf("\n");
	//move down UI
	for (y=0; y<g->height; y++) {
		// print side border
		printf ("|");
		for (x=0; x<g->width; x++) {
			// print block if it is should be in that position
			if (x>=g->x && y>=g->y
			 && x<(g->x+g->block.width)
			 && y<(g->y+g->block.height) 
			 && g->block.data[y-g->y][x-g->x]!=' ')
				printf("%c ", g->block.data[y-g->y][x-g->x]);
			// else print the game UI array value for that postion
			else
				printf("%c ", g->game[x][y]);
		}
		//print right border
		printf ("|\n");
	} // move down UI end

	//print bottom border
	for (x=0; x<2*g->width+2; x++)
		printf("=");
	//get to next line for input
	printf("\n");
}//printGame end

//return 1 if the piece cant move in a direction
int collisionTest(game *g) {
	int testX, testY;
	struct block current = g->block;

	for (int x = 0; x < current.width; x++)
		for (int y = 0; y < current.height; y++) {
			testX = g->x + x;
			testY = g->y + y;
			if ( testX < 0 || testX >= g->width)
				return 1;
			if ( current.data[y][x]!=' ') {
				if (( testY >= g->height) || (testX >= 0 && testX < g->width 
					&& testY >= 0 && g->game[testX][testY]!=' '))
					return 1;
		}
	}
	return 0;
}

//load the game with a new random block
void pickBlock(game *g) {
	//get randmon block from blocks array
	g->block = blocks[random() % BLOCKS_SIZE];
	//center the block
	g->x = (g->width / 2) - (g->block.width / 2);
	//align block to top
	g->y = 0;
	//if the block cant go down while its at the top, then the game is over
	if (collisionTest(g)) {
		g->gameover=1;
	}
}
