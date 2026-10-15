#include "../headers/canny.h"
#include "../headers/image_manipulation.h"
#include "../headers/image.h"
#include "../headers/sobel.h"
#include "../headers/shape.h"
#include "../headers/error.h"
#include "../headers/shape_tab.h"
#include "../headers/matrix.h"
#include "../headers/rotations.h"
#include <complex.h>
#include <SDL2/SDL.h>   
#include <dirent.h>
#include <SDL2/SDL_ttf.h>

#define CASE 1
#define GRID 2

void deleteFilesInDirectory(const char *directoryPath) {
    DIR *dir = opendir(directoryPath);
    if (dir == NULL) {
        fprintf(stderr, "Impossible d'ouvrir le répertoire : %s\n", directoryPath);
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char filePath[PATH_MAX];
            snprintf(filePath, sizeof(filePath), "%s/%s", directoryPath, entry->d_name);

            if (remove(filePath) != 0) {
                Error("Impossible de supprimer le fichier.");
            }
        }
    }

    closedir(dir);
}

void ApplyPreProcess(Image* image, int smooth, int type, int image_type)
{
    if (image_type == 10009090)
        printf("wsh");

    int windowsSize = smooth;

    char* path = (char*)calloc(100, 8);
    snprintf(path, 100, "tmp/pre-process_type_%d_wsize_%d", type, windowsSize);

    if (type == 1){
        Image_SmoothHistogram(image, windowsSize);
        Image_EqualizeHistogram(image);
        
        Image_Save(image, path);
        
        SobelEdgeDetection(image, 80);
        snprintf(path, 100, "tmp/sobel_type_%d_wsize_%d", type, windowsSize);
        
        Image_ErodeEdges(image); 
        Image_DilateEdges(image, 1);
        Image_GaussianFilter(image, 3);
        Image_Invert(image);
        Image_ThresholdFilter(image, 200);
        Image_Save(image, path);
    }
    else if (type == 2){
        Image_NormalizeHistogram(image);
        Image_Levelize(image, smooth / 10);

        Image_Save(image, path);
        SobelEdgeDetection(image, 1);

        snprintf(path, 100, "tmp/sobel_type_%d_wsize_%d", type, windowsSize);
        Image_Save(image, path);
        Image_ThresholdFilter(image, 2);
        Image_Invert(image);
        Image_DilateEdges(image, 1);
        Image_DilateEdges(image, 4);
        Image_GaussianFilter(image, 3);
        Image_Invert(image);
        Image_ThresholdFilter(image, 130);
    }
    else{
        Image_GaussianFilter(image, 1);
        
        Image_Save(image, path);
        
        SobelEdgeDetection(image, 1);

        snprintf(path, 100, "tmp/sobel_type_%d_wsize_%d", type, windowsSize);
        Image_Save(image, path);

        Image_ThresholdFilter(image, smooth + 20);
        Image_Invert(image);
    }
}

int GridDetection(Image* image, ShapeTab* shapes, int smooth, int type, int image_type)
{
    if (image == NULL || shapes == NULL)
        Error("NULL parameter in GridDetection()");

    int windowsSize = smooth;

    ApplyPreProcess(image, smooth, type, image_type);

    char* path = (char*)calloc(100, 8);
    snprintf(path, 100, "tmp/pre-process_type_%d_wsize_%d", type, windowsSize);
    
    Image_LabelComponent(image, shapes);

    ShapeTab_FoundGrid(shapes);
    
    if (shapes->grid == NULL){
        free(path);
        return 0;
    }

    ShapeTab_KeepOnlyGrid(shapes);

    if (shapes->size != 1){
        free(path);
        return 0;
    }

    Image_Clear(image);
    Image_DrawShape(image, shapes->grid, WHITE);
    Image_Invert(image);

    Image_LabelComponent(image, shapes);

    ShapeTab_ComputeDescriptors(shapes);
    Image_DrawDescriptorsLinks(image, shapes);

    snprintf(path, 100, "tmp/label_type_%d_wsize_%d", type, windowsSize);
    Image_Save(image, path);

    ShapeTab_RemoveExcludedShape(shapes, shapes->grid);
    ShapeTab_UpdateIsSquare(shapes);
    ShapeTab_KeepOnlySquare(shapes);

    Image_Clear(image);
    Image_DrawShapeTab(image, shapes);

    free(path);
    return 1;
}

