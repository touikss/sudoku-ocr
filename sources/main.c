#include "../headers/canny.h"
#include "../headers/solver.h"
#include "../headers/xor.h"
#include "../headers/hough.h"
#include "../headers/image_manipulation.h"

#include <dirent.h>
#include <libgen.h>
#include <time.h>
#include <sys/time.h>

#define OUTPUT_GIVEN (1 << 0) // An output file is given
#define INPUT_GIVEN (1 << 1) // An input file is given
#define STEP_BY_STEP (1 << 2) // Execute step by step
#define DISPLAY (1 << 3) // Print image information or display sudoku solution in an image
#define PRINT_TIME (1<<5) // Print execution time of the program
#define CANNY_MODE (1<<6) // Run canny filter
#define SOLVER_MODE (1<<7) // Run solver
#define XOR_MODE (1<<8) // Run xor network
#define HELP (1<<9) // Display help
#define HOUGH_MODE (1<<10) // Run hough
#define RESIZE_IMAGE (1<<11) // Resize input image
#define GRID_MODE (1<<12) // Resize input image
#define ROTATE_MODE (1<<13) // Rotate an image

char* images_dir_path = "images_test"; // Where images ar
char* grid_dir_path = "grids"; // Where grids are
char* input_filename = ""; // If an input is given its here
char* output_filename = "output/image.bmp"; // Same

int blur_straight = 1; // Straightness of Gaussian blur
float low_threshold = 50.0; // Low threshold for canny filter
float high_threshold = 80.0; // High threshold

int nb_iteration_xor = 10; // Number of xor iteration (*10000)
int display_step_xor = 10; // Number of xor displaying pas (*1000)

int hough_threshold = 250; // Threshold for hough

unsigned int flags = 0; // Stores flags

double angle = 45.0;

/*Print in the console the name of regular files in the directory at the path.*/
int PrintFilesInDirectory(char* path)
{
    DIR *dir;
    struct dirent *input;
    // Open directory
    dir = opendir(path);
    // Check errors
    if (dir == NULL) {
        fprintf(stderr,"Unable to open directory %s\n", path);
        return EXIT_FAILURE;
    }

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
    return EXIT_SUCCESS;
}

