
TARGET = rc

CXX = g++
CXXFLAGS = -g -std=c++11 -fno-builtin-memcmp -msse -msse4.2
INCFLAGS = -I ./

OBJS = rc.o main.o

all: $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS)

$(OBJS): %.o : %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o
	rm -f $(TARGET)
