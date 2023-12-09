#ifndef SHAPE_TAB_H
#define SHAPE_TAB_H

#include "../headers/shape.h"
#include <stdio.h>







/** @brief Structure representing a table of shapes. */
typedef struct {
    int capacity;      /**< The capacity of the shape table. */
    int size;          /**< The current size of the shape table. */
    double averageW;   /**< Average width of the shapes in the table. */
    double averageH;   /**< Average height of the shapes in the table. */
    double averageA;   /**< Average area of the shapes in the table. */
    double stdW;       /**< Standard deviation of width in the table. */
    double stdH;       /**< Standard deviation of height in the table. */
    double stdA;       /**< Standard deviation of area in the table. */
    Shape* data;       /**< Pointer to an array of shapes. */
    int squareShapes;  
    int tiltedShapes;  
    Shape* grid;       
} ShapeTab;










/**
 * @brief Create a ShapeTab with the given capacity.
 *
 * @param capacity Capacity of the new ShapeTab.
 * @return ShapeTab* Created ShapeTab.
 */
ShapeTab* ShapeTab_Create(int capacity);


/**
 * @brief Check if a Shape is contained in a ShapeTab.
 *
 * @param tab Pointer to the ShapeTab.
 * @param shape Pointer to the Shape to check.
 * @return 1 if the Shape is present, 0 otherwise.
 */
int ShapeTab_Contains(ShapeTab* tab, Shape* shape);


/**
 * @brief Add a shape to the ShapeTab. Resize the tab if needed.
 *
 * @param tab ShapeTab to add the shape to.
 * @param shape Shape to add.
 */
void ShapeTab_AddShape(ShapeTab* tab, Shape* shape);


/**
 * @brief Removes a shape from the ShapeTab.
 * 
 * @param tab Pointer to the ShapeTab.
 * @param shape Pointer to the shape to be removed.
 * @param free Whether to free the memory occupied by the removed shape (1 to free, 0 to keep).
 */
void ShapeTab_RemoveShape(ShapeTab* tab, Shape* shape, int free);


/**
 * @brief Clear all shapes from the ShapeTab.
 *
 * @param tab ShapeTab to clear.
 */
void ShapeTab_Clear(ShapeTab* tab);


/**
 * @brief Free the memory occupied by the ShapeTab and its shapes.
 *
 * @param tab ShapeTab to free.
 */
void ShapeTab_Free(ShapeTab* tab);


/**
 * @brief Checks if the ShapeTab contains a similar shape.
 * 
 * @param tab The ShapeTab.
 * @param shape The shape to check for similarity.
 * @return 1 if a similar shape is found, 0 otherwise.
 */
int ShapeTab_ContainsSimilar(ShapeTab* tab, Shape* shape);


/**
 * @brief Paste verified shapes from the source ShapeTab to the destination ShapeTab.
 * 
 * @param src The source ShapeTab.
 * @param dst The destination ShapeTab.
 */
void ShapeTab_PasteVerified(ShapeTab* src, ShapeTab* dst);


/**
 * @brief Remove the Shape at the specified index in the ShapeTab.
 * 
 * @param tab The ShapeTab.
 * @param index The index of the Shape to remove.
 * @param free Flag to indicate whether to free the memory of the removed Shape.
 */
void ShapeTab_RemoveAtIndex(ShapeTab* tab, int index, int free);







/**
 * @brief Get the shape with the largest area in the ShapeTab.
 *
 * @param tab ShapeTab to search.
 * @return Shape* Shape with the largest area.
 */
Shape* ShapeTab_GetBiggestShape(ShapeTab* tab);


/**
 * @brief Identifies the largest shape in the given ShapeTab as the grid.
 * 
 * @param tab Pointer to the ShapeTab.
 */
void ShapeTab_FoundGrid(ShapeTab* tab);


/**
 * @brief Remove shapes in the ShapeTab that are geographically included in a specific shape.
 *
 * @param tab ShapeTab to remove shapes from.
 * @param toCheck Shape to check inclusion against.
 */
