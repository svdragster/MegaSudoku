#include <stdio.h>
#include <stdlib.h>
#include <Math.h>
#include <Windows.h>
#include <string.h>
#include <SDL.h>
#include <SDL_main.h>
#include <SDL_TTF.h>

#define TIMELIMIT 8000
#define LOAD_SPEED 0.00000025

#define MUS_PATH "\\CantinaBand.mp3"

#define LOAD_IDLE 0
#define LOAD_CREATING 1
#define LOAD_GENERATING 2
#define LOAD_SOLVING 3

#define TAB_ALL 0
#define BUTTON_CLOSE 0
#define BUTTON_SPEAKER 1
#define BUTTON_ASCII 2

#define TAB_SETTINGS 1
#define BUTTON_SIZE_PLUS 0
#define BUTTON_SIZE_MINUS 1
#define BUTTON_DIFFICULTY_PLUS 2
#define BUTTON_DIFFICULTY_MINUS 3

#define TAB_MAIN 2
#define BUTTON_GENERATE 0
#define BUTTON_SOLVE 1
#define BUTTON_CANCER 2
#define BUTTON_MOMO 3
#define BUTTON_CLEAR 4
#define BUTTON_SAVE 5
#define BUTTON_LOAD 6

#define TAB_NUMPAD 3

int validNumber(int row, int col, int n, int size);
int solve(int size, int timelimit);
int solveBack(int size, int timelimit);
void create(int size);
void create2(int size);
void create4(int size);
void printMatrix(int size);
int compareSolutions(int size, int timelimit);
void generate(int size, int difficulty);
//void saveSudoku();
//void loadSudoku();
int getSquareSize(int size);
char* intToString(int num);
char* intToStringAscii(int num);
void initializeRects(int size, SDL_Rect* sudoku);
void initializeSize(int size, SDL_Rect* sudoku);
void drawRects(int size, SDL_Rect* sudoku, int mouseX, int mouseY);
void printFont(char *c, int x, int y);
void difficulties();
void load();
void changeSize(int newsize);
void changeDifficulty(int newdifficulty);
int digitsOf(int num);
void addbutton(int x, int y, int w, int h, char* text, int tab, int func, SDL_Color color, int tx, int ty, int tw, int th);
void initializeNumpad();
int mouseOver(SDL_Rect rect, int mouseX, int mouseY);

//int checkSudoku(int size);

void generateMomodings();
void generateMomodings2();

typedef struct Feld {
	SDL_Rect rect;
	int number;
	int row;
	int col;
	SDL_Texture* texture;
	SDL_Surface* surface;
} Feld;

typedef struct Button {
	SDL_Rect rect;
	SDL_Rect txrect;
	SDL_Texture* txtexture;
	SDL_Surface* txsurface;
	int tab;
	int function;
} Button;

typedef struct Text {
	SDL_Rect rect;
	SDL_Texture* texture;
	int id;
} Text;

struct Text textRect(SDL_Rect rect, char* c);
Feld createFeld(int num, int row, int col, int x, int y, int w, int h, SDL_Color color);
void printFeld(Feld feld);
SDL_Color newColor(int r, int g, int b, int a);
Feld updateFeld(Feld feld, int num, SDL_Color color);

int emptySavedSudoku;
int emptySudoku;
int momodings = 0;
int rendhelp = 0;
int **numbers, **generatedNumbers, **momodinger, **savedSudoku;
int calculating = 0;
Feld* felder;
SDL_Rect* fields;
int* animatedFields;
int winx;
int winy;
TTF_Font* font;
SDL_Surface* fontSurface;
SDL_Color fontColor;
SDL_Rect fontRect;
Button* buttons;
int btnAmount = 0;
SDL_Texture* fontTexture;
SDL_Renderer* renderer;
int size = 16;
int difficulty = 1;
int toRemove = 0;
int updateNumbers = 0;
int update = 1000;
int cancel = 0;
int selectedSlot = -1;

int textAmount = 0;
Text* texts;

int loadStage = 0;
double loadProgress = 0.0;
SDL_Rect loadbar;
SDL_Rect loadPointsRect;
SDL_Surface * loadPointsSF;
SDL_Texture * loadPointTX;
int muted = 0;

int displayNumpad = 0;

int randomValue = 0;
int ascii = 0;

SDL_Color standardColor[9];
SDL_Color mouseOverColor[9];
SDL_Color selectedColor[9];


//new thread for generating
DWORD WINAPI ThreadFunc(void* data) {
	selectedSlot = -1;
	srand(time(NULL));
	Sleep(randomValue);
	generate(size, difficulty);
	printMatrix(size);
	calculating = 0;
	loadStage = LOAD_IDLE;
	loadProgress = 0;
	updateNumbers = 1;
	return 0;
}

//new thread for solving
DWORD WINAPI ThreadFuncSolve(void* data) {
	selectedSlot = -1;
	solve(size, 0);
	printMatrix(size);
	calculating = 0;
	loadStage = LOAD_IDLE;
	loadProgress = 0;
	updateNumbers = 1;
	return 0;
}

//HANDLE thread = CreateThread(NULL, 0, ThreadFunc, NULL, 0, NULL);

