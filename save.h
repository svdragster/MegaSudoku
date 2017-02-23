#ifndef SAVE
#define SAVE
#endif


typedef struct Sudoku {
	int size;
	int** numbers;
	int** generatedNumbers;
} Sudoku;

Sudoku toSudoku(int** numbers, int** generatedNumbers, int size);
void fromSudoku(Sudoku sudoku, int** numbers, int** generatedNumbers);
Sudoku loadFromFile();
