#include "../headers/canny.h"
#include "../headers/image_manipulation.h"
#include "../headers/error.h"
#include "../headers/nn.h"

#include <dirent.h>
#include <libgen.h>
#include <time.h>
#include <sys/time.h>
#include <err.h>

char* images_dir_path = "images_test"; // Where images ar
char* input_filename = ""; // If an input is given its here

/*Print in the console the name of regular files in the directory at the path.*/
void PrintFilesInDirectory(char* path)
{
    DIR *dir;
    struct dirent *input;
    // Open directory
    dir = opendir(path);
    // Check errors
    if (dir == NULL)
        errx(EXIT_FAILURE, "Unable to open directory %s\n", path);

    unsigned int count = 0; // To count files
    // Read files names
    while ((input = readdir(dir)) != NULL) {
        if (input->d_type == DT_REG) { // Check if input is a regular file
            printf("%u : %s\n", count, input->d_name); // Print it
            count ++;
        }
    }
    // Close directory
    closedir(dir);
}

/*Return the name of the image at index in directory at the path*/
char* FilePathAtIndex(unsigned int index, char* path)
{
    if (path == NULL)
        errx(EXIT_FAILURE, "NULL parameter in FilePathAtIndex.\n");
    // Extract the parent directory name from the given path
    char *parent_directory = basename(strdup(path));
    char *res = NULL;

    DIR *dir;
    struct dirent *input;
    // Open the directory indicated by the path
    dir = opendir(path);

    if (dir == NULL)
        errx(EXIT_FAILURE, "Unable to open directory: %s\n", path);

    unsigned int count = 0;
    // Read file names in the directory
    while ((input = readdir(dir)) != NULL) {
        if (input->d_type == DT_REG) { // Check if the current item is a regular file
            if (count == index) {// If the current item matches the specified index
                // Allocate memory for the path of the image to open
                res = (char *)malloc(strlen(parent_directory) + strlen(input->d_name) + 2);

                if (res == NULL) {
                    closedir(dir);
                    errx(EXIT_FAILURE, "Error while allocating memory in FilePathAtIndex()\n");
                }

                // Build path and return
                sprintf(res, "%s/%s", parent_directory, input->d_name);
                closedir(dir);
                free(parent_directory);
                return res;
            }
            count ++;
        }
    }

    // Close directory
    closedir(dir);
    errx(EXIT_FAILURE, "Unable to find a file at this index : %d\n", index);
    return NULL;
}

/*Show possible choice and return the path of chosen image
 * return NULL if an error occured*/
char* FileUserChoice(char* dir_path){

    if (dir_path == NULL)
        errx(EXIT_FAILURE, "NULL parameter in FileUserChoice().");

    // Take the user choice
    unsigned int index;
    printf("Choose a file with index : ");
    scanf("%d", &index);

    // Build file path
    char* chosen_file_path = FilePathAtIndex(index, dir_path);
    
    printf("Chosen file is : %s\n\n", chosen_file_path);
    return chosen_file_path;
}


/*Manage event loop.*/
void EventLoop(SDL_Renderer *renderer, SDL_Surface *image)
{
    if (renderer == NULL || image == NULL)
        errx(EXIT_FAILURE, "NULL parameter(s) in EventLoop.\n");

    // Variable to get the events.
    SDL_Event event;

    SDL_Surface* image_from_canny = ApplyCannyFilter(image);


    int mat[16][16];
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            mat[i][j] = -1;
        }
    }

    perform_nn(0, mat);

    for (int i = 0; i < 16; i ++){
        for (int j = 0; j < 16; j ++){
            if (mat[i][j] < 0)
                printf("  |");
            else
                printf("%2d|", mat[i][j]);
        }
        printf("\n");
    }
    
    if (image_from_canny == NULL)
        Error_SDL("No result from canny in EventLoop()");

    // Creating texture from "image"
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, image_from_canny);

    if (texture == NULL)
        Error_SDL("Unable to create texture in EventLoop()");
    // Clear screen and display texture
    if (SDL_RenderClear(renderer) != 0)
        Error_SDL("Unable to clear renderer in EventLoop()");
    if (SDL_RenderCopy(renderer, texture, NULL, NULL) != 0)
        Error_SDL("Unable to copy renderer in EventLoop()");
    SDL_RenderPresent(renderer);

    // Destroy texture
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(image_from_canny);

    while (1)
    {
        // Waits for an event.
        if (SDL_WaitEvent(&event) != 1)
            Error_SDL("Error while waiting for events in EventLoop()");

        if(event.type == SDL_QUIT)
            break;
    }
}


/*Initialize SDL window, renderer and load images from file*/
void InitSdlAndLoadImage(SDL_Surface **image, SDL_Surface **image32, SDL_Renderer **renderer, SDL_Window **window){

    // SDL2 initialization
    if (SDL_Init(SDL_INIT_VIDEO) != 0) 
        Error_SDL("SDL initialization error in InitSdlAndLoadImage()");

    // List images in directory and take user choice
    PrintFilesInDirectory(images_dir_path);

    input_filename = FileUserChoice(images_dir_path);

    // Loading image from path
    *image = IMG_Load(input_filename);
    if (*image == NULL) 
        Error_SDL("Error while loading image in InitSdlAndLoadImage()");

    // 32 bits conversion
    *image32 = SDL_ConvertSurfaceFormat(*image, SDL_PIXELFORMAT_RGBA8888, 0);
    SDL_FreeSurface(*image);
    if (*image32 == NULL) 
        Error_SDL("Error while converting image to 32 bit format in InitSdlAndLoadImage()");

    // Get screen size
    SDL_DisplayMode DM;
    if (SDL_GetDesktopDisplayMode(0, &DM) != 0)
        Error_SDL("Error while taking screen size in InitSdlAndLoadImage()");


    // Resize
    *image32 = ResizeSurface(*image32, 1200, 880);
    if (*image32 == NULL) 
        Error_SDL("Error while resizing image in InitSdlAndLoadImage()");

    // Creating window
    *window = SDL_CreateWindow("Hough", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (*image32)->w, (*image32)->h, SDL_WINDOW_SHOWN);
    if (*window == NULL)
        Error_SDL("Window creation error in InitSdlAndLoadImage()");

    // Creating renderer
    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (*renderer == NULL)
        Error_SDL("Renderer creation error in InitSdlAndLoadImage()");
}

/*QUit SDL and free ressources allocated*/
void QuitSdlAndFreeResources(SDL_Surface **image32, SDL_Renderer **renderer, SDL_Window **window) {
    if (renderer != NULL && *renderer != NULL)
        SDL_DestroyRenderer(*renderer);
    if (window != NULL && *window != NULL)
        SDL_DestroyWindow(*window);
    if (image32 != NULL && *image32 != NULL)
        SDL_FreeSurface(*image32);
    SDL_Quit();
}

int main()
{
    int status = EXIT_SUCCESS;
    // Initialize pointers
    SDL_Surface *image = NULL;
    SDL_Surface *image32 = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Window *window = NULL;

    InitSdlAndLoadImage(&image, &image32, &renderer, &window);
    EventLoop(renderer, image32);
    QuitSdlAndFreeResources(&image32, &renderer, &window);

    return status;
}
