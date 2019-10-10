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
    const int elementsPerProcLenght = elementsLenght / numProcessors;

    std::vector<float> elements(elementsLenght);
    for (int i = 0; i < elementsLenght; i++)
      std::cin >> elements[i];
    
    for (int dest = 1; dest < numProcessors; dest++) {
      MPI_Send(&elementsLenght, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
      for (int i = elementsPerProcLenght*(dest -1); i - elementsPerProcLenght*dest < 
        + elementsPerProcLenght; i++) {

        MPI_Send(&elements[i], 1, MPI_FLOAT, dest, 0, MPI_COMM_WORLD);
      }
    }

    clock_t tStart = clock();

    // TODO: distribution of work and receive Sum.

    clock_t tEnd = clock();
    if (type == "all") {
      std::cout << "Value to print" << std::endl;
    
      const double duration = double(tEnd - tStart)/CLOCKS_PER_SEC;
      std::cout << "Time: " << std::setprecision(6) << duration << " seconds" << std::endl;

    } else if (type == "time") {
      const double duration = double(tEnd - tStart)/CLOCKS_PER_SEC;
      std::cout << "Time: " << std::setprecision(6) << duration << " seconds" << std::endl;

    } else {
      std::cout << "Value to print" << std::endl;
    }

  } else {
    int elementsLenght;
    MPI_Recv(&elementsLenght, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    std::vector<float> elements;
    for (int i = 0; i < elementsLenght; i++) {
      MPI_Recv(&elements[i], 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    while(elements.size() > 1) {
      const float operand = elements.back();
      elements.pop_back();

      MPI_Send(&operand, 1, MPI_FLOAT, myRank+1, 0, MPI_COMM_WORLD);
    }
    // Só sendo garantido que o número elementos é igual entre os processos.
    for (int i = 0; i < elementsLenght -1; i++) {
      const float operand1 = elements.back();
      elements.pop_back();

      float operand2;
      MPI_Recv(&operand2, 1, MPI_FLOAT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      elements.push_back( operand1 + operand2);
    }
  }

  MPI_Finalize();
  return 0;
}