//main
int main(int argc, char* argv[]) {

	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE) == -1) {
		fprintf(stderr, "Fehler: %s\n", SDL_GetError());
		return -1;
	}

	if (TTF_Init() < 0) {
		fprintf(stderr, "Fehler Schrift: %s\n", SDL_GetError());
		return -1;
	}

	/*printf("1 %s\n", _getcwd());
	int strsize1 = sizeof(_getcwd()) + 1;
	char* string1 = (char*)malloc(sizeof(_getcwd()));
	string1 = _getcwd();
	char* titel = malloc(strlen(&string1) + strlen(" abcd") + 1);
	strcpy_s(&titel, strlen(&titel) + 1, string1);
	printf("bitch ass cock faggot niggo %s\n", titel);
	strcat_s(&titel, strlen(&titel) + strlen(" abcd") + 1, " abcd");*/

	// load support for the JPG and PNG image formats
	/*int flags = IMG_INIT_JPG | IMG_INIT_PNG;
	int initted = IMG_Init(flags);
	if (initted&flags != flags) {
	printf("IMG_Init: Failed to init required jpg and png support!\n");
	printf("IMG_Init: %s\n", IMG_GetError());
	// handle error bitch
	return -1;
	}*/

	difficulties();

	int quit = 0, side = sqrt(size), i;

	fields = (SDL_Rect*)malloc(25 * 25 * sizeof(SDL_Rect));
	buttons = (Button*)malloc(btnAmount * sizeof(Button));
	felder = (Feld*)malloc(25 * 25 * sizeof(Feld));
	texts = (Text*)malloc(textAmount * sizeof(Text));
	savedSudoku = (int**)malloc(25 * sizeof(int*));
	if (savedSudoku == NULL) {
		printf("saved Numbers null\n");
		return -1;
	}
	int j;

	for (i = 0; i < 25; i++) {
		savedSudoku[i] = (int*)malloc(25 * sizeof(int));
		if (savedSudoku[i] == NULL) {
			printf("saved sudoku[%d] null\n", i);
			return -1;
		}
		for (j = 0; j < 25; j++) {
			savedSudoku[i][j] = 0;
		}
	}

	generatedNumbers = (int**)malloc(25 * sizeof(int*));
	if (generatedNumbers == NULL) {
		printf("Generated Numbers null\n");
		return -1;
	}

	for (i = 0; i < 25; i++) {
		generatedNumbers[i] = (int*)malloc(25 * sizeof(int));
		if (generatedNumbers[i] == NULL) {
			printf("Generated Numbers[%d] null\n", i);
			return -1;
		}
		for (j = 0; j < 25; j++) {
			generatedNumbers[i][j] = 0;
		}
	}

	momodinger = (int**)malloc(9 * sizeof(int*));
	if (momodinger == NULL) {
		printf("momodinger null\n");
		return -1;
	}
	for (i = 0; i < 9; i++) {
		momodinger[i] = (int*)malloc(9 * sizeof(int));
		if (momodinger[i] == NULL) {
			printf("Momodinger[%d] null\n", i);
			return -1;
		}
		for (j = 0; j < 9; j++) {
			momodinger[i][j] = 0;
		}
	}

	if (buttons == NULL) {
		printf("Buttons null\n");
		return -1;
	}
	if (fields == NULL) {
		printf("fields null\n");
		return -1;
	}
	if (felder == NULL) {
		printf("Felder null\n");
		return -1;
	}
	if (texts == NULL) {
		printf("Texts null\n");
		return -1;
	}

	SDL_DisplayMode DM;
	SDL_GetCurrentDisplayMode(0, &DM);
	winx = DM.w;
	winy = DM.h;

	SDL_Window* window = SDL_CreateWindow("MegaSudoku", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, winx, winy, SDL_WINDOW_FULLSCREEN_DESKTOP);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	//neu
	//music = (Mix_Music *)malloc(4096 * sizeof(Mix_Music*));
	//music = MUS_PATH;
	//neu ende

	int x;
	int y;
	int count = 0;

	srand(time(NULL));

	numbers = (int **)malloc(25 * sizeof(int*));

	if (numbers == NULL) {
		printf("Fehler, Speicher ist voll!");
		getchar();
		return;
	}

	for (x = 0; x < 25; x++) {
		numbers[x] = malloc(25 * sizeof(int));
		if (numbers[x] == NULL) {
			printf("Fehler 2, Speicher ist voll!");
			getchar();
			return;
		}
		for (y = 0; y < 25; y++) {
			numbers[x][y] = 0;
		}
	}

	font = TTF_OpenFont("arial.ttf", 128);
	if (font == NULL) {
		printf("Font is null\n");
		return -1;
	}
	fontColor.r = 0xff;
	fontColor.g = 0xff;
	fontColor.b = 0xff;

	int squareSize = getSquareSize(size);
	for (i = 0; i < size*size; i++) {
		Feld feld = createFeld(1, i / size, i % size, 0, 0, squareSize * 0.4, squareSize*0.8, newColor(255, 255, 255, SDL_ALPHA_OPAQUE));
		felder[i] = feld;
	}

	initializeRects(size, fields);
	initializeSize(size, fields, 0, 0);

	for (i = 0; i < 25 * 25; i++) {
		SDL_Rect rect = fields[i];
		rect.h = 0;
		rect.w = 0;
		fields[i] = rect;
	}
	animatedFields = (int*)malloc(25 * 25 * sizeof(int));
	double animationH = 0.0;
	int animationRow = 0, row = 1, col = 1;
	Uint32 start = SDL_GetTicks();
	Uint32 diff;
	Uint32 oldTime = 0;
	Uint32 loadTime = 0;

	double loadPointsX = 2;
	int direction = 0;

	loadPointsSF = TTF_RenderText_Solid(font, "loading", newColor(20, 30, 255, 255));
	loadPointsRect.h = 25;
	loadPointsRect.w = 105;
	loadPointsRect.x = 0;
	loadPointsRect.y = winy - 25;
	loadPointTX = SDL_CreateTextureFromSurface(renderer, loadPointsSF);
	load();

	SDL_Rect panel_size = { ((getSquareSize(size)) * 25) + 10, 20, 0, 0 };
	SDL_Rect panel_Difficulty = { panel_size.x, 90, 0, 0 };
	char* upperPanel = "size:";
	char* lowerPanel = "difficulty:";

	//SDL_Rect value_size = { (winy - (0.3 * panel_size.w)), 0, 0, 0 };
	//SDL_Rect value_difficulty = { value_size.x, panel_Difficulty.y, 12 * digitsOf(difficulty), 0 };

	char* string;

	SDL_Surface* surfaceSizePanel = TTF_RenderText_Solid(font, upperPanel, newColor(255, 255, 255, 255));
	SDL_Surface* surfaceDifficultyPanel = TTF_RenderText_Solid(font, lowerPanel, newColor(255, 255, 255, 255));
	string = intToString(size);
	SDL_Surface* surfaceSizeValue = TTF_RenderText_Solid(font, string, newColor(255, 255, 255, 255));
	free(string);
	string = intToString(difficulty);
	SDL_Surface* surfaceDifficultyValue = TTF_RenderText_Solid(font, string, newColor(255, 255, 255, 255));
	free(string);

	SDL_Texture* TextureSizePanel = SDL_CreateTextureFromSurface(renderer, surfaceSizePanel);
	SDL_Texture* TextureDifficultyPanel = SDL_CreateTextureFromSurface(renderer, surfaceDifficultyPanel);
	SDL_Texture* TextureSizeValue = SDL_CreateTextureFromSurface(renderer, surfaceSizeValue);
	SDL_Texture* TextureDifficultyValue = SDL_CreateTextureFromSurface(renderer, surfaceDifficultyValue);

	Text text;
	SDL_Rect rect;

	rect.y = 0;
	rect.w = 100;
	rect.h = 20;
	rect.x = (winx / 2) - (rect.w / 2);
	textRect(rect, "Mega Sudoku");

	rect.x = winx - 600; rect.y = 50; rect.w = 65; rect.h = 30;
	text = textRect(rect, "Size:");

	Text sizeValue;
	Text difficultyValue;
	Button button;
	SDL_Color blue = newColor(0x42, 0x42, 0xff, 0xff);
	SDL_Color white = newColor(0xff, 0xff, 0xff, 0xff);
	SDL_Color purple = newColor(0x120, 0, 0xff, 0xff);
	addbutton((winx - 20), 0, 20, 20, "X", TAB_ALL, BUTTON_CLOSE, newColor(255, 0, 0, 255), (winx - 15), 3, 12, 15);

	rect.y = rect.y + 40; rect.w = 130; rect.h = 30;
	textRect(rect, "Difficulty:");

	addbutton(rect.x + rect.w + 105, text.rect.y, 35, 30, "+", TAB_SETTINGS, BUTTON_SIZE_PLUS, white, rect.x + rect.w + 115, text.rect.y, 15, 30);
	addbutton(rect.x + rect.w + 15, text.rect.y, 35, 30, "-", TAB_SETTINGS, BUTTON_SIZE_MINUS, white, rect.x + rect.w + 25, text.rect.y, 15, 30);

	addbutton(rect.x + rect.w + 105, rect.y, 35, 30, "+", TAB_SETTINGS, BUTTON_DIFFICULTY_PLUS, white, rect.x + rect.w + 115, rect.y, 15, 30);
	addbutton(rect.x + rect.w + 15, rect.y, 35, 30, "-", TAB_SETTINGS, BUTTON_DIFFICULTY_MINUS, white, rect.x + rect.w + 25, rect.y, 15, 30);

	//addbutton(rect.x + rect.w + 220, winy - 350, 190, 70, "save (F11)", TAB_MAIN, BUTTON_SAVE, white, rect.x + rect.w + 235, winy - 335, 170, 40);
	//addbutton(rect.x + rect.w + 220, winy - 205, 190, 70, "load (F12)", TAB_MAIN, BUTTON_LOAD, white, rect.x + rect.w + 235, winy - 190, 170, 40);

	if (digitsOf(size) == 1) {
		rect.x = rect.x + rect.w + 45;
		rect.y = text.rect.y;
		rect.w = 15;
	}
	else {
		rect.x = rect.x + rect.w + 60;
		rect.y = text.rect.y;
		rect.w = 30;
	}
	string = intToString(size);
	sizeValue = textRect(rect, string);
	free(string);

	sizeValue.id = textAmount - 1;
	rect.x = rect.x + 8;
	rect.y = rect.y + 40;
	rect.w = 15;
	string = intToString(difficulty);
	difficultyValue = textRect(rect, string);
	free(string);

	difficultyValue.id = textAmount - 1;
	difficultyValue.rect = rect;

	addbutton(panel_Difficulty.x, winy - 400, 245, 120, "Generate", TAB_MAIN, BUTTON_GENERATE, white, panel_Difficulty.x + 32, winy - 355, 180, 45);
	addbutton(panel_Difficulty.x, winy - 275, 120, 120, "Solve", TAB_MAIN, BUTTON_SOLVE, white, panel_Difficulty.x + 5, winy - 230, 110, 45);
	addbutton(panel_Difficulty.x + 125, winy - 275, 120, 120, "Cancel", TAB_MAIN, BUTTON_CANCER, white, panel_Difficulty.x + 130, winy - 230, 110, 45);
	addbutton(panel_Difficulty.x, winy - 150, 245, 75, "Clear", TAB_MAIN, BUTTON_CLEAR, white, panel_Difficulty.x + 70, winy - 135, 100, 45);

	SDL_Rect speakerRect;
	speakerRect.x = winx - 150;
	speakerRect.y = 5;
	speakerRect.w = 120;
	speakerRect.h = 120;
	addbutton(speakerRect.x, speakerRect.y, speakerRect.w, speakerRect.h, "Nonomino", TAB_MAIN, BUTTON_MOMO, white, winx - 135, 53, 90, 22);

	SDL_Rect asciiRect;
	asciiRect.w = 120;
	asciiRect.h = 120;
	asciiRect.x = speakerRect.x - asciiRect.w - 10;
	asciiRect.y = 5;
	addbutton(asciiRect.x, asciiRect.y, asciiRect.w, asciiRect.h, "ASCII", TAB_ALL, BUTTON_ASCII, white, speakerRect.x + 10 - asciiRect.w, 45, 80, 40);
	/*SDL_Surface* speakerOnS = IMG_Load("speaker.png");
	SDL_Surface* speakerOffS = IMG_Load("speaker-off.png");
	SDL_Texture* speakerOn = SDL_CreateTextureFromSurface(renderer, speakerOnS);
	SDL_Texture* speakerOff = SDL_CreateTextureFromSurface(renderer, speakerOffS);*/

	initializeNumpad();

	generateMomodings2();

	int function = -1;
	int tab = 0;
	int disabledButton = 0;

	int mouseX = 0, mouseY = 0;
	diff = 0;

	//gameloop
	while (!quit) {
		oldTime = start;
		start = SDL_GetTicks();
		/*emptySavedSudoku = 1;

		for (row = 0; row < size; row++) {
			for (col = 0; col < size; col++) {
				if (numbers[row][col] != 0) {
					emptySudoku = 0;
				}
			}
		}

		for (row = 0; row < size; row++) {
			for (col = 0; col < size; col++) {
				if (savedSudoku[row][col] != 0) {
					emptySavedSudoku = 0;
				}
			}
		}*/

		SDL_GetMouseState(&mouseX, &mouseY);

		// input
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT)
				quit = 1;
			if (e.type == SDL_MOUSEBUTTONDOWN) {
				for (i = 0; i < btnAmount; i++) {
					button = buttons[i];
					if (mouseX >= button.rect.x && mouseX <= button.rect.x + button.rect.w
						&& mouseY >= button.rect.y && mouseY <= button.rect.y + button.rect.h) {
						tab = button.tab;
						if (button.tab == TAB_ALL) {
							if (button.function == BUTTON_CLOSE) {
								quit = 1;
							}
							else if (button.function == BUTTON_SPEAKER) {
								muted = muted ? 0 : 1;
							}
							else if (button.function == BUTTON_ASCII) {
								ascii = ascii ? 0 : 1;
								updateNumbers = 1;
								for (int sack = 0; sack < size * size; sack++) {
									felder[sack] = updateFeld(felder[sack], numbers[sack / size][sack % size], newColor(255, 255, 255, 255));
								}
								initializeNumpad();
							}
						}
						else {
							function = button.function;
						}
						break;
					}
				}
				if (calculating == 0) {
					for (i = 0; i < size*size; i++) {
						if (mouseOver(fields[i], mouseX, mouseY)) {
							if (generatedNumbers[i / size][i % size] == 0) {
								selectedSlot = i;
							}
							break;
						}
					}
				}
			}
			if (e.type == SDL_KEYDOWN) {
				if (e.key.keysym.sym == SDLK_ESCAPE) {
					quit = 1;
				}
				else if (e.key.keysym.sym == SDLK_SPACE) {
					tab = TAB_MAIN;
					function = BUTTON_GENERATE;
				}
				else if (e.key.keysym.sym == SDLK_RETURN) {
					tab = TAB_MAIN;
					function = BUTTON_SOLVE;
				}
				else if (e.key.keysym.sym == SDLK_F11) {
					tab = TAB_MAIN;
					function = BUTTON_SAVE;
				}
				else if (e.key.keysym.sym == SDLK_F12) {
					tab = TAB_MAIN;
					function = BUTTON_LOAD;
				}
				else if (e.key.keysym.sym == SDLK_UP) {
					tab = TAB_SETTINGS;
					function = BUTTON_SIZE_PLUS;
				}
				else if (e.key.keysym.sym == SDLK_DOWN) {
					tab = TAB_SETTINGS;
					function = BUTTON_SIZE_MINUS;
				}
				else if (e.key.keysym.sym == SDLK_LEFT) {
					tab = TAB_SETTINGS;
					function = BUTTON_DIFFICULTY_MINUS;
				}
				else if (e.key.keysym.sym == SDLK_RIGHT) {
					tab = TAB_SETTINGS;
					function = BUTTON_DIFFICULTY_PLUS;
				}
			}
			if (function != -1) {
				if (tab == TAB_SETTINGS) {
					if (function == BUTTON_SIZE_PLUS) {
						if (calculating == 0 && size < 25 && !momodings) {
							changeSize(pow((sqrt(size) + 1), 2));
							side = sqrt(size);
							squareSize = getSquareSize(size);
							animationH = 0;
							row = 0;
							col = 0;
							if (size != 9) {
								rect.w = 30;
								rect.x = difficultyValue.rect.x - 8;
							}
							string = intToString(size);
							fontSurface = TTF_RenderText_Solid(font, string, newColor(255, 255, 255, 255));
							free(string);
							sizeValue.texture = SDL_CreateTextureFromSurface(renderer, fontSurface);
							sizeValue.rect.w = rect.w;
							sizeValue.rect.x = rect.x;
							texts[sizeValue.id] = sizeValue;
						}
					}
					else if (function == BUTTON_SIZE_MINUS) {
						if (calculating == 0 && size > 9) {
							changeSize(pow((sqrt(size) - 1), 2));
							side = sqrt(size);
							squareSize = getSquareSize(size);
							animationH = 0;
							row = 0;
							col = 0;
							if (size == 9) {
								rect.w = 15;
								rect.x = difficultyValue.rect.x;
							}
							else {
								rect.w = 30;
							}
							string = intToString(size);
							fontSurface = TTF_RenderText_Solid(font, string, newColor(255, 255, 255, 255));
							free(string);
							sizeValue.texture = SDL_CreateTextureFromSurface(renderer, fontSurface);
							sizeValue.rect.w = rect.w;
							sizeValue.rect.x = rect.x;
							texts[sizeValue.id] = sizeValue;
						}
					}
					else if (function == BUTTON_DIFFICULTY_MINUS) {
						if (calculating == 0 && difficulty > 1) {
							changeDifficulty(difficulty - 1);
							string = intToString(difficulty);
							fontSurface = TTF_RenderText_Solid(font, string, newColor(255, 255, 255, 255));
							free(string);
							difficultyValue.texture = SDL_CreateTextureFromSurface(renderer, fontSurface);
							texts[difficultyValue.id] = difficultyValue;
						}
					}
					else if (function == BUTTON_DIFFICULTY_PLUS) {
						if (calculating == 0 && difficulty < 3) {
							changeDifficulty(difficulty + 1);
							string = intToString(difficulty);
							fontSurface = TTF_RenderText_Solid(font, string, newColor(255, 255, 255, 255));
							free(string);
							difficultyValue.texture = SDL_CreateTextureFromSurface(renderer, fontSurface);
							texts[difficultyValue.id] = difficultyValue;
						}
					}
				}
				else if (tab == TAB_MAIN) {
					if (function == BUTTON_GENERATE) {
						if (calculating == 0) {
							for (row = 0; row < size; row++) {
								for (col = 0; col < size; col++) {
									numbers[row][col] = 0;
									generatedNumbers[row][col] = 0;
								}
							}
							int r, c;
							for (i = 0; i < size*size; i++) {
								c = i % size;
								r = i / size;
								felder[i] = updateFeld(felder[i], 0, newColor(255, 255, 255, 255));
							}
							loadPointsSF = TTF_RenderText_Solid(font, "generating", newColor(20, 30, 255, 255));
							loadPointsRect.w = 120;
							loadPointTX = SDL_CreateTextureFromSurface(renderer, loadPointsSF);
							randomValue = rand() % 300;
							calculating = 1;
							cancel = 0;
							loadPointsX = 0;
							HANDLE thread = CreateThread(NULL, 0, ThreadFunc, NULL, 0, NULL);
							if (thread) {
								printf("Warten...");
							}
						}
					}
					else if (function == BUTTON_SOLVE) {
						if (calculating == 0 && cancel == 0) {
							loadPointsSF = TTF_RenderText_Solid(font, "solving", newColor(20, 30, 255, 255));
							loadPointsRect.h = 25;
							loadPointsRect.w = 105;
							loadPointsRect.x = 0;
							loadPointsRect.y = winy - 25;
							loadPointTX = SDL_CreateTextureFromSurface(renderer, loadPointsSF);
							calculating = 1;
							loadPointsX = 0;
							cancel = 0;
							HANDLE thread = CreateThread(NULL, 0, ThreadFuncSolve, NULL, 0, NULL);
							if (thread) {
								printf("Warten...");
							}
						}
						else {
							const SDL_MessageBoxColorScheme colorScheme = {
								{
									// Color Background
									{ 0x99,   0x99,   0xff },
									// Color Text
									{ 0x30, 0x30,   0xff },
									// Color btnBorder
									{ 0x80, 0x80,   0xff },
									// Color btnBackground
									{ 0x80,   0x80, 0xff },
									// Color btnSelected
									{ 0x90,   0x90, 0xff }
								}
							};

							const SDL_MessageBoxButtonData buttons[] = {
								/* .flags, .buttonid, .text */
								{ 0, 0, "s(no)wman" },
								{ 0, 1, "yes, master" },
								{ 0, 2, "whatever" },
							};

							const SDL_MessageBoxData messageboxdata = {
								SDL_MESSAGEBOX_ERROR, /* .flags */
								window, /* .window */
								"BEINHARTER FEHLER!!!", /* .title */
								"Your Sudoku isn't even fully generated yet, noob.", /* .message */
								SDL_arraysize(buttons), /* .numbuttons */
								buttons, /* .buttons */
								&colorScheme /* .colorScheme */
							};

							SDL_SetWindowSize(&messageboxdata.window, 600, 600);
							int buttonid;
							if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) {
								SDL_Log("error displaying message box");
							}
							SDL_MaximizeWindow(window);
						}
					}
					else if (function == BUTTON_CANCER) {
						if (cancel == 0 && calculating == 1) {
							cancel = 1;
							calculating = 0;
							printf("CANCEL CANCEL CANCEL CANCEL CANCEL CANCEL CANCEL CANCEL CANCEL CANCEL\n");
						}
					}
					else if (function == BUTTON_CLEAR) {
						if (calculating == 0) {
							cancel = 0;
							for (row = 0; row < size; row++) {
								for (col = 0; col < size; col++) {
									numbers[row][col] = 0;
									generatedNumbers[row][col] = 0;
								}
							}
							int r, c;
							for (i = 0; i < size*size; i++) {
								c = i % size;
								r = i / size;
								felder[i] = updateFeld(felder[i], 0, newColor(255, 255, 255, 255));
							}

							updateNumbers = 1;
						}
					}

					else if (function == BUTTON_SAVE) {
						if (calculating == 0) {
							cancel = 0;
							for (row = 0; row < size; row++) {
								for (col = 0; col < size; col++) {
									savedSudoku[row][col] = numbers[row][col];
								}
							}
							int r, c;
							for (i = 0; i < size*size; i++) {
								c = i % size;
								r = i / size;
								felder[i] = updateFeld(felder[i], 0, newColor(255, 255, 255, 255));
							}

							updateNumbers = 1;
						}
					}

					else if (function == BUTTON_LOAD) {
						if (calculating == 0) {
							cancel = 0;
							for (row = 0; row < size; row++) {
								for (col = 0; col < size; col++) {
									numbers[row][col] = savedSudoku[row][col];
								}
							}
							int r, c;
							for (i = 0; i < size*size; i++) {
								c = i % size;
								r = i / size;
								felder[i] = updateFeld(felder[i], 0, newColor(255, 255, 255, 255));
							}

							updateNumbers = 1;
						}
					}
					else if (function == BUTTON_MOMO) {
						if (calculating == 0) {
							if (momodings) {
								changeSize(9);
								momodings = 0;
							}
							else {
								momodings = 1;
								changeSize(9);
								string = intToString(size);
								fontSurface = TTF_RenderText_Solid(font, string, newColor(255, 255, 255, 255));
								free(string);
								sizeValue.texture = SDL_CreateTextureFromSurface(renderer, fontSurface);
								sizeValue.rect.w = rect.w;
								sizeValue.rect.x = rect.x;
								texts[sizeValue.id] = sizeValue;
							}
							updateNumbers = 1;
						}
					}
				}
				else if (tab == TAB_NUMPAD) {
					if (selectedSlot != -1) {
						if (generatedNumbers[selectedSlot / size][selectedSlot % size] == 0) {
							if (function == -999) {
								function = 0;
							}
							numbers[selectedSlot / size][selectedSlot % size] = function;
							updateNumbers = 1;
						}
					}
				}
				function = -1;
				tab = -1;
			}
		}
		if (quit) {
			break;
		}
		if (updateNumbers) {
			updateNumbers = 0;
			if (!calculating || (diff < 750 && update < 3000)) {
				initializeSize(size, fields);
				int r, c;
				for (i = 0; i < size*size; i++) {
					c = i % size;
					r = i / size;
					if (felder[i].number != numbers[r][c]) {
						felder[i] = updateFeld(felder[i], numbers[r][c], newColor(255, 255, 255, 255));
					}
					/*if (update > 1500) {
					Sleep(1);
					}*/
				}
			}
		}
		//rendering
		SDL_SetRenderDrawColor(renderer, 0x14, 0x14, 0x14, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);

		diff = start - oldTime;

		if (diff > 300) {
			update += 500;
		}

		if (row <= size && col <= size) {
			animationH += 0.5 * diff;
			if ((int)animationH >= squareSize) {
				animationH = 0;
				col++;
				row++;
				if (col > size) {
					col = size;
				}
				if (row > size) {
					row = size;
				}
			}
			for (int r = 0; r < row; r++) {
				for (int c = 0; c < col; c++) {
					i = c + r * size;
					SDL_Rect rect = fields[i];
					if (animatedFields[i] < squareSize) {
						if (r == c) {
							rect.h = animationH;
							rect.w = animationH;
						}
						else if (r < c) {
							rect.h = squareSize;
							rect.w = animationH;
						}
						else if (c < r) {
							rect.h = animationH;
							rect.w = squareSize;
						}
						animatedFields[i] = (int) animationH + 5;
						fields[i] = rect;
					}
					else {
						rect.h = squareSize;
						rect.w = squareSize;
						fields[i] = rect;
					}
				}
			}
		}
		if (loadStage != LOAD_IDLE) {
			if (loadProgress > 1.0) {
				loadProgress = 1.0;
				if (cancel == 0 && calculating == 1) {
					//stopMUS(music);
				}
			}
			if (loadProgress == 1.0) {
				Uint32 diff = SDL_GetTicks() - loadTime;
				if (diff < 3000) {
					loadbar.w = (double)winx;
					SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
					SDL_RenderFillRect(renderer, &loadbar);
				}
				else {
					loadProgress = 0;
					loadStage = LOAD_IDLE;
					loadPointsX = 0;
				}
			}
			else {
				loadTime = SDL_GetTicks();
				loadbar.w = (int)(loadProgress * (double)winx);
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
				SDL_RenderFillRect(renderer, &loadbar);
				if (loadbar.w > 1.5 * loadPointsRect.w) {
					if (loadPointsRect.x < 2) {
						direction = 1;
						loadPointsRect.x = 2;
					}
					else if ((loadPointsRect.x + loadPointsRect.w + 2) > loadbar.w) {
						direction = -1;
						loadPointsRect.x = 2;
					}
					loadPointsX += direction * (diff * 0.35);
					loadPointsRect.x = loadPointsX;
					SDL_RenderCopy(renderer, loadPointTX, NULL, &loadPointsRect);
				}
			}
		}
		drawRects(size, fields, mouseX, mouseY);
		for (i = 0; i < size*size; i++) {
			Feld feld = felder[i];
			printFeld(feld);
		}
		SDL_SetRenderDrawColor(renderer, 0x10, 0x10, 0xff, SDL_ALPHA_OPAQUE);
		for (i = 0; i < btnAmount; i++) {
			button = buttons[i];
			//printf("Faggot scheiß gay Knopf %d %d \n", button.tab, button.function);
			if (button.tab == TAB_ALL) {
				if (button.function == BUTTON_SPEAKER) {
					//SDL_SetRenderDrawColor(renderer, 0x14, 0x14, 0x14, SDL_ALPHA_TRANSPARENT);
					//disabledButton = 1;
					if (muted) {
						//SDL_RenderCopy(renderer, speakerOff, NULL, &speakerRect);
					}
					else {
						//SDL_RenderCopy(renderer, speakerOn, NULL, &speakerRect);
					}
					continue;
				}
			}
			else if (button.tab == TAB_MAIN) {
				if (calculating) {
					if (button.function == BUTTON_GENERATE || button.function == BUTTON_SOLVE || button.function == BUTTON_MOMO || button.function == BUTTON_CLEAR || button.function == BUTTON_SAVE || button.function == BUTTON_LOAD) {
						disabledButton = 1;
					}
				}
				else {
					if (button.function == BUTTON_CANCER) {
						disabledButton = 1;
					}
				}
				if (cancel == 1) {
					if (button.function == BUTTON_SOLVE) {
						disabledButton = 1;
					}
				}
				if (emptySudoku == 1) {
					if (button.function == BUTTON_SAVE) {
						disabledButton = 1;
					}
					if (emptySavedSudoku == 1) {
						if (button.function == BUTTON_LOAD) {
							disabledButton = 1;
						}
					}
				}
				else if (button.tab == TAB_SETTINGS) {
					if (calculating) {
						if (button.function == BUTTON_SIZE_MINUS || button.function == BUTTON_SIZE_PLUS
							|| button.function == BUTTON_DIFFICULTY_MINUS || button.function == BUTTON_DIFFICULTY_PLUS) {
							disabledButton = 1;
						}
					}
					else {
						if (momodings && button.function == BUTTON_SIZE_PLUS) {
							disabledButton = 1;
						}
						else if (size == 9 && button.function == BUTTON_SIZE_MINUS) {
							disabledButton = 1;
						}
						else if (size == 25 && button.function == BUTTON_SIZE_PLUS) {
							disabledButton = 1;
						}
						if (difficulty == 1 && button.function == BUTTON_DIFFICULTY_MINUS) {
							disabledButton = 1;
						}
						else if (difficulty == 3 && button.function == BUTTON_DIFFICULTY_PLUS) {
							disabledButton = 1;
						}
					}
				}
				else if (button.tab == TAB_NUMPAD) {
					if (selectedSlot == -1) {
						disabledButton = 1;
					}
					else if (calculating) {
						disabledButton = 1;
					}
					else {
						if (button.function > size) {
							disabledButton = 1;
						}
						if (button.function == -999) {
							disabledButton = 0;
						}
					}
				}
			}
			if (!disabledButton) {
				if (mouseX >= button.rect.x && mouseX <= button.rect.x + button.rect.w
					&& mouseY >= button.rect.y && mouseY <= button.rect.y + button.rect.h) {
					SDL_SetRenderDrawColor(renderer, 0x35, 0x35, 0xff, SDL_ALPHA_OPAQUE);
				}
			}
			else {
				SDL_SetRenderDrawColor(renderer, 0x55, 0x55, 0x55, SDL_ALPHA_OPAQUE);
			}
			disabledButton = 0;
			SDL_RenderFillRect(renderer, &button.rect);
			SDL_RenderCopy(renderer, button.txtexture, NULL, &button.txrect);
			SDL_SetRenderDrawColor(renderer, 0x10, 0x10, 0xff, SDL_ALPHA_OPAQUE);
		}
		for (i = 0; i < textAmount; i++) {
			text = texts[i];
			SDL_RenderCopy(renderer, text.texture, NULL, &text.rect);
		}
		SDL_RenderPresent(renderer);
	}
	//gameloop

	fields = NULL;
	free(fields);

	animatedFields = NULL;
	free(animatedFields);

	felder = NULL;
	free(felder);

	texts = NULL;
	free(texts);

	/*start = SDL_GetTicks();
	generate(size, DIFFICULTY);
	diff = SDL_GetTicks() - start;

	printMatrix(size);
	printf("Doof: %d\n", diff);
	getchar();

	start = SDL_GetTicks();
	solve(size, 0);

	diff = SDL_GetTicks() - start;

	printMatrix(size);
	printf("Solver A: %zu\n", diff);
	getchar();*/

	printf("DONE\n");

	for (x = 0; x < size; x++) {
		numbers[x] = NULL;
		free(numbers[x]);
	}

	numbers = NULL;
	free(numbers);

	//Mix_HaltMusic(music);
	//music = NULL;
	//Mix_FreeMusic(music);

	for (x = 0; x < size; x++) {
		for (x = 0; x < size; x++) {
			savedSudoku[x] = NULL;
			free(savedSudoku[x]);
		}

		savedSudoku = NULL;
		free(savedSudoku[x]);
	}

	savedSudoku = NULL;
	free(savedSudoku);

	for (x = 0; x < size; x++) {
		generatedNumbers[x] = NULL;
		free(generatedNumbers[x]);
	}

	generatedNumbers = NULL;
	free(generatedNumbers);

	for (x = 0; x < size; x++) {
		momodinger[x] = NULL;
		free(momodinger[x]);
	}

	momodinger = NULL;
	free(momodinger);

	// End of Mix library 

	/*font = NULL;
	fontSurface = NULL;
	fontTexture = NULL;
	renderer = NULL;
	window = NULL;
	free(font);
	free(fontSurface);
	free(fontTexture);
	free(renderer);
	free(window);*/

	TTF_Quit();
	//IMG_Quit();
	SDL_Quit();
	return 1;
}
//remove string from other string
//void removeSubstring(char *s, const char *toremove) {
// while (s = strstr(s, toremove)) {
// memmove(s, s + strlen(toremove), 1 + strlen(s + strlen(toremove)));
// }
//}

