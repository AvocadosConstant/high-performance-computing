CXX=g++
CXXFLAGS=-g -std=c++17 -Wall -Wextra -pedantic
BIN=k-nn

SRC=$(wildcard *.cpp)
OBJ=$(SRC:%.cpp=%.o)

all: $(OBJ)
	$(CXX) -Wl,--no-as-needed -ldl -o $(BIN) $^

%.o: %.c
	$(CXX) $@ -c $<

clean:
	rm -f *.o
	rm $(BIN)

v: all
	valgrind -v --leak-check=full ./k-nn
