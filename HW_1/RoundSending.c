#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char ** argv)
{
    MPI_Init(&argc, &argv);
    int world_rank, world_size;

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int msg = 0;
    if (world_rank != 0)
    {
        MPI_Recv(&msg, 1, MPI_INT, world_rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("I\'m executor %d received number %d from executor %d\n", world_rank, msg, world_rank - 1);
        msg++;
    }

    MPI_Send(&msg, 1, MPI_INT, (world_rank + 1) % world_size, 0, MPI_COMM_WORLD);

    if (world_rank == 0)
    {
        MPI_Recv(&msg, 1, MPI_INT, world_size - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("I\'m executor %d received number %d from executor %d\n", world_rank, msg, world_size - 1);
    }

    MPI_Finalize();
    return 0;
}
