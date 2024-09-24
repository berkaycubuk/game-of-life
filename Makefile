build:
	g++ src/main.cpp $(shell pkg-config --libs --cflags raylib) -o conway -std=c++11

run: build
	./conway
