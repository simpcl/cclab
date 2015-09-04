CXX		= gcc
OBJS		= semutil.o shmmq.o
LIBS		= -L/usr/lib/nptl -lpthread
INCLUDES	= -I/usr/include/nptl
HEADERS		= semutil.h shmmq.h


All: libshmmq.so

libshmmq.so: $(OBJS)
	$(CXX) -fPIC -Wall -shared -o libshmmq.so $(OBJS)

%.o: %.c
	$(CXX) -fPIC -c $< -o $@

clean:
	rm -f *.o
	rm -f *.so

