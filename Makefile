CC := mpic++
CFLAGS := -g -Wall -O3 -std=c++17

all: bin/main

run: bin/main
	mpirun -n $(n) -H localhost:$(n) bin/main

bin/main: build/main.o
	mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^

build/%.o: src/%.cpp
	mkdir -p build
	$(CC) $(CFLAGS) $(VENDOR) -c -o $@ $<

clean:
	rm bin/* build/*
