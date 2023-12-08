#include "../headers/image.h"

#include <err.h>
#include <stdlib.h>
#include <string.h>


/////////////////////////////////////////////////////////////////////////////////
//////////////////////////////    BASICS    /////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


void Pixel_Set(Pixel *pixel, Uint8 value)
{
    if (pixel == NULL)
        Error("NULL parameter in Pixel_Set()");
    pixel->r = value;
    pixel->g = value;
    pixel->b = value;
}


Pixel Pixel_InstantiateZero()
{
    Pixel pixel;
    pixel.b = 0;
    pixel.r = 0;
    pixel.g = 0;

    return pixel;
}


Image Image_CreateBlack(unsigned int width, unsigned int height)
{
    Image image;

    image.width = width;
    image.height = height;

    for (size_t i = 0; i < 256; i++) {image.histogram[i] = 0;}
    
    image.pixels = calloc(width, sizeof(Pixel *));

    if (image.pixels == NULL)
        Error("Calloc error, unable to allocate memory in Image_CreateBlack()");

    unsigned int x;
    for (x = 0; x < width; x++)
    {
        image.pixels[x] = calloc(height, sizeof(Pixel));
        if (image.pixels[x] == NULL)
            Error("Calloc error, unable to allocate memory in Image_CreateBlack()");
    }
    for (unsigned int x = 0; x < width; x++)
    {
        for (unsigned int y = 0; y < height; y++)
        {
            image.pixels[x][y].r = 0;
            image.pixels[x][y].g = 0;
            image.pixels[x][y].b = 0;
        }
    }
    image.averageColor = 0;
    return image;
}


Image Image_CreateFromSurface(SDL_Surface *surface, unsigned int width, unsigned int height)
{
    if (surface == NULL)
        Error("NULL parameter in Image_CreateFromSurface()");

    Image image;

    image.width = width;
    image.height = height;

    for (size_t i = 0; i < 256; i++) {image.histogram[i] = 0;}
    
    image.pixels = (Pixel**)calloc(width, sizeof(Pixel *));

    if (image.pixels == NULL)
        Error("Calloc error, unable to allocate memory in Image_CreateFromSurface()");

    unsigned int x;
    for (x = 0; x < width; x++)
    {
        image.pixels[x] = (Pixel*)calloc(height, sizeof(Pixel));
        if (image.pixels[x] == NULL)
            Error("Calloc error, unable to allocate memory in Image_CreateFromSurface()");

    }

    SDL_Color rgb;
    Uint32 pixel;
    double averageColor = 0;

    for (unsigned int x = 0; x < width; x++)
    {
        for (unsigned int y = 0; y < height; y++)
        {
            // Get pixel from surface
            pixel = get_pixel(surface, x, y);

            // Get RGB values from pixel
            SDL_GetRGB(pixel, surface->format, &rgb.r, &rgb.g, &rgb.b);
            int val = (rgb.r + rgb.g + rgb.b) / 3;
            image.pixels[x][y].r = val;
            image.pixels[x][y].g = val;
            image.pixels[x][y].b = val;
            averageColor += val;
            image.histogram[val] += 1;
        }
    }
    averageColor /= (width * height);
    image.averageColor = averageColor;
    return image;
}


SDL_Surface *Image_CreateSurface(Image *image)
{
    if (image == NULL)
        Error("NULL parameter in Image_CreateSurface()");

    const unsigned int width = image->width;
    const unsigned int height = image->height;

    // Create rgb surface from image
    SDL_Surface *surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);

    if (surface == NULL)
        errx(EXIT_FAILURE, "Error while creating surface in Image_CreateSurface(): %s\n", SDL_GetError()[0] == '\0'? "no error from SDL" : SDL_GetError());

    // For each pixel in the source image
    for (unsigned int x = 0; x < width; x++)
    {
        for (unsigned int y = 0; y < height; y++)
        {
            // Get pixel from image
            Pixel _pixel = image->pixels[x][y];

            // Get pixel value for SDL
            Uint32 pixel =
                SDL_MapRGB(surface->format, _pixel.r, _pixel.g, _pixel.b);

            // Put pixel in img
            put_pixel(surface, x, y, pixel);
        }
    }
    return surface;
}


Pixel **Image_CopyPixelArray(Image *image)
{
    if (image == NULL)
        Error("NULL parameter in Image_CopyPixelArray()");

    const unsigned int w = image->width;
    const unsigned int h = image->height;

    Pixel **mask = malloc((w + 1) * sizeof(Pixel *));
    if (mask == NULL)
        Error("Malloc error, unable to allocate memory in Image_CopyPixelArray");

    for (unsigned int i = 0; i < w; i++)
    {
        mask[i] = (Pixel *)malloc((h + 1) * sizeof(Pixel));

        if (mask[i] == NULL)
            Error("Malloc error, unable to allocate memory in Image_CopyPixelArray");


        for (unsigned int j = 0; j < h; j++)
        { // MEDIAN FILTER
            mask[i][j].r = image->pixels[i][j].r;
            mask[i][j].g = image->pixels[i][j].g;
            mask[i][j].b = image->pixels[i][j].b;
        }
    }
    return mask;
}


