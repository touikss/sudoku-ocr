#ifndef SUDOKUSOLVER_IMAGE_MANIPULATION_H
#define SUDOKUSOLVER_IMAGE_MANIPULATION_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

Uint32 GetPixel(SDL_Surface *image, int x, int y);

void GetPixelRGBA(SDL_Surface* image, int x, int y, Uint8 *r, Uint8 *g,Uint8 *b,Uint8 *a);

Uint8 GetPixelLuminance(SDL_Surface* image, int x, int y);

int SetPixelRGBA(SDL_Surface *image, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

void DeleteBord(SDL_Surface* image_from_canny);

void PrintImageInformation(SDL_Surface* image);

SDL_Surface* ResizeSurface(SDL_Surface* originalSurface, int newWidth, int newHeight);

int SaveSurfaceSample(SDL_Surface* sourceSurface, int x, int y, int width, int height, const char* fileName);

SDL_Surface* RotateSurface(SDL_Surface* originalSurface, double angle);

int SaveSurfaceRotated(SDL_Surface* originalSurface, double angle, char* filename, SDL_Renderer* renderer);

#endif
