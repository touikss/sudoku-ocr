#ifndef RESIZE_H
#define RESIZE_H

#include <math.h>
#include <stdio.h>

#include "../headers/bilinear_interpolation.h"
#include "../headers/image.h"

/**
 * @brief
 *
 * @param image
 * @param newwidth
 * @param newheight
 * @param matrix
 * @return Image
 */

Image resize(Image *image, unsigned int newwidth, unsigned int newheight);

#endif