void Image_Free(Image *image)
{
    if (image == NULL)
        Error("NULL parameter in Image_Free()");

    const unsigned int width = image->width;

    for (unsigned int x = 0; x < width; x++)
    {
        free(image->pixels[x]);
    }

    free(image->pixels);
    image->pixels = NULL;
}


void Image_Clear(Image* image)
{
    if (image == NULL)
        Error("NULL parameter in Image_Clear()");

    const int width = image->width;
    const int height = image->height;

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            image->pixels[x][y].r = 0;
            image->pixels[x][y].g = 0;
            image->pixels[x][y].b = 0;
        }
    }
}


/////////////////////////////////////////////////////////////////////////////////
////////////////////////////    MANIPULATION    /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


Image Image_Copy(Image *image)
{
    if (image == NULL)
        Error("NULL parameter in Image_Copy()");

    Image res;
    res.pixels = Image_CopyPixelArray(image);
    res.width = image->width;
    res.height = image->height;
    res.averageColor = image->averageColor;
    res.path = image->path;

    return res;
}


void Image_Save(Image *image, char *path)
{   
    
    if (image == NULL || path == NULL)
        Error("NULL parameter in Image_Save()");

    // Update SDL_Surface inside Image struct
    SDL_Surface *surface = Image_CreateSurface(image);

    if (SDL_SaveBMP(surface, path) != 0)
        Error_SDL("Error while saving file in Image_Save()");
    
    SDL_FreeSurface(surface);
}

Image Image_Crop(Image *image, SDL_Rect *rect)
{
    
    if (image == NULL || rect == NULL)
        Error("NULL parameter in Image_Crop()");

    Image res;
    res.width = rect->w;
    res.height = rect->h;

    if (rect->w < 0 || rect->h < 0)
        Error("Invalid dimension in Image_Crop()");

    // Allocate pixels form dimension of rect
    res.pixels = malloc((rect->w + 1) * sizeof(Pixel *));
    if (res.pixels == NULL)
    {
        Error("Malloc error, unable to allocate memory in Image_Crop()");
    }
    for (int i = 0; i < rect->w; i++)
    {
        res.pixels[i] = (Pixel *)malloc((rect->h + 1) * sizeof(Pixel));

        if (res.pixels[i] == NULL)
            Error("Malloc error, unable to allocate memory in Image_Crop()");
    }

    // Copy pixels from image to res
    for (int x = rect->x; x < rect->x + rect->w; x++)
    {
        for (int y = rect->y; y < rect->y + rect->h; y++)
        {
            res.pixels[x - rect->x][y - rect->y].r = image->pixels[x][y].r;
            res.pixels[x - rect->x][y - rect->y].g = image->pixels[x][y].g;
            res.pixels[x - rect->x][y - rect->y].b = image->pixels[x][y].b;
        }
    }
    return res;
}


void Image_PasteOn(Image *src, Image *dest, SDL_Rect *rect)
{
    
    if (src == NULL || dest == NULL || rect == NULL)
        Error("NULL parameter in Image_PasteOn()");

    for (int x = rect->x; x < rect->x + rect->w; x++)
    {
        for (int y = rect->y; y < rect->y + rect->h; y++)
        {
            if (x - rect->x < (int)src->width && y - rect->y < (int)src->height)
            {
                dest->pixels[x][y].r = src->pixels[x - rect->x][y - rect->y].r;
                dest->pixels[x][y].g = src->pixels[x - rect->x][y - rect->y].g;
                dest->pixels[x][y].b = src->pixels[x - rect->x][y - rect->y].b;
            }
        }
    }
}


void Image_Clone(Image *src, Image *dst)
{
    if (src == NULL || dst == NULL)
        Error("NULL parameter in Image_Clone()");
        
    const unsigned int width = src->width;
    const unsigned int height = src->height;

    for (unsigned int i = 0; i < width; i++)
    {
        for (unsigned int j = 0; j < height; j++)
        {
            dst->pixels[i][j].r = src->pixels[i][j].r;
            dst->pixels[i][j].g = src->pixels[i][j].g;
            dst->pixels[i][j].b = src->pixels[i][j].b;
        }
    }
}


/////////////////////////////////////////////////////////////////////////////////
////////////////////////////      FILTERS       /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


