#include <stdio.h>
#include "save.h"

Sudoku toSudoku(int** numbers, int** generatedNumbers, int size) {
	Sudoku sudoku;
	sudoku.numbers = (int**)malloc(size * sizeof(int*));
	sudoku.generatedNumbers = (int**)malloc(size * sizeof(int*));
	int r, c;
	for (r = 0; r < size; r++) {
		sudoku.numbers[r] = (int*)malloc(size * sizeof(int));
		sudoku.generatedNumbers[r] = (int*)malloc(size * sizeof(int));
		for (c = 0; c < size; c++) {
			sudoku.numbers[r][c] = numbers[r][c];
			sudoku.generatedNumbers[r][c] = generatedNumbers[r][c];
		}
	}
	sudoku.size = size;
	return sudoku;
}

void fromSudoku(Sudoku sudoku, int** numbers, int** generatedNumbers) {
	int r, c;
	for (r = 0; r < sudoku.size - 1; r++) {
		for (c = 0; c < sudoku.size - 1; c++) {
			printf("%d, %d\n", r, c);
			numbers[r][c] = sudoku.numbers[r][c];
			generatedNumbers[r][c] = sudoku.generatedNumbers[r][c];
		}
	}
}

void save(Sudoku sudoku) {

}

Sudoku loadFromFile() {
	Sudoku sudoku;
	sudoku.numbers = (int**)malloc(25 * sizeof(int*));
	sudoku.generatedNumbers = (int**)malloc(25 * sizeof(int*));
	int r, c;
	for (r = 0; r < 25; r++) {
		sudoku.numbers[r] = (int*)malloc(25 * sizeof(int));
		sudoku.generatedNumbers[r] = (int*)malloc(25 * sizeof(int));
	}
	errno_t err;
	FILE *stream;
	err = fopen_s(&stream, "stuhl.gang", "r");
	if (err != 0) {
		return;
	}
	r = 0;
	c = 0;
	int size = 0;
	int generated = 0;
	int currentNumber = 0;
	int ch;
	char zeichen;
	while (!feof(stream)) {
		ch = fgetc(stream);
		if (ch != EOF) {
			zeichen = ch;
			if (zeichen == '|') {
				r++;
				c = 0;
				continue;
			}
			if (zeichen == ' ') {
				printf("%d %d -> %d\n", r, c, currentNumber);
				if (generated) {
					sudoku.generatedNumbers[r][c] = currentNumber;
				}
				else {
					sudoku.numbers[r][c] = currentNumber;
				}
				c++;
				size++;
				currentNumber = 0;
				continue;
			}
			if (zeichen != '\n') {
				currentNumber *= 10;
				currentNumber += ch - 48;
			}
			if (zeichen == '\n') {
				printf("----> %d\n", currentNumber);
				sudoku.numbers[r][c] = currentNumber;
				size++;
				generated = 1;
				sudoku.size = r + 1;
				size = 0;
				currentNumber = 0;
				r = 0;
				c = 0;
				continue;
			}
		}
	}
	sudoku.generatedNumbers[r][c] = currentNumber;
	printf("doneeee\n");
	err = fclose(stream);
	if (err != 0) {
		printf("error\n");
		return;
	}
	return sudoku;
}
