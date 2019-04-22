#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <stdbool.h>
#include <fcntl.h>
#include <string.h>

typedef struct level{
	int l;
	int score;
	int nsec;
}level;

typedef struct block{
	char data[5][5];
	int width;
	int height;
}block;

typedef struct blockNode{	
	struct blockNode* link; 
	block piece;
}blockNode;

typedef struct playerNode{
	char name[20];
	int score;
	char date[20];
	struct playerNode * next;
}playerNode;

typedef struct game{
	//2d array of the game UI
	char **game;
	char player[20];
	int width;
	int height;
	int level;
	int gameover;
	int score;
	//struct of current falling block
	block current;
	//pointer to a queue of next blocks
	blockNode * queue;
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
	  " ZZ"},3, 2},

	{{" $$",
	  "$$ "},3, 2}
};

//array of levels with required scores, timespec tv_nsec values
//where nsec is nanoseconds of elapsed time
struct level levels[]={
	{1, 0, 1300000},
	{2, 1000, 1100000},
	{3, 5000, 900000},
	{4, 10000, 700000},
	{5, 20000, 500000},
	{6, 30000, 400000},
	{7, 40000, 300000},
	{8, 50000, 200000},
	{9, 60000, 100000},
	{10, 70000, 80000}
};

#define BLOCKS_SIZE (sizeof(blocks)/sizeof(struct block))
#define LEVELS_SIZE (sizeof(levels)/sizeof(struct level))
#define GAME_AREA_WIDTH 10
#define GAME_AREA_HEIGHT 20

//terminal IO initiate state
struct termios save;

//global prototype definitions
void runTetris();
void initialize(int, int, game *);
void printGame(game *);
int collisionTest(game *);
void pickBlock(game *);
void blockGravity(game *);
void blockFastFall(game *, struct timespec*);
void printBlock(game *);
void checkCompleteLine(game *);
void fallLine(game *, int);
int checkLevelFromScore(game *);
void rotateBlock(game *);

void initializeBlocks();
void addBlock(blockNode*, blockNode*);
void returnTerminal();
void setTerminal();
void cleanMemory(game *);

void readScores();
void writeScores(playerNode*);
void createPlayer(game*);

void viewScores();

playerNode * playerHEAD = NULL;

int main(){

	readScores();
	int choice = 0; 
	bool decision = true;

	while (decision) {

	printf("Tetris\n\n");
	printf("Created by:\nBenjamin Griggs\nNicole Griffin\nZayyad Atekoja\nAllison Babilonia\nDavid Szymanski\n\n");
	printf ("0. Exit game\n");
	printf ("1. PLay game\n");
	printf ("2. View High Score\n");
	scanf ("%d", &choice);
	fflush(stdin);

	switch (choice)
	{

	case 0: 
		printf ("Bye\n");
		decision = false; 
		break;
	case 1: 
		runTetris();
		decision = false; 
		break;
	case 2: 
		viewScores();
		break;
		
	 default:
		printf ("Error Try Again\n"); 
	}
	
}

}// main end

void initializeBlocks()
{
	int ctr = 0; 
	blockNode* pointer=(blockNode*)malloc(sizeof(blockNode));
	while (ctr < 15)
	{
	blockNode* blockN =(blockNode*)malloc(sizeof(blockNode)); 
	blockN->piece =blocks[random() % BLOCKS_SIZE];		
	addBlock(blockN,pointer);
	ctr ++; 
	}
}

void addBlock(blockNode* newblock, blockNode* pointer){
	while (pointer->link != NULL)
	{
	pointer = pointer->link; 
	}
	pointer->link = newblock; 
}

void runTetris(){
	struct timespec tm;
	game g;
	char input;
	int count=0;

	char str[20];
	printf("Enter your Name.\n");	
	scanf("%s", str);
	strcpy(g.player, str);

	setTerminal();
	initialize(GAME_AREA_WIDTH, GAME_AREA_HEIGHT, &g);
	srand(time(NULL));

	tm.tv_sec=0;
	tm.tv_nsec=1000000;

	pickBlock(&g);
	//initialize the queue here 
	while (!g.gameover) {
		nanosleep(&tm, NULL);
		count++;
		if (count%50 == 0) {
			printGame(&g);
		}
		if (count%350 == 0) {
			blockGravity(&g);
			checkCompleteLine(&g);
		}
		while ((input=getchar())>0) {
			switch (input) {
				case 'a':
				g.x--;
				if (collisionTest(&g))
				g.x++;
				break;
				case 'd':
				g.x++;
				if (collisionTest(&g))
				g.x--;
				break;
				case 's':
				blockGravity(&g);
				break;
				case ' ':
				rotateBlock(&g);
				break;
				case 'w':
				blockFastFall(&g, &tm);
				break;
			}
		}
		tm.tv_nsec=checkLevelFromScore(&g);
	}

	printGame(&g);
	printf("*** GAME OVER ***\n");

	cleanMemory(&g);
	returnTerminal();
	createPlayer(&g);
	writeScores(playerHEAD);
} //runTetris end

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
	for (x = 0; x < width; x++) {
		g->game[x] = malloc(sizeof(char)*height);
		for (y = 0; y < height; y++)
			g->game[x][y] = ' ';
	}
}//initialize end

