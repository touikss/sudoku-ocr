#ifndef SOBEL_H
#define SOBEL_H

#include <math.h>

#include "../headers/image.h"

/**
 * @brief
 *
 * @param image
 */
void SobelEdgeDetection(Image *image, int threshold);

/**
 * @brief
 *
 * @param image
 * @param kernel
 * @param row
 * @param col
 * @return double
 */
double Convolution(Image *image, double kernel[3][3], int row, int col);

#endif