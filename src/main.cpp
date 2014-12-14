#ifdef linux
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_ttf.h>
	#include <SDL2/SDL_image.h>
	#include <SDL2/SDL_mixer.h>
	#include <string>	
	#include <map>	
	#include <cstdio>
#elif _WIN32
	#include <SDL.h>
	#include <SDL_ttf.h>
	#include <SDL_image.h>
	#include <SDL_mixer.h>
	#include <string>
	#include <map>
	#include <cstdio>
#endif

//Definicoes para utilizacao do std::map
#define imgX "imgX"
#define imgO "imgO"
#define imgNewGame "imgNewGame"
#define imgGrid "imgGrid"
#define imgXWon "imgXWon"
#define imgOWon "imgOWon"
#define imgDraw "imgDraw"

#define sfxClickX "sfxClickX"
#define sfxClickO "sfxClickO"
#define sfxInvalidClick "sfxInvalidClick"
#define sfxGameEnded "sfxGameEnded"

//Usado na funcao Load()
enum file_type{ PNG, BMP, WAV };

using namespace std;

//Variavel PROPORTION controla o tamanho da tela e dos elementos
const float PROPORTION = 1;

const int SCREEN_WIDTH = 311 * PROPORTION;
const int SCREEN_HEIGHT = 308 * PROPORTION;

const int CELL_WIDTH = 94 * PROPORTION;
const int CELL_HEIGHT = 94 * PROPORTION;
const int BAR_SIZE = 4 * PROPORTION;
const int COMP = 4 * PROPORTION;

#define GRID_SIZE 3
#define CIRCLE 'O'
#define CROSS 'X'
#define NULLCHAR '\0'

int ocupados;
int pressedPosition;
int posX, posY; //Posicoes da matriz que representa o grid
char grid[GRID_SIZE][GRID_SIZE];
char player;
char winner;

//Mapa de texturas
map<string, SDL_Texture*> textureMap;

//Mapa de sons
map<string, Mix_Chunk*> soundMap;

//Janela
SDL_Window *window = NULL;

//Renderer
SDL_Renderer *renderer = NULL;

//Posicoes do grid
SDL_Rect gridRect[9];

//Eventos
SDL_Event event;

bool Load(string filename, string id, file_type type){

	if (type == PNG){

		SDL_Surface *tempSurface = IMG_Load(filename.c_str());
		SDL_Texture *texture = NULL;

		if (tempSurface == NULL){
			printf("Unable to load \"%s\"! SDL Error: %s\n", filename.c_str(), SDL_GetError());
			return false;
		}
		else if ((texture = SDL_CreateTextureFromSurface(renderer, tempSurface)) == NULL){
			printf("Unable to load texture! SDL Error: %s\n", SDL_GetError());
			return false;
		}

		SDL_FreeSurface(tempSurface);

		textureMap[id] = texture;

		return true;
	}
	else if (type == WAV){
		
		Mix_Chunk *sound = Mix_LoadWAV(filename.c_str());

		if (sound == NULL){
			printf("Unable to load \"%s\"! Mix Error: %s\n", filename.c_str(), Mix_GetError());
			return false;
		}

		soundMap[id] = sound;

		return true;
	}

}

bool LoadFiles(){

	if (!Load("img/grid.png", "imgGrid", PNG))
		return false;
	if (!Load("img/x.png", "imgX", PNG))
		return false;
	if (!Load("img/o.png", "imgO", PNG))
		return false;
	if (!Load("img/new_game.png", "imgNewGame", PNG))
		return false;
	if (!Load("img/draw.png", "imgDraw", PNG))
		return false;
	if (!Load("img/x_won.png", "imgXWon", PNG))
		return false;
	if (!Load("img/o_won.png", "imgOWon", PNG))
		return false;
	if (!Load("sfx/x_sound.wav", "sfxClickX", WAV))
		return false;
	if (!Load("sfx/o_sound.wav", "sfxClickO", WAV))
		return false;
	if (!Load("sfx/game_ended.wav", "sfxGameEnded", WAV))
		return false;
	if (!Load("sfx/invalid_click_sound.wav", "sfxInvalidClick", WAV))
		return false;

	return true;
}

