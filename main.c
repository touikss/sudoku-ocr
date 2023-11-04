#include <SDL2/SDL.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>
#include <string.h>
#include <dirent.h>
#include <libgen.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "imageManipulation.h"

#define OUTPUT_GIVEN (1 << 0)  // 0001
#define INPUT_GIVEN (1 << 1)  // 0010
#define STEP_BY_STEP (1 << 2)  // 0100
#define PRINT_IMAGE_INFO (1 << 3)  // 1000
#define TEST_MODE (1<<4)
#define DISPLAY_TIME (1<<5)


char* images_dir_path = "/SudokuSolver/images_test";
char* input_filename = "";
char* output_filename = "";

// Low and high threshold for gradients.
// It can change a lot to the final result of the image.
float low_threshold = 50.0;
float high_threshold = 80.0;

// Straightness of Gaussian blur
int blur_straight = 1;
unsigned int flags = 0;

/*
    Print some useful informations about "image" and its format on the output stream.
*/
void PrintImageInformation(SDL_Surface* image)
{
    SDL_PixelFormat* format = image->format;
    printf("Surface format :\n");
    printf("Bits per pixel : %d\n", format->BitsPerPixel);
    printf("Red (mask)   : 0x%.8X\n", format->Rmask);
    printf("Green (mask)    : 0x%.8X\n", format->Gmask);
    printf("Blue (mask)    : 0x%.8X\n", format->Bmask);
    printf("Alpha (mask)   : 0x%.8X\n", format->Amask);
    printf("Bytes per pixel : %d\n", format->BytesPerPixel);
    printf("Format : %u\n", format->format);
    printf("Bytes order: ");
    if (format->Rshift < format->Bshift) {
        printf("ABGR\n");
    } else {
        printf("RGBA\n");
    }
    printf("Dimensions : %d x %d\n\n", image->h, image->w);
}


/*
    Print in the console the name of regular files in the directory at the path.
*/
int PrintFilesInDirectory(char* path)
{
    DIR *dir;
    struct dirent *input;

    // Open directory
    dir = opendir(path);

    if (dir == NULL) {
        fprintf(stderr,"Unable to open directory %s\n", path);
        return EXIT_FAILURE;
    }

    unsigned int count = 0;
    // Read files names
    while ((input = readdir(dir)) != NULL) {
        if (input->d_type == DT_REG) { // Check if input is a regular file
            printf("%u : %s\n", count, input->d_name);
            count ++;
        }
    }

    // Close directory
    closedir(dir);

    return EXIT_SUCCESS;
}


/*
    Return the name of the image at index in directory at the path
*/
char* ImagePathAtIndex(unsigned int index, char* path)
{
    char *parent_directory = basename(strdup(path));
    char *res = NULL;

    DIR *dir;
    struct dirent *input;

    // Open directory
    dir = opendir(path);

    if (dir == NULL) {
        perror("Unable to open directory");
        return NULL;
    }

    unsigned int count = 0;

    // Read files names
    while ((input = readdir(dir)) != NULL) {
        if (input->d_type == DT_REG) { //   Check if input is a regular file
            if (count == index)
            {
                // Allocate memory for path of the image to open
                res = (char *)malloc(strlen(parent_directory) + strlen(input->d_name) + 2);

                if (res == NULL) {
                    fprintf(stderr, "Error while allocating memory in ImagePathAtIndex()\n");
                    closedir(dir);
                    return NULL;
                }

                // Build path and return
                sprintf(res, "%s/%s", parent_directory, input->d_name);
                closedir(dir);
                return res;
            }
            count ++;
        }
    }

    // Close directory
    closedir(dir);

    fprintf(stderr, "Unable to find an image at this index : %d", index);
    return NULL;
}

/*
 * Show possible choice and return the path of chosen image
 * return NULL if an error occured
 */
char* ImageUserChoice(){
    // Take the user choice
    unsigned int index;
    printf("Choose an image with index : ");
    scanf("%d", &index);

    char* chosen_image_path = ImagePathAtIndex(index, images_dir_path);

    if (!chosen_image_path){
        return NULL;
    }

    printf("Chosen image is : %s\n\n", chosen_image_path);
    return chosen_image_path;
}

