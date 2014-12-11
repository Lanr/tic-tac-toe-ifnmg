#ifdef _linux_
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_ttf.h>
	#include <SDL2/SDL_image.h>
	#include <SDL2/SDL_mixer.h>
	#include <cstdio>
#elif _WIN32
	#include <SDL.h>
	//#include <SDL_ttf.h>
	#include <SDL_image.h>
	#include <SDL_mixer.h>
	#include <cstdio>
#endif

const int SCREEN_WIDTH = 311;
const int SCREEN_HEIGHT = 308;

const int BUTTON_WIDTH = 94;
const int BUTTON_HEIGHT = 94;
const int BAR_SIZE = 4;
const int COMP = 4;

#define GRID_SIZE 3
#define CIRCLE 'O'
#define CROSS 'X'
#define NULLCHAR '\0'

int pressedPosition;
int posX, posY; //Posições da matriz que representa o grid
char grid[GRID_SIZE][GRID_SIZE];
char player;
char winner;
int ocupados;

//Imagens
SDL_Surface *imgGrid = NULL;
SDL_Surface *imgX = NULL;
SDL_Surface *imgO = NULL;
SDL_Surface *imgNewGame = NULL;
SDL_Surface *screen = NULL;

//SFX
Mix_Chunk *sfxClickX = NULL;
Mix_Chunk *sfxClickO = NULL;
Mix_Chunk *sfxInvalidClick = NULL;
Mix_Chunk *sfxGameEnded = NULL;

//Janela
SDL_Window *window = NULL;

//Posições do grid
SDL_Rect gridRect[9];

//Eventos
SDL_Event event;


bool InitWindow(){

	if (SDL_Init(SDL_INIT_VIDEO < 0 | SDL_INIT_AUDIO) < 0){
		printf("Unable to load SDL! SDL Error: %s\n", SDL_GetError());
		return 0;
	}
	else{

		if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0){
			printf("Unable to load SDL_Mixer! Mix Error: %s\n", Mix_GetError());
			return 0;
		}

		window = SDL_CreateWindow("TIC-TAC-TOE", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

		if (window == NULL){
			printf("Unable to init window! SDL Error: %s\n", SDL_GetError());
			return 0;
		}
		else{
			screen = SDL_GetWindowSurface(window);
		}

	}

	int imgFlags = IMG_INIT_PNG;

	if (!(IMG_Init(imgFlags) & imgFlags)){
		printf("Unable to load SDL_image! SDL Error:%s\n", IMG_GetError());
		return 0;
	}

	if ((imgGrid = IMG_Load("img/grid.png")) == NULL){
		printf("Unable to load \"grid.png\"! SDL Error: %s\n", SDL_GetError());
		return 0;
	}

	if ((imgX = IMG_Load("img/x.png")) == NULL){
		printf("Unable to load \"x.png\"! SDL Error: %s\n", SDL_GetError());
		return 0;
	}

	if ((imgO = IMG_Load("img/o.png")) == NULL){
		printf("Unable to laod \"o.png\"! SDL Error: %s\n", SDL_GetError());
		return 0;
	}

	if ((imgNewGame = IMG_Load("img/new_game.png")) == NULL){
		printf("Unable to load \"new_game.png\"! SDL Error: %s\n", SDL_GetError());
		return 0;
	}

	if ((sfxClickX = Mix_LoadWAV("sfx/x_sound.wav")) == NULL){
		printf("Unable to load \"x_sound.wav\"! Mix Error: %s\n", Mix_GetError());
		return 0;
	}

	if ((sfxClickO = Mix_LoadWAV("sfx/o_sound.wav")) == NULL){
		printf("Unable to load \"o_sound.wav\"! Mix Error: %s\n", Mix_GetError());
		return 0;
	}

	if ((sfxGameEnded = Mix_LoadWAV("sfx/game_ended.wav")) == NULL){
		printf("Unable to load \"game_ended.wav\"! Mix Error: %s\n", Mix_GetError());
		return 0;
	}

	if ((sfxInvalidClick = Mix_LoadWAV("sfx/invalid_click_sound.wav")) == NULL){
		printf("Unable to load \"invalid_click_sound.wav\"! Mix Error: %s\n", Mix_GetError());
		return 0;
	}

	return 1;

}

void NewGameScreen(){
	//TODO
	//O "new game" deve funcionar como um botão, e não fazer parte do background

	SDL_BlitSurface(imgNewGame, NULL, screen, NULL);

}

