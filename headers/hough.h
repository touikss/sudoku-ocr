#ifndef SUDOKUSOLVER_HOUGH_H
#define SUDOKUSOLVER_HOUGH_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>


typedef struct {
    double rho;
    double theta;
}PolarLine;


typedef struct {
    SDL_Point* data;
    size_t size;
    size_t capacity;
} PointArray;


typedef struct {
    PolarLine* data;
    size_t size;
    size_t capacity;
} PolarLineArray;


SDL_Surface* ApplyHouhgTransformByStep(SDL_Surface* original_image, SDL_Surface* image_from_canny, SDL_Renderer* renderer, int threshold, int step);

SDL_Surface* ApplyHoughTransform(SDL_Surface* original_image, SDL_Surface* image_from_canny, SDL_Renderer* renderer, int threshold);

#endif 