//check if a number is in the range of numbers depending on size
int validNumber(int row, int col, int n, int size) {
	if (n > size) {
		return 0;
	}
	if (n < 1) {
		return 0;
	}
	int r, c;
	for (r = 0; r < size; r++) {
		if (r != row) {
			if (numbers[r][col] == n) {
				return 0;
			}
		}
		for (c = 0; c < size; c++) {
			if (c != col) {
				if (numbers[row][c] == n) {
					return 0;
				}
			}
		}
		int side = sqrt(size);
		int fieldstartRow = (row / side) * side;
		int fieldstartCol = (col / side) * side;
		int r, c;
		if (!momodings) {
			for (r = fieldstartRow; r < fieldstartRow + side; r++) {
				for (c = fieldstartCol; c < fieldstartCol + side; c++) {
					if (r != row && c != col) {
						if (numbers[r][c] == n) {
							return 0;
						}
					}
				}
			}
		}
		else {
			int currentField = momodinger[row][col];
			for (r = 0; r < size; r++) {
				for (c = 0; c < size; c++) {
					if (momodinger[r][c] == currentField) {
						if (numbers[r][c] == n) {
							return 0;
						}
					}
				}
			}
		}
	}
	return 1;
}

//Test by fore- and backwardssolving if a sudoku is solvable clearly
int compareSolutions(int size, int timelimit) {
	if (cancel) {
		return 0;
	}
	int row = 0, col = 0, toReturn = 1;
	int **firstSolution = (int **)malloc(size * sizeof(int *));
	if (firstSolution == NULL) {
		printf("Nicht genug Speicher (A)\n");
		return 0;
	}
	for (row = 0; row < size; row++) {
		firstSolution[row] = malloc(size * sizeof(int));
		if (firstSolution[row] == NULL) {
			printf("Nicht genug Speicher (B)\n");
			return 0;
		}
	}
	int **startNumbers = (int **)malloc(size * sizeof(int *));
	if (startNumbers == NULL) {
		printf("Nicht genug Speicher (c)\n");
		return 0;
	}
	for (row = 0; row < size; row++) {
		startNumbers[row] = malloc(size * sizeof(int));
		if (startNumbers[row] == NULL) {
			printf("Nicht genug Speicher (D)\n");
			return 0;
		}
	}
	for (row = 0; row < size; row++) {
		for (col = 0; col < size; col++) {
			startNumbers[row][col] = numbers[row][col];
		}
	}
	if (solve(size, timelimit)) {
		for (row = 0; row < size; row++) {
			for (col = 0; col < size; col++) {
				firstSolution[row][col] = numbers[row][col];
			}
		}
	}
	for (row = 0; row < size; row++) {
		for (col = 0; col < size; col++) {
			numbers[row][col] = startNumbers[row][col];
		}
	}
	if (solveBack(size, timelimit)) {
		for (row = 0; row < size && toReturn == 1; row++) {
			for (col = 0; col < size && toReturn == 1; col++) {
				if (firstSolution[row][col] != numbers[row][col]) {
					toReturn = 0;
				}
			}
		}
	}
	for (row = 0; row < size; row++) {
		for (col = 0; col < size; col++) {
			numbers[row][col] = startNumbers[row][col];
		}
	}
	for (row = 0; row < size; row++) {
		firstSolution[row] = NULL;
		free(firstSolution[row]);
	}

	firstSolution = NULL;
	free(firstSolution);

	for (row = 0; row < size; row++) {
		startNumbers[row] = NULL;
		free(startNumbers[row]);
	}

	startNumbers = NULL;
	free(startNumbers);
	return toReturn;
}