void SetGridRect(){

	gridRect[0].x = gridRect[0].y = COMP;
	gridRect[0].w = BUTTON_WIDTH;
	gridRect[0].h = BUTTON_HEIGHT;

	gridRect[1].x = BUTTON_WIDTH + COMP * 3 + BAR_SIZE;
	gridRect[1].y = COMP;
	gridRect[1].w = BUTTON_WIDTH;
	gridRect[1].h = BUTTON_HEIGHT;

	gridRect[2].x = gridRect[1].x * 2 - COMP;
	gridRect[2].y = COMP;
	gridRect[2].w = BUTTON_WIDTH;
	gridRect[2].h = BUTTON_HEIGHT;

	gridRect[3].x = COMP;
	gridRect[3].y = BUTTON_WIDTH + BAR_SIZE + COMP * 3;
	gridRect[3].w = BUTTON_WIDTH;
	gridRect[3].h = BUTTON_HEIGHT;

	gridRect[4].x = gridRect[1].x;
	gridRect[4].y = BUTTON_WIDTH + BAR_SIZE + COMP * 3;
	gridRect[4].w = BUTTON_WIDTH;
	gridRect[4].h = BUTTON_HEIGHT;

	gridRect[5].x = gridRect[2].x;
	gridRect[5].y = BUTTON_WIDTH + BAR_SIZE + COMP * 3;
	gridRect[5].w = BUTTON_WIDTH;
	gridRect[5].h = BUTTON_HEIGHT;

	gridRect[6].x = COMP;
	gridRect[6].y = BUTTON_WIDTH * 2 + BAR_SIZE * 2 + COMP * 5;
	gridRect[6].w = BUTTON_WIDTH;
	gridRect[6].h = BUTTON_HEIGHT;

	gridRect[7].x = gridRect[1].x;
	gridRect[7].y = BUTTON_WIDTH * 2 + BAR_SIZE * 2 + COMP * 5;
	gridRect[7].w = BUTTON_WIDTH;
	gridRect[7].h = BUTTON_HEIGHT;

	gridRect[8].x = gridRect[2].x;
	gridRect[8].y = BUTTON_WIDTH * 2 + BAR_SIZE * 2 + COMP * 5;
	gridRect[8].w = BUTTON_WIDTH;
	gridRect[8].h = BUTTON_HEIGHT;
}

int GetPosition(int x, int y){

	if (x < gridRect[0].w && y < gridRect[0].h){
		pressedPosition = 1;
		posX = 0;
		posY = 0;
	}
	else if (x >= gridRect[1].x && y >= gridRect[1].y && x <= (gridRect[1].x + gridRect[1].w) && y <= (gridRect[1].y + gridRect[1].h)){
		pressedPosition = 2;
		posX = 0;
		posY = 1;
	}
	else if (x > gridRect[2].x && y > gridRect[2].y && x < (gridRect[2].x + gridRect[2].w) && y < (gridRect[2].y + gridRect[2].h)){
		pressedPosition = 3;
		posX = 0;
		posY = 2;
	}
	else if (x > gridRect[3].x && y > gridRect[3].y && x < (gridRect[3].x + gridRect[3].w) && y < (gridRect[3].y + gridRect[3].h)){
		pressedPosition = 4;
		posX = 1;
		posY = 0;
	}
	else if (x > gridRect[4].x && y > gridRect[4].y && x < (gridRect[4].x + gridRect[4].w) && y < (gridRect[4].y + gridRect[4].h)){
		pressedPosition = 5;
		posX = 1;
		posY = 1;
	}
	else if (x > gridRect[5].x && y > gridRect[5].y && x < (gridRect[5].x + gridRect[5].w) && y < (gridRect[5].y + gridRect[5].h)){
		pressedPosition = 6;
		posX = 1;
		posY = 2;
	}
	else if (x > gridRect[6].x && y > gridRect[6].y && x < (gridRect[6].x + gridRect[6].w) && y < (gridRect[6].y + gridRect[6].h)){
		pressedPosition = 7;
		posX = 2;
		posY = 0;
	}
	else if (x > gridRect[7].x && y > gridRect[7].y && x < (gridRect[7].x + gridRect[7].w) && y < (gridRect[7].y + gridRect[7].h)){
		pressedPosition = 8;
		posX = 2;
		posY = 1;
	}
	else if (x > gridRect[8].x && y > gridRect[8].y && x < (gridRect[8].x + gridRect[8].w) && y < (gridRect[8].y + gridRect[8].h)){
		pressedPosition = 9;
		posX = 2;
		posY = 2;
	}
	else{
		printf("Posição invalida.\n");
		return 0;
	}

	return 1;

}

