# Exe name
EXECUTABLE = main

CC = gcc

# Compilation options
CFLAGS = -Wall -Wextra -std=c99 -g -D_GNU_SOURCE -Wno-unused-result $(shell pkg-config gtk+-3.0 --cflags)

# Libs
LIBS = -lSDL2 -lSDL2_image -lSDL2_ttf -lm $(shell pkg-config gtk+-3.0 --libs) -I/usr/include/gtk/gtk.h -I/usr/include/gdk/gdk.h -export-dynamic

# Directories
SRC_DIR = sources
HDR_DIR = headers
GRID_DIR = grids
OBJ_DIR = objects

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)

# Object file
#OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=%.o)
OBJECTS = $(addprefix $(OBJ_DIR)/,$(notdir $(SOURCES:.c=.o)))

# Default target
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXECUTABLE) $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

.PHONY: all clean