//solve by testing 1-9 from upper left by row
int solve(int size, int timelimit) {
	if (loadStage == LOAD_IDLE) {
		loadStage = LOAD_SOLVING;
	}
	if (cancel) {
		return 0;
	}
	if (checkSudoku(size) == 0) {
		printf("Error in input detected. Cancelling...\n");
		return;
	}

	long counter = 0;
	int solved = 0, row, col, current, newNumber = 0, back = 0, valid = 0;
	int **fixedNumbers = (int **)malloc(size * sizeof(int*));
	if (fixedNumbers == NULL) {
		printf("Fehler XX\b");
		return;
	}
	for (row = 0; row < size; row++) {
		fixedNumbers[row] = malloc(size * sizeof(int));
		if (fixedNumbers[row] == NULL) {
			printf("Fehler YY\n");
			return;
		}
	}
	for (row = 0; row < size; row++) {
		for (col = 0; col < size; col++) {
			fixedNumbers[row][col] = numbers[row][col];
		}
	}
	Uint32 start = SDL_GetTicks(), diff, last = SDL_GetTicks();
	while (solved == 0) {
		row = 0;
		col = 0;
		while (row < size) {
			while (col < size) {
				if (cancel) {
					row = size;
					col = size;
					solved = 1;
					back = 0;
					break;
				}
				diff = SDL_GetTicks() - start;
				if ((SDL_GetTicks() - last) > update) {
					updateNumbers = 1;
					last = SDL_GetTicks();
				}
				if (timelimit <= 0 || diff < timelimit || row > size * 0.9) {
					current = numbers[row][col];
					if (current > size) {
						printf("Error A3B087HrW too many faggots sent to server\n");
						getchar();
						for (row = 0; row < size; row++) {
							fixedNumbers[row] = NULL;
							free(fixedNumbers[row]);
						}

						fixedNumbers = NULL;
						free(fixedNumbers);
						return 0;
					}

					if (fixedNumbers[row][col] == 0) {
						counter++;
						if (counter > 2000000) {
							counter = 0;
						}
						if (back) {
							newNumber = numbers[row][col] + 1;
						}
						else {
							newNumber++;
						}
						valid = 1;
						while (validNumber(row, col, newNumber, size) == 0 && valid == 1) {
							newNumber++;
							if (newNumber > size) {
								numbers[row][col] = 0;
								back = 1;
								valid = 0;
								newNumber = 0;
							}
						}
						if (valid) {
							if (loadProgress < 0.7) {
								loadProgress += LOAD_SPEED;
							}
							numbers[row][col] = newNumber;
							back = 0;
							newNumber = 0;
						}
					}
					if (back) {
						col--;
						if (col < 0) {
							if (row > 0) {
								col = size - 1;
								row--;
							}
							else {
								col = 0;
								// TODO: check if newNumber > size --> not solvable
							}
						}
					}
					else {
						col++;
						if (col > size) {
							col = 0;
							row++;
						}
					}
				}
				else {
					for (row = 0; row < size; row++) {
						fixedNumbers[row] = NULL;
						free(fixedNumbers[row]);
					}

					fixedNumbers = NULL;
					free(fixedNumbers);
					return 0;
				}
			}
			if (back) {
				row--;
			}
			else {
				row++;
			}
			col = 0;
		}
		solved = 1;
		if (!cancel && !checkSudoku(size)) {
			printf("\n   An error has been detected. Sudoku not solved.\n");
			solved = 0;
		}
		row = 0;
		col = 0;
		newNumber = 0;
		back = 0;
		if (cancel) {
			break;
		}
	}

	for (row = 0; row < size; row++) {
		fixedNumbers[row] = NULL;
		free(fixedNumbers[row]);
	}

	fixedNumbers = NULL;
	free(fixedNumbers);
	return 1;
}