//printf the game to the terminal
void printGame(game *g) {
	int x,y;
	//seperate new UI from last
	for (x = 0; x < 30; x++)
		printf("\n");
	//display score info
	printf("{Score: %d | Level: %d}\n", g->score, g->level);
	//print top border
	for (x = 0; x < 2 * g->width + 2; x++)
		printf("=");
	printf("\n");
	//move down UI
	for (y = 0; y < g->height; y++) {
		// print side border
		printf ("|");
		for (x = 0; x < g->width; x++) {
			// print block if it should be in that position
			if (x >= g->x && y >= g->y && x < (g->x + g->current.width)
			 && y < (g->y + g->current.height) && g->current.data[y - g->y][x - g->x] != ' '){
				printf("%c ", g->current.data[y - g->y][x - g->x]);
			}
			// else print the game UI array value for that postion
			else
				printf("%c ", g->game[x][y]);
		}
		//print right border
		printf ("|\n");
	} // move down UI end

	//print bottom border
	for (x = 0; x < 2 * g->width + 2; x++)
		printf("=");
	//get to next line for input
	printf("\n");
}//printGame end

//return 1 if the piece cant move in a direction
int collisionTest(game *g) {
	int testX, testY;
	for (int x = 0; x < g->current.width; x++)
		for (int y = 0; y < g->current.height; y++) {
			testX = g->x + x;
			testY = g->y + y;
			//if attempted move is outside the borders
			if ( testX < 0 || testX >= g->width)
				return 1;
			//if the attempted move has a char there
			if ( g->current.data[y][x]!=' ') {
				if (( testY >= g->height) || (testX >= 0 && testX < g->width && testY >= 0 &&
					g->game[testX][testY]!=' ')){
					return 1;
				}
		}
	}
	return 0;
} //collisionTest end

//load the game with a new random block
void pickBlock(game *g) {
	//get randmon block from blocks array
	g->current = blocks[random() % BLOCKS_SIZE];
	//center the block
	g->x = (g->width / 2) - (g->current.width / 2);
	//align block to top
	g->y = 0;
	//if the block cant go down while it is at the top, then the game is over
	if (collisionTest(g)) {
		g->gameover=1;
	}
} //pickBlock end

//moves the block down
void blockGravity(game *g) {
	int x,y;
	//move block down
	g->y++;
	//check if block hit bottom
	if (collisionTest(g)) {
		g->y--;
		printBlock(g);
		pickBlock(g);
	}
} //blockGravity end

//moves the block all the way down
void blockFastFall(game *g, struct timespec* t) {
	while (collisionTest(g) == 0){
		t->tv_nsec = 1;
		g->y++;
	}
	t->tv_nsec = levels[g->level].nsec;
	g->y--;
	printBlock(g);
	pickBlock(g);
} //blockGravity end

//updates the game string to have the block in it
void printBlock(game *g) {
	int X,Y;
	for (int x = 0; x < g->current.width; x++)
		for (int y = 0; y < g->current.height; y++) {
			if (g->current.data[y][x] != ' ')
				g->game[g->x + x][g->y + y] = g->current.data[y][x];
	}
} //printBlock end

//look for full lines and update score
void checkCompleteLine(game *g) {
	int isComplete;
	int p = 50;
	//game string loop
	for (int y = g->height - 1; y >= 0; y--) {
		isComplete = 1;
		//line loop
		for (int x = 0; x < g->width && isComplete; x++) {
			//if any point on the line isnt complete
			if (g->game[x][y] == ' ') {
				isComplete = 0;
			}
		}
		// if the line is complete, update score, move blocks down
		if (isComplete) {
			g->score += p + 50 * levels[g->level].l;
			//give more points for many lines cleared at once
			p *= 2;
			fallLine(g, y);
			y++;
		}
	}
} // checkClearedLines end

