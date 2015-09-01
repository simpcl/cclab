

TARGET = php_strip_tags
CC = gcc
OBJS = php_strip_tags.o main.o

all: $(OBJS)
	$(CC) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