/*
    Perform next step on image.
    return 1 if an error occurred.
    0 otherwise
*/
int PerformNextStep(int n, SDL_Surface **image, Gradient **gradients)
{
    switch (n) {
        case 0:
            printf("Step 0 : display image\n\n");
            break;
        case 1:
            printf("Step 1 : apply gray filter\n\n");
            *image = ConvertSurfaceToGrayScale(*image);
            break;
        case 2:
            printf("Step 2 : apply gaussian blur\n\n");
            *image = ConvertSurfaceToGaussianBlur(*image, blur_straight);
            break;
        case 3:
            printf("Step 3 : compute gradients and display dX\n\n");
            if (CalculateGradients(*image, *gradients) != 0)
                *image = NULL;
            else
                *image = ConvertGradientsToSurface(*gradients, (*image)->h, (*image)->w, DX_MASK);
            break;
        case 4:
            printf("Step 4 : display dY\n\n");
            *image = ConvertGradientsToSurface(*gradients, (*image)->h, (*image)->w, DY_MASK);
            break;
        case 5:
            printf("Step 5 : display gradient orientation\n\n");
            *image = ConvertGradientsToSurface(*gradients, (*image)->h, (*image)->w, ORIENTATION_MASK);
            break;
        case 6:
            printf("Step 6 : display gradient magnitude\n\n");
            *image = ConvertGradientsToSurface(*gradients, (*image)->h, (*image)->w, MAGNITUDE_MASK);
            break;
        case 7:
            printf("Step 7 : apply non maximum supression\n\n");
            if (DeleteNonMaximum(*gradients, (*image)->w, (*image)->h) != 0)
                *image = NULL;
            else
                *image = ConvertGradientsToSurface(*gradients, (*image)->h, (*image)->w, MAGNITUDE_MASK);
            break;
        case 8:
            printf("Step 8 : apply gradient treshold\n\n");
            if (ApplyGradientThreshold(*gradients, (*image)->h, (*image)->w, low_threshold, high_threshold) != 0)
                *image = NULL;
            else
                *image = ConvertGradientsToSurface(*gradients, (*image)->h, (*image)->w, STRENGTH_MASK);
            break;
        case 9:
            printf("Step 9 : apply histeresis contour tracking\n\n");
            if(TrackHysteresisContour(*gradients, (*image)->h, (*image)->w) != 0)
                *image = NULL;
            else
                *image = ConvertGradientsToSurface(*gradients, (*image)->h, (*image)->w, STRENGTH_MASK);
            break;
    }

    if (*image == NULL)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

/*
    Manage event loop.
*/
int EventLoop(SDL_Renderer *renderer, SDL_Surface *image)
{
    // Variable to get the events.
    SDL_Event event;

    // Step counter
    int step = 0;

    // Variable for keeping gradients between two steps
    Gradient *gradients = (Gradient *)malloc(image->w * image->h * sizeof(Gradient));
    if (gradients == NULL) {
        fprintf(stderr, "Error while allocating memory for gradients (EventLoop)\n");
        return EXIT_FAILURE;
    }

    // If not step by step, only show final result
    if (!(flags & STEP_BY_STEP)){
        image = ApplyCannyFilter(image, blur_straight, low_threshold, high_threshold, STRENGTH_MASK);

        if (!image)
            return EXIT_FAILURE;

        // Creating texture from "image"
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, image);
        if (texture == NULL) {
            printf("Texture creation error : %s\n", SDL_GetError());
            return EXIT_FAILURE;
        }

        // Clear screen and display texture
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        // Destroy texture
        SDL_DestroyTexture(texture);

        while (1)
        {
            // Waits for an event.
            SDL_WaitEvent(&event);
            
            switch (event.type) {
                // If the "quit" button is pushed, ends the event loop.
                case SDL_QUIT:
                    return EXIT_SUCCESS;
            }
        }
    }

    // If step by step
    while (1)
    {
        // Waits for an event.
        SDL_WaitEvent(&event);

        switch (event.type)
        {
            // If the "quit" button is pushed, ends the event loop.
            case SDL_QUIT:
                return EXIT_SUCCESS;
            case SDL_KEYDOWN:
                // Exit after step 10
                if (step == 10)
                    return EXIT_SUCCESS;

                if (PerformNextStep(step, &image, &gradients) != 0) {
                    fprintf(stderr, "Error in step %d\n", step);
                    return EXIT_FAILURE;
                }

                step++;

                // Creating texture from "image"
                SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, image);
                if (texture == NULL) {
                    printf("Texture creation error : %s\n", SDL_GetError());
                    return EXIT_FAILURE;
                }

                // Clear screen and display texture
                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, texture, NULL, NULL);
                SDL_RenderPresent(renderer);

                if (texture != NULL)
                    SDL_DestroyTexture(texture);
        }
    }
}

