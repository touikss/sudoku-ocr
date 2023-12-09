#ifndef IMAGE_2_H
#define IMAGE_2_H



#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "geometry.h"

struct Pixel2
{
    Uint8 r, g, b;
};
typedef struct Pixel2 Pixel2;

struct Image2
{
    int height;
    int width;
    SDL_Surface *surface;
    Pixel2 **pixels;
    double average_color;
};
typedef struct Image2 Image2;

GdkPixbuf *image_to_pixbuf(Image2 *image);

Image2 *pixbuf_to_image(GdkPixbuf *pixbuf);

SDL_Surface *load_image(char *path);

void save_image(SDL_Surface *image_surface, char *path);

Image2 clone_image(Image2 *source);

Image2 SDL_Surface_to_Image2(SDL_Surface *image_surface);

SDL_Surface *Image2_to_SDL_Surface(Image2 *image);

Image2 *create_image(int height, int width);

void free_Image2(Image2 *image);

void image_filter(Image2 *image, void (*filter)(Pixel2 *, int), double value);

void image_partial_filter(Image2 *image, void (*filter)(Pixel2 *, int),
    double value, int startx, int endx, int starty, int endy);

int *image_grayscale_histogram(
    Image2 *image, int startx, int endx, int starty, int endy);

int get_histogram_min(int *hist);

int get_histogram_max(int *hist);

int *Image2_to_Array(Image2 *source);

void Array_to_Image2(int *array, Image2 *container);

Image2 Array2D_to_Image2(int **array, int width, int height);

int *draw_line(Image2 *image, int w, int h, int x0, int y0, int x1, int y1,
    Uint8 r, Uint8 g, Uint8 b);

void draw_square(Image2 *image, square *sqr, int r, int g, int b);

Image2 *crop_image(Image2 *input, square *crop);

Image2 *downscale_image(Image2 *input, int size);

#endif