#include "../headers/error.h"

void Error(const char* message)
{
    errx(EXIT_FAILURE, "%s\n", message);
}

void Error_SDL(const char* message)
{
    errx(EXIT_FAILURE, "%s: %s\n", message, SDL_GetError()[0] == '\0'? "no error from SDL" : SDL_GetError());
}