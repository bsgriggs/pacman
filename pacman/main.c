#include <stdio.h>
#include <stdlib.h>
#include "SDL2/SDL.h"

int main(int argc, char** argv) {
	//Initialize Graphics 
	if(SDL_Init(SDL_INIT_VIDEO) != 0){
		//initialization failed
		printf("error initializing SDL: %s\n", SDL_GetError());
		return 1;
	}

	return (EXIT_SUCCESS);
}

