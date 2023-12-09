#include "../headers/image_manipulation.h"

/*Print some useful information about "image" and its format on the output stream.*/
void PrintImageInformation(SDL_Surface* image)
{
    SDL_PixelFormat* format = image->format;
    printf("Surface format :\n");
    printf("Bits per pixel : %d\n", format->BitsPerPixel);
    printf("Red (mask)   : 0x%.8X\n", format->Rmask);
    printf("Green (mask)    : 0x%.8X\n", format->Gmask);
    printf("Blue (mask)    : 0x%.8X\n", format->Bmask);
    printf("Alpha (mask)   : 0x%.8X\n", format->Amask);
    printf("Bytes per pixel : %d\n", format->BytesPerPixel);
    printf("Format : %u\n", format->format);
    printf("Bytes order: ");
    if (format->Rshift < format->Bshift) {
        printf("ABGR\n");
    } else {
        printf("RGBA\n");
    }
    printf("Dimensions : %d x %d\n\n", image->h, image->w);
}

/*Return value of pixel (encoded as an unsigned int on 32 bits)
 * at (x,y) position in "image".
 * Return EXIT_FAILURE if an error occurred.*/
Uint32 GetPixel(SDL_Surface *image, int x, int y)
{
    Uint32 pixel = *((Uint32 *)image->pixels + y * image->pitch / 4 + x);
    return pixel; 
}

/*Give to r, g, b and a the value of red, green, blue and alpha of
 * the image's pixel at position (x,y).
 * Return EXIT_FAILURE if an error occurred 0 otherwise.*/
void GetPixelRGBA(SDL_Surface* image, int x, int y, Uint8 *r, Uint8 *g,Uint8 *b,Uint8 *a)
{
    Uint32 pixel = GetPixel(image, x, y);
    SDL_GetRGBA(pixel, image->format, r, g, b, a);
}

/*Return the value of luminance of the image's pixel at position (x,y).
 * Return EXIT_FAILURE if an error occurred.*/
Uint8 GetPixelLuminance(SDL_Surface* image, int x, int y)
{
    Uint32 pixel = GetPixel(image, x, y);
    Uint8 r,g,b,a;
    SDL_GetRGBA(pixel, image->format, &r, &g, &b, &a);
    return (Uint8)(0.299 * r + 0.587 * g + 0.114 * b);
}

/*
    Assign to the pixel of "image" at position (x,y) the values r,g,b and a.
    Return EXIT_FAILURE if an error occurred, 0 otherwise.
*/
int SetPixelRGBA(SDL_Surface *image, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    Uint32 new_pixel = SDL_MapRGBA(image->format, r, g, b, a);
    *((Uint32 *)image->pixels + y * image->pitch / 4 + x) = new_pixel;
    return EXIT_SUCCESS; 
}


void DeleteBord(SDL_Surface* image_from_canny){
    for (int x = 0; x < 3; x ++){
        for (int y = 0; y < image_from_canny->h; y++) {
            SetPixelRGBA(image_from_canny, x, y, 0, 0, 0, 0);
        }
    }
    for (int x = image_from_canny->w - 3; x < image_from_canny->w; x ++) {
        for (int y = 0; y < image_from_canny->h; y++) {
            SetPixelRGBA(image_from_canny, x, y, 0, 0, 0, 0);
        }
    }

    for (int y = 0; y < 3; y ++){
        for (int x = 0; x < image_from_canny->w; x++) {
            SetPixelRGBA(image_from_canny, x, y, 0, 0, 0, 0);
        }
    }
    for (int y = image_from_canny->h - 3; y < image_from_canny->h; y ++) {
        for (int x = 0; x < image_from_canny->w; x++) {
            SetPixelRGBA(image_from_canny, x, y, 0, 0, 0, 0);
        }
    }
}


