#ifndef SOLVER_HEXA_H
#define SOLVER_HEXA_H

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define N 16




char** getFromFile(char* file);
int allowedCol(char** grid, int col, char num);
int allowedRow(char** grid, int row, char num);
int allowedBox(char** grid, int boxRow, int boxCol, char num);
int isEmpty(char** grid, int *row, int *col);
int checkValid(char** grid, int row, int col, char num);
void dispBoard(char** grid);
void buildNotes(char** grid);
int solveSudoku(char** grid);
int unique(char** grid);
int singles(char** grid);
int suducul(char** grid);

#endif