void Image_GaussianFilter1(Image* image, int smoothness)
{
    if (image == NULL || smoothness < 0)
        Error("Invalid parameter in Image_GaussianFilter()");

    int width = image->width;
    int height = image->height;

    int histTmp[256] = {0}; 

    Image tmp = Image_CreateBlack(width, height); 

    // Compute the number of pixel that we will use to compute the new pixel value
    const int pixel_count = (2 * smoothness + 1) * (2 * smoothness + 1);

    for (int x = 0; x < width; x ++){
        for (int y = 0; y < height; y ++){
            int average = 0;
             // Browse pixels around
            for (int i = x - smoothness; i <= (x + smoothness); i ++){
                for (int j = y - smoothness; j <= y + smoothness; j ++){
                    if (j >= 0 && i >= 0 && j < height && i < width){
                         Uint8 pixel = image->pixels[i][j].r;
                        average += pixel;
                    }
                }
            }
            // Compute average
            average /= pixel_count;
            tmp.pixels[x][y].r = average;
            tmp.pixels[x][y].g = average;
            tmp.pixels[x][y].b = average;
            histTmp[average] += 1;
        }
    }
    Image_Clone(&tmp, image);
    for (size_t i = 0; i < 256; i++){ image->histogram[i] = histTmp[i]; }
    Image_Free(&tmp);
}


void Image_GaussianFilter(Image* image, int smoothness) 
{
    if (image == NULL || smoothness < 0)
        Error("Invalid parameter in Image_GaussianFilter()");

    int width = image->width;
    int height = image->height;

    int histTmp[256] = {0}; 

    Image tmp = Image_CreateBlack(width, height); 

    for (int x = 0; x < width; x ++) {
        for (int y = 0; y < height; y ++) {
            int sum = 0;
            int count = 0;

            // Iterate over a square neighborhood
            for (int i = x - smoothness; i <= x + smoothness; i++) {
                for (int j = y - smoothness; j <= y + smoothness; j++) {
                    // Use replication of the border
                    int px = (i < 0) ? 0 : (i >= width) ? width - 1 : i;
                    int py = (j < 0) ? 0 : (j >= height) ? height - 1 : j;

                    sum += image->pixels[px][py].r;
                    count++;
                }
            }

            // Compute average
            if (count > 0) {
                int average = sum / count;
                tmp.pixels[x][y].r = average;
                tmp.pixels[x][y].g = average;
                tmp.pixels[x][y].b = average;
                histTmp[average] += 1;
            }
        }
    }

    Image_Clone(&tmp, image);

    // Copy histogram directly
    for (size_t i = 0; i < 256; i++) {
        image->histogram[i] = histTmp[i];
    }

    Image_Free(&tmp);
}



int CompareNumbers(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}


void Image_MedianFilter(Image* image)
{
    if (image == NULL)
        Error("NULL parameter in Image_MedianFilter()");

    unsigned int width = image->width;
    unsigned int height = image->height;

    int histTmp[256] = {0};

    Image tmp = Image_CreateBlack(width, height);
    
    for (unsigned int x = 0; x < width; x ++){
        for (unsigned int y = 0; y < height; y ++){
            int mat[9];

            for (int i = 0; i < 9; i ++) mat[i] = 0;

            for (int i = -(x != 0); i < -(x == (width - 1)) + 2; i++)
            {
                for (int j = -(y != 0); j < -(y == (height - 1)) + 2; j++)
                {
                    mat[(i + 1) * 3 + j + 1] += image->pixels[x + i][y + j].r;
                }
            }
            qsort(mat, 9, sizeof(int), CompareNumbers);
            int val = mat[5];
            tmp.pixels[x][y].r = val;
            tmp.pixels[x][y].g = val;
            tmp.pixels[x][y].b = val;
            histTmp[val] += 1;
        }
    }
    Image_Clone(&tmp, image);
    for (size_t i = 0; i < 256; i++){ image->histogram[i] = histTmp[i]; }
    
    Image_Free(&tmp);
}


void Image_ContrastFilter(Image* image, double contrastFactor, int lowThreshold, int highThreshold, int exposure) {
    if (image == NULL || contrastFactor < 0 || lowThreshold < 0 || highThreshold < 0)
        Error("Invalid parameter in Image_ContrastFilter()");

    const unsigned int width = image->width;
    const unsigned int height = image->height;

    // Calcul de l'histogramme
    int histogram[256] = {0};

    for (unsigned int x = 0; x < width; x++)
    {
        for (unsigned int y = 0; y < height; y++)
        {
            int val = image->pixels[x][y].r;

            if (val <= lowThreshold)
                val = val - exposure < 0? 0 : val - exposure;
            else if (val >= highThreshold)
                val = val + exposure > 255? 255 : val + exposure;

            // Set val
            image->pixels[x][y].r = val;
            image->pixels[x][y].g = val;
            image->pixels[x][y].b = val;
            histogram[val]++;
        }
    }

    for (int i = 0; i < 256; i ++)
        image->histogram[i] = histogram[i];

    return;

    // For each pixel of the image
    for (unsigned int x = 0; x < width; x++)
    {
        for (unsigned int y = 0; y < height; y++)
        {
            histogram[image->pixels[x][y].r]++;
        }
    }

    return;

    // Trouver les valeurs min et max non nulles dans l'histogramme
    int minVal = 0;
    int maxVal = 255;
    while (histogram[minVal] == 0 && minVal < 255) {
        minVal++;
    }
    while (histogram[maxVal] == 0 && maxVal > 0) {
        maxVal--;
    }

    // For each pixel of the image
    for (unsigned int x = 0; x < width; x++)
    {
        for (unsigned int y = 0; y < height; y++)
        {
            double normalized = (image->pixels[x][y].r - minVal) / (double)(maxVal - minVal);
            double adjusted = 255.0 * contrastFactor * (normalized - 0.5) + 0.5;
            int val = (adjusted < 0) ? 0 : ((adjusted > 255) ? 255 : (unsigned char)adjusted);
            // Compute the average of the pixel
            image->pixels[x][y].r = val;
            image->pixels[x][y].g = val;
            image->pixels[x][y].b = val;
        }
    }
}


