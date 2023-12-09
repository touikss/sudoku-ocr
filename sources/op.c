#include "../headers/op.h"

void init_sdl()
{
    
    
    if (SDL_Init(SDL_INIT_VIDEO) == -1)
        errx(1, "Could not initialize SDL: %s.\n", SDL_GetError());
}

SDL_Surface *load_image(char *path)
{
    SDL_Surface *img;

    
    
    img = IMG_Load(path);
    if (!img)
        errx(3, "can't load %s: %s", path, IMG_GetError());

    return img;
}

SDL_Surface *display_image(SDL_Surface *img)
{
    return img;
}

void wait_for_keypressed()
{
}

void SDL_FreeSurface(SDL_Surface *surface);
