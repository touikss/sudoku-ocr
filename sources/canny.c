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

void GridDetection(Image* image, ShapeTab* shapes, int smooth, int type)
{
    if (image == NULL || shapes == NULL)
        Error("NULL parameter in GridDetection()");

    int windowsSize = smooth;

    printf("WINDOW SIZE = %d\n", windowsSize);

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
        Image_Levelize(image, smooth / 10 + 5);

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
    
    Image_LabelComponent(image, shapes);

    ShapeTab_FoundGrid(shapes);
    
    if (shapes->grid == NULL)
        Error("Error while searching the grid in TabShape in GridDetection()");

    ShapeTab_KeepOnlyGrid(shapes);

    if (shapes->size != 1)
        errx(EXIT_FAILURE, "Trop de formes détectées. Ou pas assez...");

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
}



void Image_Rotate(Image *image, double angle) 
{
    int center_x = image->width / 2;
    int center_y = image->height / 2;
    int width = image->width;
    int height = image->height;
    Image out = Image_CreateBlack(image->width, image->height);
    //Image out = Image_Copy(image);
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



SDL_Surface* ApplyCannyFilter(SDL_Surface *surface)
{    
    if (surface == NULL)
        Error("NULL parameter in ApplyCannyFilter");

    //Image image = Image_CreateFromSurface(surface, surface->w, surface->h);
    Image copy = Image_CreateFromSurface(surface, surface->w, surface->h);

    ShapeTab* verified = ShapeTab_Create(70);
    Shape* matrice[16][16] = {NULL};
    Shape* matrice2[16][16] = {NULL};
    Shape* matrice3[16][16] = {NULL};

    int detected = 0;
    int iteration = 0;
    int filter = 1;
    int bestFilter = 1;
    int prev = 0;

    for (filter = 1;detected != 1 && filter < 4; filter++)
    {
        iteration = 0;
        int smooth = 10;
        int len = Matrix_Count(matrice);

        if (len > prev){
            prev = len;
            bestFilter = filter;
        }

        if (filter == 3){
            // Copy matrice in matrice 2
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
                    // Copy matrice in matrice 3
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

            GridDetection(&image, shapes, smooth, filter);

            Image_Save(&image, path);

            ShapeTab_ComputeDescriptors(shapes);
            ShapeTab_UpdateIsSquare(shapes);

            ShapeTab_KeepOnlySquare(shapes);          

            ShapeTab_UpdateCountSquaresAndTilted(shapes);
            if (shapes->data->next){
                Shape* grid = ShapeTab_GetBiggestShape(shapes);
                Shape_ComputeDescriptors(grid);
                //printf("ANGLE GRID = %lf\n", Shape_ComputeAngle(grid) * 180 / M_PI);
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

    // Keep the best matrice
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


    int hm = Matrix_Height(matrice);
    int wm = Matrix_Width(matrice);

    if ((hm == 9 || hm == 16) && (wm == 9 || wm == 16)){
        Matrix_EstimateCase(matrice);
        Image_Clear(&copy);
        Image_DrawMatrix(&copy, matrice);
        detected = Matrix_Count(matrice) == 81 || Matrix_Count(matrice) == 256;
    }

    if (detected){
        char* path = (char*)malloc(40); 
        Image origin = Image_CreateFromSurface(surface, surface->w, surface->h);
        
        double a = 0;
        int titled;
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
            Image_Save(&origin, "ROTATED");
        }

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
                        //Image_Rotate(&caseImage, angle - M_PI_2 * 3);

                        Image_DrawPoint(&origin, (Point){new_x, new_y}, RED, 3);
                    }

                    Image caseImage = Image_Crop(&origin, &rect);
                        
                    
                    snprintf(path, 20, "output/case_%x%x", i, j);
                    Image_Save(&caseImage, path);
                    Image_Free(&caseImage); 
                    Image_Save(&origin, "FINAL");
                }
            }
        }
        double angle = ShapeTab_ComputeAngle(verified) * 180 / M_PI;
        printf("COUNT = %d\n", count);
        rotate(&origin, angle);
        Image_Save(&origin, "ROTATÉE");
        Image_Free(&origin);
    }

    else{
        Error("No grid detected.");
    }


    ShapeTab_Free(verified);

    printf("----- RESULT -----\n");
    printf("ITERAION = %d\n", iteration);
    printf("FOUND = %s\n", detected? "YES" : "NO");

    SDL_Surface* res =  Image_CreateSurface(&copy);
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