void Image_ThresholdFilter(Image* image, int threshold){
    if (image == NULL || threshold < 0)
        Error("Invalid parameter in Image_ThresholdFilter()");
    // Appliquez la dilatation  
    int width = image->width;
    int height = image->height;
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            int val = image->pixels[x][y].r < threshold ? 255 : 0;
            image->pixels[x][y].r = val;
            image->pixels[x][y].g = val;
            image->pixels[x][y].b = val;
        }
    }
}


void Image_Invert(Image* image)
{   
    if (image == NULL)
        Error("NULL parameter in Image_Invert()");
    const int width = image->width;
    const int height = image->height;

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            // Si le pixel n'a pas encore été visité et est blanc
            int val = 255 - image->pixels[x][y].r;
            image->pixels[x][y].r = val;
            image->pixels[x][y].g = val;
            image->pixels[x][y].b = val;
        }
    }
}


void Image_Levelize(Image* image, size_t n)
{
    for (size_t x = 0; x < image->width; x++)
    {
        for (size_t y = 0; y < image->height; y++)
        {
            float v = image->pixels[x][y].r;
            for (size_t i = 0; i < n; i++)
            {
                if (v >= i * (255.f / n) && v <= (i + 1) * (255.f / n))
                {
                    Pixel_Set(&(image->pixels[x][y]), (i + 1) * (255.f / n));
                }
            }
        }
    }
}


/////////////////////////////////////////////////////////////////////////////////
//////////////////////////// HISTOGRAM MANIPULATION /////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


void Image_NormalizeHistogram(Image* image)
{    
    if (image == NULL)
        Error("NULL parameter in Image_NormalizeHistogram()");
    
    int min = 0;
    int max = 0;
    const int width = image->width;
    const int height = image->height;
    int toExclude = (height * width)/100;
    int newHist[256] = {0};
    for (size_t i = 0; i < 256; i++)
    {
        if (image->histogram[i] > toExclude){
            min = i;
            break;
        }
        toExclude -= image->histogram[i];
    }
    for (int i = 255; i >= 0; i--)
    {
        if (image->histogram[i] > toExclude){
            max = i;
            break;
        }
        toExclude -= image->histogram[i];
    }

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            int val = 255 * abs(image->pixels[i][j].r - min) / (max - min);
            image->pixels[i][j].r = val;
            image->pixels[i][j].g = val;
            image->pixels[i][j].b = val;
            newHist[image->pixels[i][j].r] += 1;
        }
    }    

    // Copier les valeurs de newHist dans image->histogram
    for (size_t i = 0; i < 256; i++) {
        image->histogram[i] = newHist[i];
    }
}


void Image_SmoothHistogram(Image* image, int windowSize) 
{
    if (image == NULL || windowSize < 0)
        Error("Invalid parameter in Image_SmoothHistogram()");

    int output[256] = {0};

    for (int i = 0; i < 256; ++i) {
        int sum = 0;
        int count = 0;
        for (int j = i - windowSize / 2; j <= i + windowSize / 2; ++j) {
            if (j >= 0 && j < 256) {
                sum += image->histogram[j];
                count++;
            }
        }
        output[i] = sum / count;
    }
    for (size_t i = 0; i < 256; i++)
    {
        image->histogram[i] = output[i];
    }
}


void Image_EqualizeHistogram(Image* image) {
    
    if (image == NULL)
        Error("NULL parameter in Image_EqualizeHistogram()");

    int j = 0;
    int curve[256] = {0};    

    for (int i = 1; i < 255; ++i) {
        if (image->histogram[i] > image->histogram[i - 1] && image->histogram[i] > image->histogram[i + 1]) {
            // Pic détecté
            curve[j] = i;
            j ++;
        }
        else if (image->histogram[i] < image->histogram[i - 1] && image->histogram[i] < image->histogram[i + 1]) {
            // Creu détecté
            curve[j] = i;
            j ++;
        }
    }
    curve[j] = 255;

     // Réinitialise l'histograme
    for (size_t i = 0; i < 256; i++)
    {
        image->histogram[i] = 0;
    }
    
    const int width = image->width;
    const int height = image->height;

    Pixel pixel;

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {  
            int lum = image->pixels[i][j].r;
            int k = 0;
            while (k < 256 && curve[k] != -1){
                if (lum < curve[k]){
                    int val = curve[k]; // Peak
                    if (k % 2 == 1 && k != 0)
                        val = curve[k - 1]; // Valley
                    pixel.r = val;
                    pixel.g = val;
                    pixel.b = val;
                    image->pixels[i][j] = pixel;
                    image->histogram[lum] += 1;
                    break;
                }
                k ++;
            }
        }
    }

}


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////    EDGES MANIPULATION     /////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