//solve by testing 9-1 from lower right by row
int solveBack(int size, int timelimit) {
	if (checkSudoku(size) == 0) {
		printf("Error in input detected. Cancelling...\n");
		return;
	}
	if (cancel) {
		return 0;
	}
	int solved = 0, row, col, current, newNumber = size + 1, back = 0, valid = 0, counter = 0;
	int **fixedNumbers = (int **)malloc(size * sizeof(int*));
	if (fixedNumbers == NULL) {
		printf("Fehler XX\b");
		return;
	}
	for (row = 0; row < size; row++) {
		fixedNumbers[row] = malloc(size * sizeof(int));
		if (fixedNumbers[row] == NULL) {
			printf("Fehler YY\n");
			return;
		}
	}
	for (row = 0; row < size; row++) {
		for (col = 0; col < size; col++) {
			fixedNumbers[row][col] = numbers[row][col];
		}
	}
	Uint32 start = SDL_GetTicks(), diff, last = SDL_GetTicks();
	while (solved == 0) {
		row = size - 1;
		col = size - 1;
		while (row >= 0) {
			while (col >= 0 && row >= 0) {
				if (cancel) {
					row = -1;
					col = -1;
					solved = 1;
					break;
				}
				diff = SDL_GetTicks() - start;
				if ((SDL_GetTicks() - last) > update) {
					updateNumbers = 1;
					last = SDL_GetTicks();
				}
				if (timelimit <= 0 || diff < timelimit) {
					current = numbers[row][col];
					if (current < 0) {
						printf("ERROR!!! current value is smaller than 0 (%d < 0)!!!\n", current);
						getchar();
						return 0;
					}
					if (fixedNumbers[row][col] == 0) {
						counter++;
						if (counter > 2000000) {
							counter = 0;
						}
						if (back) {
							newNumber = numbers[row][col] - 1;
						}
						else {
							newNumber--;
						}
						valid = 1;
						while (validNumber(row, col, newNumber, size) == 0 && valid == 1) {
							newNumber--;
							if (newNumber < 1) {
								numbers[row][col] = 0;
								back = 1;
								valid = 0;
								newNumber = size + 1;
							}
						}
						if (valid) {
							if (loadProgress < 0.8) {
								loadProgress += LOAD_SPEED;
							}
							numbers[row][col] = newNumber;
							back = 0;
							newNumber = size + 1;
						}
					}
					if (back) {
						col++;
						if (col >= size) {
							if (row < size) {
								col = 0;
								row++;
							}
							else {
								col = size - 1;
								// TODO: check if newNumber > size --> not solvable
							}
						}
					}
					else {
						col--;
						if (col < 0) {
							col = size - 1;
							row--;
						}
					}
				}
				else {
					for (row = 0; row < size; row++) {
						fixedNumbers[row] = NULL;
						free(fixedNumbers[row]);
					}

					fixedNumbers = NULL;
					free(fixedNumbers);
					return 0;
				}
			}
			if (back) {
				row++;
			}
			else {
				row--;
			}
			col = size - 1;
		}
		solved = 1;
		if (checkSudoku(size)) {
		}
		else {
			printf("\n   An error has been detected. Sudoku not solved.\n");
			solved = 0;
		}
		row = 0;
		col = 0;
		newNumber = 0;
		back = 0;
	}
	for (row = 0; row < size; row++) {
		fixedNumbers[row] = NULL;
		free(fixedNumbers[row]);
	}

	fixedNumbers = NULL;
	free(fixedNumbers);
	return 1;
}

//create a full sudoku
void create(int size) {
	if (cancel) {
		return;
	}
	int row, col, newNumber = 1, back = 0, attempts = 0, valid = 0;
	double progress = 0;
	for (row = 0; row < size; row++) {
		for (col = 0; col < size; col++) {
			numbers[row][col] = 0;
		}
	}
	int maxAttempts = size * 4;
	row = 0;
	col = 0;
	Uint32 last = SDL_GetTicks();
	while (row < size) {
		while (col < size) {
			if (cancel) {
				row = size;
				col = size;
				break;
			}
			if (row >= size) {
				break;
			}
			if ((SDL_GetTicks() - last) > update) {
				updateNumbers = 1;
				last = SDL_GetTicks();
			}
			do {
				if (back) {
					newNumber = numbers[row][col] + 1;
					if (newNumber > size) {
						newNumber = 1;
					}
				}
				else {
					newNumber = (rand() % (size)) + 1;
				}
				attempts++;
				if (attempts > maxAttempts) {
					back = 1;
				}
				else {
					valid = validNumber(row, col, newNumber, size);
				}
			} while (valid == 0 && attempts <= maxAttempts);
			attempts = 0;

			if (valid) {
				numbers[row][col] = newNumber;
				//loadProgress += 1/toRemove;
				//printf("Progress B: %lf\n", loadProgress);
				if (loadProgress < 0.5) {
					loadProgress += LOAD_SPEED;
				}
				back = 0;
			}
			else {
				numbers[row][col] = 0;
			}

			if (back) {
				col--;
				if (col < 0) {
					if (row > 0) {
						col = size - 1;
						row--;
					}
					else {
						col = 0;
						back = 0;
					}
				}
			}
			else {
				col++;
				if (col >= size) {
					col = 0;
					row++;
				}
			}
		}
		if (back) {
			row--;
		}
		else {
			row++;
		}
	}
}

//create a full sudoku
void create2(int size) {
	if (cancel) {
		return;
	}
	int row, col, newNumber, iterations = 0;
	for (row = 0; row < size; row++) {
		for (col = 0; col < size; col++) {
			numbers[row][col] = 0;
		}
	}
	Uint32 last = SDL_GetTicks();
	int side = sqrt(size);
	while (iterations == 0) {
		row = 0;
		col = 0;
		while (row < size - side) {
			while (col < size) {
				if (cancel) {
					row = size;
					col = size;
					iterations = 1;
					break;
				}
				if ((SDL_GetTicks() - last) > update) {
					updateNumbers = 1;
					last = SDL_GetTicks();
				}
				do {
					newNumber = (rand() % size) + 1;
					//loadProgress += 1 / toRemove;
					if (loadProgress < 0.5) {
						loadProgress += LOAD_SPEED;
					}
				} while (validNumber(row, col, newNumber, size) == 0);
				numbers[row][col] = newNumber;
				col++;
			}
			row += side;
			col = 0;
		}
		iterations++;
	}
	solve(size, 0);
}

//create a full sudoku
void create4(int size) {
	if (cancel) {
		return;
	}
	int row, col, newNumber, iterations = 0, back = 0, valid = 0, attempts = 0, maxAttempts = size * 3;
	int **fixedNumbers = (int **)malloc(size * sizeof(int *));
	for (row = 0; row < size; row++) {
		for (col = 0; col < size; col++) {
			numbers[row][col] = 0;
		}
	}
	for (row = 0; row < size; row++) {
		fixedNumbers[row] = malloc(size * sizeof(int));
		if (fixedNumbers[row] == NULL) {
			printf("Fehler YY\n");
			return;
		}
	}
	Uint32 last = SDL_GetTicks();
	int side = sqrt(size);
	while (iterations < side - 2 && !cancel) {
		row = iterations * side;
		col = 0;
		while (row < side * (iterations + 1) && !cancel) {
			while (col < size && !cancel) {
				if (cancel) {
					printf("Creating was cancelled A %d %d\n", row, col);
					row = side * (iterations + 1);
					col = size;
					iterations = side - 2;
					break;
				}
				if (iterations == side - 2) {
					if (row > side * (iterations)) {
						row = size;
						col = size;
						break;
					}
				}
				if (row >= side * (iterations + 1)) {
					break;
				}
				if ((SDL_GetTicks() - last) > update) {
					updateNumbers = 1;
					last = SDL_GetTicks();
				}
				do {
					if (back) {
						newNumber = numbers[row][col] + 1;
						if (newNumber > size) {
							newNumber = 1;
						}
					}
					else {
						newNumber = (rand() % (size)) + 1;
					}
					attempts++;
					if (attempts > maxAttempts) {
						back = 1;
					}
					else {
						valid = validNumber(row, col, newNumber, size);
					}
				} while (valid == 0 && attempts <= maxAttempts);

				attempts = 0;

				if (valid) {
					numbers[row][col] = newNumber;
					//loadProgress += 1.0 / (double) toRemove;
					if (loadProgress < 0.5) {
						loadProgress += LOAD_SPEED;
					}

					//printf("Progress D: %lf........", loadProgress);
					back = 0;
				}
				else {
					numbers[row][col] = 0;
				}

				if (back) {
					col--;
					if (col < 0) {
						if (row > iterations) {
							col = size - 1;
							row--;
						}
						else {
							col = 0;
							back = 0;
						}
					}
				}
				else {
					col++;
					if (col >= size) {
						col = 0;
						row++;
					}
				}
			}
			if (back) {
				row--;
			}
			else {
				row++;
			}
		}
		iterations++;
	}
	printf("generating %dx%d Sudoku ...\n", size, size);
	if (!cancel && !solve(size, TIMELIMIT)) {
		for (row = 0; row < size; row++) {
			fixedNumbers[row] = NULL;
			free(fixedNumbers[row]);
		}

		fixedNumbers = NULL;
		free(fixedNumbers);
		create4(size);
		return;
	}
	printf("done A\n");
	for (row = 0; row < size; row++) {
		fixedNumbers[row] = NULL;
		free(fixedNumbers[row]);
	}

	fixedNumbers = NULL;
	free(fixedNumbers);
}

//print the global numbers[] IntArray
void printMatrix(int size) {
	printf("\n");
	int x;
	int y;
	int totalItems = 0;
	int countx = 0, county = 0;
	for (int i = 0; i <= size; i++) {
		printf("%c%c", 220, 220);
	}
	for (int i = 0; i < sqrt(size) + (sqrt(size) - 1); i++) {
		printf("%c", 220);
	}
	printf("\n");
	for (x = 0; x < size; x++) {
		printf("%c ", 219);
		countx++;
		for (y = 0; y < size; y++) {
			if (numbers[x][y] != 0) {
				totalItems++;
			}
			county++;
			if (numbers[x][y] == 0) {
				printf("  ");
			}
			else {
				char c = numbers[x][y] + 48;
				printf("%1c ", c);
			}
			if (county >= sqrt(size)) {
				printf("%c ", 219);
				county = 0;
			}
		}
		if (countx >= sqrt(size)) {
			printf("\n");
			for (int i = 0; i <= size; i++) {
				printf("%c%c", 223, 223);
			}
			for (int i = 0; i < sqrt(size) + (sqrt(size) - 1); i++) {
				printf("%c", 223);
			}
			countx = 0;
		}
		printf("\n");
	}
	printf("\n");
	int progress = (int)(((float)totalItems / (float)((float)size * (float)size)) * 100);
	printf("Progress: %d%%\n", progress);
}

