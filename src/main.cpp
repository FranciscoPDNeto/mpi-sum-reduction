#include <mpi.h>
#include <time.h>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

void send(const float value, const int to);
float receive(const int from);

int main(int argc, char** argv) {
  int processors;
  int rank;

  MPI_Init(nullptr, nullptr);
  MPI_Comm_size(MPI_COMM_WORLD, &processors);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  const int master = 0;
  const int last = processors - 1;

  clock_t start;
  std::string type;
  if (rank == master) {
    start = clock();
    std::cin >> type;

    int elements;
    std::cin >> elements;
    for (int i = 0; i < elements; i++) {
      float value;
      std::cin >> value;
      send(value, i);
    }
  }

  float value = receive(master);
  if (rank % 2 == 0)
    send(value, rank + 1);
  for (int d = 0; d < log2(processors); d++) {
    const int power = pow(2, d + 1);
    if ((rank + 1) % power != 0)
      continue;
    value += receive(rank - power / 2);
    if (rank != last)
      send(value, rank + power);
  }

  if (rank == last)
    send(value, master);
  if (rank == master) {
    value = receive(last);
    double duration = (clock() - start) / CLOCKS_PER_SEC;
    if (type == "sum" || type == "all")
      std::cout << std::setprecision(2) << value << std::endl;
    if (type == "time" || type == "all")
      std::cout << duration << std::endl;
  }

  MPI_Finalize();
  return 0;
}

void send(const float value, const int to) {
  MPI_Request request;
  MPI_Isend(&value, 1, MPI_FLOAT, to, 0, MPI_COMM_WORLD, &request);
}
float receive(const int from) {
  float value;
  MPI_Recv(&value, 1, MPI_FLOAT, from, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  return value;
}
