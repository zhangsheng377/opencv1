CXX = g++
EXEC = main
SOURCES = main.cpp
OBJECTS = main.o
LDFLAGS = `pkg-config opencv --libs --cflags opencv`

all :
	$(CXX) $(SOURCES) $(LDFLAGS) -o $(EXEC)

clean:
	rm -f $(EXEC) *.gdb *.o