void GetEvents(int player, bool &quit, bool &newGame){

	while (SDL_PollEvent(&event) != 0){

		if (event.type == SDL_QUIT){
			quit = true;
			return;
		}
		if (event.type == SDL_MOUSEBUTTONDOWN){

			int x, y;

			SDL_GetMouseState(&x, &y);

			if (newGame){

				if (((x >= 75) && (x <= 220)) && ((y >= 140) && (y <= 160))){
					newGame = false;
					return;
				}
				else if (((x >= 115) && (x <= 180)) && ((y >= 170) && (y <= 190))){
					quit = true;
					return;
				}

			}
			else{
				if (GetPosition(x, y)){

					if (player == CIRCLE){
						Mix_PlayChannel(-1, sfxClickO, 0);
					}
					else if (player == CROSS){
						Mix_PlayChannel(-1, sfxClickX, 0);
					}
				}
				else{
					Mix_PlayChannel(-1, sfxInvalidClick, 0);
				}
			}
		}
		if (event.type == SDL_KEYDOWN){

			if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
				quit = true;
				return;
			}

		}

	}

}

void NewRound(){

	for (int i = 0; i < GRID_SIZE; i++){
		for (int j = 0; j < GRID_SIZE; j++){
			grid[i][j] = NULLCHAR;
		}
	}

	imgGrid = IMG_Load("img/grid.png");

	if (imgGrid == NULL){
		printf("Unable to load \"grid.png\"! SDL Error: %s\n", SDL_GetError());
	}

	winner = NULLCHAR;
	ocupados = 0;
	player = CIRCLE;

}

bool CheckPosition(){

	if (pressedPosition != 0){
		if (grid[posX][posY] == NULLCHAR){
			grid[posX][posY] = player;
			player == CROSS ? player = CIRCLE : player = CROSS;
			return true;
		}
	}

	return false;
}

void CheckIfWon(char player){

	bool won;
	player == CROSS ? player = CIRCLE : player = CROSS;

	//Verifica linhas
	for (int i = 0; i < GRID_SIZE; i++){
		won = true;
		for (int j = 0; j < GRID_SIZE; j++){
			if (grid[i][j] != player){
				won = false;
			}
		}

		if (won){
			printf("Linhas!\n");
			winner = player;
			return;
		}
	}

	//Verifica colunas
	for (int i = 0; i < GRID_SIZE; i++){
		won = true;

		for (int j = 0; j < GRID_SIZE; j++){
			if (grid[j][i] != player){
				won = false;
			}
		}

		if (won){
			printf("Colunas!\n");
			winner = player;
			return;
		}
	}

	//Verifica diagonal principal
	won = true;
	for (int i = 0; i < GRID_SIZE; i++){
		if (grid[i][i] != player){
			won = false;
		}
	}

	if (won){
		printf("Diagonal principal!\n");
		winner = player;
		return;
	}

	//Verifica diagonal secundária
	won = true;
	for (int i = GRID_SIZE - 1, j = 0; i >= 0; i--, j++){
		if (grid[j][i] != player){
			won = false;
		}
	}

	if (won){
		printf("Diagonal secundária!\n");
		winner = player;
		return;
	}

}

void GameLoop(bool &quit, bool &newGame){

	while (!quit){
		pressedPosition = 0;

		GetEvents(player, quit, newGame);

		if (quit){
			break;
		}
		if (newGame){
			NewGameScreen();
		}
		else{

			if (CheckPosition()){
				SDL_BlitSurface(player == CIRCLE ? imgX : imgO, NULL, imgGrid, &gridRect[pressedPosition - 1]);
				ocupados++;
				if (ocupados > 4){
					//TODO
					CheckIfWon(player);

					if ((winner == CIRCLE) || (winner == CROSS) || (ocupados == 9)){
						if ((winner == CIRCLE) || (winner == CROSS)){
							printf("Player %c ganhou\n", winner);
							SDL_Delay(50);
							Mix_PlayChannel(-1, sfxGameEnded, 0);
						}
						else{
							printf("Empate!\n");
						}
						SDL_BlitSurface(imgGrid, NULL, screen, NULL);
						SDL_BlitSurface(player == CIRCLE ? imgX : imgO, NULL, imgGrid, &gridRect[pressedPosition - 1]);
						SDL_UpdateWindowSurface(window);
						SDL_Delay(2000);
						newGame = true;
						break;
					}
				}
			}

			SDL_BlitSurface(imgGrid, NULL, screen, NULL);
		}

		SDL_UpdateWindowSurface(window);
	}
}

void Close(){

	SDL_DestroyWindow(window);

	SDL_FreeSurface(imgGrid);
	SDL_FreeSurface(imgX);
	SDL_FreeSurface(imgO);
	SDL_FreeSurface(imgNewGame);

	Mix_FreeChunk(sfxClickX);
	Mix_FreeChunk(sfxClickO);
	Mix_FreeChunk(sfxGameEnded);

}

int main(int argc, char *argv[]){

	bool quit = false;
	bool newGame = true;

	if (!InitWindow()){
		printf("Erro total!\n");
		return 0;
	}

	while (!quit){
		GetEvents(player, quit, newGame);
		SetGridRect();
		NewRound();
		GameLoop(quit, newGame);
	}

	return 0;
}
