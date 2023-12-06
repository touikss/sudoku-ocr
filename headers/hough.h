#ifndef SUDOKUSOLVER_HOUGH_H
#define SUDOKUSOLVER_HOUGH_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Struct for line in polar coordinates
typedef struct {
    double rho;
    double theta;
}PolarLine;

// Struct for a dynamic tab of SDL_Point
typedef struct {
    SDL_Point* data;
    size_t size;
    size_t capacity;
} PointArray;

// Struct for a dynamic tab of PolarLine
typedef struct {
    PolarLine* data;
    size_t size;
    size_t capacity;
} PolarLineArray;


SDL_Surface* ApplyHouhgTransformByStep(SDL_Surface* original_image, SDL_Surface* image_from_canny, SDL_Renderer* renderer, int threshold, int step);

SDL_Surface* ApplyHoughTransform(SDL_Surface* original_image, SDL_Surface* image_from_canny, SDL_Renderer* renderer, int threshold);

#endif //SUDOKUSOLVER_HOUGH_H
