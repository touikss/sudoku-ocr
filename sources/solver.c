
#include "../headers/solver.h"

#include <stdio.h>
#include <stdlib.h>


const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 640;


const int GRID_SIZE = 9;
const int CELL_SIZE = SCREEN_WIDTH / GRID_SIZE;


int** read_sudoku_from_file(const char* filename) {

    
    int** sudoku = malloc(9 * sizeof(int*)); 
    for (int i = 0; i < 9; i++) {
        sudoku[i] = malloc(9 * sizeof(int)); 
    }

    
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file"); 
        exit(1); 
    }

    
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {

	    
	    
            char cell[4];

            if (fscanf(file, " %c", cell) != 1) { 
                fclose(file);
                fprintf(stderr, "Error reading from file.\n"); 
                exit(1); 
            }

            if (cell[0] == '.') {
                sudoku[i][j] = 0; 
            } 
	    
	    else {
                sudoku[i][j] = atoi(cell); 
            }
        }
    }

    fclose(file); 
    return sudoku; 
}



void print_grid(int** grid) {
    int count = 0;
    int i = 0;
    int j = 0;
    while(i < 9) {
        while(j < 9) {
            
            if (j % 3 == 0)
                printf(" ");

            
            printf("%d", grid[i][j]);
        j += 1;
        }

        
        
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





int is_valid(int** grid, int row, int col, int num) {
    int res = 1; 

    
    int i = 0;
    while (i < 9 && res) {
        if (grid[row][i] == num)
            res = 0; 
        i++;
    }

    
    i = 0;
    while (i < 9 && res) {
        if (grid[i][col] == num)
            res = 0; 
        i++;
    }

    
    int subGrid_col = col - col % 3; 
    int subGrid_row = row - row % 3; 
    i = 0;
    int j = 0;
    while (i < 3 && res) {
        while (j < 3 && res) {
            if (grid[subGrid_row + i][subGrid_col + j] == num)
                res = 0; 
            j++;
        }
    j = 0;
        i++;
    }

    
    return res;
}






int solve_sudoku(int** grid, int row, int col) {
    
    
    if (row == 8 && col == 9)
        return 1;

    
    if (col == 9) {
        row++;
        col = 0;
    }

    
    if (grid[row][col] > 0)
        return solve_sudoku(grid, row, col + 1);

    
    for (int i = 1; i <= 9; i++) {
        if (is_valid(grid, row, col, i) == 1) {
            grid[row][col] = i; 

            
            if (solve_sudoku(grid, row, col + 1) == 1)
                return 1; 

            grid[row][col] = 0; 
        }
    }

    return 0; 
}

/*SDL_Surface* create_sudoku_surface(int** sudoku_grid, int size) {
    int SCREEN_SIZE = 200;
    SDL_Surface* surface = NULL;

    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL n'a pas pu être initialisé. SDL_Error: %s\n", SDL_GetError());
        return NULL;
    }

    
    surface = SDL_CreateRGBSurface(0, SCREEN_SIZE, SCREEN_SIZE, 32, 0, 0, 0, 0);
    if (surface == NULL) {
        printf("La surface SDL n'a pas pu être créée. SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return NULL;
    }

    
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 255, 255));  

    int cell_size = SCREEN_SIZE / size;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            SDL_Rect cell_rect = {j * cell_size, i * cell_size, cell_size, cell_size};
            SDL_Color color = {255, 0, 0, 255};  
            
            if (sudoku_grid[i][j] != 0) {
                
                char text[2];
                snprintf(text, sizeof(text), "%d", sudoku_grid[i][j]);
                SDL_Surface* text_surface = TTF_RenderText_Solid(font, text, color);
                SDL_BlitSurface(text_surface, NULL, surface, &cell_rect);
                SDL_FreeSurface(text_surface);
            } else {
                
                SDL_FillRect(surface, &cell_rect, SDL_MapRGB(surface->format, 255, 255, 255));
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &cell_rect);
            }
        }
    }

    return surface;
}*/

void displaySudoku(SDL_Renderer* renderer, int grid[GRID_SIZE][GRID_SIZE], TTF_Font* font) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); 
    SDL_RenderClear(renderer);

    int count = 0;
    
    for (int i = 0; i <= GRID_SIZE; ++i) {
        int x = i * CELL_SIZE;
        int y = i * CELL_SIZE;

        if(count == 3){  
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

    
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int col = 0; col < GRID_SIZE; ++col) {
            int number = grid[row][col];
            if (number != 0) {
                
                int textX = col * CELL_SIZE + CELL_SIZE / 4;
                int textY = row * CELL_SIZE + CELL_SIZE / 4;

                
                char text[2];
                snprintf(text, 2, "%d", number);

                SDL_Color textColor = {0, 0, 0, 255};
                SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, textColor);
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

                
                SDL_Rect destRect = { textX, textY, CELL_SIZE / 2, CELL_SIZE / 2 };
                SDL_RenderCopy(renderer, textTexture, NULL, &destRect);

                
                SDL_FreeSurface(textSurface);
                SDL_DestroyTexture(textTexture);
            }
        }
    }

    SDL_RenderPresent(renderer);
}




int Solver(char* file_name, int display) {

    
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

        
        for (int row = 0; row < GRID_SIZE; ++row) {
            for (int col = 0; col < GRID_SIZE; ++col) {
                sudokuGrid[row][col] = sudoku[row][col]; 
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
        
    else if (solve_sudoku(sudoku, 0, 0) == 1){
        print_grid(sudoku); 
    }

    else{
        printf("No solution exists\n"); 
    }

    
    for (int i = 0; i < 9; i++) {
        free(sudoku[i]);
    }
    free(sudoku);

    return 0; 
}

