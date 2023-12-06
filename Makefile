# Exe name
EXECUTABLE = main

CC = gcc

# Compilation options
CFLAGS = -Wall -Wextra -std=c99 -g -D_GNU_SOURCE

# Libs
LIBS = -lSDL2 -lSDL2_image -lSDL2_ttf -lm

# Directories
SRC_DIR = sources
HDR_DIR = headers
GRID_DIR = grids

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)

# Object file
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=%.o)

# Default target
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXECUTABLE) $(LIBS)

%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

.PHONY: all clean
