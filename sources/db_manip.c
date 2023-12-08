#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <err.h>
#include <time.h>


#define MAX_LINE_LENGTH 1024
#define MAX_LINES_TO_COPY 10000
#define LINES_TO_SKIP 10000

void removeFirstNLines(const char *inputFileName, const char *outputFileName, size_t n) {
    FILE *inputFile = fopen(inputFileName, "r");
    if (inputFile == NULL) {
        fprintf(stderr, "Impossible d'ouvrir le fichier d'entrée.\n");
        exit(EXIT_FAILURE);
    }

    FILE *outputFile = fopen(outputFileName, "w");
    if (outputFile == NULL) {
        fprintf(stderr, "Impossible de créer le fichier de sortie.\n");
        fclose(inputFile);
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t lineSize = 0;
    size_t lineCount = 0;

    // Parcourir ligne par ligne la base de données et copier les lignes dans le fichier de sortie
    while (getline(&line, &lineSize, inputFile) != -1) {
        lineCount++;

        // Écrire les lignes dans le fichier de sortie seulement si elles ne font pas partie des premières N lignes
        if (lineCount > n) {
            fputs(line, outputFile);
        }
    }

    // Libérer la mémoire allouée par getline
    free(line);

    fclose(inputFile);
    fclose(outputFile);
}

void shuffleDatabase(const char *inputFileName, const char *outputFileName) {
    FILE *inputFile = fopen(inputFileName, "r");
    if (inputFile == NULL) {
        fprintf(stderr, "Impossible d'ouvrir le fichier d'entrée.\n");
        exit(EXIT_FAILURE);
    }

    FILE *outputFile = fopen(outputFileName, "w");
    if (outputFile == NULL) {
        fprintf(stderr, "Impossible de créer le fichier de sortie.\n");
        fclose(inputFile);
        exit(EXIT_FAILURE);
    }

    char **lines = NULL;
    size_t lineCount = 0;
    size_t maxLines = 1000; // Choisissez une valeur appropriée pour votre cas, par exemple 1000

    // Allouer de l'espace pour le tableau de pointeurs de lignes
    lines = (char **)malloc(maxLines * sizeof(char *));
    if (lines == NULL) {
        fprintf(stderr, "Erreur d'allocation de mémoire.\n");
        fclose(inputFile);
        fclose(outputFile);
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t lineSize = 0;

    // Lire les lignes dans un tableau
    while (getline(&line, &lineSize, inputFile) != -1) {
        // Allouer de l'espace pour chaque ligne
        lines[lineCount] = (char *)malloc((lineSize + 1) * sizeof(char));
        if (lines[lineCount] == NULL) {
            fprintf(stderr, "Erreur d'allocation de mémoire pour la ligne %zu.\n", lineCount);
            for (size_t i = 0; i < lineCount; i++) {
                free(lines[i]);
            }
            free(lines);
            fclose(inputFile);
            fclose(outputFile);
            free(line);
            exit(EXIT_FAILURE);
        }

        // Copier la ligne lue dans le tableau
        strcpy(lines[lineCount], line);
        lineCount++;

        // Si on atteint la taille maximale, augmenter la taille du tableau
        if (lineCount >= maxLines) {
            maxLines *= 2;
            lines = realloc(lines, maxLines * sizeof(char *));
            if (lines == NULL) {
                fprintf(stderr, "Erreur de réallocation de mémoire.\n");
                fclose(inputFile);
                fclose(outputFile);
                free(line);
                exit(EXIT_FAILURE);
            }
        }
    }

    // Libérer la mémoire allouée par getline
    free(line);

    // Initialiser le générateur de nombres aléatoires avec une graine basée sur l'heure actuelle
    srand((unsigned int)time(NULL));

    // Appliquer l'algorithme de Fisher-Yates pour mélanger les lignes
    for (size_t i = lineCount - 1; i > 0; i--) {
        size_t j = rand() % (i + 1);

        // Échanger les lignes i et j
        char *temp = lines[i];
        lines[i] = lines[j];
        lines[j] = temp;
    }

    // Écrire les lignes mélangées dans le fichier de sortie
    for (size_t i = 0; i < lineCount; i++) {
        fputs(lines[i], outputFile);
    }

    // Libérer la mémoire allouée
    for (size_t i = 0; i < lineCount; i++) {
        free(lines[i]);
    }
    free(lines);

    fclose(inputFile);
    fclose(outputFile);
}

void cloneDatabase(const char *inputFileName, const char *outputFileName) {
    FILE *inputFile = fopen(inputFileName, "r");
    if (inputFile == NULL) {
        fprintf(stderr, "Impossible d'ouvrir le fichier d'entrée.\n");
        exit(EXIT_FAILURE);
    }

    FILE *outputFile = fopen(outputFileName, "w");
    if (outputFile == NULL) {
        fprintf(stderr, "Impossible de créer le fichier de sortie.\n");
        fclose(inputFile);
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t lineSize = 0;

    // Parcourir ligne par ligne la base de données et copier dans le fichier de sortie
    while (getline(&line, &lineSize, inputFile) != -1) {
        char *comma = strchr(line, ',') + 1;
        if (isdigit(comma[0]))
            fputs(comma, outputFile);
    }

    // Libérer la mémoire allouée par getline
    free(line);

    fclose(inputFile);
    fclose(outputFile);
}

void mergeDatabases(const char *inputFile1, const char *inputFile2, const char *outputFile) {
    FILE *file1 = fopen(inputFile1, "r");
    FILE *file2 = fopen(inputFile2, "r");
    FILE *output = fopen(outputFile, "w");

    if (file1 == NULL || file2 == NULL || output == NULL) {
        fprintf(stderr, "Erreur lors de l'ouverture des fichiers.\n");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];

    // Fusionner les données du premier fichier
    while (fgets(line, sizeof(line), file1) != NULL) {
        fputs(line, output);
    }

    // Fusionner les données du deuxième fichier
    while (fgets(line, sizeof(line), file2) != NULL) {
        fputs(line, output);
    }

    fclose(file1);
    fclose(file2);
    fclose(output);
}

int compareDatabases(const char *file1, const char *file2) {
    FILE *db1 = fopen(file1, "r");
    FILE *db2 = fopen(file2, "r");

    if (db1 == NULL || db2 == NULL) {
        fprintf(stderr, "Erreur lors de l'ouverture des fichiers.\n");
        exit(EXIT_FAILURE);
    }

    char line1[MAX_LINE_LENGTH];
    char line2[MAX_LINE_LENGTH];
    int lineNumber = 1;

    while (fgets(line1, sizeof(line1), db1) != NULL && fgets(line2, sizeof(line2), db2) != NULL) {
        // Comparer les lignes
        if (strcmp(line1, line2) != 0) {
            printf("Différence détectée à la ligne %d.\n", lineNumber);
            fclose(db1);
            fclose(db2);
            return 0;  // Les bases de données ne sont pas égales
        }

        lineNumber++;
    }

    // Vérifier si l'une des bases de données a plus de lignes que l'autre
    if (fgets(line1, sizeof(line1), db1) != NULL || fgets(line2, sizeof(line2), db2) != NULL) {
        printf("Les bases de données n'ont pas le même nombre de lignes.\n");
        fclose(db1);
        fclose(db2);
        return 0;  // Les bases de données ne sont pas égales
    }

    printf("Les bases de données sont égales.\n");

    fclose(db1);
    fclose(db2);
    return 1;  // Les bases de données sont égales
}

void parseDatabase(const char *inputFileName, const char *outputFileName) {
    FILE *inputFile = fopen(inputFileName, "r");
    if (inputFile == NULL) {
        fprintf(stderr, "Impossible d'ouvrir le fichier d'entrée.\n");
        exit(EXIT_FAILURE);
    }

    FILE *outputFile = fopen(outputFileName, "w");
    if (outputFile == NULL) {
        fprintf(stderr, "Impossible de créer le fichier de sortie.\n");
        fclose(inputFile);
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t lineSize = 0;

    // Parcourir ligne par ligne la base de données et copier dans le fichier de sortie
    while (getline(&line, &lineSize, inputFile) != -1) {
        char *comma = strchr(line, ',') + 1;
        if (isdigit(comma[0]))
            fputs(comma, outputFile);
    }

    // Libérer la mémoire allouée par getline
    free(line);

    fclose(inputFile);
    fclose(outputFile);
}

void letterDatabase(const char *inputFileName, const char *outputFileName) {
    FILE *inputFile = fopen(inputFileName, "r");
    if (inputFile == NULL) {
        fprintf(stderr, "Impossible d'ouvrir le fichier d'entrée.\n");
        exit(EXIT_FAILURE);
    }

    FILE *outputFile = fopen(outputFileName, "w");
    if (outputFile == NULL) {
        fprintf(stderr, "Impossible de créer le fichier de sortie.\n");
        fclose(inputFile);
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t lineSize = 0;

    // Parcourir ligne par ligne la base de données et copier dans le fichier de sortie
    while (getline(&line, &lineSize, inputFile) != -1) {
        char *comma = line;//strchr(line, ',') + 1;
        if (comma[1] == ',' && (comma[0] == '0' || comma[0] == '1' || comma[0] == '2' || comma[0] == '3' ||
        comma[0] == '4' || comma[0] == '5')){
            comma[0] += 17;
            fputs(comma, outputFile);
        }
    }

    // Libérer la mémoire allouée par getline
    free(line);

    fclose(inputFile);
    fclose(outputFile);
}


void copyFirstNLines(const char *inputFileName, const char *outputFileName, size_t n) {
    FILE *inputFile = fopen(inputFileName, "r");
    if (inputFile == NULL) {
        fprintf(stderr, "Impossible d'ouvrir le fichier d'entrée.\n");
        exit(EXIT_FAILURE);
    }

    FILE *outputFile = fopen(outputFileName, "w");
    if (outputFile == NULL) {
        fprintf(stderr, "Impossible de créer le fichier de sortie.\n");
        fclose(inputFile);
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t lineSize = 0;
    size_t lineCount = 0;

    // Parcourir ligne par ligne la base de données et copier les premières N lignes dans le fichier de sortie
    while (lineCount < n && getline(&line, &lineSize, inputFile) != -1) {
        fputs(line, outputFile);
        lineCount++;
    }

    // Libérer la mémoire allouée par getline
    free(line);

    fclose(inputFile);
    fclose(outputFile);
}

/*
int main(int argc, char** argv) {

    if (argc < 2){
        printf("equal, shuffle, clone, merge, parse, cpline, remove, letter.\n");
        errx(EXIT_FAILURE, "Invalid format in command line.\n");
    }
    const char *input1FileName = argc > 2? argv[2] : "./data/94_character_TMNIST.csv";
    const char *input2FileName = argc > 3? argv[3] :"./data/mnist_train.csv";
    const char *outputFileName = argc > 4? argv[4] :"output.csv";

    //printf("S = %s\n", outputFileName);

    if (strcmp(argv[1], "equal") == 0){
        if (compareDatabases(input1FileName, input2FileName)) {
            printf("Les bases de données sont égales.\n");
        } else {
            printf("Les bases de données ne sont pas égales.\n");
        }
    }
    else if (strcmp(argv[1], "merge") == 0){
        mergeDatabases(input1FileName, input2FileName, outputFileName);
        printf("Fusion des bases de données terminée avec succès.\n");
    }
    else if (strcmp(argv[1], "shuffle") == 0){
        shuffleDatabase(input1FileName, outputFileName);
        printf("Mélange des lignes de la base de données terminé avec succès.\n");
    }
    else if (strcmp(argv[1], "clone") == 0){
        cloneDatabase(input1FileName, outputFileName);
        printf("Clonage de la base de données terminé avec succès.\n");
    }
    else if (strcmp(argv[1], "parse") == 0){
        parseDatabase(input1FileName, outputFileName);
        printf("Copie de la base de données terminée avec succès.\n");
    }
    else if (strcmp(argv[1], "cpline") == 0){
        copyFirstNLines(input1FileName, outputFileName, MAX_LINES_TO_COPY);
        printf("Copie des premières %d lignes de la base de données terminée avec succès.\n", MAX_LINES_TO_COPY);
    }
    else if (strcmp(argv[1], "remove") == 0){
        removeFirstNLines(input1FileName, outputFileName, LINES_TO_SKIP);
        printf("Suppression des premières %d lignes de la base de données terminée avec succès.\n", LINES_TO_SKIP);

    }
    else if (strcmp(argv[1], "letter") == 0){
        letterDatabase(input1FileName, outputFileName);
        printf("Copie de la base de données terminée avec succès.\n");
    }
    else{
        printf("equal, shuffle, clone, merge, parse, cpline, remove, letter.\n");
        errx(EXIT_FAILURE, "Invalid format in command line.\n");
    }

    return EXIT_SUCCESS;
}*/


//mnist_train.csv
//94_character_TMNIST


