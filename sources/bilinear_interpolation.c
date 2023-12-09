#include "../headers/bilinear_interpolation.h"

double bilinearly_interpolate(unsigned int top, unsigned int bottom,
                              unsigned int left, unsigned int right,
                              double horizontal_position,
                              double vertical_position, Pixel **pixels)
{
    
    double top_left = pixels[left][top].r;
    double top_right = pixels[right][top].r;
    double bottom_left = pixels[left][bottom].r;
    double bottom_right = pixels[right][bottom].r;

    
    
    double horizontal_progress = horizontal_position - (double)left;
    double vertical_progress = vertical_position - (double)top;

    
    
    double top_block = top_left + horizontal_progress * (top_right - top_left);

    
    
    double bottom_block =
        bottom_left + horizontal_progress * (bottom_right - bottom_left);

    
    
    return top_block + vertical_progress * (bottom_block - top_block);
}