void Image_ErodeEdges(Image* image) {

    if (image == NULL)
        Error("NULL parameter in Image_ErodeEdges()");

    // Obtenez les dimensions de la surface
    int width = image->width;
    int height = image->height;

    Image tmp = Image_CreateBlack(width, height);

    // Appliquez l'érosion  
    for (int x = 1; x < width - 1; ++x) {
        for (int y = 1; y < height - 1; ++y) {
            // Obtenez les valeurs des pixels voisins
            Uint8 p00 = image->pixels[x-1][y-1].r;
            Uint8 p01 = image->pixels[x][y-1].r; 
            Uint8 p02 = image->pixels[x+1][y-1].r; 
            Uint8 p10 = image->pixels[x-1][y].r; 
            Uint8 p11 = image->pixels[x][y].r; 
            Uint8 p12 = image->pixels[x+1][y].r; 
            Uint8 p20 = image->pixels[x-1][y+1].r;
            Uint8 p21 = image->pixels[x][y+1].r; 
            Uint8 p22 = image->pixels[x+1][y+1].r; 

            // Si tous les voisins sont blancs, mettez le pixel actuel à blanc, sinon à noir
            Uint8 result = ((p00 == 255) + (p01 == 255) + (p02 == 255) + (p10 == 255) + 
            (p11 == 255) * 2 + (p12 == 255) + (p20 == 255) + (p21 == 255) + (p22 == 255)) >= 7 ? 255 : 0;

            tmp.pixels[x][y].r = result;
            tmp.pixels[x][y].g = result;
            tmp.pixels[x][y].b = result;
        }
    }
    Image_Clone(image, &tmp);
    Image_Free(&tmp);
}


void Image_DilateEdges(Image* image, int n) {

    if (image == NULL || n < 0)
        Error("Invalid parameter in Image_DilateEdges()");

    // Obtenez les dimensions de la surface
    int width = image->width;
    int height = image->height;

    Image tmp = Image_CreateBlack(width, height);

    // Appliquez l'érosion  
    for (int x = 1; x < width - 1; ++x) {
        for (int y = 1; y < height - 1; ++y) {
            // Obtenez les valeurs des pixels voisins
            int count = 0;
            for (int i = x-n; i <= x + n; i++)
            {
                for (int j = y-n; j <= y + n; j++)
                {
                    if (i >= 0 && j >= 0 && i < width && j < height && image->pixels[i][j].r != 0)
                        count ++;
                }
            }
            if (count > 2){
                tmp.pixels[x][y].r = 255;
                tmp.pixels[x][y].g = 255;
                tmp.pixels[x][y].b = 255;
            }
        }
    }
    Image_Clone(&tmp, image);
    Image_Free(&tmp);
}


/////////////////////////////////////////////////////////////////////////////////
////////////////////////////      DRAW       ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


void Image_DrawPoint(Image* image, Point point, Color color, int thickness)
{
    if (image == NULL || thickness <= 0)
        Error("Invalid parameter in Image_DrawPoint()");

    const int width = image->width;
    const int height = image->height;
    for (int dx = -(thickness - 1) ; dx <= thickness - 1; dx ++){
        for (int dy = -(thickness - 1); dy <= thickness - 1; dy++){
            int x = point.x + dx;
            int y = point.y + dy;
            if (x > 0 && x < width && y > 0 && y < height){
                image->pixels[x][y].r = color.r;
                image->pixels[x][y].g = color.g;
                image->pixels[x][y].b = color.b;
            }
        }
    }
}


void Image_DrawShapeExtremum(Image* image, ShapeTab* tab, Color color)
{
    if (image == NULL || tab == NULL)
        Error("NULL parameter in Image_DrawShapeExtremum()");

    for (Shape* current = tab->data->next; current; current = current->next){
        Shape_ComputeDescriptors(current);
        for (int i = 0; i < 4; i++){
            Image_DrawPoint(image, current->descriptor[i], color, 3);
        }
    }
}


void Image_DrawAveragePoints(Image* image, ShapeTab* tab, Color color)
{
    if (image == NULL || tab == NULL)
        Error("NULL parameter in Image_DrawAveragePoints()");

    for (Shape* current = tab->data->next; current; current = current->next){
        Point point = {(int)current->averageX, (int)current->averageY};
        if (current->isSquare)
            Image_DrawPoint(image, point, color, 3);
        else
            Image_DrawPoint(image, point, (Color){255 - color.r, 255 - color.g, 255 - color.b}, 3);
        if (current->isTilted)
            Image_DrawPoint(image, point, RED, 2);

    }
}


