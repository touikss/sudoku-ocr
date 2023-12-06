#include "../headers/verbose.h"

void saveVerbose(int verbose, Image *image, char *output_folder,
                 char *output_path, int save, int free)
{
    if (save)
    {
        if (verbose)
        {
            printf("<-- 💾 Saved picture to %s/%s.bmp\n", output_folder,
                   output_path);
        }
        char output[200];
        snprintf(output, sizeof(output), "%s/%s.bmp", output_folder,
                 output_path);
        Image_Save(image, output);
    }
    if (free)
    {
        Image_Free(image);
    }
}

void printVerbose(int verbose, int gui, char *message, char *terminal)
{
    if (verbose)
    {
        printf("%s", message);
    }
    if (gui)
    {
        printf("%s\n", terminal);
    }
}
