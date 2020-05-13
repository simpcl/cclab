

CXX=g++
CXXFLAGS=-I/opt/boost-1.69/include --std=c++11
LIBFLAGS=-L/opt/boost-1.69/lib -lboost_system -lboost_thread
OBJS=main.o http_parser.o tcp_server.o http_server.o iobuf.o
TARGET=http_test

$(TARGET): $(OBJS)
	$(CXX) $(LIBFLAGS) -o $(TARGET) $(OBJS)

$(OBJS): %.o : %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
