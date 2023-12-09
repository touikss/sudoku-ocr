#include "../headers/hough.h"
#include "../headers/image_manipulation.h"

#include <unistd.h>

int rho_margin_clean = 40;
int theta_margin_clean = 20;

// Initialize PointArray
void InitializeSDLPointArray(PointArray* arr, size_t initial_capacity) {
    arr->data = (SDL_Point*)malloc(initial_capacity * sizeof(SDL_Point));
    arr->size = 0;
    arr->capacity = initial_capacity;
}

// Initialize Line array
void InitializePolarLineArray(PolarLineArray* arr, size_t initial_capacity) {
    arr->data = (PolarLine*)malloc(initial_capacity * sizeof(PolarLine));
    arr->size = 0;
    arr->capacity = initial_capacity;
}

// Add a point to array
void AddPoint(PointArray* arr, SDL_Point point) {
    if (arr->size == arr->capacity) {
        // If size is reached increase capacity
        arr->capacity *= 2;
        arr->data = (SDL_Point*)realloc(arr->data, arr->capacity * sizeof(SDL_Point));
    }

    arr->data[arr->size++] = point;
}

// Add polar line to array
void AddPolarLine(PolarLineArray* arr, PolarLine line) {
    if (arr->size == arr->capacity) {
        // If size is reached increase capacity
        arr->capacity = arr->capacity == 0? 2 : arr->capacity * 2;
        arr->data = (PolarLine*)realloc(arr->data, arr->capacity * sizeof(PolarLine));
    }
    arr->data[arr->size++] = line;
}

// Remove a point dynamically at index in array
void RemovePointAtIndex(PointArray* arr, size_t index) {
    if (index < arr->size) {
        for (size_t i = index; i < arr->size - 1; i++) {
            arr->data[i] = arr->data[i + 1];
        }
        arr->size--;
    }
}

// Remove a line dynamically at index in array
void RemoveLineAtIndex(PolarLineArray* arr, size_t index) {
    if (index < arr->size) {
        for (size_t i = index; i < arr->size - 1; i++) {
            arr->data[i] = arr->data[i + 1];
        }
        arr->size--;
    }
}

