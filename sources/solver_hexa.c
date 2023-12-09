#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <err.h>
#include "../headers/solver_hexa.h"

#define N 16












const int SCREEN_WIDTH_HEXA = 640;
const int SCREEN_HEIGHT_HEXA = 640;


const int CELL_SIZE_HEXA = SCREEN_WIDTH_HEXA / N;

char allowed[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
char notes[N][N][16] = {0};


char** getFromFile(char* file) {
    char** grid = malloc(16 * sizeof(char*)); 
    for (int i = 0; i < 16; i++) {
        grid[i] = malloc(16); 
    }
    FILE *fp = fopen(file, "r");

    if (fp == NULL) {
        errx(1, "Error opening the file.\n");
    }

    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            char currentChar;

            
            do {
                if (fscanf(fp, " %c", &currentChar) != 1) {
                    printf("Error reading from file.\n");
                }
            } while (currentChar == ' ' || currentChar == '\n');

            if (currentChar != '.')
                grid[row][col] = currentChar;
            else
                grid[row][col] = 'X';
        }
    }

    fclose(fp);
    return grid;
}


int allowedCol(char** grid, int col, char num) {
    for (int row = 0; row < N; row++)
        if (grid[row][col] == num)
            return 1;
    return 0;
}


int allowedRow(char** grid, int row, char num) {
    for (int col = 0; col < N; col++)
        if (grid[row][col] == num)
            return 1;
    return 0;
}


int allowedBox(char** grid, int boxRow, int boxCol, char num) {
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            if (grid[row + boxRow][col + boxCol] == num)
                return 1;
    return 0;
}


int isEmpty(char** grid, int *row, int *col) {
    for (*row = 0; *row < N; (*row)++)
        for (*col = 0; *col < N; (*col)++)
            if (grid[*row][*col] == 'X')
                return 1;
    return 0;
}


int checkValid(char** grid, int row, int col, char num) {
    return !allowedBox(grid, row - row % 4, col - col % 4, num) && !allowedCol(grid, col, num) && !allowedRow(grid, row, num);
}


void dispBoard(char** grid) {
    printf(" --------------------------------------------\n");
    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            if (col == 4 || col == 8 || col == 12 || col == 0)
                printf(" | ");
            if (grid[row][col] == 'X')
                printf("_ ");
            else
                printf("%c ", grid[row][col]);
            if (col == 15)
                printf("| ");
        }
        if (row == 3 || row == 7 || row == 11) {
            printf("\n");
            for (int i = 0; i < 4; i++)
                printf("-----------");
        }
        printf("\n");
    }
    printf(" --------------------------------------------\n");
}


void buildNotes(char **grid) {
    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            if (grid[row][col] == 'X') {
                for (int num = 0; num < 16; num++) {
                    if (checkValid(grid, row, col, allowed[num])) {
                        notes[row][col][num] = 1;
                    }
                }
            }
        }
    }
}


int solveSudoku(char **grid) {
    int row, col;
    if (!isEmpty(grid, &row, &col))
        return 1;

    for (int num = 0; num < 16; num++) {
        if (notes[row][col][num]) {
            char currentNum = allowed[num];
            if (checkValid(grid, row, col, currentNum)) {
                grid[row][col] = currentNum;
                if (solveSudoku(grid))
                    return 1;
                grid[row][col] = 'X';
            }
        }
    }
    return 0;
}


int unique(char **grid) {
    for (int row = 0; row < N; row++) {
        int found[16] = {0};
        for (int col = 0; col < N; col++) {
            if (grid[row][col] == 'X') {
                for (int num = 0; num < 16; num++) {
                    if (notes[row][col][num]) {
                        found[num]++;
                    }
                }
            }
        }
        for (int num = 0; num < 16; num++) {
            if (found[num] == 1) {
                for (int col = 0; col < N; col++) {
                    if (grid[row][col] == 'X' && notes[row][col][num]) {
                        grid[row][col] = allowed[num];
                        for (int r = 0; r < N; r++) {
                            if (grid[r][col] == 'X')
                                notes[r][col][num] = 0;
                        }
                        return 1;
                    }
                }
            }
        }
    }

    for (int col = 0; col < N; col++) {
        int found[16] = {0};
        for (int row = 0; row < N; row++) {
            if (grid[row][col] == 'X') {
                for (int num = 0; num < 16; num++) {
                    if (notes[row][col][num]) {
                        found[num]++;
                    }
                }
            }
        }
        for (int num = 0; num < 16; num++) {
            if (found[num] == 1) {
                for (int row = 0; row < N; row++) {
                    if (grid[row][col] == 'X' && notes[row][col][num]) {
                        grid[row][col] = allowed[num];
                        for (int c = 0; c < N; c++) {
                            if (grid[row][c] == 'X')
                                notes[row][c][num] = 0;
                        }
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}


int singles(char **grid) {
    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            if (grid[row][col] == 'X') {
                int count = 0, lastNum = 0;
                for (int num = 0; num < 16; num++) {
                    if (notes[row][col][num]) {
                        count++;
                        lastNum = num;
                    }
                }
                if (count == 1) {
                    char num = allowed[lastNum];
                    grid[row][col] = num;

                    for (int r = 0; r < N; r++) {
                        if (grid[r][col] == 'X')
                            notes[r][col][lastNum] = 0;
                    }

                    for (int c = 0; c < N; c++) {
                        if (grid[row][c] == 'X')
                            notes[row][c][lastNum] = 0;
                    }
                    return 1;
                }
            }
        }
    }
    return 0;
}


int suducul(char** grid)
{
    dispBoard(grid);

    
    buildNotes(grid);

    
    while (unique(grid)) {
        while (singles(grid));
    }

    
    if (solveSudoku(grid)) {
        printf("Solved\n");
        dispBoard(grid);
    }
        
    else {
        printf("No Solution\n");
    }

    return 0;
}