void Image_Rotate(Image *image, double angle) 
{
    /* a full turn (0 by default in the GUI) must not resample the image */
    if (fabs(remainder(angle, 2 * M_PI)) < 1e-9)
        return;

    int center_x = image->width / 2;
    int center_y = image->height / 2;
    int width = image->width;
    int height = image->height;
    Image out = Image_CreateBlack(image->width, image->height);
    
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            float dx = x - center_x;
            float dy = y - center_y;

            float new_x_float = center_x + dx * cos(angle) - dy * sin(angle);
            float new_y_float = center_y + dx * sin(angle) + dy * cos(angle);

            float new_x_frac = new_x_float - floor(new_x_float);
            float new_y_frac = new_y_float - floor(new_y_float);

            int x1 = (int)floor(new_x_float);
            int x2 = x1 + 1;
            int y1 = (int)floor(new_y_float);
            int y2 = y1 + 1;

            if (x1 >= 0 && x2 < width && y1 >= 0 && y2 < height)
            {
                out.pixels[x][y].r = (1 - new_x_frac) * (1 - new_y_frac) * image->pixels[x1][y1].r +
                                     new_x_frac * (1 - new_y_frac) * image->pixels[x2][y1].r +
                                     (1 - new_x_frac) * new_y_frac * image->pixels[x1][y2].r +
                                     new_x_frac * new_y_frac * image->pixels[x2][y2].r;

                out.pixels[x][y].g = (1 - new_x_frac) * (1 - new_y_frac) * image->pixels[x1][y1].g +
                                     new_x_frac * (1 - new_y_frac) * image->pixels[x2][y1].g +
                                     (1 - new_x_frac) * new_y_frac * image->pixels[x1][y2].g +
                                     new_x_frac * new_y_frac * image->pixels[x2][y2].g;

                out.pixels[x][y].b = (1 - new_x_frac) * (1 - new_y_frac) * image->pixels[x1][y1].b +
                                     new_x_frac * (1 - new_y_frac) * image->pixels[x2][y1].b +
                                     (1 - new_x_frac) * new_y_frac * image->pixels[x1][y2].b +
                                     new_x_frac * new_y_frac * image->pixels[x2][y2].b;
            }
        }
    }

    Image_Clone(&out, image);
    Image_Free(&out);
}


void Image_Rotate2(Image *image, double angle) 
{
    int center_x = image->width / 2;
    int center_y = image->height / 2;
    int width = image->width;
    int height = image->height;
    Image out = Image_CreateBlack(image->width, image->height);
    
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            float dx = x - center_x;
            float dy = y - center_y;

            float new_x_float = center_x + dx * cos(angle) - dy * sin(angle);
            float new_y_float = center_y + dx * sin(angle) + dy * cos(angle);

            float new_x_frac = new_x_float - floor(new_x_float);
            float new_y_frac = new_y_float - floor(new_y_float);

            int x1 = (int)floor(new_x_float);
            int x2 = x1 + 1;
            int y1 = (int)floor(new_y_float);
            int y2 = y1 + 1;

            if (x1 >= 0 && x2 < width && y1 >= 0 && y2 < height)
            {
                out.pixels[x][y].r = (1 - new_x_frac) * (1 - new_y_frac) * image->pixels[x1][y1].r +
                                     new_x_frac * (1 - new_y_frac) * image->pixels[x2][y1].r +
                                     (1 - new_x_frac) * new_y_frac * image->pixels[x1][y2].r +
                                     new_x_frac * new_y_frac * image->pixels[x2][y2].r;

                out.pixels[x][y].g = (1 - new_x_frac) * (1 - new_y_frac) * image->pixels[x1][y1].g +
                                     new_x_frac * (1 - new_y_frac) * image->pixels[x2][y1].g +
                                     (1 - new_x_frac) * new_y_frac * image->pixels[x1][y2].g +
                                     new_x_frac * new_y_frac * image->pixels[x2][y2].g;

                out.pixels[x][y].b = (1 - new_x_frac) * (1 - new_y_frac) * image->pixels[x1][y1].b +
                                     new_x_frac * (1 - new_y_frac) * image->pixels[x2][y1].b +
                                     (1 - new_x_frac) * new_y_frac * image->pixels[x1][y2].b +
                                     new_x_frac * new_y_frac * image->pixels[x2][y2].b;
            }
        }
    }

    Image_Clone(&out, image);
    Image_Free(&out);
}

