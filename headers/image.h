#ifndef IMAGE_H
#define IMAGE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

//#include "../headers/op.h"
#include "../headers/pixel_operations.h"
#include "../headers/shape.h"
#include "../headers/shape_tab.h"


/** @file
 * @brief All you need to manipulate images.
 */

/////////////////////////////////////////////////////////////////////////////////
////////////////////////////STRUCTURES DEFINITION////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


/** @brief Structure representing a pixel with red (r), green (g), and blue (b) components. */
typedef struct Pixel
{
    Uint8 r, g, b;
} Pixel;


/**
 * @brief Represents a color using RGB values.
 */
typedef struct Color
{
    Uint8 r; /**< Red component of the color. */
    Uint8 g; /**< Green component of the color. */
    Uint8 b; /**< Blue component of the color. */
} Color;


/** @brief Structure representing an image with width, height, average color, histogram, pixels, and path. */
typedef struct
{
    unsigned int width;    /**< Width of the image. */
    unsigned int height;   /**< Height of the image. */
    double averageColor;   /**< Average color of the image. */
    int histogram[256];    /**< Histogram of pixel intensities. */
    Pixel **pixels;        /**< 2D array of pixels representing the image. */
    char *path;            /**< File path of the image. */
} Image;


/////////////////////////////////////////////////////////////////////////////////
////////////////////////////      MACROS      ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


#define RED    ((Color){255, 0, 0})
#define GREEN  ((Color){0, 255, 0})
#define BLUE   ((Color){0, 0, 255})
#define WHITE  ((Color){255, 255, 255})
#define PURPLE ((Color){255, 0, 255})
#define ORANGE ((Color){255, 100, 0})


/////////////////////////////////////////////////////////////////////////////////
////////////////////////////FUNCTIONS PROTOTYPE//////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//////////////////////////////    BASICS    /////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


/**
 * @brief Set all RGB components of a pixel to a given value.
 *
 * @param pixel Pointer to the Pixel structure.
 * @param value The value to set for each component.
 */
void Pixel_Set(Pixel *pixel, Uint8 value);


/**
 * @brief Create a black image with the specified width and height.
 *
 * @param width Width of the image.
 * @param height Height of the image.
 * @return Image representing a black image.
 */
Image Image_CreateBlack(unsigned int width, unsigned int height);


/**
 * @brief Create an Image structure from an SDL_Surface.
 *
 * @param surface SDL_Surface representing an image.
 * @param width Width of the image.
 * @param height Height of the image.
 * @return Image structure created from the SDL_Surface.
 */
Image Image_CreateFromSurface(SDL_Surface *surface, unsigned int width, unsigned int height);


/**
 * @brief Create an SDL_Surface from an Image structure.
 *
 * @param image Pointer to the Image structure.
 * @return SDL_Surface created from the Image.
 */
SDL_Surface *Image_CreateSurface(Image *image);


/**
 * @brief Copy the pixel array of an Image structure.
 *
 * @param image Pointer to the Image structure.
 * @return Copied pixel array.
 */
Pixel **Image_CopyPixelArray(Image *image);


/**
 * @brief Free memory allocated for an Image structure.
 *
 * @param image Pointer to the Image structure.
 */
void Image_Free(Image *image);


/**
 * @brief Clear the image by setting all pixel values to black.
 * @param image Pointer to the Image structure.
 */
void Image_Clear(Image* image);


/////////////////////////////////////////////////////////////////////////////////
////////////////////////////    MANIPULATION    /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


/**
 * @brief Crop an image based on a given rectangle.
 *
 * @param image Pointer to the original Image structure.
 * @param rect Pointer to the SDL_Rect specifying the region to crop.
 * @return Image representing the cropped region.
 */
Image Image_Crop(Image *image, SDL_Rect *rect);


/**
 * @brief Paste one image onto another at a specified rectangle.
 *
 * @param src Pointer to the source Image structure.
 * @param dest Pointer to the destination Image structure.
 * @param rect Pointer to the SDL_Rect specifying the region to paste onto in the destination.
 */
void Image_PasteOn(Image *src, Image *dest, SDL_Rect *rect);


/**
 * @brief Clone the content of one image into another.
 *
 * @param src Pointer to the source Image structure.
 * @param dst Pointer to the destination Image structure.
 */
void Image_Clone(Image *src, Image *dst);


/**
 * @brief Copy an Image structure.
 *
 * @param image Pointer to the Image structure to be copied.
 * @return Copied Image structure.
 */
Image Image_Copy(Image *image);


/**
 * @brief Save the Image structure to a BMP file.
 *
 * @param image Pointer to the Image structure.
 * @param path File path to save the image.
 */
void Image_Save(Image *image, char *path);


/////////////////////////////////////////////////////////////////////////////////
////////////////////////////      FILTERS       /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


/**
 * @brief Apply a Gaussian filter (blur) to an image.
 * Update Histogram.
 *
 * @param image Pointer to the Image structure.
 * @param smoothness The degree of smoothing (how many pixels to use).
 */
void Image_GaussianFilter(Image *image, int smoothness);


void Image_GaussianFilter1(Image *image, int smoothness);


/**
 * @brief Apply a median filter to an image and update the histogram.
 *
 * @param image Pointer to the Image structure.
 */
void Image_MedianFilter(Image *image);


/**
 * @brief Apply threshold to an image. Keep only pixels higher than "threshold".
 * @param image Pointer to the Image structure.
 * @param threshold Threshold value for pixel intensity.
 */
void Image_ThresholdFilter(Image* image, int threshold);


