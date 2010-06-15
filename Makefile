# Makefile

CC=gcc
CFLAGS=-c -Wall -O3
LIBS=-lsndfile
SOURCES=autotalent.c mayer_fft.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=autotune

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LIBS) main.c $(OBJECTS) -o $@

.c.o:
	$(CC) $(LIBS) $(CFLAGS) $< -o $@


clean:
	rm *.o autotune
