#include "../headers/rotations.h"

void rotate(Image *image, double angleDegree)
{
    const unsigned int width = image->width;
    const unsigned int height = image->height;

    const double middleX = ((double)width / 2.0);
    const double middleY = ((double)height / 2.0);

    const double angle = angleDegree * M_PI / 180.0;

    
    Pixel **_pixels = Image_CopyPixelArray(image);
    
    for (unsigned int x = 0; x < width; x++)
    {
        for (unsigned int y = 0; y < height; y++)
        {
            
            Pixel_Set(&(image->pixels[x][y]), 0);
        }
    }

    double newX;
    double newY;
    
    unsigned int top;
    unsigned int bottom;
    unsigned int left;
    unsigned int right;
    for (unsigned int x = 0; x < width; x++)
    {
        for (unsigned int y = 0; y < height; y++)
        {
            
            newX = ((double)(cos(angle) * ((double)x - middleX)
                             - sin(angle) * ((double)y - middleY))
                    + middleX);
            newY = ((double)(cos(angle) * ((double)y - middleY)
                             + sin(angle) * ((double)x - middleX))
                    + middleY);

            
            
            top = floor(newY);
            bottom = top + 1;
            left = floor(newX);
            right = left + 1;

            
            
            
            
            if (top < height && bottom < height && left < width
                && right < width)
            {
                unsigned int interpolated = bilinearly_interpolate(
                    top, bottom, left, right, newX, newY, _pixels);
                Pixel_Set(&(image->pixels[x][y]), interpolated);
            }
        }
    }

    for (unsigned int i = 0; i < width; i++)
    {
        free(_pixels[i]);
    }
    free(_pixels);
}