void DisplayHelpExit(char** argv)
{
    printf("Usage: %s <command> [<options>..]\n\n", argv[0]);
    printf("Commands :\n");
    printf("    run: run the sudoku solver\n");
    printf("    test: test functions (for dev)\n");
    printf("Options:\n");
    printf("    -d: display image information\n");
    printf("    -s: run step by step\n");
    printf("    -t : display processing time\n");
    printf("    -o <filename>: store output image in a file\n");
    printf("    -i <filename>: choose input image from a file\n");
    printf("    -h <value>: set high threshold to value (float)\n");
    printf("    -l <value>: set low threshold to value (float)\n");
}

void TestFunctions(SDL_Surface* image)
{
    double elapsed_cmp = 0.0; // Computing time
    clock_t begin_cmp = clock();

    for(int i = 0; i<image->h; i ++){
        for (int j = 0; j<image->w; j ++){

        }
    }

    clock_t end_cmp = clock();
    elapsed_cmp += (double)(end_cmp - begin_cmp) / CLOCKS_PER_SEC;

    printf("Computing time 1 : %f seconds\n", elapsed_cmp);

    double elapsed_cmp2 = 0.0; // Computing time
    begin_cmp = clock();
    for(int i = 0; i<image->h; i ++){
        for (int j = 0; j<image->w; j ++){

        }
    }

    end_cmp = clock();
    elapsed_cmp2 += (double)(end_cmp - begin_cmp) / CLOCKS_PER_SEC;

    printf("Computing time 1 : %f seconds\n", elapsed_cmp);
    printf("Computing time 2 : %f seconds\n", elapsed_cmp2);
}

int ParseArguments(int argc, char** argv)
{
    // Test arg number
    if (argc < 2)
        return -1;

    int i = 1;
    // Command
    if (strcmp(argv[1], "test") == 0) {
        flags |= TEST_MODE;
        i++;
    }
    else if (strcmp(argv[1], "run") == 0) {
        i++;
    }
    else if (argv[1][0] != '-'){
        fprintf(stderr, "Argument Error: '%s' command unknown\n\n", argv[1]);
        return -1;
    }

    char* options_with_parameter = "oilhb";

    for(; i < argc; i++) {
        // Options
        if (argv[i][0] == '-'){
            // With args
            if (strchr(options_with_parameter, argv[i][1]) && argv[i][2] == '\0'){
                if (argv[i + 1] == NULL) {
                    fprintf(stderr, "Argument Error: '%s' option missing <parameter>\n\n", argv[i]);
                    return -1;
                }
                // Input
                if (argv[i][1] == 'i')
                {
                    flags |= INPUT_GIVEN;
                    input_filename = argv[i+1];
                    i++;
                    continue;
                }
                // Output
                else if (argv[i][1] == 'o') {
                    flags |= OUTPUT_GIVEN;
                    output_filename = argv[i+1];
                    i++;
                    continue;
                }
                else if (argv[i][1] == 'l') {
                    char** end = &argv[i+1];
                    float low_th_tmp = strtof(argv[i+1], end);
                    if (low_th_tmp == 0.0 && *end == argv[i+1]){
                        fprintf(stderr, "Argument Error: '%s' option <parameter> is not a float\n\n", argv[i]);
                        return -1;
                    }
                    low_threshold = low_th_tmp;
                    i++;
                    continue;
                }
                else if (argv[i][1] == 'h') {
                    char** end = &argv[i+1];
                    float high_th_tmp = strtof(argv[i+1], end);
                    if (high_th_tmp == 0.0 && *end == argv[i+1]){
                        fprintf(stderr, "Argument Error: '%s' option <parameter> is not a float\n\n", argv[i]);
                        return -1;
                    }
                    high_threshold = high_th_tmp;
                    i++;
                    continue;
                }
                else if (argv[i][1] == 'b') {
                    char** end = &argv[i+1];
                    float blur = strtol(argv[i+1], end, 10);
                    if (blur == 0 && *end == argv[i+1]){
                        fprintf(stderr, "Argument Error: '%s' option <parameter> is not an integer\n\n", argv[i]);
                        return -1;
                    }
                    blur_straight = blur;
                    i++;
                    continue;
                }
            }

            int j = 1;
            // Simple flags
            while (argv[i][j] != '\0'){
                if (argv[i][j] == 'd')
                    flags |= PRINT_IMAGE_INFO;
                else if (argv[i][j] == 's')
                    flags |= STEP_BY_STEP;
                else if (argv[i][j] == 't')
                    flags |= DISPLAY_TIME;
                else{
                    fprintf(stderr, "Argument Error: '%c' unknown option\n\n", argv[i][j]);
                    return -1;
                }

                j++;
            }

        }
        else {
            fprintf(stderr, "Argument Error: '%s' unknown option\n\n", argv[i]);
            return -1;
        }
    }
    return 0;
}


