
LIBS=
CC=gcc
CFLAGS=-g -Wall -O2 -fno-strict-aliasing
TARGETS=ae_test
OBJECTS=zmalloc.o ae.o main.o

all: $(TARGETS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

ae_test: $(OBJECTS)
	$(CC) $(CFLAGS) $(LIBS) -o ae_test $(OBJECTS)

clean:
	rm -f *.o ae_test

