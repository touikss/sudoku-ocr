#include "../headers/img.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <err.h>

#define MAXCHAR 60000

int parse_token(char* hexString)
{
    char *endptr;
    long int decimalValue = strtol(hexString, &endptr, 16);

    // Vérifier les erreurs éventuelles
    if (*endptr != '\0') {
        errx(EXIT_FAILURE, "Erreur de conversion. Caractère non valide : %c\n", *endptr);
    } 
	if (decimalValue < 0 || decimalValue > 15){
		errx(EXIT_FAILURE, "Erreur de conversion. Chaine non valide : %s\n", hexString);
	}

    return decimalValue;
}

Img** csv_to_imgs(char* file_string, int number_of_imgs) {
	FILE *fp;
	Img** imgs = malloc(number_of_imgs * sizeof(Img*));
	char row[MAXCHAR];
	fp = fopen(file_string, "r");

	// Read the first line 
	fgets(row, MAXCHAR, fp);

	int num_elements = 0;
    for (int i = 0; row[i] != '\0'; i++) {
        if (row[i] == ',') {
            num_elements++;
        }
    }
    num_elements++;


	int delta = num_elements - 784; // nombre de ligne qui ne sont pas des datas (labels et noms)

	if (delta < 0)
		delta = 1;

	printf("DELTA : %d\n", delta);
	int i = 0;
	while (feof(fp) != 1 && i < number_of_imgs) {

		int j = 0;

		fgets(row, MAXCHAR, fp);
		
		char* token = strtok(row, ",");
		for (int to_pass = 1; to_pass < delta; to_pass ++)
			token = strtok(NULL, ",");

		/*if (token[1] != '\0' || !isdigit(token[0])){
			continue;
		}*/

		imgs[i] = malloc(sizeof(Img));
		
		printf("LABEL : %s, %d\n", token, i);

		imgs[i]->img_data = matrix_create(28, 28);
		while (token != NULL) {
			if (j > delta - 2) {
				if (j == delta - 1) {
					imgs[i]->label = parse_token(token);
				} else {
					imgs[i]->img_data->entries[(j-delta) / 28][(j-delta) % 28] = atoi(token) / 256.0;
				}
			}
			token = strtok(NULL, ",");
			j++;
		}
		i++;
	}

	printf("Image loaded : %d\n", i);

	fclose(fp);
	return imgs;
}

double normalize_pixel(uint8_t pixel_value) {
    return (double)pixel_value / 256.0;
}

void bmp_to_csv(const char *bmp_file, const char *csv_file) {
    SDL_Surface *image_surface = SDL_LoadBMP(bmp_file);
    if (image_surface == NULL) {
        fprintf(stderr, "Impossible de charger le fichier BMP : %s. %s\n", bmp_file, SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    FILE *csv_file_ptr = fopen(csv_file, "w");
    if (csv_file_ptr == NULL) {
        SDL_FreeSurface(image_surface);
        fprintf(stderr, "Impossible de créer le fichier CSV : %s\n", csv_file);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // Lire les données des pixels de l'image BMP
    Image image;
    for (int i = 0; i < 28; i++) {
        for (int j = 0; j < 28; j++) {
            Uint32 pixel = *(Uint32 *)(image_surface->pixels + (i * image_surface->pitch) + (j * image_surface->format->BytesPerPixel));
            image.pixels[i][j].value = (pixel & 0xFF);  // Récupérer la composante bleue (en niveaux de gris)
        }
    }

	fprintf(csv_file_ptr, "\n");  // Première ligne
    // Écrire dans le fichier CSV
    fprintf(csv_file_ptr, "0");  // Première colonne

    for (int i = 0; i < 28; i++) {
        for (int j = 0; j < 28; j++) {
			//printf("val : %d\n", image.pixels[i][j].value);
            fprintf(csv_file_ptr, ",%d", image.pixels[i][j].value);
        }
    }

    fprintf(csv_file_ptr, "\n");

    // Fermer les fichiers et libérer la mémoire
    fclose(csv_file_ptr);
    SDL_FreeSurface(image_surface);
    SDL_Quit();
}

void img_print(Img* img) {
	matrix_print(img->img_data);
	printf("Img Label: %d\n", img->label);
}

void img_free(Img* img) {
	matrix_free(img->img_data);
	free(img);
	img = NULL;
}

void imgs_free(Img** imgs, int n) {
	for (int i = 0; i < n; i++) {
		img_free(imgs[i]);
	}
	free(imgs);
	imgs = NULL;
}