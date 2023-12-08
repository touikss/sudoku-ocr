CC = gcc

CFLAGS = -Wunused -Wall -fPIC -I. -O3 -DREENTRANT
CFLAGS += -DGTK_DISABLE_DEPRECATED=1 -DGDK_DISABLE_DEPRECATED
CFLAGS += -DGDK_PIXBUF_DISABLE_DEPRECATED -DG_DISABLE_DEPRECATED
CFLAGS += -DGTK_MULTIHEAD_SAFE=1 -DGTK_MULTIDEVICE_SAFE=1
CFLAGS += `pkg-config gtk+-3.0 --cflags`

LDFLAGS = `pkg-config gtk+-3.0 --libs` `pkg-config gmodule-2.0 --libs`
LDFLAGS += -lm -lSDL2 -lSDL2_image -lpthread

FILES = test.o image.o pixel_operations.o


.c.o:
	$(CC) -c $< $(CFLAGS)

all: $(FILES)
	$(CC) -o test $(FILES) $(LDFLAGS)

clean:
	rm -f *.o test *core*