void Image_DrawShape(Image* image, Shape* shape, Color color)
{
    if (shape == NULL || image == NULL)
        Error("NULL parameter in Image_DrawShape()");

    for (unsigned int i = 0; i < shape->size; i ++)
    {
        unsigned int x = shape->points[i].x;
        unsigned int y = shape->points[i].y;
        if (x > image->width || y > image->height)
            Error("Shape point out of image bounds in Image_DrawShape()");
        image->pixels[x][y]. r = color.r;
        image->pixels[x][y]. g = color.g;
        image->pixels[x][y]. b = color.b;
    }
}


void Image_DrawShapeTab(Image* image, ShapeTab* tab)
{
    if (tab == NULL || image == NULL)
        Error("NULL parameter(s) in Image_DrawShapeTab()");

    double i = 1;
    for (Shape* current = tab->data->next; current; current = current->next){
        i *= 2.345;
        Color color;
        color.r = (int)((1 + cos(i)) * 127);
        color.g = (int)((1 + sin(i * i)) * 127);
        color.b = (int)((cos(i) * sin(i) + 1) *64 );
        Image_DrawShape(image, current, color);
    }
}


void Image_DrawLine(Image* image, Point p1, Point p2, Color color) 
{
    if (image == NULL)
        Error("NULL parameter in Image_DrawLine()");

    int dx = p2.x - p1.x;
    int dy = p2.y - p1.y;
    int steps, k;

    float xIncrement, yIncrement, x = p1.x, y = p1.y;

    if (abs(dx) > abs(dy)) {
        steps = abs(dx);
    } else {
        steps = abs(dy);
    }

    xIncrement = (float)dx / (float)steps;
    yIncrement = (float)dy / (float)steps;

    Image_DrawPoint(image, p1, color, 1);

    for (k = 0; k < steps; k++) {
        x += xIncrement;
        y += yIncrement;
        Point point = {(int)x, (int)y};
        Image_DrawPoint(image, point, color, 1);
    }
}

void Image_DrawDescriptorsLinks(Image* image, ShapeTab* tab)
{
    if (image == NULL || tab == NULL)
        Error("NULL parameter in Image_DrawExtremumForm()");

    for (Shape* current = tab->data->next; current; current = current->next){
        Image_DrawLine(image, current->descriptor[0], current->descriptor[2], GREEN);
        Image_DrawLine(image, current->descriptor[2], current->descriptor[1], GREEN);
        Image_DrawLine(image, current->descriptor[1], current->descriptor[3], GREEN);
        Image_DrawLine(image, current->descriptor[3], current->descriptor[0], GREEN);
        Image_DrawLine(image, current->descriptor[4], current->descriptor[5], BLUE);
        Image_DrawLine(image, current->descriptor[5], current->descriptor[7], BLUE);
        Image_DrawLine(image, current->descriptor[7], current->descriptor[6], BLUE);
        Image_DrawLine(image, current->descriptor[6], current->descriptor[4], BLUE);
    }
}


void Image_DrawShapePoint(Image* image, Shape* shape, Color color)
{
     if (image == NULL || shape == NULL)
        Error("NULL parameter in Image_DrawShapePoint()");

    Image_DrawPoint(image, (Point){shape->averageX, shape->averageY}, color, 3);
}


void Image_DrawMatrix(Image* image, Shape* mat[][16])
{
    int count = 0;
    for (int i = 0; i < 16; i ++){
        for (int j = 0; j < 16; j++){
            Color col = {0, 0, 0};
            if (count < 9)
                col.r = 50 + count * 20;
            else if (count < 18){
                col.b = 50 + count * 20;
                col.g = 30;
            }
            else if (count < 27)
                col.g = 50 + count * 20;

            if (mat[i][j] != NULL){
                Image_DrawShape(image, mat[i][j], col);
                count ++;
                if (count >= 27)
                    count = 0;
            }
        }
    }
}


/////////////////////////////////////////////////////////////////////////////////
////////////////////////////   MASS DETECTION    ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


