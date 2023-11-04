#include "../headers/canny.h"
#include "../headers/image_manipulation.h"

/*
    Return "image" modified in gray scale.
    Return NULL if an error occurred;
*/
SDL_Surface* ConvertSurfaceToGrayScale(SDL_Surface *image)
{
    if (image == NULL){
        fprintf(stderr, "Parameter \"image\" in ConvertSurfaceToGrayScale() is NULL\n");
        return NULL; // Error
    }

    for (int y = 0; y < image->h; y += 1) {
        for (int x = 0; x < image->w; x += 1) {
            Uint8 r,g,b,a;
            GetPixelRGBA(image, x, y, &r, &g, &b, &a);
            Uint8 gray = r * 0.2989 + g * 0.5870 + b * 0.1140; // Create gray value from rgb
            SetPixelRGBA(image, x, y, gray, gray, gray, a); // Assign value to (x,y) pixel
        }
    }
    return image; // OK
}

/*
    Return a copy of "image" with a Gaussian blur applied with a standard deviation of "smoothness".
    Return NLL if an error occurred.
*/
SDL_Surface* ConvertSurfaceToGaussianBlur(SDL_Surface *image, int smoothness)
{
    if (image == NULL){
        fprintf(stderr, "Parameter \"image\" in ConvertSurfaceToGaussianBlur() is NULL\n");
        return NULL; // Error
    }
    if (smoothness <= 0)
    {
        fprintf(stderr, "Parameter \"n\" in ConvertSurfaceToGaussianBlur() is less than 0\n");
        return NULL; // Error
    }

    SDL_Surface *blured_image = SDL_CreateRGBSurface(0, image->w, image->h, 32,
                                                     0, 0, 0, 0);
    if (blured_image == NULL)
    {
        fprintf(stderr, "Error in ConvertSurfaceToGaussianBlur(): %s\n", SDL_GetError());
        return NULL; // Error
    }

    // Compute the number of pixel that we will use to compute the new pixel value
    const int pixel_count = (2 * smoothness + 1) * (2 * smoothness + 1);

    for (int x = 0; x < image->w; x ++){
        for (int y = 0; y < image->h; y ++){
            int r = 0;
            int g = 0;
            int b = 0;
            int a = 0;
            // Browse pixels around
            for (int i = x - smoothness; i <= x + smoothness; i ++){
                for (int j = y - smoothness; j <= y + smoothness; j ++){
                    if (j >= 0 && i >= 0 && j < image->h && i < image->w){
                        Uint8 r1,g1,b1,a1;
                        GetPixelRGBA(image, i, j, &r1, &g1, &b1, &a1);
                        r += r1;
                        g += g1;
                        b += b1;
                        a += a1;
                    }
                }
            }
            // Compute average
            r /= pixel_count;
            g /= pixel_count;
            b /= pixel_count;
            a /= pixel_count;
            SetPixelRGBA(blured_image, x, y, (Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a);
        }
    }

    return blured_image; // OK
}

/*
    Compute gradients (dy, dx, magnitude and orientation) of each pixel of "image".
*/
int CalculateGradients(SDL_Surface *image, Gradient* gradients) {

    if (image == NULL || gradients == NULL) {
        fprintf(stderr, "Invalid argument in CalculateGradients()\n");
        return EXIT_FAILURE;
    }

    for (int y = 1; y < image->h - 1; y++) {
        for (int x = 1; x < image->w - 1; x++) {

            double dIy =  GetPixelLuminance(image, x - 1, y - 1) + 2 * GetPixelLuminance(image, x, y - 1) +
            GetPixelLuminance(image, x + 1, y - 1) - GetPixelLuminance(image, x - 1, y + 1) -
            2 * GetPixelLuminance(image, x, y + 1) - GetPixelLuminance(image, x + 1, y + 1);

            double dIx = GetPixelLuminance(image, x - 1, y - 1) + 2 * GetPixelLuminance(image, x - 1, y) +
            GetPixelLuminance(image, x - 1, y + 1) - GetPixelLuminance(image, x + 1, y - 1) -
            2 * GetPixelLuminance(image, x + 1, y) - GetPixelLuminance(image, x + 1, y + 1);

            gradients[y * image->w + x].magnitude = sqrt(pow(dIx, 2) + pow(dIy, 2));
            gradients[y * image->w + x].orientation = atan2(dIy, dIx);
            gradients[y * image->w + x].dIx = dIx;
            gradients[y * image->w + x].dIy = dIy;
        }
    }
    return EXIT_SUCCESS;
}

/*
    Return a copy of "Gradient".
    Return NULL if an error occurred.
*/
Gradient* CopyGradients(const Gradient* gradients, int width, int height) {

    Gradient* copy = (Gradient*)malloc(width * height * sizeof(Gradient));

    if (copy == NULL) {
        fprintf(stderr, "Malloc error in CopyGradients()\n");
        return NULL;
    }

    memcpy(copy, gradients, width * height * sizeof(Gradient));

    return copy;
}

int DeleteNonMaximum(Gradient* gradients, int width, int height)
{

    double a[4];
    a[0] = M_PI/8;
    for (int k = 1; k< 4; k ++) a[k] = a[k-1] + M_PI/4;

    // Copy gradients into a new tab
    Gradient* G = CopyGradients(gradients, width, height);
    if (!G)
        return EXIT_FAILURE;

    for (int j = 1; j < height - 1; j++) {
        for (int i = 1; i < width - 1; i++) {
            Gradient g1, g2;
            if (G[j * width + i].magnitude != 0) {
                double theta = G[j * width + i].orientation;
                if (theta > 0) {

                    if (theta<a[0] || theta>a[3]) {g1 = G[j * width + i+1]; g2 = G[j * width + i+1];}
                    else if(theta<a[1]) {g1 = G[(j+1) * width + i + 1]; g2 = G[(j-1) * width + i-1];}
                    else if (theta<a[2]) {g1 = G[(j+1) * width + i]; g2 = G[(j-1) * width + i];}
                    else {g1 = G[(j+1) * width + i-1]; g2 = G[(j-1) * width + i+1];}
                }
                else if (theta<0){
                    if (theta<-a[3]) {g1 = G[j * width + i+1]; g2 = G[j * width + i-1];}
                    else if (theta<-a[2]) {g1 = G[(j-1) * width + i-1]; g2 = G[(j+1) * width + i+1];}
                    else if (theta<-a[1]) {g1 = G[(j-1) * width + i]; g2 = G[(j+1) * width + i];}
                    else if (theta<-a[0]) {g1 = G[(j-1) * width + i+1]; g2 = G[(j+1) * width + i-1];}
                    else {g1 = G[j * width + i+1]; g2 = G[j * width + i-1];}
                }
                if (G[j * width + i].magnitude < g1.magnitude || G[j * width + i].magnitude<g2.magnitude)
                    gradients[j * width + i].magnitude = 0.0;
            }
        }
    }
    // Free copy memory
    free(G);
    return EXIT_SUCCESS;
}
/*
    Apply threshold to gradient.
*/
int ApplyGradientThreshold(Gradient* gradients, int height, int width, double low_threshold, double high_threshold)
{
    // Copy gradients into a new tab
    Gradient* copy = CopyGradients(gradients, width, height);
    if (!copy)
        return EXIT_FAILURE;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double gradient_magnitude = copy[y * width + x].magnitude;

            if (gradient_magnitude >= high_threshold) {
                gradients[y * width + x].strength = 1;
            } else if (gradient_magnitude >= low_threshold) {
                gradients[y * width + x].strength = 0;
            } else {
                gradients[y * width + x].strength = -1;
            }
        }
    }

    // Free copy memory
    free(copy);
    return EXIT_SUCCESS;
}

