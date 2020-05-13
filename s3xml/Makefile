
TARGET = a

CXX = g++
CXXFLAGS = -g -std=c++11 -O0 -fno-builtin-memcmp -msse -msse4.2
#INCFLAGS = -I./ -I/home/simpcl/github/
INCFLAGS = -I./ -I/home/simpcl/work/s3-gateway/third/rapidxml/

OBJS = s3_xml.o main.o

all: $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS)

$(OBJS): %.o : %.cc
	$(CXX) $(CXXFLAGS) $(INCFLAGS) -c $< -o $@

clean:
	rm -f *.o
	rm -f $(TARGET)
