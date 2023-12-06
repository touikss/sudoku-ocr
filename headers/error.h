#ifndef ERROR_H
#define ERROR_H

#include <err.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

/** @brief Display an error message and terminate the program.
 *
 * @param message The error message to display.
 */
void Error(const char* message);


/**
 * @brief Display an error message and exit with a failure status.
 *
 * This function prints an error message along with the SDL error (if any) and exits the program
 * with a failure status.
 *
 * @param message The custom error message to display.
 */
void Error_SDL(const char* message);


#endif 
