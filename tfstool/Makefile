
TARGET=tfstool.so
OBJS=tfstool.o
CC=gcc
PYTHON_INC=/usr/include/python2.6/ #/opt/python/include/python2.7/
CFLAGS=-fpic -c -I $(PYTHON_INC)

all: $(OBJS)
	$(CC) -shared -o $(TARGET) $(OBJS)

tfstool.o: tfstool.c
	$(CC) -o tfstool.o $(CFLAGS) tfstool.c

clean:
	rm -f $(OBJS) $(TARGET) *.pyc

