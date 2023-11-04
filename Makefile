# Nom de l'exécutable
EXECUTABLE = main

# Compilateur
CC = gcc

# Options de compilation
CFLAGS = -Wall -Wextra -std=c99 -g -D_GNU_SOURCE

# Bibliothèques
LIBS = -lSDL2 -lSDL2_image -lm

# Répertoires
SRC_DIR = sources
HDR_DIR = headers
GRID_DIR = grids

# Fichiers source
SOURCES = $(wildcard $(SRC_DIR)/*.c)

# Fichiers objets
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=%.o)

# Cible par défaut
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXECUTABLE) $(LIBS)

%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

.PHONY: all clean
