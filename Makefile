CC := mpic++
CFLAGS := -g -Wall -O3 -std=c++17

all: bin/main

run: bin/main
	mpirun -n $(n) -H localhost:$(n) bin/main
run-sequential: bin/sequential
	mpirun -n $(n) -H localhost:$(n) bin/sequential

bin/main: TARGET := PARALLEL
bin/sequential: TARGET := SEQUENTIAL
bin/main bin/sequential: build/main.o
	mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^$(TARGET)

build/%.o: src/%.cpp
	mkdir -p build
	$(CC) $(CFLAGS) $(VENDOR) -D$(TARGET) -c -o $@$(TARGET) $<

clean:
	rm bin/* build/*
