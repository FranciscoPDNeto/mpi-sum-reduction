#include <cassert>
#include <iomanip>
#include <iostream>
#include <mpi.h>
#include <string>
#include <vector>
#define MAINPROC 0

int main(int argc, char** argv) {
  int numProcessors;
  int myRank;

  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcessors);
  MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

  if (myRank == MAINPROC) {
    
    std::string type;
    std::cin >> type;

    int elementsLenght;
    std::cin >> elementsLenght;
    const int elementsPerProcLenght = elementsLenght / (numProcessors - 1);

    std::vector<float> elements(elementsLenght);
    for (int i = 0; i < elementsLenght; i++)
      std::cin >> elements[i];
    
    for (int dest = 1; dest < numProcessors; dest++) {
      MPI_Send(&elementsPerProcLenght, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
      for (int i = elementsPerProcLenght*(dest -1); i - elementsPerProcLenght*(dest -1) < 
        elementsPerProcLenght; i++) {

        MPI_Send(&elements[i], 1, MPI_FLOAT, dest, 0, MPI_COMM_WORLD);
      }
    }

    float sum = 0;
    for (int dest = 1; dest < numProcessors; dest++) {
      float element;
      MPI_Recv(&element, 1, MPI_FLOAT, dest, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      sum += element;
    }

    clock_t tStart = clock();

    // TODO: distribution of work and receive Sum.

    clock_t tEnd = clock();
    if (type == "all") {
      std::cout << "Value to print: " << sum << std::endl;
    
      const double duration = double(tEnd - tStart)/CLOCKS_PER_SEC;
      std::cout << "Time: " << std::setprecision(6) << duration << " seconds" << std::endl;

    } else if (type == "time") {
      const double duration = double(tEnd - tStart)/CLOCKS_PER_SEC;
      std::cout << "Time: " << std::setprecision(6) << duration << " seconds" << std::endl;

    } else {
      std::cout << "Value to print: " << sum << std::endl;
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

    int processComm = myRank % 2 == 0 ? 
      myRank - 1 
    : 
      myRank + 1 < numProcessors ? myRank + 1 : 1;

    // O processo master não recebe número pra fazer soma, só faz a final.
    if (processComm == MAINPROC)
      processComm += 1;

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

    const float operand = elements.back();
    elements.pop_back();
    assert(elements.empty());
    MPI_Send(&operand, 1, MPI_FLOAT, MAINPROC, 0, MPI_COMM_WORLD);

  }

  MPI_Finalize();
  return 0;
}