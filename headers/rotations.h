#ifndef ROTATIONS_H
#define ROTATIONS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>
#include <math.h>
#include <stdio.h>

#include "../headers/bilinear_interpolation.h"
#include "../headers/image.h"
//#include "Imagery/Utils/noise_reduction.h"

/**
 * @brief Rotate an image
 * @param Image The image to rotate
 * @param angleDegree The angle of rotation in degree
 * @return The rotated image
 */

void rotate(Image *image, double angleDegree);

#endif
