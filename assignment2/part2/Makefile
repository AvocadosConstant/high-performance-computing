all:
	g++ -o exec.out sphere.cpp -O3 -fopenmp

test: all
	./exec.out false 100000 16 20 100
clean:
	rm *.out
