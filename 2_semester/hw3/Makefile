MAKEFLAGS += --silent

all:
	cmake -S . -B build && cmake --build build -t exe -j 4
	./build/exe

.PHONY: all
