# mpi-sum-reduction

> Sum reduction using MPI.

## Build

```sh
make
```

## Running

You can run the program using the `run` make rule as shown below, where `n` is the number of processes used.

```sh
make run n=4
```

The program will expect from the standard input a string `mode`, the number of items and then the numbers to be added.

## Modes

- `time` Shows only the time spent
- `sum` Shows only the sum
- `all` Shows both time and sum
