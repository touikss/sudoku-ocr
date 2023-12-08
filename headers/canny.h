#ifndef IMAGEMANIPULATION
#define IMAGEMANIPULATION

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "../headers/image.h"

#define DY_MASK (1<<1)
#define DX_MASK (1<<2)
#define ORIENTATION_MASK (1<<3)
#define MAGNITUDE_MASK (1<<4)
#define STRENGTH_MASK (1<<5)

typedef struct {
    double dIx;  // Partial derivative in x direction
    double dIy;  // Partial derivative in y direction
    double magnitude;
    double orientation;
    int strength;
} Gradient;

SDL_Surface* ConvertSurfaceToGrayScale(SDL_Surface *image);

SDL_Surface* ConvertSurfaceToGaussianBlur(SDL_Surface *image, int smoothness);

int CalculateGradients(SDL_Surface *image, Gradient* gradients);

Gradient* CopyGradients(const Gradient* gradients, int width, int height);

int DeleteNonMaximum(Gradient* gradients, int width, int height);

int ApplyGradientThreshold(Gradient* gradients, int height, int width, double lowTresh, double highTresh);

int TrackHysteresisContour(Gradient* gradients, int height, int width);

SDL_Surface* ConvertGradientsToSurface(Gradient* gradients, int height, int width, int flag);

SDL_Surface* ApplyCannyFilter(SDL_Surface *image);

void Image_Rotate(Image *image, double angle);

#endif