/**
 * @brief Increase contrast of image by "contrastFactor".
 *
 * @param image Pointer to the Image structure.
 * @param contrastFactor Factor to adjust contrast.
 * @param lowThreshold Lower threshold for pixel values.
 * @param highThreshold Upper threshold for pixel values.
 * @param exposure Exposure adjustment value.
 */
void Image_ContrastFilter(Image* image, double contrastFactor, int lowThreshold, int highThreshold, int exposure);


/**
 * @brief Invert the colors of the image.
 * @param image Pointer to the Image structure.
 */
void Image_Invert(Image* image);


/////////////////////////////////////////////////////////////////////////////////
//////////////////////////// HISTOGRAM MANIPULATION /////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


/**
 * @brief Extend histogram of image to use the full color range.
 *
 * @param image Pointer to the Image structure.
 */
void Image_NormalizeHistogram(Image* image);


/**
 * @brief Smooth the curve of an histogram (do not change the image).
 * Needed for histogram equalization.
 * @param image Pointer to the Image structure.
 * @param windowSize Size of the window used to compute the average and smooth the histogram.
 */
void Image_SmoothHistogram(Image* image, int windowSize);


/**
 * @brief Equalize the histogram of an image by modifying it.
 * Assumes that the histogram is already smoothed.
 * @param image Pointer to the Image structure.
 */
void Image_EqualizeHistogram(Image* image);


/**
 * @brief Levelizes the image pixel values into n levels.
 * 
 * @param image The input image.
 * @param n The number of levels for levelization.
 */
void Image_Levelize(Image* image, size_t n);


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////    EDGES MANIPULATION     /////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


/**
 * @brief Erode an image by reducing edge thickness and removing alone pixels.
 * @param image Pointer to the Image structure.
 */
void Image_ErodeEdges(Image* image);


/**
 * @brief Dilate edges of an image by increasing their thickness.
 * @param image Pointer to the Image structure.
 * @param n Thickness factor.
 */
void Image_DilateEdges(Image* image, int n);


/////////////////////////////////////////////////////////////////////////////////
////////////////////////////      DRAW       ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


/**
 * @brief Draws a colored point on the image.
 *
 * @param image Pointer to the image.
 * @param point Coordinates of the point.
 * @param color Color of the point.
 * @param thickness Thickness of the point.
 */
void Image_DrawPoint(Image* image, Point point, Color color, int thickness);


/**
 * @brief Draws the extremum points of shapes on the image with a specified color.
 *
 * @param image Pointer to the image.
 * @param tab Pointer to the shape tab containing shapes.
 * @param color Color of the extremum points.
 */
void Image_DrawShapeExtremum(Image* image, ShapeTab* tab, Color color);


/**
 * @brief Draws the average points of shapes on the image with a specified color.
 *
 * @param image Pointer to the image.
 * @param tab Pointer to the shape tab containing shapes.
 * @param color Color of the average points.
 */
void Image_DrawAveragePoints(Image* image, ShapeTab* tab, Color color);


/**
 * @brief Draws a shape on the image with a specified color.
 *
 * @param image Pointer to the image.
 * @param shape Pointer to the shape to be drawn.
 * @param color Color of the shape.
 */
void Image_DrawShape(Image* image, Shape* shape, Color color);


/**
 * @brief Draw shapes from the ShapeTab on an image with varying colors.
 *
 * @param image Image to draw on.
 * @param tab ShapeTab containing shapes to draw.
 */
void Image_DrawShapeTab(Image* image, ShapeTab* tab);


/**
 * @brief Draws a line between two points on the image with a specified color.
 *
 * @param image Pointer to the image.
 * @param p1 Starting point of the line.
 * @param p2 Ending point of the line.
 * @param color Color of the line.
 */
void Image_DrawLine(Image* image, Point p1, Point p2, Color color); 


/**
 * @brief Draws lines connecting the extremum points of shapes in the provided ShapeTab on the given Image.
 *
 * This function iterates through shapes in the ShapeTab and draws lines connecting the extremum
 * points of each shape on the provided Image.
 *
 * @param image  Pointer to the Image where the extremum point lines will be drawn.
 * @param tab    Pointer to the ShapeTab containing the shapes.
 */
void Image_DrawDescriptorsLinks(Image* image, ShapeTab* tab);


void Image_DrawShapePoint(Image* image, Shape* shape, Color color);


void Image_DrawMatrix(Image* image, Shape* mat[][16]);

/////////////////////////////////////////////////////////////////////////////////
////////////////////////////   MASS DETECTION    ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


/**
 * @brief Label all components of an image with color, excluding those that are not squares.
 * @param image Pointer to the Image structure.
 * @param shapeTab Pointer to the ShapeTab structure.
 */
void Image_LabelComponent(Image* image, ShapeTab* shapeTab);


/**
 * @brief Find squares in labeled image.
 * @param image Pointer to the Image structure.
 */
void Image_FindSquareInLabeled(Image* image);


/**
 * @brief Find cases in the labeled image and store their dimensions in the tab array.
 * @param image Pointer to the Image structure.
 * @param labeledImage Pointer to the labeled Image structure.
 * @param tab Array to store the dimensions of cases.
 */
void findCases(SDL_Surface* image, SDL_Surface* labeledImage, Point tab[180]);


/**
 * @brief Create a rectangle around the white pixels in the image.
 * @param image Pointer to the Image structure.
 */
void Image_RectCase(Image* image);


void Image_UpdateHistogram(Image* image);

void Image_HighPassFilter(Image* image, int threshold);

#endif