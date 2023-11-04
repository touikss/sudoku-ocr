#ifndef SOLVER_H
#define SOLVER_H

int** read_sudoku_from_file(const char* filename);
void print_grid(int** grid);
int is_valid(int** grid, int row, int col, int num);
int solve_sudoku(int** grid, int row, int col);
int Solver(char* file_name);

#endif