/*
    Apply hysteresis contour tracking to gradients.
*/
int TrackHysteresisContour(Gradient* gradients, int height, int width)
{
    // Copy gradients into a new tab
    Gradient* copy = CopyGradients(gradients, width, height);
    if (!copy)
        return EXIT_FAILURE;

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {

                if (copy[y * width + x].strength == 0){

                    int has_white_neighbour = 0;
                    for (int j = y - 1; j < y + 2; j ++){
                        for (int i = x - 1; i < x + 2; i ++){
                            if (copy[j * width + i].strength > 0)
                            {
                                has_white_neighbour = 1;
                            }

                        }
                    }
                    if (has_white_neighbour == 1)
                        gradients[y * width + x].strength = 1;
                    else
                        gradients[y * width + x].strength = -1;
                }
        }
    }

    // Free copy memory
    free(copy);
    return EXIT_SUCCESS;
}

/*
    Return a surface from gradients.
    Manner depend on flag
*/
SDL_Surface* ConvertGradientsToSurface(Gradient* gradients, int height, int width, int flag)
{
    SDL_Surface *image = SDL_CreateRGBSurface(0,width, height, 32, 0, 0, 0, 0);
    if (flag == DX_MASK)
    {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                Uint8 d = (Uint8)gradients[y * width + x].dIx;
                SetPixelRGBA(image, x, y, d, d, d, 255);
            }
        }
    }
    else if (flag == DY_MASK)
    {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                Uint8 d = (Uint8)gradients[y * width + x].dIy;
                SetPixelRGBA(image, x, y, d, d, d, 255);
            }
        }
    }
    else if (flag == ORIENTATION_MASK)
    {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                Uint8 d = (Uint8)gradients[y * width + x].orientation;
                SetPixelRGBA(image, x, y, d, d, d, 255);
            }
        }
    }
    else if (flag == MAGNITUDE_MASK)
    {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                Uint8 d = (Uint8)gradients[y * width + x].magnitude;
                SetPixelRGBA(image, x, y, d, d, d, 255);
            }
        }
    }
    else if (flag == STRENGTH_MASK)
    {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (gradients[y * width + x].strength == 1)
                    SetPixelRGBA(image, x, y, 255, 255, 255, 255);
            }
        }
    }
    else{
        fprintf(stderr, "Unknown flag in gradientToSurface(): %d\n", flag);
        return NULL;
    }

    return image;
}

/*
    Apply canny filter to image and return the result.
*/
SDL_Surface* ApplyCannyFilter(SDL_Surface *image, int blur_straight, double low_threshold, double high_threshold, int display_flag)
{
    Gradient *gradients = (Gradient *)malloc(image->w * image->h * sizeof(Gradient));
    if (gradients == NULL) {
        fprintf(stderr, "Malloc error in ApplyCannyFilter()\n");
        return NULL;
    }

    image = ConvertSurfaceToGaussianBlur(image, blur_straight);
    if (!image)
        return NULL;

    if (CalculateGradients(image, gradients) != 0)
        return NULL;
    if (DeleteNonMaximum(gradients, image->w, image->h) != 0)
        return NULL;
    if (ApplyGradientThreshold(gradients, image->h, image->w, low_threshold, high_threshold) != 0)
        return NULL;
    if (TrackHysteresisContour(gradients, image->h, image->w) != 0)
        return NULL;
    return ConvertGradientsToSurface(gradients, image->h, image->w, display_flag);
}











