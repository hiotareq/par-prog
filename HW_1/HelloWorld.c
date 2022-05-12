#include <mpi.h>
#include <stdio.h>

int main(int argc, char ** argv)
{
    MPI_Init(NULL, NULL);
    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    printf("\x1b[31m[PROCESSOR Quantity %i]\x1b[0m Rank number is %i\n", world_size, world_rank);
    MPI_Finalize();
    return 0;
}
