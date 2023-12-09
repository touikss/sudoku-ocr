#ifndef SHAPE_H
#define SHAPE_H

#include "../headers/error.h"
#include <math.h>
#include <stdlib.h>

/////////////////////////////////////////////////////////////////////////////////
////////////////////////////STRUCTURES DEFINITION////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

// Definition of the Point structure with integer coordinates x and y.
typedef struct {
    unsigned int x;  ///< X-coordinate of the point.
    unsigned int y;  ///< Y-coordinate of the point.
} Point;

// Definition of the Shape structure representing a shape with various properties.
typedef struct Shape{
    int isSquare;           ///< Indicates if the shape is a square (1) or not (0).
    int isTilted;           ///< Indicates if the shape is tilted or not (to care only if it is a square)
    unsigned int height;    ///< Height of the shape.*/
    unsigned int width;     ///< Width of the shape.
    unsigned int size;      ///< Size of the shape (may represent the total number of points, for example).
    unsigned int capacity;  ///< Initial capacity of the shape in pixels.
    unsigned int extremum[4];        ///< Array of extremes: [0] = x min, [1] = x max, [2] = y min, [3] = y max.
    Point descriptor[8];   ///< Array of point descriptors.
    double averageX;        ///< Average of the X coordinates of the shape's points.
    double averageY;        ///< Average of the Y coordinates of the shape's points.
    Point* points;          ///< Pointer to a dynamically allocated array of points representing the shape.
    struct Shape* next;     ///< Pointer to the next shape in a linked data structure.
} Shape;


/////////////////////////////////////////////////////////////////////////////////
////////////////////////////FUNCTIONS PROTOTYPE//////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


/** @brief Calculate the Euclidean distance between two coordinates.
 *
 * @param x1 The x-coordinate of the first point.
 * @param y1 The y-coordinate of the first point.
 * @param x2 The x-coordinate of the second point.
 * @param y2 The y-coordinate of the second point.
 * @return The Euclidean distance between the two points.
 */
double CoordinateDistance(int x1, int y1, int x2, int y2);


/** @brief Calculate the Euclidean distance between two points.
 *
 * @param point1 The first point.
 * @param point2 The second point.
 * @return The Euclidean distance between the two points.
 */
double Point_Distance(Point point1, Point point2);


/**
 * @brief Calculate the angle between two points.
 * 
 * @param p1 The first point.
 * @param p2 The second point.
 * @return double The calculated angle.
 */
double Point_Angle(Point p1, Point p2);


/** @brief Create a shape with the given pixel capacity.
 *
 * @param capacity The initial capacity of the shape in pixels.
 * @param maxWidth The maximum width of the shape.
 * @param maxHeight The maximum height of the shape.
 * @return A pointer to the created shape.
 */
Shape* Shape_Create(int capacity, int maxWidth, int maxHeight);

/** @brief Add a point to a shape and resize it if needed.
 * Update extremums and dimensions if needed.
 *
 * @param shape The shape to which the point is added.
 * @param x The x-coordinate of the new point.
 * @param y The y-coordinate of the new point.
 */
void Shape_AddPoint(Shape* shape, unsigned int x, unsigned int y);

/** @brief Compare extremum of two shapes and return 1 if container contains the content, 0 otherwise.
 *
 * @param container The potential container shape.
 * @param content The shape to check for inclusion.
 * @return 1 if the container contains the content, 0 otherwise.
 */
int Shape_IsIncluded(Shape* container, Shape* content);

/** @brief Free a shape and all its points.
 *
 * @param shape The shape to be freed.
 */
void Shape_Free(Shape* shape);


/** @brief Compute descriptors for a shape based on the image dimensions.
 *
 * @param shape The shape for which to compute descriptors.
 * @param imageWidth The width of the image.
 * @param imageHeight The height of the image.
 */
void Shape_ComputeDescriptors(Shape* shape);


/** @brief Compute angle of director vector represented by two points (in radians).*/
double PointAngle(int x1, int y1, int x2, int y2);


/** @brief Compute scale factor from height and width and return
 * the max between height/width and width/height*/
double ScaleFactor(double height, double width);


/**
 * @brief Prints information about a shape to the console.
 *
 * @param shape Pointer to the Shape structure to print information about.
 */
void Shape_PrintInfos(Shape* shape);


/**
 * @brief Update the 'isSquare' property of a Shape based on its descriptors.
 * 
 * This function checks if the shape is square by comparing the distances between
 * its descriptors. If the distances between opposite sides are approximately equal,
 * the shape is considered a square.
 * 
 * @param shape A pointer to the Shape to update.
 */
void Shape_UpdateIsSquare(Shape* shape);


/**
 * @brief Check if src is to the right of cmp.
 * 
 * @param src The source Shape.
 * @param cmp The comparison Shape.
 * @return 1 if src is to the right of cmp, 0 otherwise.
 */
int Shape_IsRight(Shape* src, Shape* cmp);


/**
 * @brief Check if src is to the left of cmp.
 * 
 * @param src The source Shape.
 * @param cmp The comparison Shape.
 * @return 1 if src is to the left of cmp, 0 otherwise.
 */
int Shape_IsLeft(Shape* src, Shape* cmp);


/**
 * @brief Check if src is above cmp.
 * 
 * @param src The source Shape.
 * @param cmp The comparison Shape.
 * @return 1 if src is above cmp, 0 otherwise.
 */
int Shape_IsAbove(Shape* src, Shape* cmp);


/**
 * @brief Check if src is below cmp.
 * 
 * @param src The source Shape.
 * @param cmp The comparison Shape.
 * @return 1 if src is below cmp, 0 otherwise.
 */
int Shape_IsBelow(Shape* src, Shape* cmp);


/**
 * @brief Compute the average angle of the given shape.
 * 
 * This function calculates the average angle of the provided quadrilateral.
 * If the shape is tilted, it considers the angles formed by adjacent points.
 * If the shape is not tilted, it considers the angles formed by opposite sides.
 * 
 * @param shape - Pointer to the shape for which to compute the angle.
 * @return The average angle of the shape in radians.
 */
double Shape_ComputeAngle(Shape* shape);


Shape* Shape_Copy(Shape* tcpy);


void Shape_UpdateAveragePointAndDimension(Shape* shape);


void Shape_Translate(Shape* shape, int dx, int dy);


#endif