void Image_LabelComponent(Image* image, ShapeTab* shapeTab) 
{
    if (image == NULL || shapeTab == NULL)
        Error("NULL parameter in Image_LabelComponent()");

    const int width = image->width;
    const int height = image->height;
    int label = 1; // Étiquette pour la première région

    Image res = Image_CreateBlack(width, height);
    
    // Tableau pour stocker les pixels à explorer
    Point* stack = (Point*)calloc(width * height * 3, sizeof(Point)); // WTF?? normalement ca devrait marcher sans le *2 mais bon...
    if (stack == NULL)
        Error("Malloc error, unable to allocate memory in Image_LabelComponent()");

    for (int x = 0; x < width; x++) 
    {
        for (int y = 0; y < height; y++) 
        {
            // Si le pixel n'a pas encore été visité et est blanc
            if (res.pixels[x][y].r == 0 && image->pixels[x][y].r == 255) 
            {
                Shape* currentShape = Shape_Create(width * height/100, width, height);
                
                // Empilez le pixel
                stack[0].x = x;
                stack[0].y = y;
                int stackSize = 1;
                
                // Démarrez une nouvelle région avec une nouvelle étiquette
                label += 1 + tan(label) * 100;
                int r = 100;
                int g = (int)(0.5 * 255.0 - label * (1.0 + sin((x + y) * M_PI / (width + height)))) % 256;
                int b = (int)(0.5 * label * (1.0 + tan((x + y) * M_PI / (width + height)))) % 256;
                
                // Parcourez la pile
                while (stackSize > 0) 
                {
                    
                    Point currentPixel = stack[--stackSize];
                    int currentX = currentPixel.x;
                    int currentY = currentPixel.y;
                    
                    // Étiquetez le pixel avec l'étiquette actuelle
                    res.pixels[currentX][currentY].r = r;
                    res.pixels[currentX][currentY].g = g;
                    res.pixels[currentX][currentY].b = b;
                    
                    // Parcourez les voisins du pixel
                    for (int dx = -1; dx <= 1; dx++) 
                    {
                        for (int dy = -1; dy <= 1; dy++) 
                        {
                            int nx = currentX + dx;
                            int ny = currentY + dy;

                            // Si le voisin est dans les limites de l'image et est blanc
                            if (nx >= 0 && nx < width && ny >= 0 && ny < height &&
                                res.pixels[nx][ny].r == 0 && image->pixels[nx][ny].r == 255) 
                                {
                                // Ajouter le point à la shape
                                Shape_AddPoint(currentShape, currentX, currentY);
                                // Empilez le voisin
                                stack[stackSize].x = nx;
                                stack[stackSize].y = ny;
                                stackSize += 1;
                            }
                        }
                    }
                }
                ShapeTab_AddShape(shapeTab, currentShape);
            }
        }
    }
    
    // Libérez la mémoire utilisée par la pile
    free(stack);
    Image_Clone(&res, image);
    Image_Free(&res);
}


void Image_FindSquareInLabeled(Image* image)
{
    if (image == NULL)
        Error("NULL parameter in Image_FindSquareInLabeled()");

    int width = image->width;
    int height = image->height;
    int label = 1; // Étiquette pour la première région

    Image res = Image_CreateBlack(width, height);
    // Tableau pour stocker les pixels à explorer
    Point* stack = malloc(width * height * sizeof(Point));
    if (stack == NULL)
        Error("Malloc error, unable to allocate memory in Image_FindSquareInLabeled()");
    Point* stack_square = malloc(width * height * sizeof(Point));
    if (stack_square == NULL)
        Error("Malloc error, unable to allocate memory in Image_FindSquareInLabeled()");
    if (stack == NULL || stack_square == NULL) {
        fprintf(stderr, "Erreur lors de l'allocation de mémoire pour la pile.\n");
        return;
    }
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            // Si le pixel n'a pas encore été visité et est blanc
            if (res.pixels[x][y].r == 0 && image->pixels[x][y].r == 255) {
                // Empilez le pixel
                stack[0].x = x;
                stack[0].y = y;
                int stackSize = 1;
                stack_square[0].x = x;
                stack_square[0].y = y;
                int stack_squareSize = 1;

                int minX = x; // Coordonnées du point min
                int minY = y;
                double minDist = CoordinateDistance(0,0,x,y); // Distance minimale
                int maxX = x; // Coordonnées du point max
                int maxY = y;
                double maxDist = minDist;
                double formHeight = 0; // Dimensions de la forme
                double formWidth = 0;
                // Démarrez une nouvelle région avec une nouvelle étiquette
                label += 1 + tan(label) * 100;
                int r = (int)(0.5 * label + 255.0 * (1.0 + cos((x + y) * M_PI / (width + height)))) % 256;
                int g = (int)(0.5 * 255.0 - label * (1.0 + sin((x + y) * M_PI / (width + height)))) % 256;
                int b = (int)(0.5 * label * (1.0 + tan((x + y) * M_PI / (width + height)))) % 256;
                if (r <= 0)
                    r = 120;
                // Parcourez la pile
                while (stackSize > 0) {
                    Point currentPixel = stack[--stackSize];
                    int currentX = currentPixel.x;
                    int currentY = currentPixel.y;
                    double currentDist = CoordinateDistance(0,0,currentX,currentY);
                    if (currentDist < minDist){ // Vérifier min dist et max dist
                        minDist = currentDist;
                        minX = currentX;
                        minY = currentY;
                    }
                    else if (currentDist > maxDist){
                        maxDist = currentDist;
                        maxX = currentX;
                        maxY = currentY;
                    }

                    // Étiquetez le pixel avec l'étiquette actuelle
                    res.pixels[currentX][currentY].r = r;
                    res.pixels[currentX][currentY].g = g;
                    res.pixels[currentX][currentY].b = b;
                    // Parcourez les voisins du pixel
                    for (int dx = -1; dx <= 1; ++dx) {
                        for (int dy = -1; dy <= 1; ++dy) {
                            int nx = currentX + dx;
                            int ny = currentY + dy;

                            // Si le voisin est dans les limites de l'image et est blanc
                            if (nx >= 0 && nx < width && ny >= 0 && ny < height &&
                                res.pixels[nx][ny].r == 0 && image->pixels[nx][ny].r == 255) {
                                // Empilez le voisin
                                stack[stackSize].x = nx;
                                stack[stackSize].y = ny;
                                //stack_square[stack_squareSize].x = nx;
                                //stack_square[stack_squareSize].y = ny;
                                stackSize += 1;
                                //stack_squareSize += 1;
                                if (stackSize >= height){
                                    //printf("SIIIIZE = %d\n", width * height);
                                    //printf("WESH FRERO : %d\n", stackSize);
                                }
                            }
                        }
                    }
                }
                double angle = PointAngle(minX, minY, maxX, maxY);
                formWidth = cos(angle) * (maxDist - minDist);
                formHeight = sin(angle) * (maxDist - minDist);
                printf("HEIGHT = %.2f, WIDTH = %.2f, RAPPORT = %f", formHeight, formWidth, ScaleFactor(formHeight, formWidth));
                if ( formWidth != 0 && ScaleFactor(formHeight, formWidth) < 0.2 && formWidth > 100){
                    // Parcourez la pile
                    printf(" CARRÉ DANS L'AXE!");
                    while (stack_squareSize > 0) {
                        Point currentPixel = stack_square[--stack_squareSize];
                        int currentX = currentPixel.x;
                        int currentY = currentPixel.y;
                        // Étiquetez le pixel avec l'étiquette actuelle
                        res.pixels[currentX][currentY].r = 255;
                        res.pixels[currentX][currentY].g = 255;
                        res.pixels[currentX][currentY].b = 255;
                    }
                }
                else {
                    // Parcourez la pile
                    while (stack_squareSize > 0) {
                        Point currentPixel = stack_square[--stack_squareSize];
                        int currentX = currentPixel.x;
                        int currentY = currentPixel.y;
                        // Étiquetez le pixel avec l'étiquette actuelle
                        image->pixels[currentX][currentY].r = 255;
                        image->pixels[currentX][currentY].g = 255;
                        image->pixels[currentX][currentY].b = 255;
                        res.pixels[currentX][currentY].r = 0;
                        res.pixels[currentX][currentY].g = 0;
                        res.pixels[currentX][currentY].b = 0;
                    }
                }
                printf("\n");
            }
        }
    }

    // Libérez la mémoire utilisée par la pile
    free(stack);
    free(stack_square);
    Image_Clone(&res, image);
    Image_Free(&res);
}