//Check if a sudoku is solved correctly
int checkSudoku(int size) {
	int row, col;
	for (row = 0; row < size; row++) {
		for (col = 0; col < size; col++) {
			if (numbers[row][col] > 0) {
				if (validNumber(row, col, numbers[row][col], size) == 0) {
					printf("Error at %d %d -> %d\n", row, col, numbers[row][col]);
					return 0;
				}
			}
		}
	}
	return 1;
}

//create a full sudoku and remove numbers (depending on size and difficulty)
void generate(int size, int difficulty) {
	if (cancel) {
		printf("Removing was cancelled XYZ\n");
		return;
	}
	loadStage = LOAD_CREATING;
	int col, row, testnum, zero = 0, attempts = 0;
	double progress = 0.0;
	if ((int)sqrt(size) % 1 != 0) {
		printf("size (%d) ist keine Quadratzahl\n", size);
		return;
	}
	int **alreadyTried = (int **)malloc(size * sizeof(int*));
	if (alreadyTried == NULL) {
		printf("Fehler XX\b");
		return;
	}
	for (row = 0; row < size; row++) {
		alreadyTried[row] = malloc(size * sizeof(int));
		if (alreadyTried[row] == NULL) {
			printf("Fehler YY\n");
			return;
		}
	}
	for (row = 0; row < size; row++) {
		for (col = 0; col < size; col++) {
			alreadyTried[row][col] = 0;
			numbers[row][col] = 0;
		}
	}

	difficulties();

	printf("Creating!\n");

	//loadPointsSF = TTF_RenderText_Solid(font, "creating", newColor(20, 30, 255, 255));
	//loadPointTX = SDL_CreateTextureFromSurface(renderer, loadPointsSF);
	if (size == 9) {
		create(size);
	}
	else if (size == 16) {
		if (difficulty == 1) {
			create2(size);
		}
		else if (difficulty == 2) {
			create2(size);
		}
		else {
			create4(size);
		}
	}
	else {
		create4(size);
	}
	//loadPointsSF = TTF_RenderText_Solid(font, "removing numbers", newColor(20, 30, 255, 255));
	//loadPointTX = SDL_CreateTextureFromSurface(renderer, loadPointsSF);
	//loadPointsRect.w = 200;
	row = rand() % size;
	col = rand() % size;
	printf("created! %d\n", toRemove);

	loadStage = LOAD_GENERATING;

	int finalToRemove = toRemove;
	Uint32 last = SDL_GetTicks();

	while (toRemove > 0) {
		//printf("Removing...%d, %d\n", toRemove, cancel);
		if (cancel) {
			printf("Removing was cancelled\n");
			toRemove = 0;
			finalToRemove = 0;
			break;
		}
		do {
			attempts++;
			if (attempts > size * 4) {
				for (row = 0; row < size; row++) {
					alreadyTried[row] = NULL;
					free(alreadyTried[row]);
				}

				alreadyTried = NULL;
				free(alreadyTried);
				if (cancel) {
					printf("Removing was cancelled B\n");
					toRemove = 0;
					finalToRemove = 0;
					attempts = size * 4 + 1;
					break;
				}
				if (!cancel) {
					printf("generating AGAIN\n");
					generate(size, difficulty);
				}
				printf("Returning!!!\n");
				return;
			}
			row = rand() % size;
			col = rand() % size;
			printf("random!!!\n");
		} while (numbers[row][col] == 0 || alreadyTried[row][col] == 1);
		attempts = 0;
		testnum = numbers[row][col];
		numbers[row][col] = 0;
		if ((SDL_GetTicks() - last) > update) {
			updateNumbers = 1;
			last = SDL_GetTicks();
		}
		printf("comparing solutions!!!\n");
		if (compareSolutions(size, 0) == 0) {
			numbers[row][col] = testnum;
			toRemove++;
		}
		else {
			//loadProgress += LOAD_SPEED;
			if (loadProgress < 0.99) {
				loadProgress += 1.0 / ((double)finalToRemove);
			}
		}
		alreadyTried[row][col] = 1;
		toRemove--;
		printf("bli bla blub\n");
		//progress += (100 / size * size);
		/*for (row = 0; row < size; row++) {
		for (col = 0; col < size; col++) {
		if (numbers[row][col] == 0) {
		zero++;
		}
		}
		}
		zero = 0;*/
	}
	printf("ENDE A\n");

	loadProgress = 1;

	for (row = 0; row < size; row++) {
		alreadyTried[row] = NULL;
		free(alreadyTried[row]);
	}

	for (row = 0; row < size; row++) {
		for (col = 0; col < size; col++) {
			generatedNumbers[row][col] = numbers[row][col];
		}
	}

	alreadyTried = NULL;
	free(alreadyTried);
	printf("ENDE B\n");
}

//calculate the width/heigth the single squares have
int getSquareSize(int size) {
	if (winx > winy) {
		return (winy * 0.75) / size;
	}
	return (winx * 0.75) / size;
}

//initialize SDL_Rects for sudokufields
void initializeRects(int size, SDL_Rect* sudoku) {
	for (int i = 0; i < size * size; i++) {
		SDL_Rect rect = { 0, 0, 0, 0 };
		sudoku[i] = rect;
	}
}

//initialize sudokufields, buttons, panels, etc.
void initializeSize(int size, SDL_Rect* sudoku) {
	int col = 0, row = 0, dim2 = 0;
	int dim = getSquareSize(size);
	Feld feld;
	SDL_Rect rect;
	int x = 20, y = 20, side = sqrt(size);
	for (int i = row * size; i < size * size; i++) {
		rect = sudoku[i];
		feld = felder[i];
		if (rect.w != dim) {
			rect.w = dim;
		}
		if (rect.h != dim) {
			rect.h = dim;
		}
		if ((col) % side == 0) {
			x += 10;
		}
		x += dim;
		if (!ascii && digitsOf(numbers[row][col]) > 1 || numbers[row][col] == 16) {
			feld.rect.w = dim * 0.8;
		}
		else {
			feld.rect.w = dim * 0.4;
		}
		feld.rect.x = x + dim / 2 - feld.rect.w / 2;
		feld.rect.y = y + dim / 2 - feld.rect.h / 2;


		rect.x = x;
		rect.y = y;
		sudoku[i] = rect;
		felder[i] = feld;

		x += 5;

		col++;
		if (col >= size) {
			col = 0;
			row++;
			x = 20;
			y += dim;
			if ((row) % side == 0) {
				y += 10;
			}
			y += 5;
		}
	}
}

//draw all SDL_Rects
void drawRects(int size, SDL_Rect* sudoku, int mouseX, int mouseY) {
	int mouseOvered = calculating;
	int side = sqrt(size), col = 0, row = 0, color = 0, i;
	SDL_Rect rect;
	SDL_Rect frame;
	int momocolor = 0;
	if (side % 2 == 1) {
		for (i = 0; i < size*size; i++) {
			rect = sudoku[i];
			if (momodings) {
				momocolor = momodinger[row][col] - 1;
				printf("%d, %d, %d\n", row, col, momocolor);
			}
			if (generatedNumbers[row][col] == 0) {
				frame.x = rect.x - 2;
				frame.y = rect.y - 2;
				frame.w = rect.w + 4;
				frame.h = rect.h + 4;
				SDL_SetRenderDrawColor(renderer, 0x55, 0x55, 0xff, SDL_ALPHA_OPAQUE);
				SDL_RenderFillRect(renderer, &frame);
				if (color == 0 || momocolor > 0) {
					if (selectedSlot == i) {
						if (momocolor > 0) {
							SDL_SetRenderDrawColor(renderer, selectedColor[momocolor].r, selectedColor[momocolor].g, selectedColor[momocolor].b, SDL_ALPHA_OPAQUE);
						}
						else {
							SDL_SetRenderDrawColor(renderer, 0x75, 0x75, 0xff, SDL_ALPHA_OPAQUE);
						}
					}
					else {
						if (!mouseOvered && mouseOver(rect, mouseX, mouseY)) {
							if (momocolor > 0) {
								SDL_SetRenderDrawColor(renderer, mouseOverColor[momocolor].r, mouseOverColor[momocolor].g, mouseOverColor[momocolor].b, SDL_ALPHA_OPAQUE);
							}
							else {
								SDL_SetRenderDrawColor(renderer, 0x65, 0x65, 0xff, SDL_ALPHA_OPAQUE);
							}
							mouseOvered = 1;
						}
						else {
							if (momocolor > 0) {
								SDL_SetRenderDrawColor(renderer, standardColor[momocolor].r, standardColor[momocolor].g, standardColor[momocolor].b, SDL_ALPHA_OPAQUE);
							}
							else {
								SDL_SetRenderDrawColor(renderer, 0x45, 0x45, 0xff, SDL_ALPHA_OPAQUE);
							}
						}
					}
				}
				else {
					if (selectedSlot == i) {
						SDL_SetRenderDrawColor(renderer, 0x60, 0x60, 0xff, SDL_ALPHA_OPAQUE);
					}
					else {
						if (!mouseOvered && mouseOver(rect, mouseX, mouseY)) {
							SDL_SetRenderDrawColor(renderer, 0x50, 0x50, 0xff, SDL_ALPHA_OPAQUE);
							mouseOvered = 1;
						}
						else {
							SDL_SetRenderDrawColor(renderer, 0x30, 0x30, 0xff, SDL_ALPHA_OPAQUE);
						}
					}
				}
			}
			else {
				if (color == 0 || momocolor > 0) {
					if (momocolor > 0) {
						SDL_SetRenderDrawColor(renderer, standardColor[momocolor].r, standardColor[momocolor].g, standardColor[momocolor].b, SDL_ALPHA_OPAQUE);
					}
					else {
						SDL_SetRenderDrawColor(renderer, 0x45, 0x45, 0xff, SDL_ALPHA_OPAQUE);
					}
				}
				else {
					SDL_SetRenderDrawColor(renderer, 0x30, 0x30, 0xff, SDL_ALPHA_OPAQUE);
				}
			}
			SDL_RenderFillRect(renderer, &rect);
			col++;
			if (col % side == 0 && col < size) {
				if (!momodings) {
					color = color == 1 ? 0 : 1;
				}
			}
			if (col >= size) {
				row++;
				col = 0;
				if (row % side == 0) {
					if (!momodings) {
						color = color == 1 ? 0 : 1;
					}
				}
			}
		}
	}
	else {
		for (i = 0; i < size*size; i++) {
			rect = sudoku[i];
			if (generatedNumbers[row][col] == 0) {
				frame.x = rect.x - 2;
				frame.y = rect.y - 2;
				frame.w = rect.w + 4;
				frame.h = rect.h + 4;
				SDL_SetRenderDrawColor(renderer, 0x55, 0x55, 0xff, SDL_ALPHA_OPAQUE);
				SDL_RenderFillRect(renderer, &frame);
				if (color == 0 || momocolor > 0) {
					if (selectedSlot == i) {
						if (momocolor > 0) {
							SDL_SetRenderDrawColor(renderer, selectedColor[momocolor].r, selectedColor[momocolor].g, selectedColor[momocolor].b, SDL_ALPHA_OPAQUE);
						}
						else {
							SDL_SetRenderDrawColor(renderer, 0x75, 0x75, 0xff, SDL_ALPHA_OPAQUE);
						}
					}
					else {
						if (!mouseOvered && mouseOver(rect, mouseX, mouseY)) {
							if (momocolor > 0) {
								SDL_SetRenderDrawColor(renderer, mouseOverColor[momocolor].r, mouseOverColor[momocolor].g, mouseOverColor[momocolor].b, SDL_ALPHA_OPAQUE);
							}
							else {
								SDL_SetRenderDrawColor(renderer, 0x65, 0x65, 0xff, SDL_ALPHA_OPAQUE);
							}
							mouseOvered = 1;
						}
						else {
							if (momocolor > 0) {
								SDL_SetRenderDrawColor(renderer, standardColor[momocolor].r, standardColor[momocolor].g, standardColor[momocolor].b, SDL_ALPHA_OPAQUE);
							}
							else {
								SDL_SetRenderDrawColor(renderer, 0x45, 0x45, 0xff, SDL_ALPHA_OPAQUE);
							}
						}
					}
				}
				else {
					if (selectedSlot == i) {
						SDL_SetRenderDrawColor(renderer, 0x60, 0x60, 0xff, SDL_ALPHA_OPAQUE);
					}
					else {
						if (!mouseOvered && mouseOver(rect, mouseX, mouseY)) {
							SDL_SetRenderDrawColor(renderer, 0x50, 0x50, 0xff, SDL_ALPHA_OPAQUE);
							mouseOvered = 1;
						}
						else {
							SDL_SetRenderDrawColor(renderer, 0x30, 0x30, 0xff, SDL_ALPHA_OPAQUE);
						}
					}
				}
			}
			else {
				if (color == 0 || momocolor > 0) {
					if (momocolor > 0) {
						SDL_SetRenderDrawColor(renderer, standardColor[momocolor].r, standardColor[momocolor].g, standardColor[momocolor].b, SDL_ALPHA_OPAQUE);
					}
					else {
						SDL_SetRenderDrawColor(renderer, 0x45, 0x45, 0xff, SDL_ALPHA_OPAQUE);
					}
				}
				else {
					SDL_SetRenderDrawColor(renderer, 0x30, 0x30, 0xff, SDL_ALPHA_OPAQUE);
				}
			}
			SDL_RenderFillRect(renderer, &rect);
			col++;
			if (col % side == 0) {
				if (!momodings) {
					color = color == 1 ? 0 : 1;
				}
			}
			if (col >= size) {
				row++;
				col = 0;
				if (row % side == 0) {
					if (!momodings) {
						color = color == 1 ? 0 : 1;
					}
				}
			}
		}
	}
}