//move everything above the cleared line down 1
void fallLine(game *g, int line) {
	int x;
	//start at the complete line and move up
	for (int y = line; y > 0; y--) {
		for (x = 0; x < g->width; x++)
			g->game[x][y] = g->game[x][y-1];
	}
	//?
	//for (x = 0; x < g->width; x++)
		//g->game[x][0] = ' ';
} //fall end

// returns the nsec of the level the user is on, updates the level if the user leveled up
int checkLevelFromScore(game *g) {
	for (int i = 0; i < LEVELS_SIZE; i++) {
		//the players score is enough to increase the level
		if (g->score >= levels[i].score) {
			g->level = i+1;
		} else break;
	}
	return levels[g->level-1].nsec;
} // checkLevelFromScore end

//rotates the games current block and checks for collision
void rotateBlock(game *g) {
	//block to be rotated
	block rotate = g->current;
	//store the original state 
	block safe = rotate;
	int x,y;
	rotate.width = safe.height;
	rotate.height = safe.width;
	//rotate the block
	for (x = 0; x < safe.width; x++)
		for (y = 0; y < safe.height; y++) {
			rotate.data[x][y] = safe.data[safe.height - y - 1][x];
		}
	//save the original position in case of restore
	x = g->x;
	y = g->y;
	//align the rotated block to the same position as the original block
	g->x -= (rotate.width - safe.width) / 2;
	g->y -= (rotate.height - safe.height) / 2;
	//set the game's block to the rotated one
	g->current = rotate;
	//if the rotated block colides with something, restore the safe version
	if (collisionTest(g)) {
		g->current = safe;
		g->x = x;
		g->y = y;
	}
} // rotateBlock end

//return terminal to pre-game state
void returnTerminal() {
	tcsetattr(fileno(stdin),TCSANOW,&save);
}

//Make input auto-enter?
void setTerminal() {
	struct termios custom;
	int fd = fileno(stdin);
	tcgetattr(fd, &save);
	custom=save;
	custom.c_lflag &= ~(ICANON|ECHO);
	tcsetattr(fd,TCSANOW,&custom);
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0)|O_NONBLOCK);
}

//release memory
void cleanMemory(game *g) {
	int x;
	for (x = 0; x < g->width; x++) {
		free(g->game[x]);
	}
	free(g->game);
}// cleanMemory end

//get the scores from the file
void readScores(){
	FILE *fin;
	fin = fopen("scores.dat", "a");
	if (fin == NULL){
		printf("scores.dat was not found.\n");
		fclose(fin);
		return;
	}
	playerHEAD = (playerNode*)malloc(sizeof(playerNode));
	playerNode * current = playerHEAD;
	while (fscanf(fin,"%20s%20s%d\n", current->name, current->date, &current->score) != EOF){
		current->next = (playerNode*)malloc(sizeof(playerNode));
		current = current->next;
	}
	fclose(fin);
}// readScores end

//store the scores to the file
void writeScores(playerNode* head){
	FILE *fout;
	fout = fopen("scores.dat", "w");
	playerNode * current = head;
	int counter = 1;
	while (current != NULL){
		fprintf(fout,"%-20s%-20s%d\n", current->name, current->date ,current->score);
		current = current->next;
	}
	fclose(fout);
}// writeScores end

void createPlayer(game *g){
	if (g->score == 0)
		return;
	char date[20] = "today";
	if(playerHEAD == NULL){
		playerNode * temp = (playerNode*)malloc(sizeof(playerNode));
		strcpy(temp->name, g->player);
		temp->score = g->score;
		strcpy(temp->date,date);
		temp->next = NULL;
		playerHEAD = temp;
		return;
	}
	if (g->score > playerHEAD->score){
		playerNode * temp = (playerNode*)malloc(sizeof(playerNode));
		strcpy(temp->name, g->player);
		temp->score = g->score;
		strcpy(temp->date,date);
		temp->next = playerHEAD;
		playerHEAD = temp;
		return;
	}
	playerNode * temp = (playerNode*)malloc(sizeof(playerNode));
	strcpy(temp->name, g->player);
	temp->score = g->score;
	strcpy(temp->date,date);

	playerNode * current = playerHEAD;
	while(g->score < current->next->score)
		current = current->next;
	temp->next = current->next;
	current->next = temp;
}// createPlayer end

void viewScores(){
	if(playerHEAD == NULL)
		printf("There are no scores yet.\n");
	playerNode * current = playerHEAD;
	while(current != NULL){
		printf("%-20s%-20s%d\n", current->name, current->date ,current->score);
		current = current->next;
	}
}