// Free memory of point array
void FreePointArray(PointArray* arr) {
    free(arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->capacity = 0;
}

// Free memory of line array
void FreePolarLineArray(PolarLineArray* arr) {
    free(arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->capacity = 0;
}

SDL_Surface* ConvertHoughSpaceToSurface(int n_theta, int n_rho, int polar_tab[n_theta][n_rho])  {
    SDL_Surface *hough_space = SDL_CreateRGBSurface(0,n_theta, n_rho,
                                                    32, 0, 0, 0, 0);
    if (hough_space == NULL) {
        fprintf(stderr, "Error while crating surface in ConvertHoughSpaceToSurface: %s\n", SDL_GetError());
        return NULL;
    }
    for (int x = 0; x < n_theta; x ++){
        for (int y = 0; y < n_rho; y++) {
            int val = polar_tab[x][y];
            if (val > 255)
                val = 255;
            SetPixelRGBA(hough_space, x, y, val, val, val, 255);
        }
    }
    return hough_space;
}

// Draw a point on renderer with rgb value at (x,y) position and with thickness
void DrawThickPoint(SDL_Renderer* renderer, int x, int y, int thickness, Uint8 r, Uint8 g, Uint8 b) {
    // Draw central point
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderDrawPoint(renderer, x, y);

    // Draw adjacent points to increase thickness
    for (int i = 1; i <= thickness; i++) {
        // Draw top, bottom, right, left
        SDL_RenderDrawPoint(renderer, x, y - i);
        SDL_RenderDrawPoint(renderer, x, y + i);
        SDL_RenderDrawPoint(renderer, x - i, y);
        SDL_RenderDrawPoint(renderer, x + i, y);

        // Draw diagonal
        SDL_RenderDrawPoint(renderer, x - i, y - i);
        SDL_RenderDrawPoint(renderer, x - i, y + i);
        SDL_RenderDrawPoint(renderer, x + i, y - i);
        SDL_RenderDrawPoint(renderer, x + i, y + i);
    }
}

/*
// From a canny surface, compute equivalent in a hough space (polar coordinates)
// Stores it in an accumulator
void ConvertSurfaceToLineAccumulator(SDL_Surface* image_from_canny, int n_theta, int n_rho, int accum[n_theta][n_rho], double d_theta, double d_rho, int n_y, int n_x)
{
    for (int j = 0; j < n_y; j ++){
        for (int i = 0; i < n_x; i ++){
            // Check if pixel is white
            if (GetPixelLuminance(image_from_canny, i, j) > 0)
                for (int i_theta = 0; i_theta < n_theta; i_theta ++){
                    double theta = i_theta * d_theta;
                    double rho = i*cos(theta)+(n_y-j)*sin(theta);
                    int i_rho = (int)(rho/d_rho);
                    if (i_rho>0 && i_rho<n_rho)
                        accum[i_theta][i_rho] += 1;
                }
        }
    }
}*/

void ConvertSurfaceToLineAccumulator(SDL_Surface* image_from_canny, int n_theta, int n_rho, int (*accum)[n_rho], double d_theta, double d_rho, int n_y, int n_x)
{
    for (int j = 0; j < n_y; j++) {
        for (int i = 0; i < n_x; i++) {
            // Check if pixel is white
            if (GetPixelLuminance(image_from_canny, i, j) > 0) {
                for (int i_theta = 0; i_theta < n_theta; i_theta++) {
                    double theta = i_theta * d_theta;
                    double rho = i * cos(theta) + (n_y - j) * sin(theta);
                    int i_rho = (int)(rho / d_rho);
                    if (i_rho > 0 && i_rho < n_rho) {
                        (*accum)[i_theta * n_rho + i_rho] += 1;
                    }
                }
            }
        }
    }
}


// Keep only values higher than e thershold in the accumulator
void ApplyThresholdToAccumulator(int threshold, int *nb_lines, int n_theta, int n_rho, int accum[n_theta][n_rho])
{
    for (int x = 0; x < n_theta; x ++){
        for (int y = 0; y < n_rho; y++) {
            if (accum[x][y] < threshold)
                accum[x][y] = 0;
            else{
                *nb_lines += 1;
            }
        }
    }
}

// Keep only one point when there are many in the same place
void KeepOnlyOnePointFromGroups(int *nb_lines, int n_theta, int n_rho, int accum[n_theta][n_rho], double rho_margin, double theta_margin)
{
    nb_lines ++;
    // Run through accumulator
    for (int x = 0; x < n_theta; x ++){
        for (int y = 0; y < n_rho; y++) {
            if (accum[x][y] != 0) { // If pixel white detected
                int rho_average = 0;
                int theta_average = 0;
                int count = 0;
                int max_value = 0;

                for (int x_tmp = x; x_tmp < x + theta_margin && x_tmp < n_theta; x_tmp += 1) {    // Compute average in pixels around
                    for (int y_tmp = y; y_tmp < y + rho_margin && y_tmp < n_rho; y_tmp += 1) {
                        if (accum[x_tmp][y_tmp] != 0){
                            rho_average += y_tmp * accum[x_tmp][y_tmp];
                            theta_average += x_tmp * accum[x_tmp][y_tmp];
                            count += accum[x_tmp][y_tmp];
                            if (accum[x_tmp][y_tmp] > max_value)
                                max_value = accum[x_tmp][y_tmp];
                            accum[x_tmp][y_tmp] = 0;
                        }
                    }
                }
                rho_average /= count;
                theta_average /= count;
                // Only keep the one which is at average coordinates
                accum[theta_average][rho_average] = 255;
                //printf("MAX VAUE: %d\n", max_value);
            }
        }
    }
}


// From accumulator, deduce polar line
void ComputePolarLineCoordinates(PolarLineArray* lines, int n_theta, int n_rho, int accum[n_theta][n_rho], double d_theta, double d_rho)
{
    for (int i_theta = 0; i_theta < n_theta; i_theta ++){
        for (int i_rho = 0; i_rho < n_rho; ++i_rho) {
            if (accum[i_theta][i_rho] != 0){
                PolarLine tmp_polar;
                tmp_polar.theta = i_theta*d_theta;
                tmp_polar.rho = i_rho*d_rho;
                AddPolarLine(lines, (PolarLine){tmp_polar.rho, tmp_polar.theta});
            }
        }
    }
}

// Draw all lines 
int DrawLinesOnRenderer(SDL_Renderer* renderer, PolarLineArray lines, int n_y)
{
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Set render color
    for (size_t i_polar = 0; i_polar < lines.size; i_polar++ ){
        // Compute coordinates in cartesian space
        double theta = lines.data[i_polar].theta;
        double rho = lines.data[i_polar].rho;
        double a = cos(theta);
        double b = sin(theta);
        double x0 = a*rho;
        double y0 = b*rho;
        int x1 = (int)(x0 + 1000*(-b)); //*1000 is to be sure that the line will not end in the screen
        int y1 = n_y - (int)(y0 + 1000*(a));
        int x2 = (int)(x0 - 1000*(-b));
        int y2 = n_y - (int)(y0 - 1000*(a));
        // Draw the line
        if (SDL_RenderDrawLine(renderer, x1, y1, x2, y2) != 0)
            return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// Compute intersection point of two polar line with their parameter
void ComputeIntersectionPointsBetweenTwoLines(PolarLine polar_1, PolarLine polar_2, SDL_Point* intersection, int n_y)
{
    if (polar_1.theta - polar_2.theta != 0){
        int x = (int)((polar_1.rho * sin(polar_2.theta) - polar_2.rho * sin(polar_1.theta)) / sin(polar_2.theta - polar_1.theta));
        int y = n_y - (int)((polar_2.rho * cos(polar_1.theta) - polar_1.rho * cos(polar_2.theta)) / sin(polar_2.theta - polar_1.theta));
        (*intersection).x = x;
        (*intersection).y = y;
    }
    else{
        (*intersection).x = -1;
        (*intersection).y = -1;
    }
}

// Compute every intersection points between every line and keep the ones which ar in the screen
void ComputeAllIntersectionPoints(PolarLineArray polar_lines, int n_y, int n_x, PointArray* points)
{
    SDL_Point intersection;
    // Run through all pair of lines
    for (size_t i_line_1 = 0; i_line_1 < polar_lines.size - 1; i_line_1 += 1) {
        for (size_t i_line_2 = i_line_1 + 1; i_line_2 < polar_lines.size; i_line_2 += 1){
            // Compute intersection
            ComputeIntersectionPointsBetweenTwoLines(polar_lines.data[i_line_1], polar_lines.data[i_line_2], &intersection, n_y);
            // Check if it is in the screen
            if (intersection.x >= 0 && intersection.x < n_x && intersection.y > 0 && intersection.y < n_y){
                AddPoint(points, (SDL_Point){intersection.x, intersection.y}); // Add it
            }
        }
    }
}

// Draw all points on renderer
void DrawAllPoints(PointArray points, SDL_Renderer* renderer){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set render color
    for (size_t i = 0; i < points.size; i++) {
        DrawThickPoint(renderer, points.data[i].x, points.data[i].y, 8, 0, 0, 255); // DRaw
    }
}

/*Comparison function to sort polar lines
 * Compare with theta and after with rho*/
int ComparePolarLineCoordinates(const void* a, const void* b) {
    PolarLine* polarA = (PolarLine*)a;
    PolarLine* polarB = (PolarLine*)b;

    // Compare with theta admitting a margin
    if ((polarA->theta - polarB->theta) < -M_PI/5) return -1;
    if ((polarA->theta - polarB->theta) > M_PI/5) return 1;

    // If theta equals, compare with rho
    if (polarA->rho < polarB->rho) return -1;
    if (polarA->rho > polarB->rho) return 1;

    return 0; // Struct are equals
}

// Sort polar line with theta first and rho secondly
void SortPolarLineByTheta(PolarLineArray* lines) {
    qsort(lines->data, lines->size, sizeof(PolarLine), ComparePolarLineCoordinates);
}

// Remove lines that are too close together
void CleanSortedLineTooClose(PolarLineArray* lines)
{
    for (size_t i = 1; i < (*lines).size; ++i) {
        if(fabs((*lines).data[i].theta - (*lines).data[i - 1].theta) > 0.01 && fabs((*lines).data[i].rho - (*lines).data[i - 1].rho) < 40){
            if (fabs((*lines).data[i-1].theta) > 0.2) {
                if (fabs((*lines).data[i-1].theta - M_PI / 2) >
                    fabs((*lines).data[i].theta - M_PI / 2))
                    RemoveLineAtIndex(lines, i-1);
                else
                    RemoveLineAtIndex(lines, i);
            }
            else {
                if (fabs((*lines).data[i-1].theta) >
                    fabs((*lines).data[i].theta))
                    RemoveLineAtIndex(lines, i-1);
                else
                    RemoveLineAtIndex(lines, i);
            }
            i--;
        }
    }
}

// Compute distance between too points
double CalculateDistance(SDL_Point point1, SDL_Point point2) {
    int dx = point2.x - point1.x;
    int dy = point2.y - point1.y;
    return sqrt(dx * dx + dy * dy);
}

// Remove points which are several times in the array
void RemoveTwinPoints(PointArray* points)
{
    for (size_t i = 0; i < points->size; ++i) {
        for (size_t j = 0; j < points->size; ++j) {
            if (j == i)
                continue;
            if (points->data[i].x == points->data[j].x && points->data[i].y == points->data[j].y){
                RemovePointAtIndex(points, j);
                j --;
            }
        }
    }
}

// Find indexes of four closest points to point at index
void FindClosestPointsIndex(PointArray* points, size_t index, size_t closest_index[4]) {

    if (points->size <= 5)
        fprintf(stderr, "Too few points in array\n");
    // Set starting index

    double min_dist; // Variable for comparison
    double min_tmp;
    size_t min_start;

    for (int j = 0; j < 4; j += 1) { // Take first four closest
        min_start = 0;
        while ( (min_start == closest_index[0] || min_start == closest_index[1] || min_start == closest_index[2] ||
                min_start == closest_index[3] || min_start == index) && min_start < points->size){
            min_start ++;
        }
        min_dist = CalculateDistance(points->data[index], points->data[min_start]);
        for (size_t i = 0; i < points->size; i += 1) {
            if (i == index || i == closest_index[0] || i == closest_index[1] || i == closest_index[2] ||
                i == closest_index[3]) // If already taken, skip
                continue;
            min_tmp = CalculateDistance(points->data[index], points->data[i]);
            if (min_tmp < min_dist) { // Compare distance and change if new min found
                min_dist = min_tmp;
                closest_index[j] = i;
            }
        }
    }
}

// Determine if there is a point at each direction (and which) of current point in the closest
void FindClosestPointsDirection(PointArray* points, size_t closest[4], size_t *left, size_t *right, size_t *top, size_t *bottom, size_t index){
    // Left
    if (points->data[index].x > points->data[closest[0]].x + 10)
        *left = closest[0];
    else if (points->data[index].x > points->data[closest[1]].x + 10)
        *left = closest[1];
    else if (points->data[index].x > points->data[closest[2]].x + 10)
        *left = closest[2];
    else if (points->data[index].x > points->data[closest[3]].x + 10)
        *left = closest[3];

    // Right
    if (points->data[index].x < points->data[closest[0]].x - 10)
        *right = closest[0];
    else if (points->data[index].x < points->data[closest[1]].x - 10)
        *right = closest[1];
    else if (points->data[index].x < points->data[closest[2]].x - 10)
        *right = closest[2];
    else if (points->data[index].x < points->data[closest[3]].x - 10)
        *right = closest[3];

    // Bottom
    if (points->data[index].y < points->data[closest[0]].y - 10)
        *bottom = closest[0];
    else if (points->data[index].y < points->data[closest[1]].y - 10)
        *bottom = closest[1];
    else if (points->data[index].y < points->data[closest[2]].y - 10)
        *bottom = closest[2];
    else if (points->data[index].y < points->data[closest[3]].y - 10)
        *bottom = closest[3];

    // Top
    if (points->data[index].y > points->data[closest[0]].y + 10)
        *top= closest[0];
    else if (points->data[index].y > points->data[closest[1]].y + 10)
        *top= closest[1];
    else if (points->data[index].y > points->data[closest[2]].y + 10)
        *top= closest[2];
    else if (points->data[index].y > points->data[closest[3]].y + 10)
        *top= closest[3];
}

// Find the closest point to origin in terms of distance
void FindClosestPointToOrigin(PointArray* points, size_t *index)
{
    double min = sqrt(points->data[0].y * points->data[0].y + points->data[0].y * points->data[0].x);
    *index = 0;
    for (size_t i = 0; i < points->size; ++i) {
        double tmp = sqrt(points->data[i].y * points->data[i].y + points->data[i].x * points->data[i].x);
        if ( tmp < min) {
            min = tmp;
            *index = i;
        }
    }
}


void DrawPointAndClosest(SDL_Renderer* renderer, size_t top, size_t bottom, size_t left, size_t right, size_t current, PointArray* points){
    SDL_Point current_point = points->data[current];
    if (left != 1000){
        SDL_Point left_point = points->data[left];
        DrawThickPoint(renderer, left_point.x, left_point.y, 5, 0, 0, 255);
    }
    if (right != 1000){
        SDL_Point right_point = points->data[right];
        DrawThickPoint(renderer, right_point.x, right_point.y, 5, 0, 0, 255);
    }
    if (top != 1000) {
        SDL_Point top_point = points->data[top];
        DrawThickPoint(renderer, top_point.x, top_point.y, 5, 0, 0, 255);
    }
    if (bottom != 1000) {
        SDL_Point bottom_point = points->data[bottom];
        DrawThickPoint(renderer, bottom_point.x, bottom_point.y, 5, 0, 0, 255);
    }

    DrawThickPoint(renderer, current_point.x, current_point.y, 8, 0, 255, 0);
    SDL_RenderPresent(renderer);
}

// Run through points, take the closest points in every direction and if it is possible, save case in a file
void SaveEachCase(PointArray* points, SDL_Surface* image, SDL_Renderer* renderer, int show)
{
    size_t current = 0;
    FindClosestPointToOrigin(points, &current);
    size_t left; size_t right; size_t top; size_t bottom;
    size_t closest[4];
    size_t end_count = 0;
    int case_count = 0;


    while (points->size > 5 && end_count < 2){ // end_count >= 2 means that we are on the last row
        if (show)
            usleep(50000);
        // FIn closest points
        FindClosestPointsIndex(points,current, closest);
        left = 1000; right = 1000; top = 1000; bottom = 1000;
        // Find closest points in every direction
        FindClosestPointsDirection(points, closest, &left, &right, &top, &bottom,current);

        if (bottom != 1000 && right != 1000){ // It is in the top left corner of a case
            // then draw it
            DrawThickPoint(renderer, points->data[current].x, points->data[current].y, 8, 0, 255, 0);
            SDL_RenderPresent(renderer);
            // Compute width of the case
            int width = abs(points->data[current].x - points->data[right].x);
            int height = abs(points->data[current].y - points->data[bottom].y);
            // Create file name
            char buffer[90];
            snprintf(buffer, sizeof(buffer), "output/case_%d%d.png", case_count/9, case_count%9);
            printf("Case saved : %d, %d\n", case_count/9, case_count%9);
            // Save the case
            SaveSurfaceSample(image, points->data[current].x + 15,points->data[current].y + 10, width - 35, height - 20, buffer);
            case_count ++;
            // Remove the point
            RemovePointAtIndex(points,current);
            current = right - 1;
            end_count = 0;
        }
        else{ // Then go to a new line by searching the closest point to origin
            FindClosestPointToOrigin(points, &current);
            end_count ++;
        }
    }
}

// Perform hough transform step by step
// Clearly less optimized than canny step by step
SDL_Surface* ApplyHouhgTransformByStep(SDL_Surface* original_image, SDL_Surface* image_from_canny, SDL_Renderer* renderer, int threshold, int step){
    DeleteBord(image_from_canny); // Delete border to increase precision

    // Initialize variable that we will use to pass in polar coordinates

    int n_y = image_from_canny->h;
    int n_x = image_from_canny->w;

    double rho = 1.0;
    double theta = 1.0;

    int n_theta = (int)(180.0/theta);
    int n_rho = (int)(floor(sqrt(n_x*n_x+n_y*n_y))/rho);
    double d_theta = M_PI/n_theta;
    double d_rho = floor(sqrt(n_x*n_x+n_y*n_y))/n_rho;

    int accum[n_theta][n_rho]; // Accumulator (Houhg space)

    // Initialize all element to 0
    for (int i = 0; i < n_theta; i++) {
        for (int j = 0; j < n_rho; j++) {
            accum[i][j] = 0;
        }
    }

    PolarLineArray lines; // Dynamic line array
    InitializePolarLineArray(&lines, 10);
    PointArray points; // Points array
    InitializeSDLPointArray(&points, 10);


    // Compute curves
    ConvertSurfaceToLineAccumulator(image_from_canny, n_theta, n_rho, accum, d_theta, d_rho, n_y, n_x);

    SDL_Surface * res = NULL;

    if (step == 1) {
        res = ConvertHoughSpaceToSurface(n_theta, n_rho, accum);
        goto Quit;
    }

    int nb_lines = 0;
    ApplyThresholdToAccumulator(threshold, &nb_lines, n_theta, n_rho, accum); // Apply threshold

    if (step == 2) {
        res = ConvertHoughSpaceToSurface(n_theta, n_rho, accum);
        goto Quit;
    }

    // Clean lines
    KeepOnlyOnePointFromGroups(&nb_lines, n_theta, n_rho, accum, rho_margin_clean, theta_margin_clean);

    if (step == 3) {
        res = ConvertHoughSpaceToSurface(n_theta, n_rho, accum);
        goto Quit;
    }

    // Compute polar coordinates from accumulator
    ComputePolarLineCoordinates(&lines, n_theta, n_rho, accum, d_theta, d_rho);

    if (step == 4) {
        // CLear renderer
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        // Creating texture from "image"
        // We do all this to display points and lines not only surface
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, original_image);
        if (texture == NULL) {
            printf("Texture creation error : %s\n", SDL_GetError());
            goto Quit;
        }
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        if (DrawLinesOnRenderer(renderer, lines, n_y) != 0)
            goto Quit;
        SDL_Surface *screenshot = SDL_CreateRGBSurface(0,image_from_canny->w,
                                                       image_from_canny->h, 32,
                                                       0, 0, 0, 0);
        if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888,
                                 screenshot->pixels, screenshot->pitch) != 0) {
            fprintf(stderr, "Error while capturing screen : %s\n", SDL_GetError());
            goto Quit;
        }
        res = screenshot;
        goto Quit;
    }

    // Sort lines
    SortPolarLineByTheta(&lines);
    // And clean it
    CleanSortedLineTooClose(&lines);

    // 100 lines remaining means that there is an error... or it is not a grid
   /* if (nb_lines > 100){
        fprintf(stderr, "Too many lines found with Hough transform, change parameters and try again\n");
        goto Quit;
    }
    else if (lines.size < 10){
        fprintf(stderr, "Too few lines found with Hough transform, change parameters and try again\n");
        goto Quit;
    }*/
    if (step == 5) {

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, original_image);
        if (texture == NULL) {
            printf("Texture creation error : %s\n", SDL_GetError());
            goto Quit;
        }
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        DrawLinesOnRenderer(renderer, lines, n_y);
        SDL_Surface *screenshot = SDL_CreateRGBSurface(0, image_from_canny->w,image_from_canny->h,
                                                       32, 0, 0, 0, 0);
        if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, screenshot->pixels, screenshot->pitch) != 0) {
            SDL_Log("Error while capturing screen%s", SDL_GetError());
            goto Quit;
        }
        res = screenshot;
        goto Quit;
    }

    // Compute points value
    ComputeAllIntersectionPoints(lines, n_y, n_x, &points);

    // Clean it
    RemoveTwinPoints(&points); // Normally not required, but if there are twins, it will sabotage the rest of the program
    if (step == 6) {
        // Same as above, we want to display surface, lines and points
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, original_image);
        if (texture == NULL) {
            printf("Texture creation error : %s\n", SDL_GetError());
            goto Quit;
        }
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        DrawLinesOnRenderer(renderer, lines, n_y);
        DrawAllPoints(points, renderer);
        SDL_Surface *screenshot = SDL_CreateRGBSurface(0, image_from_canny->w, image_from_canny->h, 32, 0, 0, 0, 0);
        if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, screenshot->pixels, screenshot->pitch) != 0) {
            SDL_Log("Error while capturing screen%s", SDL_GetError());
            goto Quit;
        }
        res = screenshot;
        goto Quit;
    }

    if (step == 7)
    {
        // Save cases in files
        SaveEachCase(&points, original_image, renderer, 1);
        SDL_Surface *screenshot = SDL_CreateRGBSurface(0, image_from_canny->w, image_from_canny->h, 32, 0, 0, 0, 0);
        if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, screenshot->pixels, screenshot->pitch) != 0) {
            SDL_Log("Error while capturing screen%s", SDL_GetError());
            goto Quit;
        }
        res = screenshot;
        goto Quit;
    }

