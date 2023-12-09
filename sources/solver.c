
#include "../headers/solver.h"

#include <stdio.h>
#include <stdlib.h>

// Dimention of window
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 640;

// Dimension of the grid
const int GRID_SIZE = 9;
const int CELL_SIZE = SCREEN_WIDTH / GRID_SIZE;

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
    print_grid(grid);
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

/*SDL_Surface* create_sudoku_surface(int** sudoku_grid, int size) {
    int SCREEN_SIZE = 200;
    SDL_Surface* surface = NULL;

    // Initialisation de SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL n'a pas pu être initialisé. SDL_Error: %s\n", SDL_GetError());
        return NULL;
    }

    // Création de la surface
    surface = SDL_CreateRGBSurface(0, SCREEN_SIZE, SCREEN_SIZE, 32, 0, 0, 0, 0);
    if (surface == NULL) {
        printf("La surface SDL n'a pas pu être créée. SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return NULL;
    }

    // Dessin de la grille de sudoku
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 255, 255));  // Fond blanc

    int cell_size = SCREEN_SIZE / size;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            SDL_Rect cell_rect = {j * cell_size, i * cell_size, cell_size, cell_size};
            SDL_Color color = {255, 0, 0, 255};  // Couleur par défaut (rouge)
            
            if (sudoku_grid[i][j] != 0) {
                // Si la valeur de la case est non nulle, dessiner le nombre dans la case
                char text[2];
                snprintf(text, sizeof(text), "%d", sudoku_grid[i][j]);
                SDL_Surface* text_surface = TTF_RenderText_Solid(font, text, color);
                SDL_BlitSurface(text_surface, NULL, surface, &cell_rect);
                SDL_FreeSurface(text_surface);
            } else {
                // Si la valeur de la case est nulle, dessiner une cellule vide
                SDL_FillRect(surface, &cell_rect, SDL_MapRGB(surface->format, 255, 255, 255));
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &cell_rect);
            }
        }
    }

    return surface;
}*/
// This function display the grid in an image
void displaySudoku(SDL_Renderer* renderer, int grid[GRID_SIZE][GRID_SIZE], TTF_Font* font) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Couleur de fond blanche
    SDL_RenderClear(renderer);

    int count = 0;
    // Draw the grid
    for (int i = 0; i <= GRID_SIZE; ++i) {
        int x = i * CELL_SIZE;
        int y = i * CELL_SIZE;

        if(count == 3){  //Change the thickness if the line is one of the big square that contains 9 squares
            SDL_Rect horizonLine = {0, y, SCREEN_WIDTH, 4};
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &horizonLine);

            SDL_Rect vertLine = {x, 0, 4, SCREEN_HEIGHT};
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &vertLine);
            count = 0;
        }

        else{
            SDL_Rect horizonLine = {0, y, SCREEN_WIDTH, 2};
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &horizonLine);

            SDL_Rect vertLine = {x, 0, 2, SCREEN_HEIGHT};
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &vertLine);
        }

        count++;
    }

    // Draw the digits in the cases
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int col = 0; col < GRID_SIZE; ++col) {
            int number = grid[row][col];
            if (number != 0) {
                // Place the text in the middle of the case
                int textX = col * CELL_SIZE + CELL_SIZE / 4;
                int textY = row * CELL_SIZE + CELL_SIZE / 4;

                // Create SDL renderer with the digit
                char text[2];
                snprintf(text, 2, "%d", number);

                SDL_Color textColor = {0, 0, 0, 255};
                SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, textColor);
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

                // Draw the text
                SDL_Rect destRect = { textX, textY, CELL_SIZE / 2, CELL_SIZE / 2 };
                SDL_RenderCopy(renderer, textTexture, NULL, &destRect);

                // Free memory
                SDL_FreeSurface(textSurface);
                SDL_DestroyTexture(textTexture);
            }
        }
    }

    SDL_RenderPresent(renderer);
}



// Main function that serves as the entry point of the Sudoku solver program.
int Solver(char* file_name, int display) {

    // Read the Sudoku grid from the specified file and store it in a matrix.
    int** sudoku = read_sudoku_from_file(file_name);


    if(solve_sudoku(sudoku, 0, 0) == 1 && display){

        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
            return 1;
        }

        if (TTF_Init() != 0) {
            fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
            SDL_Quit();
            return 1;
        }

        SDL_Window* window = SDL_CreateWindow("Sudoku Solver", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        // Charger la police pour afficher les chiffres
        TTF_Font* font = TTF_OpenFont("res/font.ttf", 24);

        if (font == NULL) {
            fprintf(stderr, "TTF_OpenFont failed: %s\n", TTF_GetError());
            TTF_Quit();
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }

        int sudokuGrid[GRID_SIZE][GRID_SIZE];

        // Initialisation de la grille de Sudoku
        for (int row = 0; row < GRID_SIZE; ++row) {
            for (int col = 0; col < GRID_SIZE; ++col) {
                sudokuGrid[row][col] = sudoku[row][col]; // Vous pouvez mettre les chiffres initiau
            }
        }

        int quit = 0;
        SDL_Event event;

        while (!quit) {
            while (SDL_PollEvent(&event) != 0) {
                if (event.type == SDL_QUIT) {
                    quit = 1;
                }
            }

            displaySudoku(renderer, sudokuGrid, font);
        }

        TTF_CloseFont(font);
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
        // Attempt to solve the Sudoku puzzle starting from the top-left corner (0, 0).
    else if (solve_sudoku(sudoku, 0, 0) == 1){
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

    return 0; // Exit the program with a success code (0).
}

