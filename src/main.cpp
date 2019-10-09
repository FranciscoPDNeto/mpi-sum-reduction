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
    MPI_Bcast(&elementsLenght, 1, MPI_INT, MAINPROC, MPI_COMM_WORLD);
    const int elementsPerProcLenght = elementsLenght / numProcessors;

    std::vector<float> elements(elementsLenght);
    for (int i = 0; i < elementsLenght; i++)
      std::cin >> elements[i];

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
    MPI_Bcast(&elementsLenght, 1, MPI_INT, MAINPROC, MPI_COMM_WORLD);
  }

  MPI_Finalize();
  return 0;
}