void Image_RectCase(Image* image){
    if (image == NULL)
        Error("NULL parameter in Image_RectCase()");
    const unsigned int width = image->width;
    const unsigned int height = image->height;

    Image res = Image_CreateBlack(width, height);

    Point* stack = malloc(width * height * sizeof(Point));
    if (stack == NULL)
        Error("Malloc error, unable to allocate memory in findCases()");
    Point dim[4] = {
    {0, height}, // Première dimension
    {0, 0}, // Deuxième dimension
    {width, 0}, // Troisième dimension
    {0, 0}  // Quatrième dimension
    };
    if (stack == NULL) {
        fprintf(stderr, "Erreur lors de l'allocation de mémoire pour la pile.\n");
        return;
    }
    for (unsigned int x = 0; x < width; x++) {
        for (unsigned int y = 0; y < height; y++) {
            // Si le pixel n'a pas encore été visité et est blanc
            if (res.pixels[x][y].r == 0 && image->pixels[x][y].r == 255) {
                dim[0].y = (y < dim[0].y) ? y : dim[0].y;
                dim[1].y = (y > dim[1].y) ? y : dim[1].y;
                dim[2].x = (x < dim[2].x) ? x : dim[2].x;
                dim[3].x = (x > dim[3].x) ? x : dim[3].x;
            }
        }
    }
}

void Image_UpdateHistogram(Image* image)
{
    int hist[256] = {0};
    for (unsigned int i = 0; i < image->width; i ++){
        for (unsigned int j = 0; j < image->height; j ++){
            hist[image->pixels[i][j].r] += 1;
        }
    }
    for (int i = 0; i < 256; i ++){
        image->histogram[i] = hist[i];
    }
}

void Image_HighPassFilter(Image* image, int threshold){
    if (image == NULL || threshold < 0)
        Error("Invalid parameter in Image_ThresholdFilter()");
    // Appliquez la dilatation  
    int width = image->width;
    int height = image->height;
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            int val = image->pixels[x][y].r > threshold ? 255 : 0;
            image->pixels[x][y].r = val;
            image->pixels[x][y].g = val;
            image->pixels[x][y].b = val;
        }
    }
}