Quit:
    FreePolarLineArray(&lines); // Free point array
    FreePointArray(&points); // Free line array
    return res;
}

int testIfBlanc(SDL_Surface* img, int x, int y, int length, int angle, SDL_Renderer* renderer) {
    // Convertissez l'angle en radians
    double angleRad = angle * M_PI / 180.0;
    SDL_RenderPresent(renderer);

    // Calculez les coordonnées du vecteur direction
    double dx = cos(angleRad);
    double dy = sin(angleRad);

    // Utilisez l'algorithme de Bresenham pour parcourir la droite
    int countBlanc = 0;

    int currentX = x;
    int currentY = y;

    for (int i = 0; i <= length; ++i) {
        // Vérifiez si les coordonnées sont à l'intérieur de l'image
        if (currentX >= 0 && currentX < img->w && currentY >= 0 && currentY < img->h) {
        
            // Vérifiez si le pixel est blanc (la valeur dépend du format de surface)
            if (GetPixelLuminance(img, currentX, currentY) != 0) {
                countBlanc++;
            }
        }
        // Déplacez-vous le long de la droite
        currentX = x + i * dx;
        currentY = y + i * dy;
    }

    return countBlanc;
}


int count_accum(int x_max, int y_max, int accum[x_max][y_max]){
    int count = 0;
    for (int j = 0; j < x_max; j ++){
        for (int i = 0; i < y_max; i ++){
            if (accum[j][i] != 0)
                count ++;
        }
    }
    return count;
}

