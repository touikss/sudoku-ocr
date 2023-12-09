#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <err.h>
#include "../headers/solver_hexa.h"

#define N 16

// Dimension of window
//const int SCREEN_WIDTH_HEXA = 640;
//const int SCREEN_HEIGHT_HEXA = 640;

// Dimension of the grid
//const int CELL_SIZE_HEXA = SCREEN_WIDTH_HEXA / N;

//char allowed[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
//char notes[N][N][16] = {0};

// Dimension of window
const int SCREEN_WIDTH_HEXA = 640;
const int SCREEN_HEIGHT_HEXA = 640;

// Dimension of the grid
const int CELL_SIZE_HEXA = SCREEN_WIDTH_HEXA / N;

char allowed[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
char notes[N][N][16] = {0};

// Function to read the Sudoku grid from a file
char** getFromFile(char* file) {
    char** grid = malloc(16 * sizeof(char*)); // Allocate memory for rows
    for (int i = 0; i < 16; i++) {
        grid[i] = malloc(16); // Allocate memory for columns
    }
    FILE *fp = fopen(file, "r");

    if (fp == NULL) {
        errx(1, "Error opening the file.\n");
    }

    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            char currentChar;

            // Skip spaces and newline characters
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

// Function to check if a number is allowed in a column
int allowedCol(char** grid, int col, char num) {
    for (int row = 0; row < N; row++)
        if (grid[row][col] == num)
            return 1;
    return 0;
}

// Function to check if a number is allowed in a row
int allowedRow(char** grid, int row, char num) {
    for (int col = 0; col < N; col++)
        if (grid[row][col] == num)
            return 1;
    return 0;
}

// Function to check if a number is allowed in a 4x4 box
int allowedBox(char** grid, int boxRow, int boxCol, char num) {
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            if (grid[row + boxRow][col + boxCol] == num)
                return 1;
    return 0;
}

// Function to find an empty cell in the grid
int isEmpty(char** grid, int *row, int *col) {
    for (*row = 0; *row < N; (*row)++)
        for (*col = 0; *col < N; (*col)++)
            if (grid[*row][*col] == 'X')
                return 1;
    return 0;
}

// Function to check if a number is valid in a cell
int checkValid(char** grid, int row, int col, char num) {
    return !allowedBox(grid, row - row % 4, col - col % 4, num) && !allowedCol(grid, col, num) && !allowedRow(grid, row, num);
}

// Function to display the Sudoku grid in the console
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

// Function to initialize notes array
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

// Function to solve the Sudoku
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

// Function to find unique values in a row or column
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

// Function to fill in cells with only one possibility
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

/*
int main(int argc, char** argv)
{
    // Check if the program is executed with the correct number of arguments
    if (argc < 2) {
        printf("You must use this program with at least one argument");
        return 1;
    }

    // Display the initial Sudoku problem
    printf("Here Is The Sudoku Problem:\n");
    char** grid = getFromFile(argv[1]);
    dispBoard(grid);

    // Build notes for possible values in empty cells
    buildNotes(grid);

    // Use techniques to solve Sudoku, such as elimination of unique possibilities and singles
    while (unique(grid)) {
        while (singles(grid));
    }

    // If Sudoku is solved without displaying the image, print the solution
    if (solveSudoku(grid)) {
        printf("Solved\n");
        dispBoard(grid);
    }
        // If Sudoku has no solution
    else {
        printf("No Solution\n");
    }

    return 0;
}*/