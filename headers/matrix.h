#ifndef MATRIX_H
#define MATRIX_H

#include "../headers/shape.h"
#include "../headers/shape_tab.h"
#include <stdio.h>


/**
 * @brief Print the matrix, indicating the presence of Shapes.
 * 
 * @param mat The matrix of Shapes.
 */
void Matrix_Print(Shape* mat[][16]);

/**
 * @brief Check if the matrix contains a specific Shape.
 * 
 * @param mat The matrix of Shapes.
 * @param shape The Shape to check for.
 * @return 1 if the Shape is in the matrix, 0 otherwise.
 */
int Matrix_Contains(Shape* mat[][16], Shape* shape);


/**
 * @brief Push all Shapes to the right in the matrix, starting from the specified column.
 * 
 * @param mat The matrix of Shapes.
 * @param start The starting column.
 */
void Matrix_PushRight(Shape* mat[][16], int start);

/**
 * @brief Push all Shapes down in the matrix, starting from the specified row.
 * 
 * @param mat The matrix of Shapes.
 * @param start The starting row.
 */
void Matrix_PushDown(Shape* mat[][16], int start);


/**
 * @brief Push the given Shape into the matrix, updating the maximum X and Y positions.
 * 
 * @param mat The matrix of Shapes.
 * @param shape The Shape to push.
 * @param maxX Pointer to the maximum X position.
 * @param maxY Pointer to the maximum Y position.
 */
int Matrix_Push(Shape* mat[][16], Shape* shape, int* maxX, int* maxY);


/**
 * @brief Check the consistency of the ShapeTab by arranging Shapes in a matrix and validating the pattern.
 * 
 * @param tab The ShapeTab to check.
 * @param grid The grid Shape (can be NULL).
 * @param mat The matrix to arrange Shapes.
 * @return 1 if the ShapeTab is consistent, 0 otherwise.
 */
int Matrix_IsConsistent(ShapeTab* tab, Shape* grid, Shape* mat[][16]);


/**
 * @brief Clears a 16x16 matrix of Shape pointers.
 *
 * This function sets all elements of the matrix to NULL.
 *
 * @param mat The 16x16 matrix of Shape pointers to be cleared.
 */
void Matrix_Clear(Shape* mat[][16]);


int Matrix_Count(Shape* mat[][16]);


Point Matrix_FindIntersection(Shape* mat[][16], int row, int col);


Point FindIntersectionBtwLines(Point A, Point B, Point C, Point D);


void Matrix_EstimateCase(Shape* mat[][16]);


int Matrix_Width(Shape* mat[][16]);

int Matrix_Height(Shape* mat[][16]);
#endif
