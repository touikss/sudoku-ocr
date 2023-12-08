#include "../headers/image_2.h"
#include "../headers/image.h"


Image convert_im2_to_im(Image2 image2)
{   
    Image res = Image_CreateBlack(image2.width, image2.height);

    for (int i = 0; i < image2.width; i ++) {
        for (int j = 0; j < image2.height; j ++){
            res.pixels[i][j].r = image2.pixels[i][j].r;
            res.pixels[i][j].g = image2.pixels[i][j].g;
            res.pixels[i][j].b = image2.pixels[i][j].b;
        }
    }

    return res;
}

Image2* convert_im_to_im2(Image image)
{
    Image2* res = create_image(image.height, image.width);

    for (unsigned int i = 0; i < image.width; i ++) {
        for (unsigned int j = 0; j < image.height; j ++){
            res->pixels[i][j].r = image.pixels[i][j].r;
            res->pixels[i][j].g = image.pixels[i][j].g;
            res->pixels[i][j].b = image.pixels[i][j].b;
        }
    }

    return res;
}