Image Image_Resize28(Image *original) {
    Image resized = Image_CreateBlack(28, 28);
    
    double stepX = (double)original->width / 28.0;
    double stepY = (double)original->height / 28.0;

    for (int i = 0; i < 28; i ++){
        for (int j = 0; j < 28; j ++){
            int startX = (int)(j * stepX);
            int endX = (int)((j + 1) * stepX + 0.5);

            int startY = (int)(i * stepY);
            int endY = (int)((i + 1) * stepY + 0.5);

            int sum = 0;
            int count = 0;
            for (int x = startX; x < endX; x ++){
                for (int y = startY; y < endY; y ++){
                    sum += original->pixels[x][y].r;
                    
                    
                    count ++;
                }
            }

            if (count > 0)
                sum /= count;

            resized.pixels[j][i].r = sum;
            resized.pixels[j][i].g = sum;
            resized.pixels[j][i].b = sum;
        }
    }
    
    return resized;
}

void createAndWriteToFile(int hexa) {
    
    const char *fileName = "./tmp/hexa.txt";

    
    FILE *file = fopen(fileName, "w");

    
    if (file != NULL) {
        
        if (hexa == 1)
            fprintf(file, "1");
        else
            fprintf(file, "0");
        
        fclose(file);

        
    } else {
        
        
    }
}

int verifieEtSupprime(const char *nomFichier) {
    
    FILE *fichier = fopen(nomFichier, "r");

    
    if (fichier == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return -1;  
    }

    
    int premierCaractere = fgetc(fichier);

    
    fclose(fichier);

    
    if (premierCaractere == '1') {
        
        if (remove(nomFichier) != 0) {
            perror("Erreur lors de la suppression du fichier");
            return -1;  
        }

        return 1;  
    } else {
        return 0;  
    }
}

SDL_Surface* draw_sudoku_grid(SDL_Surface* surface, int sudoku_grid[][16], int size) {
    
    if (TTF_Init() == -1) {
    
        fprintf(stderr, "Erreur d'initialisation de SDL_ttf : %s\n", TTF_GetError());
    
    
    }else {
        
    }

    /*char cwd[1024];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Répertoire actuel : %s\n", cwd);
    }*/

    TTF_Font* font = TTF_OpenFont("res/font.ttf", 20);
    if (font == NULL) {
        
        fprintf(stderr, "Erreur de chargement de la police : %s\n", TTF_GetError());
        
        
    }
    
    
    if (surface == NULL || sudoku_grid == NULL) {
        return NULL;
    }

    
    int width = surface->w;
    int height = surface->h;

    
    int cell_size = width / size;

    
    Uint32 white = SDL_MapRGB(surface->format, 255, 255, 255);

    
    for (int i = 0; i <= size; i++) {
        
        SDL_Rect horizontal_line = {0, i * cell_size, width, 3};
        SDL_FillRect(surface, &horizontal_line, white);

        
        SDL_Rect vertical_line = {i * cell_size, 0, 3, height};
        SDL_FillRect(surface, &vertical_line, white);
    }

    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            
            if (sudoku_grid[i][j] >= 0) {
                
                char digit[3];
                snprintf(digit, sizeof(digit), "%X", sudoku_grid[i][j]);

                
                SDL_Color text_color = {255, 255, 255, 255};  
                SDL_Surface* text_surface = TTF_RenderText_Solid(font, digit, text_color);

                 
                SDL_Rect text_position = {
                    j * cell_size + (cell_size - text_surface->w) / 2,
                    i * cell_size + (cell_size - text_surface->h) / 2,
                    0,
                    0};
                
                  

                

                
                SDL_BlitSurface(text_surface, NULL, surface, &text_position);

                
                SDL_FreeSurface(text_surface);
            }
        }
    }
    TTF_CloseFont(font);
    TTF_Quit();
    return surface;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}