SDL_Surface* ResizeSurface(SDL_Surface* original_surface, int new_width, int new_height) {
    int original_width = original_surface->w;
    int original_height = original_surface->h;

    
    float aspect_ratio = (float)original_width / (float)original_height;

    
    if (aspect_ratio < 1.0f) {
        printf("OUI OUI OUI ");
        
        new_height = (int)((float)new_width / aspect_ratio);
    } else {
        
        new_width = (int)((float)new_height * aspect_ratio);
    }

    

    
    SDL_Surface* new_surface = SDL_CreateRGBSurface(0, new_width, new_height, original_surface->format->BitsPerPixel, original_surface->format->Rmask, original_surface->format->Gmask, original_surface->format->Bmask, original_surface->format->Amask);

    
    if (SDL_BlitScaled(original_surface, NULL, new_surface, NULL) != 0) {
        fprintf(stderr, "Error while resizing surface : %s\n", SDL_GetError());
        SDL_FreeSurface(new_surface);
        return NULL;
    }
    SDL_FreeSurface(original_surface);

    return new_surface;
}


int SaveSurfaceSample(SDL_Surface* source_surface, int x, int y, int width, int height, const char* file_name) {
    
    if (source_surface == NULL) {
        return EXIT_FAILURE;
    }
    
    SDL_Surface* sample_surface = SDL_CreateRGBSurface(0, width, height, source_surface->format->BitsPerPixel,
                                                      source_surface->format->Rmask, source_surface->format->Gmask,
                                                      source_surface->format->Bmask, source_surface->format->Amask);

    if (sample_surface == NULL) {
        return EXIT_FAILURE;
    }

    
    SDL_Rect sample_rect = {x, y, width, height};
    if (SDL_BlitSurface(source_surface, &sample_rect, sample_surface, NULL) != 0)
        return EXIT_FAILURE;

    
    if (IMG_SavePNG(sample_surface, file_name) != 0) {
        SDL_FreeSurface(sample_surface);
        return EXIT_FAILURE;
    }

    SDL_FreeSurface(sample_surface);
    return EXIT_SUCCESS;
}


SDL_Surface* RotateSurface(SDL_Surface* original_surface, double angle) {
    
    int original_width = original_surface->w;
    int original_height = original_surface->h;

    
    double angle_radians = angle * M_PI / 180.0;
    int dest_width = (int)(original_width * fabs(cos(angle_radians)) + original_height * fabs(sin(angle_radians)));
    int dest_height = (int)(original_height * fabs(cos(angle_radians)) + original_width * fabs(sin(angle_radians)));

    
    SDL_Surface* rotated_surface = SDL_CreateRGBSurfaceWithFormat(0, dest_width, dest_height, original_surface->format->BitsPerPixel, original_surface->format->format);

    if (rotated_surface == NULL)
        return NULL;

    
    Uint8 r, g, b, a;
    GetPixelRGBA(original_surface, original_surface->w - 5, original_surface->h - 5, &r, &g, &b, &a);

    
    SDL_FillRect(rotated_surface, NULL, SDL_MapRGB(rotated_surface->format, r, g, b));

    
    int xOffset = (dest_width - original_width) / 2;
    int yOffset = (dest_height - original_height) / 2;

    
    SDL_Rect src_rec = {0, 0, original_width, original_height};
    SDL_Rect dst_rec = {xOffset, yOffset, original_width, original_height};

    
    if (SDL_BlitSurface(original_surface, &src_rec, rotated_surface, &dst_rec) != 0)
        return NULL;

    return rotated_surface;
}


int SaveSurfaceRotated(SDL_Surface* original_surface, double angle, char* filename, SDL_Renderer* renderer){

    SDL_Surface* rotated_surface = RotateSurface(original_surface, angle);
    
    if (rotated_surface == NULL)
        return EXIT_FAILURE;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, rotated_surface);
    SDL_FreeSurface(rotated_surface); 
    
    if (texture == NULL)
        return EXIT_FAILURE;
    
    
    Uint8 r, g, b, a;
    GetPixelRGBA(original_surface, original_surface->w - 5, original_surface->h - 5, &r, &g, &b, &a);
    
    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    
    SDL_RenderClear(renderer);

    
    SDL_RenderCopyEx(renderer, texture, NULL, NULL, angle, NULL, SDL_FLIP_NONE);
    
    
    SDL_Surface *screenshot = SDL_CreateRGBSurface(0, original_surface->w, original_surface->h, 32, 0, 0, 0, 0);
    if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, screenshot->pixels, screenshot->pitch) != 0) {
        SDL_Log("Error while capturing screen %s\n:", SDL_GetError());
        return EXIT_FAILURE;
    }

    if (SDL_SaveBMP(screenshot, filename) != 0) {
        printf("Error while saving image : %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}