int main(int argc, char** argv){

    // To compute execution time
    struct timeval begin_abs, end_abs; // Absolute
    long elapsed_abs;
    gettimeofday(&begin_abs, NULL);
    double elapsed_cmp = 0.0; // Computing time
    clock_t begin_cmp = clock();


    int status = EXIT_SUCCESS;

    // Parse command line
    if (ParseArguments(argc, argv) != 0) {
        DisplayHelpExit(argv);
        return EXIT_FAILURE;
    }

    // Initialize pointers
    SDL_Surface *image = NULL;
    SDL_Surface *image32 = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Window *window = NULL;

    // SDL2 initialization
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL initialization error : %s\n", SDL_GetError());
        status = EXIT_FAILURE;
        goto Quit;
    }

    // List images in directory and take user choice
    if (!(flags & INPUT_GIVEN)){
        if (PrintFilesInDirectory(images_dir_path) != 0)
            goto Quit;
        input_filename = ImageUserChoice();
    }


    // Loading image from path
    image = IMG_Load(input_filename);
    if (image == NULL) {
        fprintf(stderr, "Error while loading image : %s\n", SDL_GetError());
        status = EXIT_FAILURE;
        goto Quit;
    }

    // Display image infos
    if (flags & PRINT_IMAGE_INFO)
        PrintImageInformation(image);

    // 32 bits conversion
    image32 = SDL_ConvertSurfaceFormat(image, SDL_PIXELFORMAT_RGBA8888, 0);
    SDL_FreeSurface(image);
    if (image32 == NULL) {
        fprintf(stderr,"Converting image to 32 bits format error : %s\n", SDL_GetError());
        status = EXIT_FAILURE;
        goto Quit;
    }

    // Display image infos
    if (flags & PRINT_IMAGE_INFO)
        PrintImageInformation(image32);

    // Creating window
    window = SDL_CreateWindow("Image manipulation", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, image->w, image->h, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window creation error : %s\n", SDL_GetError());
        status = EXIT_FAILURE;
        goto Quit;
    }

    // Creating renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer creation error : %s\n", SDL_GetError());
        status = EXIT_FAILURE;
        goto Quit;
    }

    if (flags & TEST_MODE) {
        TestFunctions(image32);
        goto Quit;
    }

    // Start handling events
    status = EventLoop(renderer, image32);

Quit:
    if (renderer != NULL)
        SDL_DestroyRenderer(renderer);
    if (window != NULL)
        SDL_DestroyWindow(window);
    if (image32 != NULL)
        SDL_FreeSurface(image32);
    SDL_Quit();

    if (status == 0){
        // Ending time
        gettimeofday(&end_abs, NULL);
        elapsed_abs = (end_abs.tv_sec - begin_abs.tv_sec) * 1000 + (end_abs.tv_usec - begin_abs.tv_usec) / 1000;
        clock_t end_cmp = clock();
        elapsed_cmp += (double)(end_cmp - begin_cmp) / CLOCKS_PER_SEC;

        // Display time if asked
        if (flags & DISPLAY_TIME){
            printf("Computing time : %f seconds\n", elapsed_cmp);
            printf("Absolute time: %ld milliseconds\n", elapsed_abs);
        }
    }
    return status;
}