SDL_Surface* ApplyCannyFilter(SDL_Surface *surface)
{    
    deleteFilesInDirectory("output");
    deleteFilesInDirectory("tmp");

    if (surface == NULL)
        Error("NULL parameter in ApplyCannyFilter");

    
    Image copy = Image_CreateFromSurface(surface, surface->w, surface->h);

    ShapeTab* verified = ShapeTab_Create(70);
    Shape* matrice[16][16] = {NULL};
    Shape* matrice2[16][16] = {NULL};
    Shape* matrice3[16][16] = {NULL};

    int detected = 0;
    int iteration = 0;
    int filter = 1;
    int prev = 0;
    

    for (filter = 1;detected != 1 && filter < 4; filter++)
    {
        iteration = 0;
        int smooth = 10;
        int len = Matrix_Count(matrice);

        if (len > prev){
            prev = len;
            
        }

        if (filter == 3){
            
            for (int i = 0; i < 16; i ++){
                for (int j = 0; j < 16; j ++){
                    matrice2[i][j] = matrice[i][j];
                }
            }
            Matrix_Clear(matrice);
            for (Shape* current = verified->data->next; current;){
                ShapeTab_RemoveShape(verified, current, 0);
            }
        }

        while(detected != 1 && iteration < 5)
        {

            if (filter == 3){
                int h = Matrix_Height(matrice);
                int w = Matrix_Width(matrice);

                if ((h == 9 || h == 16 || w == 9 || w == 16 ) && Matrix_Count(matrice) > Matrix_Count(matrice3))
                {
                    
                    for (int i = 0; i < 16; i ++){
                        for (int j = 0; j < 16; j ++){
                            matrice3[i][j] = matrice[i][j];
                        }
                    }
                }
                Matrix_Clear(matrice);
                for (Shape* current = verified->data->next; current;){
                    ShapeTab_RemoveShape(verified, current, 0);
                }
            }

            Image image = Image_CreateFromSurface(surface, surface->w, surface->h);
            ShapeTab* shapes = ShapeTab_Create(90);

           /* printf("----- ITERATION : %d -----\n", iteration + 1);
            printf("SMOOTHNESS = %d\n", smooth);
            printf("----- BEFORE -----\n");

            ShapeTab_PrintInfos(shapes);
            printf("\n");*/

            char* path = (char*)calloc(100, 8);
            snprintf(path, 100, "tmp/filter_%d_iteration_%d", filter, iteration);

            if (!GridDetection(&image, shapes, smooth, filter, GRID)){
                Image_Free(&image);
                ShapeTab_Free(shapes);
                iteration ++;
                smooth += 10;
                continue;
            }

            Image_Save(&image, path);

            ShapeTab_ComputeDescriptors(shapes);
            ShapeTab_UpdateIsSquare(shapes);

            ShapeTab_KeepOnlySquare(shapes);          

            ShapeTab_UpdateCountSquaresAndTilted(shapes);
            if (shapes->data->next){
                Shape* grid = ShapeTab_GetBiggestShape(shapes);
                Shape_ComputeDescriptors(grid);
                
                ShapeTab_ComputeAverage(shapes, grid);
                ShapeTab_ComputeStandardDeviation(shapes, grid);
                ShapeTab_RemoveInconsistency(shapes, grid, 2);
            }

            iteration ++;
            smooth += 10;

            ShapeTab_PasteVerified(shapes, verified);
            ShapeTab_ComputeAverage(verified, NULL);
            ShapeTab_ComputeStandardDeviation(shapes, NULL);
            ShapeTab_RemoveInconsistency(verified, NULL, 1);
            ShapeTab_SetTilted(verified);

            detected = Matrix_IsConsistent(verified, NULL, matrice);

            snprintf(path, 100, "tmp/matrix_fi lter_%d_iteration_%d", filter, iteration);
            Image_Clear(&image);
            Image_DrawMatrix(&image, matrice);
            if (matrice[2][7])
                Image_DrawShape(&image, matrice[2][7], WHITE);
            Image_Save(&image, path);



        /*    printf("----- AFTER -----\n");
            printf("----- SHAPES -----\n");
            ShapeTab_PrintInfos(shapes);
            printf("----- VERIFIED -----\n");
            ShapeTab_PrintInfos(verified);

            printf("\n\n");*/
            
            Image_Clone(&image, &copy);

            Image_Free(&image);
            ShapeTab_Free(shapes);
        }
    }

    
    if (Matrix_Count(matrice2) > Matrix_Count(matrice)){
        for (int i = 0; i < 16; i ++){
            for (int j = 0; j < 16; j ++){
                matrice[i][j] = matrice2[i][j];
            }
        }
    }
    if (Matrix_Count(matrice3) > Matrix_Count(matrice)){
        for (int i = 0; i < 16; i ++){
            for (int j = 0; j < 16; j ++){
                matrice[i][j] = matrice3[i][j];
            }
        }
    }

    Matrix_Print(matrice);

    int wm = Matrix_Width(matrice);
    int hm = Matrix_Height(matrice);

    if ((hm == 9 || hm == 16) && (wm == 9 || wm == 16)){
        Matrix_EstimateCase(matrice);
        Image_Clear(&copy);
        Image_DrawMatrix(&copy, matrice);
        detected = Matrix_Count(matrice) == 81 || Matrix_Count(matrice) == 256;
    }
    int hexa = 0;
    if (Matrix_Width(matrice) == 9){
        createAndWriteToFile(0);
        hexa = 0;
    }
    else{
        createAndWriteToFile(1);
        hexa = 1;
    }

    if (!detected)
        return NULL;

    

    if (detected){
        char* path = (char*)malloc(40); 
        Image origin = Image_CreateFromSurface(surface, surface->w, surface->h);
        
        double a = 0;
        int titled = 0;
        for (int i = 0; i < 16; i ++){
            for (int j = 0; j < 16; j ++){
                if (matrice[i][j] && matrice[i][j]->isTilted){
                    titled ++;
                    a += Point_Angle(matrice[i][j]->descriptor[3], matrice[i][j]->descriptor[1]);
                }
            }
        }

        if (titled > 40){
            a /= titled;
            Image_Rotate(&origin, a - M_PI_2 * 3);
            
        }

        Image_UpdateHistogram(&origin);
        Image_SmoothHistogram(&origin, 10);
        Image_EqualizeHistogram(&origin);

        int count = 0;
        for (int i = 0; i < 16; i ++){
            for (int j = 0; j < 16; j ++){  
                Shape* current = matrice[i][j];
                if (current != NULL){
                    count ++;
                    snprintf(path, 20, "output/ha_%d%d", i, j);
                    SDL_Rect rect;

                    int width = current->width;
                    int height = current->height;
                    
                    double angle = Point_Angle(current->descriptor[3], current->descriptor[1]) - M_PI_2 * 3;
                    if (angle == NAN)
                        Error("Invalid angle in ApplyCannyFilter()");

                    if (current->isTilted){
                        width = current->descriptor[1].x - current->descriptor[0].x;
                        height = current->descriptor[3].y - current->descriptor[2].y;
                    }
                    rect.x = current->averageX - width / 2;
                    rect.y = current->averageY - height / 2;
                    rect.w = current->width;
                    rect.h = current->height;

                    if (current->isTilted){
                        double b = -a + M_PI_2 * 3;
                        float dx = current->averageX - origin.width/2;
                        float dy = current->averageY - origin.height/2;

                        float new_x_float = origin.width/2 + dx * cos(b) - dy * sin(b);
                        float new_y_float = origin.height/2 + dx * sin(b) + dy * cos(b);

                        unsigned int new_x = (int)floor(new_x_float);
                        unsigned int new_y = (int)floor(new_y_float);
                        rect.x = new_x - width/2;
                        rect.y = new_y - height/2;
                        

                        
                    }

                    snprintf(path, 20, "output/case_%x%x", i, j);

                    Image caseImage = Image_Crop(&origin, &rect);

                    
                    

                    Image_UpdateHistogram(&caseImage);
                    
                    
                    
                    
                    

                    Image resized = Image_Resize28(&caseImage);
                    Image copy = Image_Copy(&resized);
                    Image_Invert(&copy);
                    ShapeTab* tab = ShapeTab_Create(3);
                    Image_HighPassFilter(&copy, 150);
                    Image_LabelComponent(&copy, tab);
                    Image_ThresholdFilter(&resized, 100);
                    
                    

                    if (tab->size > 0){
                        Shape* shape = ShapeTab_GetBiggestShape(tab);
                        int minx = shape->extremum[0], maxx = shape->extremum[1];
                        int miny = shape->extremum[2], maxy = shape->extremum[3];
                        int s = max(maxx - minx, maxy - miny) + 6;
                        SDL_Rect rec;
                        rec.x = (minx + maxx) / 2 - s / 2;
                        rec.y = (miny + maxy) / 2 - s / 2;
                        rec.w = s;
                        rec.h = s;
                        if (rec.x < 0) rec.x = 0;
                        if (rec.y < 0) rec.y = 0;
                        if (rec.x + rec.w > 28) rec.w = 28 - rec.x;
                        if (rec.y + rec.h > 28) rec.h = 28 - rec.y;
                        if (rec.w > 0 && rec.h > 0){
                            Image num = Image_Crop(&resized, &rec);
                            Image fin = Image_Resize28(&num);
                            Image_Save(&fin, path);
                        }
                        else
                            Image_Save(&resized, path);
                    }
                    

                    
                    Image_Free(&caseImage); 
                    Image_Free(&resized);
                    
                    
                }
            }
        }
        double angle = ShapeTab_ComputeAngle(verified) * 180 / M_PI;
        
        rotate(&origin, angle);
        
        Image_Free(&origin);
    }

    else{
        
    }

    Image rese = Image_CreateFromSurface(surface, surface->w, surface->h);

    printf("----- RESULT -----\n");
    printf("ITERAION = %d\n", iteration);
    printf("FOUND = %s\n", detected? "YES" : "NO");
    printf("HEXA = %s\n", hexa? "YES" : "NO");

    SDL_Rect rect;
    if (hexa == 0){
        rect.w = abs((int)matrice[0][8]->extremum[1] - (int)matrice[8][0]->extremum[0]);
        rect.h = abs((int)matrice[8][8]->extremum[3] - (int)matrice[0][0]->extremum[2]);
    }
    else {
        rect.w = abs((int)matrice[0][15]->extremum[1] - (int)matrice[15][0]->extremum[0]);
        rect.h = abs((int)matrice[15][15]->extremum[3] - (int)matrice[0][0]->extremum[2]);
    }

    rect.w = (rect.w + 50);
    rect.h = (rect.h + 50);
    
    Shape_ComputeDescriptors(matrice[0][0]);
    Shape_ComputeDescriptors(matrice[0][8]);

    rect.y = matrice[0][0]->extremum[2];
    rect.x = matrice[8][0]->extremum[0];
    rect.y = abs(rect.y - 10);
    rect.x = abs(rect.x - 10);

    if (rect.x + rect.w >= (int)rese.width)
        rect.w = rese.width - rect.x - 5;
    if (rect.y + rect.h >= (int)rese.height)
        rect.h = rese.height - rect.y - 5;

    if (rect.x < 0)
        rect.x = 0;
    if (rect.y < 0)
        rect.y = 0;


    
    
    Image t = Image_Crop(&rese, &rect);
    Image_Save(&t, "tmp/croped");
    Image_Free(&t);
    Image_DrawMatrix(&rese, matrice);
    ShapeTab_Free(verified);
    SDL_Surface* res =  Image_CreateSurface(&rese);
    
    Image_Free(&copy);

    for (int i = 0; i < 16; i ++){
        for (int j = 0; j < 16; j ++){  
            Shape* current = matrice[i][j];
            if (current != NULL)
                Shape_Free(current);
        }
    }

    return res;
}