void ShapeTab_RemoveIncludedShape(ShapeTab* tab, Shape* toCheck);


/**
 * @brief Remove shapes in the ShapeTab that are not geographically included in a specific shape.
 *
 * @param tab ShapeTab to remove shapes from.
 * @param toCheck Shape to check exclusion against.
 */
void ShapeTab_RemoveExcludedShape(ShapeTab* tab, Shape* toCheck);


/**
 * @brief Remove all shapes in the ShapeTab except the largest one.
 *
 * @param tab ShapeTab to filter.
 */
void ShapeTab_KeepOnlyGrid(ShapeTab* tab);


/**
 * @brief Remove shapes from the ShapeTab that are not squares or have a size less than 100.
 *
 * @param tab ShapeTab to filter.
 */
void ShapeTab_KeepOnlySquare(ShapeTab* tab);


/**
 * @brief Keeps only the shapes in the tab that are verified based on a reference grid.
 *
 * This function compares the height and width factors of each shape in the tab
 * with those of a reference grid. If a shape's factors are outside the specified
 * range, the shape is removed from the tab.
 *
 * @param tab The ShapeTab containing the shapes to be filtered.
 */
void ShapeTab_KeepOnlyVerified(ShapeTab* tab);


void ShapeTab_RemoveInconsistency(ShapeTab* tab, Shape* grid, double factor);







/**
 * @brief Print the shape pointers in the ShapeTab for debugging.
 *
 * @param tab ShapeTab to print.
 * @param shape Shape to mark as the current shape.
 */
void ShapeTab_Print(ShapeTab* tab, Shape* shape);


/**
 * @brief Print statistical information about the ShapeTab.
 *
 * @param tab ShapeTab to print information about.
 */
void ShapeTab_PrintInfos(ShapeTab* tab);


/**
 * @brief Prints information about each shape in the given ShapeTab.
 *
 * This function iterates through the shapes in the ShapeTab and prints detailed
 * information about each shape using the Shape_PrintInfos function.
 *
 * @param tab A pointer to the ShapeTab containing the shapes.
 */
void ShapeTab_PrintShapes(ShapeTab* tab);







/**
 * @brief Compute average size, height, and width for the shapes in a ShapeTab.
 *
 * @param tab Pointer to the ShapeTab.
 * @param toExclude Pointer to the Shape to exclude from the computation (can be NULL).
 */
void ShapeTab_ComputeAverage(ShapeTab* tab, Shape* toExclude);


/**
 * @brief Compute standard deviation of size, height, and width for the shapes in a ShapeTab.
 *
 * @param tab Pointer to the ShapeTab.
 * @param toExclude Pointer to the Shape to exclude from the computation (can be NULL).
 */
void ShapeTab_ComputeStandardDeviation(ShapeTab* tab, Shape* toExclude);


/**
 * @brief Computes descriptors for all shapes in the given ShapeTab.
 *
 * This function iterates through all shapes in the ShapeTab and calls Shape_ComputeDescriptors
 * to calculate descriptors for each shape based on the provided image dimensions.
 *
 * @param tab          Pointer to the ShapeTab containing the shapes.
 * @param imageWidth   Width of the image used for descriptor computation.
 * @param imageHeight  Height of the image used for descriptor computation.
 */
void ShapeTab_ComputeDescriptors(ShapeTab* tab);


/**
 * @brief Update the 'isSquare' property for all shapes in a ShapeTab.
 * 
 * This function iterates through all shapes in the ShapeTab and updates
 * their 'isSquare' property using the Shape_UpdateIsSquare function.
 * 
 * @param tab A pointer to the ShapeTab containing the shapes to update.
 */
void ShapeTab_UpdateIsSquare(ShapeTab* tab);


/**
 * @brief Updates the count of squares and tilted shapes in a list of shapes.
 * 
 * @param tab List of shapes.
 */
void ShapeTab_UpdateCountSquaresAndTilted(ShapeTab* tab);


double ShapeTab_ComputeAngle(ShapeTab* tab);

void ShapeTab_SetTilted(ShapeTab* tab);


#endif 