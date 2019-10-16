#include <cassert>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <mpi.h>
#include <string>
#include <vector>
#include <cmath>
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
    const int lastElementLenght = elementsPerProcLenght + elementsLenght % (numProcessors - 1);

    std::vector<float> elements;
    elements.resize(lastElementLenght * (numProcessors - 1), 0.0);
    for (int i = 0; i < elementsLenght; i++)
      std::cin >> elements[i];
    
    startTime = std::chrono::high_resolution_clock::now();

    for (int dest = 1; dest < numProcessors; dest++) {
      MPI_Send(&lastElementLenght, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
      for (int i = lastElementLenght*(dest -1); i - lastElementLenght*(dest -1) < 
        lastElementLenght; i++) {

        MPI_Send(&elements[i], 1, MPI_FLOAT, dest, 0, MPI_COMM_WORLD);
      }
    }
  } else {
    int elementsLenght;
    MPI_Recv(&elementsLenght, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    std::vector<float> elements;
    elements.reserve(elementsLenght);
    for (int i = 0; i < elementsLenght; i++) {
      float element;
      MPI_Recv(&element, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      elements.push_back(element);
    }

    const int processComm = myRank % 2 == 0 ? 
      myRank - 1 
    : 
      myRank + 1 < numProcessors ? myRank + 1 : myRank;

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
      MPI_Recv(&operand2, 1, MPI_FLOAT, processComm, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      elements.push_back(operand + operand2);
    }

    value = elements.back();
    elements.pop_back();
    assert(elements.empty());
  }

  if (myRank % 2 == 0)
    MPI_Send(&value, 1, MPI_FLOAT, myRank + 1, 0, MPI_COMM_WORLD);
  for (int d = 0; d < log2(numProcessors); d++) {
    const int power = pow(2, d + 1);
    if ((myRank + 1) % power != 0)
      continue;
    float received;
    MPI_Recv(&received, 1, MPI_FLOAT, myRank - power / 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    value += received;
    if (myRank != numProcessors - 1)
      MPI_Send(&value, 1, MPI_FLOAT, myRank + power, 0, MPI_COMM_WORLD);
  }
  if (myRank == numProcessors - 1)
    MPI_Send(&value, 1, MPI_FLOAT, MAINPROC, 0, MPI_COMM_WORLD);

  if (myRank == MAINPROC) {
    float sum;
    MPI_Recv(&sum, 1, MPI_FLOAT, numProcessors - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    const auto endTime = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      endTime - startTime);
    if (type == "all") {
      std::cout << sum << std::endl;
    
      std::cout << duration.count() << std::endl;

    } else if (type == "time") {
      std::cout << duration.count() << std::endl;

    } else {
      std::cout << sum << std::endl;
    }

  }

  MPI_Finalize();
  return 0;
}