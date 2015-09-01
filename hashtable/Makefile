
PREFIX		= /usr/local/hashtable/

INCLUDES	= -I.
LIBS		= -L.
CC		= gcc
CFLAGS		= -g -Wall -O1 -fno-strict-aliasing
TARGETS		= hashtable
OBJECTS		= hashtable.o hash.o slabs.o main.o

all: $(TARGETS)

%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

$(TARGETS): $(OBJECTS)
	$(CC) $(CFLAGS) $(LIBS) -o $(TARGETS) $(OBJECTS)

clean:
	rm -f *.o $(TARGETS)

install:
	mkdir -p $(PREFIX)