bool InitWindow(){

	if (SDL_Init(SDL_INIT_VIDEO < 0 | SDL_INIT_AUDIO) < 0){
		printf("Unable to load SDL! SDL Error: %s\n", SDL_GetError());
		return 0;
	}
	else{

		int imgFlags = IMG_INIT_PNG;

		window = SDL_CreateWindow("TIC-TAC-TOE", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

		if (window == NULL){
			printf("Unable to init window! SDL Error: %s\n", SDL_GetError());
			return 0;
		}
		else{
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
			if (renderer == NULL){
				printf("Unable to init renderer! SDL Error: %s\n", SDL_GetError());
				return 0;
			}
		}

		if (!(IMG_Init(imgFlags) & imgFlags)){
			printf("Unable to load SDL_image! SDL Error:%s\n", IMG_GetError());
			return 0;
		}
		if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0){
			printf("Unable to load SDL_Mixer! Mix Error: %s\n", Mix_GetError());
			return 0;
		}
	}

	return 1;
}

void SetGridRect(){

	gridRect[0].x = gridRect[0].y = COMP;
	gridRect[0].w = CELL_WIDTH;
	gridRect[0].h = CELL_HEIGHT;

	gridRect[1].x = CELL_WIDTH + COMP * 3 + BAR_SIZE;
	gridRect[1].y = COMP;
	gridRect[1].w = CELL_WIDTH;
	gridRect[1].h = CELL_HEIGHT;

	gridRect[2].x = gridRect[1].x * 2 - COMP;
	gridRect[2].y = COMP;
	gridRect[2].w = CELL_WIDTH;
	gridRect[2].h = CELL_HEIGHT;

	gridRect[3].x = COMP;
	gridRect[3].y = CELL_WIDTH + BAR_SIZE + COMP * 3;
	gridRect[3].w = CELL_WIDTH;
	gridRect[3].h = CELL_HEIGHT;

	gridRect[4].x = gridRect[1].x;
	gridRect[4].y = CELL_WIDTH + BAR_SIZE + COMP * 3;
	gridRect[4].w = CELL_WIDTH;
	gridRect[4].h = CELL_HEIGHT;

	gridRect[5].x = gridRect[2].x;
	gridRect[5].y = CELL_WIDTH + BAR_SIZE + COMP * 3;
	gridRect[5].w = CELL_WIDTH;
	gridRect[5].h = CELL_HEIGHT;

	gridRect[6].x = COMP;
	gridRect[6].y = CELL_WIDTH * 2 + BAR_SIZE * 2 + COMP * 5;
	gridRect[6].w = CELL_WIDTH;
	gridRect[6].h = CELL_HEIGHT;

	gridRect[7].x = gridRect[1].x;
	gridRect[7].y = CELL_WIDTH * 2 + BAR_SIZE * 2 + COMP * 5;
	gridRect[7].w = CELL_WIDTH;
	gridRect[7].h = CELL_HEIGHT;

	gridRect[8].x = gridRect[2].x;
	gridRect[8].y = CELL_WIDTH * 2 + BAR_SIZE * 2 + COMP * 5;
	gridRect[8].w = CELL_WIDTH;
	gridRect[8].h = CELL_HEIGHT;
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
		printf("Posicao invalida.\n");
		return 0;
	}

	return 1;

}