/*Return the name of the image at index in directory at the path*/
char* FilePathAtIndex(unsigned int index, char* path)
{
    // Extract the parent directory name from the given path
    char *parent_directory = basename(strdup(path));
    char *res = NULL;

    DIR *dir;
    struct dirent *input;
    // Open the directory indicated by the path
    dir = opendir(path);

    if (dir == NULL) {
        perror("Unable to open directory");
        return NULL;
    }

    unsigned int count = 0;
    // Read file names in the directory
    while ((input = readdir(dir)) != NULL) {
        if (input->d_type == DT_REG) { // Check if the current item is a regular file
            if (count == index) {// If the current item matches the specified index
                // Allocate memory for the path of the image to open
                res = (char *)malloc(strlen(parent_directory) + strlen(input->d_name) + 2);

                if (res == NULL) {
                    fprintf(stderr, "Error while allocating memory in FilePathAtIndex()\n");
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

    fprintf(stderr, "Unable to find a file at this index : %d\n", index);
    return NULL;
}

/*Show possible choice and return the path of chosen image
 * return NULL if an error occured*/
char* FileUserChoice(char* dir_path){
    // Take the user choice
    unsigned int index;
    printf("Choose a file with index : ");
    scanf("%d", &index);
    // Build file path
    char* chosen_file_path = FilePathAtIndex(index, dir_path);

    if (!chosen_file_path){
        return NULL;
    }
    
    printf("Chosen file is : %s\n\n", chosen_file_path);
    return chosen_file_path;
}

/*Perform next step on image.
 * return 1 if an error occurred.
 * 0 otherwise*/
int PerformNextStep(int n, SDL_Surface **image, SDL_Surface **image_from_canny, SDL_Surface** original_image,
                    Gradient **gradients, SDL_Renderer **renderer)
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
        case 10:
            printf("Step 10 : compute houhg transforms and store it in an accumulator\n\n");
            *image = ApplyHouhgTransformByStep(*image, *image_from_canny, *renderer, hough_threshold, 1);
            break;
        case 11:
            printf("Step 11 : apply threshold to keep main lines\n\n");
            *image = ApplyHouhgTransformByStep(*image, *image_from_canny, *renderer, hough_threshold, 2);
            break;
        case 12:
            printf("Step 12 : keep only one line from groups\n\n");
            *image = ApplyHouhgTransformByStep(*image, *image_from_canny, *renderer, hough_threshold, 3);
            break;
        case 13:
            printf("Step 13 : convert line to cartesian coordinates\n\n");
            *image = ApplyHouhgTransformByStep(*original_image, *image_from_canny, *renderer, hough_threshold, 4);
            break;
        case 14:
            printf("Step 14 : clean lines\n\n");
            *image = ApplyHouhgTransformByStep(*original_image, *image_from_canny, *renderer, hough_threshold, 5);
            break;
        case 15:
            printf("Step 15 : compute intersections\n\n");
            *image = ApplyHouhgTransformByStep(*original_image, *image_from_canny, *renderer, hough_threshold, 6);
            break;
        case 16:
            printf("Step 16 : save each cases\n\n");
            *image = ApplyHouhgTransformByStep(*original_image, *image_from_canny, *renderer, hough_threshold, 7);
            break;
    }

    if (*image == NULL)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

/*Manage event loop.*/
int EventLoop(SDL_Renderer *renderer, SDL_Surface *image)
{
    // Variable to get the events.
    SDL_Event event;

    // Step counter
    int step = 0;
    if (flags & HOUGH_MODE)
        step = 10;

    // Variable for keeping gradients between two steps
    Gradient *gradients = (Gradient *)malloc(image->w * image->h * sizeof(Gradient));
    if (gradients == NULL) {
        fprintf(stderr, "Error while allocating memory for gradients (EventLoop)\n");
        return EXIT_FAILURE;
    }

    SDL_Surface* image_from_canny = ApplyCannyFilter(image, blur_straight, low_threshold, high_threshold, STRENGTH_MASK);

    // If not step by step, only show final result
    if (!(flags & STEP_BY_STEP)){

        if (flags & HOUGH_MODE || flags & GRID_MODE)
            image = ApplyHoughTransform(image, image_from_canny, renderer, hough_threshold);
        else
            image = image_from_canny;

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

    SDL_Surface* original_image_copy = SDL_CreateRGBSurface(0, image->w, image->h, image->format->BitsPerPixel,
                                                            image->format->Rmask, image->format->Gmask,
                                                            image->format->Bmask, image->format->Amask);
    SDL_BlitSurface(image, NULL, original_image_copy, NULL);

    printf("INFO: Press a key to go to next step!\n\n");

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
                if (step >= 10 && (flags & CANNY_MODE))
                    return EXIT_SUCCESS;
                else if (step >= 17)
                    return EXIT_SUCCESS;

                if (PerformNextStep(step, &image, &image_from_canny, &original_image_copy, &gradients, &renderer) != 0) {
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

                if (step == 11 || step == 12 || step == 13){
                    double scale = 1.0;
                    int desiredWidth = image->w;
                    int desiredHeight = image->h;
                    if ((image)->w > desiredWidth) scale = (double)desiredWidth / (image)->w;
                    if ((image)->h * scale > desiredHeight) scale = (double)desiredHeight / (image)->h;
                    int scaledWidth = (image)->w * scale;
                    int scaledHeight = (image)->h * scale;
                    SDL_Rect destRect = {0, 0, scaledWidth, scaledHeight};
                    SDL_RenderCopy(renderer, texture, NULL, &destRect);
                }
                else SDL_RenderCopy(renderer, texture, NULL, NULL);

                SDL_RenderPresent(renderer);

                if (texture != NULL)
                    SDL_DestroyTexture(texture);
        }
    }
}

/*Print the command line usage of the program on stdout*/
void DisplayHelp(char** argv)
{
    if (argv)
        printf("Usage: %s <command> [<options>..]\n\n", argv[0]);
    printf("Commands :\n");
    printf("    canny: run canny filter\n");
    printf("    solver: run sudoku solver\n");
    printf("    xor: run xor neural network\n");
    printf("    hough: run hough line detection\n");
    printf("    grid: run grid detection detection (canny then hough)\n");
    printf("    rotate: rotate an image\n");
    printf("Options:\n");
    printf("    -d: display image information\n");
    printf("    -z: resize image (keep proportions), to fit in the screen (CANNY, HOUGH, GRID)\n");
    printf("    -t: display processing time\n");
    printf("    -o <filename>: store output in a file\n");
    printf("    -i <filename>: choose input from a file\n");
    printf("    -s: run step by step (CANNY, HOUGH, GRID)\n");
    printf("    -h <value>: set high threshold to value (float) (CANNY, HOUGH, GRID)\n");
    printf("    -l <value>: set low threshold to value (float) (CANNY, HOUGH, GRID)\n");
    printf("    -b <value>: set blur straight to value (int) (CANNY, HOUGH, GRID)\n");
    printf("    -x <value>: set nb iteration of xor to value * 10000 (int) (XOR)\n");
    printf("    -p <value>: set displaying step of xor to value * 1000 (int) (XOR)\n");
    printf("    -r <value>: set hough threshold to value (int) (HOUGH, GRID)\n");
    printf("    -a <value>: set rotation angle to value (float) (ROTATE)\n");
}

/*Parse command line and set flag in consequence*/
int ParseArguments(int argc, char** argv)
{
    // Test arg number
    if (argc < 2)
        return -1;

    int i = 1;
    // Command
    if (strcmp(argv[1], "canny") == 0) {
        flags |= CANNY_MODE;
        i++;
    }
    else if (strcmp(argv[1], "grid") == 0) {
        flags |= GRID_MODE;
        i++;
    }
    else if (strcmp(argv[1], "xor") == 0) {
        flags |= XOR_MODE;
        i++;
    }
    else if (strcmp(argv[1], "solver") == 0) {
        flags |= SOLVER_MODE;
        i++;
    }
    else if (strcmp(argv[1], "help") == 0) {
        flags |= HELP;
        return EXIT_SUCCESS;
    }
    else if (strcmp(argv[1], "hough") == 0) {
        flags |= HOUGH_MODE;
        i++;
    }
    else if (strcmp(argv[1], "rotate") == 0) {
        flags |= ROTATE_MODE;
        i++;
    }
    else {
        fprintf(stderr, "Argument Error: '%s' command unknown\n\n", argv[1]);
        return -1;
    }

    char* options_with_parameter = "oilhbxpra";

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
                    int blur = strtol(argv[i+1], end, 10);
                    if (blur == 0 && *end == argv[i+1]){
                        fprintf(stderr, "Argument Error: '%s' option <parameter> is not an integer\n\n", argv[i]);
                        return -1;
                    }
                    blur_straight = blur;
                    i++;
                    continue;
                }
                else if (argv[i][1] == 'x') {
                    char** end = &argv[i+1];
                    int iteration = strtol(argv[i+1], end, 10);
                    if (iteration == 0 && *end == argv[i+1]){
                        fprintf(stderr, "Argument Error: '%s' option <parameter> is not an integer\n\n", argv[i]);
                        return -1;
                    }
                    nb_iteration_xor = iteration;
                    i++;
                    continue;
                }
                else if (argv[i][1] == 'p') {
                    char** end = &argv[i+1];
                    int step = strtol(argv[i+1], end, 10);
                    if (step == 0 && *end == argv[i+1]){
                        fprintf(stderr, "Argument Error: '%s' option <parameter> is not an integer\n\n", argv[i]);
                        return -1;
                    }
                    display_step_xor = step;
                    i++;
                    continue;
                }
                else if (argv[i][1] == 'r') {
                    char** end = &argv[i+1];
                    int threshold = strtol(argv[i+1], end, 10);
                    if (threshold == 0 && *end == argv[i+1]){
                        fprintf(stderr, "Argument Error: '%s' option <parameter> is not an integer\n\n", argv[i]);
                        return -1;
                    }
                    hough_threshold = threshold;
                    i++;
                    continue;
                }
                else if (argv[i][1] == 'a') {
                    char** end = &argv[i+1];
                    float angle_tmp = strtof(argv[i+1], end);
                    if (angle_tmp == 0.0 && *end == argv[i+1]){
                        fprintf(stderr, "Argument Error: '%s' option <parameter> is not a float\n\n", argv[i]);
                        return -1;
                    }
                    angle = angle_tmp;
                    i++;
                    continue;
                }
            }

            int j = 1;
            // Simple flags
            while (argv[i][j] != '\0'){
                if (argv[i][j] == 'd')
                    flags |= DISPLAY;
                else if (argv[i][j] == 's')
                    flags |= STEP_BY_STEP;
                else if (argv[i][j] == 't')
                    flags |= PRINT_TIME;
                else if (argv[i][j] == 'z')
                    flags |= RESIZE_IMAGE;
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

/*Initialize SDL window, renderer and load images from file*/
int InitSdlAndLoadImage(SDL_Surface **image, SDL_Surface **image32, SDL_Renderer **renderer, SDL_Window **window){

    // SDL2 initialization
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL initialization error : %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // List images in directory and take user choice
    if (!(flags & INPUT_GIVEN)){
        if (PrintFilesInDirectory(images_dir_path) != 0)
            return EXIT_FAILURE;
        input_filename = FileUserChoice(images_dir_path);
    }


    // Loading image from path
    *image = IMG_Load(input_filename);
    if (*image == NULL) {
        fprintf(stderr, "Error while loading image : %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Display image infos
    if (flags & DISPLAY)
        PrintImageInformation(*image);

    // 32 bits conversion
    *image32 = SDL_ConvertSurfaceFormat(*image, SDL_PIXELFORMAT_RGBA8888, 0);
    SDL_FreeSurface(*image);
    if (*image32 == NULL) {
        fprintf(stderr,"Converting image to 32 bits format error : %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Get screen size
    SDL_DisplayMode DM;
    if (SDL_GetDesktopDisplayMode(0, &DM) != 0) {
        fprintf(stderr, "Error while taking screen size %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    if (flags & RESIZE_IMAGE)
        *image32 = ResizeSurface(*image32, DM.w, DM.h - 200);

    // Display image infos
    if (flags & DISPLAY)
        PrintImageInformation(*image32);

    // Creating window
    *window = SDL_CreateWindow("Hough", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (*image32)->w, (*image32)->h, SDL_WINDOW_SHOWN);
    if (*window == NULL) {
        printf("Window creation error : %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // Creating renderer
    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (*renderer == NULL) {
        printf("Renderer creation error : %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/*QUit SDL and free ressources allocated*/
void QuitSdlAndFreeResources(SDL_Surface **image32, SDL_Renderer **renderer, SDL_Window **window) {
    if (renderer != NULL)
        SDL_DestroyRenderer(*renderer);
    if (window != NULL)
        SDL_DestroyWindow(*window);
    if (image32 != NULL)
        SDL_FreeSurface(*image32);
    SDL_Quit();
}

/*Execute canny filter*/
int MainCannyFilter()
{
    int status = EXIT_FAILURE;

    // Initialize pointers
    SDL_Surface *image = NULL;
    SDL_Surface *image32 = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Window *window = NULL;

    if (InitSdlAndLoadImage(&image, &image32, &renderer, &window) == 0){
        status = EventLoop(renderer, image32);
    }

    QuitSdlAndFreeResources(&image32, &renderer, &window);

    return status;
}

/*Execute Solver*/
int MainSolver() {

    // List images in directory and take user choice
    if (!(flags & INPUT_GIVEN)){
        if (PrintFilesInDirectory(grid_dir_path) != 0)
            return EXIT_FAILURE;
        input_filename = FileUserChoice(grid_dir_path);
    }

    return Solver(input_filename, (int)(flags & DISPLAY)); // Exit the program with a success code (0).
}

/*Execute Xor neural network learning*/
int MainXorNetwork() {
    return Xor(nb_iteration_xor, display_step_xor);
}

/*Execute Hough line detection.
 *Performs canny filter before line detection*/
int MainHoughLineDetection() {
    int status = EXIT_FAILURE;

    // Initialize pointers
    SDL_Surface *image = NULL;
    SDL_Surface *image32 = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Window *window = NULL;
    SDL_Surface* image_from_canny = NULL;

    if (InitSdlAndLoadImage(&image, &image32, &renderer, &window) == 0){
        status = EventLoop(renderer, image32);
    }

    QuitSdlAndFreeResources(&image32, &renderer, &window);

    if (image_from_canny != NULL)
        SDL_FreeSurface(image_from_canny);

    return status;
}

/*Execute canny then hough*/
int MainGridDetection(){
    int status = EXIT_FAILURE;

    // Initialize pointers
    SDL_Surface *image = NULL;
    SDL_Surface *image32 = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Window *window = NULL;

    if (InitSdlAndLoadImage(&image, &image32, &renderer, &window) == 0){
        status = EventLoop(renderer, image32);
    }

    QuitSdlAndFreeResources(&image32, &renderer, &window);

    return status;
}

/*Rotate an image adn save it*/
int MainRotate(){
    int status = EXIT_FAILURE;

    // Initialize pointers
    SDL_Surface *image = NULL;
    SDL_Surface *image32 = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Window *window = NULL;

    if (InitSdlAndLoadImage(&image, &image32, &renderer, &window) == 0){
        status = SaveSurfaceRotated(image32, angle, output_filename, renderer);
        SDL_RenderPresent(renderer);
        // Variable to get the events.
        SDL_Event event;
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



    QuitSdlAndFreeResources(&image32, &renderer, &window);

    return status;
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
        DisplayHelp(argv);
        return EXIT_FAILURE;
    }

    if (flags & CANNY_MODE)
        status = MainCannyFilter();

    else if (flags & SOLVER_MODE)
        status = MainSolver();

    else if (flags & XOR_MODE)
        status = MainXorNetwork();

    else if (flags & HOUGH_MODE)
        status = MainHoughLineDetection();

    else if (flags & HELP)
        DisplayHelp(argv);

    else if (flags & GRID_MODE)
        status = MainGridDetection();

    else if (flags & ROTATE_MODE)
        status = MainRotate();

    if (status == 0){
        // Ending time
        gettimeofday(&end_abs, NULL);
        elapsed_abs = (end_abs.tv_sec - begin_abs.tv_sec) * 1000 + (end_abs.tv_usec - begin_abs.tv_usec) / 1000;
        clock_t end_cmp = clock();
        elapsed_cmp += (double)(end_cmp - begin_cmp) / CLOCKS_PER_SEC;

        // Display time if asked
        if (flags & PRINT_TIME){
            printf("\nComputing time : %f seconds\n", elapsed_cmp);
            printf("Absolute time: %ld milliseconds\n", elapsed_abs);
        }
    }
    return status;
}