//Print the designated string at the specified coordinates
void printFont(char *c, int x, int y) {
	fontSurface = TTF_RenderText_Solid(font, c, fontColor);
	fontRect.x = x;
	fontRect.y = y;
	int sq = getSquareSize(size);
	fontRect.h = sq;
	fontRect.w = sq / 2;
	fontTexture = SDL_CreateTextureFromSurface(renderer, fontSurface);
	SDL_RenderCopy(renderer, fontTexture, NULL, &fontRect);
}

//Create a SDL_Color (red, green, blue, transparency)
SDL_Color newColor(int r, int g, int b, int a) {
	SDL_Color color = { r, g, b, a };
	return color;
}

//Create Struct Feld with all data
Feld createFeld(int num, int row, int col, int x, int y, int w, int h, SDL_Color color) {
	Feld feld;
	SDL_Rect rect = { 0, 0, 0, 0 };
	feld.rect = rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	//char *c = num + 48;
	feld.number = num;
	feld.row = row;
	feld.col = col;
	if (num < 10) {
		Uint16 ch;
		if (num == 0) {
			ch = 'x';
		}
		else {
			ch = num + 48;
		}
		if (ch < 0) {
			ch = 0;
		}
		feld.surface = TTF_RenderGlyph_Solid(font, ch, color);
	}
	else {
		char* c = intToString(num);
		feld.surface = TTF_RenderText_Solid(font, c, color);
		c = NULL;
		free(c);
	}
	feld.texture = SDL_CreateTextureFromSurface(renderer, feld.surface);
	return feld;
}

//convert a number of type Integer to a string
char* intToString(int num) {
	int digits = floor(log10(abs(num))) + 1, i;
	char* string = (char*)malloc((digits + 1) * sizeof(char));
	for (i = digits - 1; i >= 0; i--) {
		string[i] = (num % 10) + 48;
		num /= 10;
	}
	string[digits] = '\0';
	return string;
}

//convert a number of type Integer to a string with length 2 (ASCII + '\0')
char* intToStringAscii(int num) {
	char* string = (char*)malloc(2 * sizeof(char));
	char c = num + 48;
	string[0] = c;
	string[1] = '\0';
	return string;
}

//update changes of a 'Feld'
Feld updateFeld(Feld feld, int num, SDL_Color color) {
	if (ascii || num <= 9) {
		Uint16 ch;
		if (num == 0) {
			ch = 32;
		}
		else {
			ch = num + 48;
		}
		if (ch < 0) {
			ch = 0;
		}
		feld.surface = TTF_RenderGlyph_Solid(font, ch, color);
	}
	else {
		char* c = intToString(num);
		feld.surface = TTF_RenderText_Solid(font, c, color);
		c = NULL;
		free(c);
	}
	feld.texture = SDL_CreateTextureFromSurface(renderer, feld.surface);
	feld.number = num;
	return feld;
}

//draw a 'Feld'
void printFeld(Feld feld) {
	SDL_RenderCopy(renderer, feld.texture, NULL, &feld.rect);
}

//initialize loadbar
void load() {
	loadbar.x = 0;
	loadbar.y = winy - 25;
	loadbar.w = 0;
	loadbar.h = 25;
}

//calculate numbers to remove (depending on size & difficulty
void difficulties() {
	if (size == 9) {
		if (difficulty == 1) {
			toRemove = size*size * 0.6;
		}
		else if (difficulty == 2) {
			toRemove = size*size * 0.67;
		}
		else {
			toRemove = size*size * 0.72;
		}
		update = 100;
	}
	else if (size == 16) {
		if (difficulty == 1) {
			toRemove = size*size * 0.42;
		}
		else if (difficulty == 2) {
			toRemove = size*size * 0.47;
		}
		else {
			toRemove = size*size * 0.5;
		}
		update = 600;
	}
	else {
		if (difficulty == 1) {
			toRemove = size*size * 0.3;
		}
		else if (difficulty == 2) {
			toRemove = size*size * 0.34;
		}
		else {
			toRemove = size*size * 0.38;
		}
		update = 32 * size;
	}
}

//change the size (during runtime)
void changeSize(int newsize) {
	SDL_RenderClear(renderer);
	size = newsize;
	int i, j;

	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			numbers[i][j] = 0;
			generatedNumbers[i][j] = 0;
		}
	}
	int squareSize = getSquareSize(size);
	for (i = 0; i < size*size; i++) {
		Feld feld = createFeld(0, i / size, i % size, 0, 0, squareSize * 0.4, squareSize*0.8, newColor(255, 255, 255, SDL_ALPHA_OPAQUE));
		felder[i] = feld;
		if (!momodings) {
			animatedFields[i] = 0;
		}
	}
	initializeRects(size, fields);
	initializeSize(size, fields, 0, 0);
	for (i = 0; i < size*size; i++) {
		SDL_Rect rect = fields[i];
		rect.h = 0;
		rect.w = 0;
		fields[i] = rect;
	}
	selectedSlot = -1;
}

//change the difficulty (during runtime)
void changeDifficulty(int newdifficulty) {
	difficulty = newdifficulty;
	difficulties();

	int squareSize = getSquareSize(size);
}

//get the number of digits an Integervalue has
int digitsOf(int num) {
	int digits = floor(log10(abs(num))) + 1;
	return digits;
}

//add a button to the programm's surface
void addbutton(int x, int y, int w, int h, char* text, int tab, int func, SDL_Color color, int tx, int ty, int tw, int th) {
	Button button;
	button.function = func;
	button.tab = tab;
	SDL_Rect btnrect;
	btnrect.x = x;
	btnrect.y = y;
	btnrect.w = w;
	btnrect.h = h;
	button.rect = btnrect;
	button.txrect.x = tx;
	button.txrect.y = ty;
	button.txrect.w = tw;
	button.txrect.h = th;
	button.txsurface = TTF_RenderText_Solid(font, text, color);
	button.txtexture = SDL_CreateTextureFromSurface(renderer, button.txsurface);
	btnAmount++;
	Button* newbuttons = realloc(buttons, btnAmount * sizeof(Button));
	if (newbuttons) {
		buttons = newbuttons;
		buttons[btnAmount - 1] = button;
	}
	else {
		btnAmount--;
	}
}

//place a rectangle on the surface (related to a corner of a rectangle)
void placeRect(SDL_Rect* master, SDL_Rect* slave, int corner, int dx, int dy) {
	/*
	corners:
	0 --- 1
	|
	|
	2 --- 3
	*/
	if (corner == 0) {
		slave->x = master->x + dx;
		slave->y = master->y + dy;
	}
	if (corner == 1) {
		slave->x = master->x + dx + master->w;
	}
	if (corner == 2) {
		slave->y = master->y + dy + master->h;
	}
	if (corner == 3) {
		slave->x = master->x + dx + master->w;
		slave->y = master->y + dy + master->h;
	}
}

//produce a rectangle for the texture on the rechtangle in parameters
struct Text textRect(SDL_Rect rect, char* c) {
	Text text;
	fontSurface = TTF_RenderText_Solid(font, c, newColor(255, 255, 255, 255));
	text.texture = SDL_CreateTextureFromSurface(renderer, fontSurface);
	text.rect = rect;
	textAmount++;
	Text* newtexts = realloc(texts, textAmount * sizeof(Text));
	if (newtexts) {
		texts = newtexts;
		texts[textAmount - 1] = text;
	}
	else {
		textAmount--;
	}
	return text;
}

//initialize the buttons for the Numpad
void initializeNumpad() {
	char* string;
	SDL_Color color = newColor(255, 255, 255, 255);
	int row = 0, col = 0, i = 1, j, w, x, existing = 0;
	Button button;
	for (row = 0; row < 3; row++) {
		for (col = 0; col < 7; col++) {
			if (ascii) {
				string = intToStringAscii(i);
				w = i == 16 ? 40 : 20;
				x = i == 16 ? winx - (70 * (7 - col)) - 90 : winx - (70 * (7 - col)) - 80;
			}
			else {
				if (i > 9) {
					string = intToString(i);
					w = 40;
					x = winx - (70 * (7 - col)) - 90;
				}
				else {
					string = intToStringAscii(i);
					w = 20;
					x = winx - (70 * (7 - col)) - 80;
				}
			}
			for (j = 0; j < btnAmount && !existing; j++) {
				button = buttons[j];
				if (button.tab == TAB_NUMPAD && button.function == j) {
					button.txsurface = TTF_RenderText_Solid(font, string, color);
					button.txtexture = SDL_CreateTextureFromSurface(renderer, button.txsurface);
					buttons[j] = button;
					existing = 1;
				}
			}
			if (!existing) {
				addbutton(winx - (70 * (7 - col)) - 100, 180 + row * 60, 60, 50, string, TAB_NUMPAD, i, color, x, 185 + row * 60, w, 40);
			}
			i++;
			free(string);
		}
	}
	for (col = 0; col < 4; col++) {
		if (ascii) {
			string = intToStringAscii(i);
			w = i == 16 ? 40 : 20;
			x = i == 16 ? winx - (70 * (7 - col)) - 90 : winx - (70 * (7 - col)) - 80;
		}
		else {
			if (i > 9) {
				string = intToString(i);
				w = 40;
				x = winx - (70 * (7 - col)) - 90;
			}
			else {
				string = intToStringAscii(i);
				w = 20;
				x = winx - (70 * (7 - col)) - 80;
			}
		}
		for (j = 0; j < btnAmount && !existing; j++) {
			button = buttons[j];
			if (button.tab == TAB_NUMPAD && button.function == j) {
				button.txsurface = TTF_RenderText_Solid(font, string, color);
				button.txtexture = SDL_CreateTextureFromSurface(renderer, button.txsurface);
				buttons[j] = button;
				existing = 1;
			}
		}
		if (!existing) {
			addbutton(winx - (70 * (7 - col)) - 100, 180 + row * 60, 60, 50, string, TAB_NUMPAD, i, color, x, 185 + row * 60, w, 40);
		}
		i++;
		free(string);
	}
	for (j = 0; j < btnAmount && !existing; j++) {
		button = buttons[j];
		if (button.tab == TAB_NUMPAD && button.function == j) {
			button.txsurface = TTF_RenderText_Solid(font, string, color);
			button.txtexture = SDL_CreateTextureFromSurface(renderer, button.txsurface);
			buttons[j] = button;
			existing = 1;
		}
	}
	if (!existing) {
		addbutton(winx - (70 * (7 - col)) - 100, 180 + row * 60, 60, 50, "DEL", TAB_NUMPAD, -999, color, winx - (70 * (7 - col)) - 100, 185 + row * 60, 60, 40);
	}
}