void GetEvents(bool &quit, bool &newGame){

	while (SDL_PollEvent(&event) != 0){

		if (event.type == SDL_QUIT){
			quit = true;
			return;
		}
		if (event.type == SDL_MOUSEBUTTONDOWN){

			int x, y;

			SDL_GetMouseState(&x, &y);

			if (newGame){
				if (((x >= 75 * PROPORTION) && (x <= 220 * PROPORTION)) && ((y >= 140 * PROPORTION) && (y <= 160 * PROPORTION))){
					newGame = false;
					return;
				}
				else if (((x >= 115 * PROPORTION) && (x <= 180 * PROPORTION)) && ((y >= 170 * PROPORTION) && (y <= 190 * PROPORTION))){
					quit = true;
					return;
				}

			}
			else{
				if (GetPosition(x, y)){
					if (grid[posX][posY] == NULLCHAR){
						if (player == CIRCLE){
							Mix_PlayChannel(-1, soundMap[sfxClickO], 0);
						}
						else{
								Mix_PlayChannel(-1, soundMap[sfxClickX], 0);
						}
					}
					else{
						Mix_PlayChannel(-1, soundMap[sfxInvalidClick], 0);
					}
				}
				else{
					Mix_PlayChannel(-1, soundMap[sfxInvalidClick], 0);
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
		printf("Diagonal secundaria!\n");
		winner = player;
		return;
	}

}

void NewRound(){

	for (int i = 0; i < GRID_SIZE; i++){
		for (int j = 0; j < GRID_SIZE; j++){
			grid[i][j] = NULLCHAR;
		}
	}

	winner = NULLCHAR;
	player = CIRCLE;
	ocupados = 0;

}

void NewGameScreen(){
	//TODO
	//O "new game" deve funcionar como um botão, e não fazer parte do background

	SDL_RenderCopy(renderer, textureMap[imgNewGame], NULL, NULL);

}

void GameScreen(){

	int pos = 0;

	for (int i = 0; i < GRID_SIZE; i++){
		for (int j = 0; j < GRID_SIZE; j++){
			if (grid[i][j] != NULLCHAR){
				SDL_RenderCopy(renderer, grid[i][j] == CROSS ? textureMap[imgX] : textureMap[imgO], NULL, &gridRect[pos]);
			}
			pos++;
		}
	}

	SDL_RenderPresent(renderer);
	
}

void EndGameScreen(){

	if (winner == NULLCHAR){
		SDL_RenderCopy(renderer, textureMap[imgDraw], NULL, NULL);
	}
	else{
		SDL_RenderCopy(renderer, winner == CIRCLE ? textureMap[imgOWon] : textureMap[imgXWon], NULL, NULL);
	}

	SDL_RenderPresent(renderer);
	SDL_Delay(2000);
}

void GameLoop(bool &quit, bool &newGame){

	while (!quit){
		
		pressedPosition = 0;

		GetEvents(quit, newGame);

		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, textureMap[imgGrid], NULL, NULL);

		if (quit){
			break;
		}
		if (newGame){
			NewGameScreen();
		}
		else{
			if (CheckPosition()){
				ocupados++;
				if (ocupados > 4){
					//TODO
					CheckIfWon(player);

					if ((winner != NULLCHAR) || (ocupados == 9)){
						if (winner != NULLCHAR){
							printf("Player %c ganhou\n", winner);
							Mix_PlayChannel(-1, soundMap[sfxGameEnded], 0);
						}
						else{
							printf("Empate!\n");
						}
						GameScreen();
						SDL_Delay(500);
						EndGameScreen();
						newGame = true;
						break;
					}
				}
			}

			GameScreen();
		}

		SDL_RenderPresent(renderer);
	}
}

void Close(){

	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);

	SDL_DestroyTexture(textureMap[imgGrid]);
	SDL_DestroyTexture(textureMap[imgX]);
	SDL_DestroyTexture(textureMap[imgXWon]);
	SDL_DestroyTexture(textureMap[imgO]);
	SDL_DestroyTexture(textureMap[imgOWon]);
	SDL_DestroyTexture(textureMap[imgDraw]);
	SDL_DestroyTexture(textureMap[imgNewGame]);

	textureMap.clear();

	Mix_FreeChunk(soundMap[sfxClickX]);
	Mix_FreeChunk(soundMap[sfxClickO]);
	Mix_FreeChunk(soundMap[sfxInvalidClick]);
	Mix_FreeChunk(soundMap[sfxGameEnded]);

	soundMap.clear();

	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char *argv[]){

	bool quit = false;
	bool newGame = true;

	if (!InitWindow()){
		printf("Erro total!\n");
		return 0;
	}
	if (!LoadFiles()){
		printf("Erro total!\n");
		return 0;
	}

	while (!quit){
		SetGridRect();
		NewRound();
		GameLoop(quit, newGame);
	}

	return 0;
}
