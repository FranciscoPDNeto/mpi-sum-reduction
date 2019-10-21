#include <mpi.h>
#include <cassert>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#define MAINPROC 0

int main(int argc, char** argv) {
  int numProcessors;
  int myRank;

  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcessors);
  MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

  std::string type;
  auto startTime = std::chrono::high_resolution_clock::now();
  float value = 0;

  if (myRank == MAINPROC) {
    std::cin >> type;

    int elementsLenght;
    std::cin >> elementsLenght;
    const int elementsPerProcLenght = elementsLenght / (numProcessors - 1);
    const int lastElementLenght =
        elementsPerProcLenght + elementsLenght % (numProcessors - 1);

    std::vector<float> elements;
    elements.resize(lastElementLenght * (numProcessors - 1), 0.0);
    for (int i = 0; i < elementsLenght; i++)
      std::cin >> elements[i];

    startTime = std::chrono::high_resolution_clock::now();

    for (int dest = 1; dest < numProcessors; dest++) {
      MPI_Send(&lastElementLenght, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
      for (int i = lastElementLenght * (dest - 1);
           i - lastElementLenght * (dest - 1) < lastElementLenght; i++) {
        MPI_Send(&elements[i], 1, MPI_FLOAT, dest, 0, MPI_COMM_WORLD);
      }
    }
  } else {
    int elementsLenght;
    MPI_Recv(&elementsLenght, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);

    std::vector<float> elements;
    elements.reserve(elementsLenght);
    for (int i = 0; i < elementsLenght; i++) {
      float element;
      MPI_Recv(&element, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      elements.push_back(element);
    }

    const int processComm =
        myRank % 2 == 0 ? myRank - 1
                        : myRank + 1 < numProcessors ? myRank + 1 : myRank;

    // Manda todos os elementos para o outro processo.
    for (int i = 1; i < elementsLenght; i++) {
      const float operand2 = elements.back();
      elements.pop_back();
      MPI_Send(&operand2, 1, MPI_FLOAT, processComm, 0, MPI_COMM_WORLD);
    }

    // Recebe todos os elementos do outro processo.
    for (int i = 0; i < elementsLenght - 1; i++) {
      const float operand = elements.back();
      elements.pop_back();

      float operand2;
      MPI_Recv(&operand2, 1, MPI_FLOAT, processComm, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);

      elements.push_back(operand + operand2);
    }

    value = elements.back();
    elements.pop_back();
    assert(elements.empty());
  }

  #ifndef SEQUENTIAL
  // Remoção de processos excedentes
  const int limit = pow(2, (int)log2(numProcessors)) -1;
  const int numExceeds = (numProcessors - 1 - limit);
  const int initIndex = numProcessors - 1 - numExceeds*2;
  if (myRank > initIndex) {
    if (myRank <= limit) {
      float received;
      MPI_Recv(&received, 1, MPI_FLOAT, myRank + numExceeds, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      value += received;
    } else {
      MPI_Send(&value, 1, MPI_FLOAT, myRank - numExceeds, 0, MPI_COMM_WORLD);
    }
  }
  // Árvore de redução
  if (myRank <= limit) {
    if (myRank % 2 == 0)
      MPI_Send(&value, 1, MPI_FLOAT, myRank + 1, 0, MPI_COMM_WORLD);
    for (int d = 0; d < log2(limit + 1); d++) {
      const int power = pow(2, d + 1);
      if ((myRank + 1) % power != 0)
        continue;
      float received;
      MPI_Recv(&received, 1, MPI_FLOAT, myRank - power / 2, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      value += received;
      if (myRank != limit)
        MPI_Send(&value, 1, MPI_FLOAT, myRank + power, 0, MPI_COMM_WORLD);
    }
  }
  #else
  const int limit = numProcessors - 1;
  // Redução sequencial
  if (myRank == MAINPROC)
    MPI_Send(&value, 1, MPI_FLOAT, 1, 0, MPI_COMM_WORLD);
  else if (myRank <= limit) {
    float acc;
    MPI_Recv(&acc, 1, MPI_FLOAT, myRank - 1, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    value += acc;
    if (myRank != limit)
      MPI_Send(&value, 1, MPI_FLOAT, myRank + 1, 0, MPI_COMM_WORLD);
  }
  #endif

  if (myRank == limit)
    MPI_Send(&value, 1, MPI_FLOAT, MAINPROC, 0, MPI_COMM_WORLD);

  // Apresentação do resultado
  if (myRank == MAINPROC) {
    float sum;
    MPI_Recv(&sum, 1, MPI_FLOAT, limit, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    const auto endTime = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime);
    if (type == "sum" || type == "all") {
      std::cout << sum << std::endl;
    }
    if (type == "time" || type == "all") {
      std::cout << duration.count() << std::endl;
    }
  }

  MPI_Finalize();
  return 0;
}