void display_accum(SDL_Renderer* renderer, SDL_Surface* img){
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, img);
    
    if (texture == NULL) {
        printf("Texture creation error : %s\n", SDL_GetError());
        return;
    }

    //SDL_RenderCopy(renderer, texture, NULL, NULL);
    
    /*SDL_Surface* screenshot = SDL_CreateRGBSurface(0, img->w, img->h, 32, 0, 0, 0, 0);
    if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, screenshot->pixels, screenshot->pitch) != 0) {
        SDL_Log("Error while capturing screen%s", SDL_GetError());
        return NULL;
    }*/

    double scale = 1.0;
    int desiredWidth = img->w;
    int desiredHeight = img->h;
    if ((img)->w > desiredWidth) scale = (double)desiredWidth / (img)->w;
    if ((img)->h * scale > desiredHeight) scale = (double)desiredHeight / (img)->h;
    int scaledWidth = (img)->w * scale;
    int scaledHeight = (img)->h * scale;
    SDL_Rect destRect = {0, 0, scaledWidth, scaledHeight};
    SDL_RenderCopy(renderer, texture, NULL, &destRect);

    SDL_RenderPresent(renderer);
}

// Same function as above but we do this in a single
SDL_Surface* ApplyHoughTransform(SDL_Surface* original_image, SDL_Surface* image_from_canny, SDL_Renderer* renderer, int threshold)
{
    DeleteBord(image_from_canny); // Delete border to increase precision
    SDL_Surface * res = NULL;
    // Initialize variable that we will use to pass in polar coordinates
    int n_y = image_from_canny->h;
    int n_x = image_from_canny->w;
    double rho = 1.0;
    double theta = 1.0;
    int n_theta = (int)(180.0/theta);
    int n_rho = (int)(floor(sqrt(n_x*n_x+n_y*n_y))/rho);
    double d_theta = M_PI/n_theta;
    double d_rho = floor(sqrt(n_x*n_x+n_y*n_y))/n_rho;
    int accum[n_theta][n_rho]; // Accumulator (Houhg space)
    int nb_lines = 0;
    // Initialize all element to 0
    for (int i = 0; i < n_theta; i++) {
        for (int j = 0; j < n_rho; j++) {
            accum[i][j] = 0;
        }
    }
    PointArray points; // Points array
    InitializeSDLPointArray(&points, 1);
    PolarLineArray lines; // Dynamic line array
    InitializePolarLineArray(&lines, 100);
    // Compute curves
    ConvertSurfaceToLineAccumulator(image_from_canny, n_theta, n_rho, accum, d_theta, d_rho, n_y, n_x);
    ApplyThresholdToAccumulator(threshold, &nb_lines, n_theta, n_rho, accum); // Apply threshold
    KeepOnlyOnePointFromGroups(&nb_lines, n_theta, n_rho, accum, rho_margin_clean, theta_margin_clean);
    ComputePolarLineCoordinates(&lines, n_theta, n_rho, accum, d_theta, d_rho);
    /*
    if (count_accum(n_theta, n_rho, accum) != 20) {
        int past_threshold = threshold;
        for (int i = 0; i < 200; ++i) {
            int tmp = threshold;
            printf("ITER : %d, NB LINES : %d, THRESHOLD = %d\n", i, count_accum(n_theta, n_rho, accum), threshold);
            if (count_accum(n_theta, n_rho, accum) < 20) {
                printf("INFÉRIEUR : ");
                if (past_threshold <= threshold)
                    threshold -= 20;
                else
                    threshold -= (past_threshold - threshold) / 2;
            } else if (count_accum(n_theta, n_rho, accum) > 20) {
                printf("SUPÉRIEUR : ");
                if (past_threshold >= threshold)
                    threshold += threshold / 6;
                else
                    threshold += (threshold - past_threshold) / 2;
            } else continue;
            printf("\n");
            past_threshold = tmp;
            // Compute curves
            for (size_t j = 0; j < lines.size; j++) {
                RemoveLineAtIndex(&lines, j);
            }

            for (int j = 0; j < n_theta; j ++){
                for (int i = 0; i < n_rho; i ++){
                    accum[j][i] = 0;
                }
            }
            //FreePolarLineArray(&lines);

            ConvertSurfaceToLineAccumulator(image_from_canny, n_theta, n_rho, accum, d_theta, d_rho, n_y, n_x);
            ApplyThresholdToAccumulator(threshold, &nb_lines, n_theta, n_rho, accum); // Apply threshold
            // Clean lines
            KeepOnlyOnePointFromGroups(&nb_lines, n_theta, n_rho, accum, rho_margin_clean, theta_margin_clean);
            // Compute polar coordinates from accumulator
            //ComputePolarLineCoordinates(&lines, n_theta, n_rho, accum, d_theta, d_rho);
            printf("ITER : %d, NB LINES : %d, THRESHOLD = %d\n", i, count_accum(n_theta, n_rho, accum), threshold);


            // DISPLAY

            for (size_t j = 0; j < lines.size; ++j) {
                RemoveLineAtIndex(&lines, j);
            }

            ComputePolarLineCoordinates(&lines, n_theta, n_rho, accum, d_theta, d_rho);

            SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, original_image);
            if (texture == NULL) {
                printf("Texture creation error : %s\n", SDL_GetError());
                return NULL;
            }
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            DrawLinesOnRenderer(renderer, lines, n_y);
            //DrawAllPoints(points, renderer);
            SDL_Surface *screenshot = SDL_CreateRGBSurface(0, original_image->w, original_image->h, 32, 0, 0, 0, 0);
            if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, screenshot->pixels, screenshot->pitch) != 0) {
                SDL_Log("Error while capturing screen%s", SDL_GetError());
                return NULL;
            }
            SDL_RenderPresent(renderer);


            res = ConvertHoughSpaceToSurface(n_theta, n_rho, accum);

            display_accum(renderer, res);
        }
    

        if (count_accum(n_theta, n_rho, accum) > 20) {
            fprintf(stderr, "Too many lines found with Hough transform, change parameters and try again\n");
            return NULL;
        } else if (count_accum(n_theta, n_rho, accum) < 20) {
            fprintf(stderr, "Too few lines found with Hough transform, change parameters and try again\n");
            return NULL;
        }
    }*/

    // Sort lines
    SortPolarLineByTheta(&lines);
    // And clean it
    CleanSortedLineTooClose(&lines);

    ComputeAllIntersectionPoints(lines, n_y, n_x, &points);
    RemoveTwinPoints(&points);

    DrawLinesOnRenderer(renderer, lines, n_y);
    DrawAllPoints(points, renderer);

    SDL_RenderPresent(renderer);

    sleep(5);

    //res = ConvertHoughSpaceToSurface(n_theta, n_rho, accum);

    printf("POINTS : %zu\n", points.size);

    /*for (size_t i = 0; i < points.size; i++)
    {
        printf("Point %d : (%d,%d)\n", i, points.data[i].x, points.data[i].y);
    }*/

    for (size_t i = 0; i < points.size; i++)
    {
        int x = points.data[i].x;
        int y = points.data[i].y;

        //if (GetPixelLuminance(image_from_canny, x, y) != 0){
            int count = 0;
            for (int k = x - 5 < 0 ? 0 : - 5; k < (x + 5 >= n_x? n_x - x : 5); k++)
            {
                for (int l = y - 5 < 0 ? 0 : -5; l < (y + 5 >= n_y ? n_y - y : 5); l++)
                {
                    int x_t = x + k;
                    int y_t = y + l;
                    int angle1 = -1;
                    int angle2 = -1;
                    int angle3 = -1;
                    for (size_t angle = 0; angle < 360; angle+= 1)
                    {
                        int current = testIfBlanc(image_from_canny, x_t, y_t, 50, angle, renderer);
                        if (current > 5){
                            if (angle1 == -1)
                                angle1 = angle;
                            else if (angle2 == -1){
                                angle2 = angle;
                            }
                            else if (angle3 == -1){
                                angle3 = angle;
                                break;
                            }
                            angle += 70;
                            count ++;
                        }
                    }
                    if (angle3 == -1 && angle1 != -1 && angle2 != -1 && abs(angle1 - angle2) < 120){
                        DrawThickPoint(renderer, x_t, y_t, 10, 0, 255, 0);
                        printf("MAX = %d\n", count);
                    }
                }
            }
        //}   
    }
    
    SDL_RenderPresent(renderer);

    sleep(3);

    //SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    //SDL_RenderClear(renderer);
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, original_image);
    
    if (texture == NULL) {
        printf("Texture creation error : %s\n", SDL_GetError());
        return NULL;
    }

    printf("HOPE2\n");


    SDL_RenderCopy(renderer, texture, NULL, NULL);
    DrawLinesOnRenderer(renderer, lines, n_y);
    DrawAllPoints(points, renderer);

    SDL_Surface *screenshot = SDL_CreateRGBSurface(0, original_image->w, original_image->h, 32, 0, 0, 0, 0);
    if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, screenshot->pixels, screenshot->pitch) != 0) {
        SDL_Log("Error while capturing screen%s", SDL_GetError());
        return NULL;
    }
    SDL_RenderPresent(renderer);

    res = screenshot;

    sleep(1);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    res = ConvertHoughSpaceToSurface(n_theta, n_rho, accum);

    sleep(2);


    FreePolarLineArray(&lines);
    FreePointArray(&points);

    return res;
}