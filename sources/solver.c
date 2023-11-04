#include <stdio.h>
#include <stdlib.h>


// This function transforms the given sudoku file into an int matrix
int** read_sudoku_from_file(const char* filename) {

    // First, we need to allocate memory for the final matrix 
    int** sudoku = malloc(9 * sizeof(int*)); // Allocate memory for rows
    for (int i = 0; i < 9; i++) {
        sudoku[i] = malloc(9 * sizeof(int)); // Allocate memory for columns
    }

    // Open the given file (return an error message if the file is empty)
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file"); // Print an error message if opening the file fails
        exit(1); // Exit the program with an error code
    }

    // Read the Sudoku data from the file and store it in the matrix
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {

	    // Declare a character array (string) named 'cell' with a size of 4 characters.
	    // This array is used to temporarily store characters read from the input file.
            char cell[4];

            if (fscanf(file, " %c", cell) != 1) { // Read a character from the file
                fclose(file);
                fprintf(stderr, "Error reading from file.\n"); // Print an error message if reading fails
                exit(1); // Exit the program with an error code
            }

            if (cell[0] == '.') {
                sudoku[i][j] = 0; // Use 0 to represent empty cells
            } 
	    
	    else {
                sudoku[i][j] = atoi(cell); // Convert the string to an integer
            }
        }
    }

    fclose(file); // Close the file after reading
    return sudoku; // Return the Sudoku matrix
}


// This function is responsible for printing a Sudoku grid.
void print_grid(int** grid) {
    int count = 0;
    int i = 0;
    int j = 0;
    while(i < 9) {
        while(j < 9) {
            // Add a space before each group of 3 columns for better readability.
            if (j % 3 == 0)
                printf(" ");

            // Print the value of the cell in the Sudoku grid.
            printf("%d", grid[i][j]);
	    j += 1;
        }

        // Add a newline after every row. Additionally, add an extra newline after every third row
        // to separate the grid into 3x3 subgrids for better visualization.
	count += 1;
        if (count == 3 && i < 8){
            printf("\n");
	    count = 0;
	}

        printf("\n");
	i += 1;
	j = 0;
    }
}


// This function verifies if a given number can be added to the Sudoku grid at a specified position.
// It checks if the number is not already present in the same row, column, or 3x3 grid.
// This function returns 1 if the number is valid to be added, and 0 otherwise.
int is_valid(int** grid, int row, int col, int num) {
    int res = 1; // Initialize the result to assume the number is valid.

    // Check if the number is already present in the same row.
    int i = 0;
    while (i < 9 && res) {
        if (grid[row][i] == num)
            res = 0; // Set the result to 0 if the number is found in the row.
        i++;
    }

    // Check if the number is already present in the same column.
    i = 0;
    while (i < 9 && res) {
        if (grid[i][col] == num)
            res = 0; // Set the result to 0 if the number is found in the column.
        i++;
    }

    // Check if the number is already present in the same 3x3 grid.
    int subGrid_col = col - col % 3; // Calculate the starting column of the 3x3 subgrid.
    int subGrid_row = row - row % 3; // Calculate the starting row of the 3x3 subgrid.
    i = 0;
    int j = 0;
    while (i < 3 && res) {
        while (j < 3 && res) {
            if (grid[subGrid_row + i][subGrid_col + j] == num)
                res = 0; // Set the result to 0 if the number is found in the 3x3 subgrid.
            j++;
        }
	j = 0;
        i++;
    }

    // Return the final result, which indicates whether the number is valid to be added.
    return res;
}



// This function attempts to solve a Sudoku puzzle using a recursive backtracking algorithm.
// It starts at the specified row and column and recursively fills in the grid's cells.
// Returns 1 if a solution is found, 0 otherwise.
int solve_sudoku(int** grid, int row, int col) {
    // Base case: If we have reached the end of the grid (row 8, column 9), a solution is found.
    if (row == 8 && col == 9)
        return 1;

    // If we have reached the end of a row, move to the next row and start from the beginning of that row.
    if (col == 9) {
        row++;
        col = 0;
    }

    // If the current cell already contains a number greater than 0, move to the next cell.
    if (grid[row][col] > 0)
        return solve_sudoku(grid, row, col + 1);

    // Try to place numbers from 1 to 9 in the current cell and recursively solve the puzzle.
    for (int i = 1; i <= 9; i++) {
        if (is_valid(grid, row, col, i) == 1) {
            grid[row][col] = i; // Place the number in the current cell.

            // Recursively attempt to solve the puzzle starting from the next cell.
            if (solve_sudoku(grid, row, col + 1) == 1)
                return 1; // If a solution is found, return 1.

            grid[row][col] = 0; // Backtrack if the assumption was wrong by resetting the cell to 0.
        }
    }

    return 0; // If no valid solution is found, return 0.
}


// Main function that serves as the entry point of the Sudoku solver program.
int Solver(char* file_name) {

    // Read the Sudoku grid from the specified file and store it in a matrix.
    int** sudoku = read_sudoku_from_file(file_name);

    // Attempt to solve the Sudoku puzzle starting from the top-left corner (0, 0).
    if (solve_sudoku(sudoku, 0, 0) == 1){
        print_grid(sudoku); // If a solution is found, print the solved grid.
    }

    else{
        printf("No solution exists\n"); // If no solution is found, print a message.
    }

    // Free the memory allocated for the Sudoku matrix.
    for (int i = 0; i < 9; i++) {
        free(sudoku[i]);
    }
    free(sudoku);

    return EXIT_SUCCESS; // Exit the program with a success code (0).
}