//get ID of Rect, the mouse is on
int mouseOver(SDL_Rect rect, int mouseX, int mouseY) {
	if (mouseX >= rect.x && mouseX <= rect.x + rect.w
		&& mouseY >= rect.y && mouseY <= rect.y + rect.h) {
		return 1;
	}
	return 0;
}

//generate Nonomino-regions
// not working yet
void generateMomodings() {
	int row = 0, col = 0, currentSize = 0, number = 1, checkedSides = 0, random, attempts = 0, r, c, step = 0;
	for (row = 0; row < size; row++) {
		for (col = 0; col < size; col++) {
			momodinger[row][col] = 0;
		}
	}
	row = 0;
	col = 0;
	while (number <= size) {
		while (currentSize < size) {
			if (momodinger[row][col] == 0) {
				momodinger[row][col] = number;
				currentSize++;
			}
			if (step == 0) {
				if (col > 0 && rand() % 6 == 0) {
					col = 0;
					row++;
				}
				else {
					col++;
					if (col >= size) {
						for (r = row + 1; r < size; r++) {
							for (c = 0; c < size; c++) {
								if (momodinger[r][c] == 0) {
									row = r;
									col = c;
									r = size;
									c = size;
								}
							}
						}
					}
				}
				printf("%d, %d -> %d\n", row, col, number);
				if (row >= size) {
					for (r = 0; r < size; r++) {
						for (c = 0; c < size; c++) {
							if (momodinger[r][c] == 0) {
								row = r;
								col = c;
								r = size;
								c = size;
							}
						}
					}
					printf("next row A %d\n", number);
				}
			}
			else if (step == 1) {
				if (col < size - 1 && rand() % 2 == 0) {
					col = size - 1;
					row++;
				}
				else {
					col--;
				}
				if (row >= size) {
					step = 2;
					row = 0;
					col = size - 1;
					printf("next row A %d\n", number);
				}
			}
			else {
				currentSize = size;
				number = size + 1;
			}
		}
		col = 0;
		row++;
		number++;
		currentSize = 0;
	}
	//do {
	//
	while (currentSize < size) {
		//
		if (currentSize == 0) {
			//
			attempts = 0;
			//
			for (r = 0; r < size; r++) {
				//
				for (c = 0; c < size; c++) {
					//
					if (momodinger[r][c] == 0) {
						//
						row = r;
						//
						col = c;
						//
						r = size;
						//
						c = size;
						//
					}
					//
				}
				//
			}
			//
		}
		//
		else {
			//
			attempts++;
			//
			if (attempts > 10000 * size) {
				//
				attempts = 0;
				//
				printf("too many attempts\n");
				//
				for (row = 0; row < size; row++) {
					//
					for (col = 0; col < size; col++) {
						//
						numbers[row][col] = momodinger[row][col];
						//
					}
					//
				}
				//
				printMatrix(size);
				//
				getchar();
				//
				generateMomodings();
				//
				return;
				//
			}
			//
			random = rand() % 3;
			//
			if (random <= 1) {
				//
				if (row < size - 1) {
					//
					row++;
					//
				}
				//
				else {
					//
					/*for (r = 0; r < size; r++) {
					//
					for (c = 0; c < size; c++) {
					//
					if (momodinger[r][c] == 0) {
					//
					row = r;
					//
					col = c;
					//
					r = size;
					//
					c = size;
					//
					}
					//
					}
					//
					}*/
					//
					continue;
					//
				}
				//
			}
			//
			else if (random == 2) {
				//
				if (col < size - 1) {
					//
					col++;
					//
				}
				//
				else {
					//
					/*for (r = 0; r < size; r++) {
					//
					for (c = 0; c < size; c++) {
					//
					if (momodinger[r][c] == 0) {
					//
					row = r;
					//
					col = c;
					//
					r = size;
					//
					c = size;
					//
					}
					//
					}
					//
					}*/
					//
					continue;
					//
				}
				//
			}
			//
		}
		//
		if (momodinger[row][col] != 0) {
			//
			continue;
			//
		}
		//
		//printf("%d, %d     %d, %d      |%d attempts\n", row, col, currentSize, number, attempts);
		//
		if (currentSize == 0) {
			//
			momodinger[row][col] = number;
			//
			currentSize++;
			//
			continue;
			//
		}
		//
		checkedSides = 0;
		//
		if (row > 0) {
			//
			if (momodinger[row - 1][col] == number) {
				//
				momodinger[row][col] = number;
				//
				currentSize++;
				//
				continue;
				//
			}
			//
			else {
				//
				attempts++;
				//
			}
			//
		}
		//
		if (row < size - 1) {
			//
			if (momodinger[row + 1][col] == number) {
				//
				momodinger[row][col] = number;
				//
				currentSize++;
				//
				continue;
				//
			}
			//
			else {
				//
				attempts++;
				//
			}
			//
		}
		//
		if (col > 0) {
			//
			if (momodinger[row][col - 1] == number) {
				//
				momodinger[row][col] = number;
				//
				currentSize++;
				//
				continue;
				//
			}
			//
			else {
				//
				attempts++;
				//
			}
			//
		}
		//
		if (col < size - 1) {
			//
			if (momodinger[row][col + 1] == number) {
				//
				momodinger[row][col] = number;
				//
				currentSize++;
				//
				continue;
				//
			}
			//
			else {
				//
				attempts++;
				//
			}
			//
		}
		//
	}
	//
	//for (row = 0; row < size; row++) {
	//
	// for (col = 0; col < size; col++) {
	//
	// numbers[row][col] = momodinger[row][col];
	//
	// }
	//
	//}
	//
	number++;
	//
	currentSize = 0;
	//} while (number <= size);
	for (row = 0; row < size; row++) {
		for (col = 0; col < size; col++) {
			numbers[row][col] = momodinger[row][col];
		}
	}
	printMatrix(size);
	getchar();
}

//initialize fix Nonomino-regions
void generateMomodings2() {
	int line1[] = { 1, 1, 1, 2, 3, 3, 3, 3, 3 };
	int line2[] = { 1, 1, 1, 2, 2, 2, 3, 3, 3 };
	int line3[] = { 1, 4, 4, 4, 4, 2, 2, 2, 3 };
	int line4[] = { 1, 1, 4, 5, 5, 5, 5, 2, 2 };
	int line5[] = { 4, 4, 4, 4, 5, 6, 6, 6, 6 };
	int line6[] = { 7, 7, 5, 5, 5, 5, 6, 8, 8 };
	int line7[] = { 9, 7, 7, 7, 6, 6, 6, 6, 8 };
	int line8[] = { 9, 9, 9, 7, 7, 7, 8, 8, 8 };
	int line9[] = { 9, 9, 9, 9, 9, 7, 8, 8, 8 };
	int i;
	for (i = 0; i < 9; i++) {
		momodinger[0][i] = line1[i];
	}
	for (i = 0; i < 9; i++) {
		momodinger[1][i] = line2[i];
	}
	for (i = 0; i < 9; i++) {
		momodinger[2][i] = line3[i];
	}
	for (i = 0; i < 9; i++) {
		momodinger[3][i] = line4[i];
	}
	for (i = 0; i < 9; i++) {
		momodinger[4][i] = line5[i];
	}
	for (i = 0; i < 9; i++) {
		momodinger[5][i] = line6[i];
	}
	for (i = 0; i < 9; i++) {
		momodinger[6][i] = line7[i];
	}
	for (i = 0; i < 9; i++) {
		momodinger[7][i] = line8[i];
	}
	for (i = 0; i < 9; i++) {
		momodinger[8][i] = line9[i];
	}
	//momodinger[1] = line2;
	//momodinger[2] = line3;
	//momodinger[3] = line4;
	//momodinger[4] = line5;
	//momodinger[5] = line6;
	//momodinger[6] = line7;
	//momodinger[7] = line8;
	//momodinger[8] = line9;

	for (i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			//printf("momocolor: %d %d -> %d\n", i, j, momodinger[i][j]);
		}
	}

	standardColor[0] = newColor(178, 34, 34, 255);
	standardColor[1] = newColor(128, 0, 128, 255);
	standardColor[2] = newColor(47, 79, 79, 255);
	standardColor[3] = newColor(210, 105, 30, 255);
	standardColor[4] = newColor(0, 128, 0, 255);
	standardColor[5] = newColor(128, 128, 0, 255);
	standardColor[6] = newColor(184, 134, 111, 255);
	standardColor[7] = newColor(255, 69, 150, 255);
	standardColor[8] = newColor(46, 139, 87, 255);

	mouseOverColor[0] = newColor(198, 54, 54, 255);
	mouseOverColor[1] = newColor(148, 20, 148, 255);
	mouseOverColor[2] = newColor(67, 99, 99, 255);
	mouseOverColor[3] = newColor(230, 125, 50, 255);
	mouseOverColor[4] = newColor(20, 148, 20, 255);
	mouseOverColor[5] = newColor(148, 148, 20, 255);
	mouseOverColor[6] = newColor(204, 154, 131, 255);
	mouseOverColor[7] = newColor(255, 89, 170, 255);
	mouseOverColor[8] = newColor(76, 149, 97, 255);

	selectedColor[0] = newColor(198, 54, 54, 255);
	selectedColor[1] = newColor(148, 20, 148, 255);
	selectedColor[2] = newColor(67, 99, 99, 255);
	selectedColor[3] = newColor(230, 125, 50, 255);
	selectedColor[4] = newColor(20, 148, 20, 255);
	selectedColor[5] = newColor(148, 148, 20, 255);
	selectedColor[6] = newColor(204, 154, 131, 255);
	selectedColor[7] = newColor(255, 89, 170, 255);
	selectedColor[8] = newColor(76, 149, 97, 255);
}

/*//save current game
void saveSudoku() {
	for (int row = 0; row < size; row++) {
		for (int col = 0; col < size; col++) {
			savedSudoku[row][col] = numbers[row][col];
		}
	}
}

//load saved game
void loadSudoku() {
	for (int row = 0; row < size; row++) {
		for (int col = 0; col < size; col++) {
			generatedNumbers[row][col] = savedSudoku[row][col];
		}
	}
}*/
