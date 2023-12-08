#pragma once

#include "../headers/matrix_n.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

typedef struct {
	Matrix* img_data;
	int label;
} Img;

void bmp_to_csv(const char *bmp_file, const char *csv_file);
Img** csv_to_imgs(char* file_string, int number_of_imgs);
void img_print(Img* img);
void img_free(Img *img);
void imgs_free(Img **imgs, int n);

// added
typedef struct {
    uint8_t value;
} Pixel3;

typedef struct {
    Pixel3 pixels[28][28];
} Image3;