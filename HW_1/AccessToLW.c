#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char ** argv)
{
    MPI_Init(&argc, &argv);

    int world_rank, world_size, step;
    int SEND_QUANTITY = 10000;
    double time_start, time_finish;

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    double msg_1 = 111.000;
    double msg_2 = 222.000;

    time_start = MPI_Wtime();
    for (step = 0; step < SEND_QUANTITY; step++)
    {
        if (world_rank == 0) MPI_Send(&msg_1, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
        if (world_rank == 1) MPI_Recv(&msg_2, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    if (world_rank == 0)
    {
        time_finish = MPI_Wtime();
        printf("Forwarding time: %1.16lf seconds\n", (time_finish - time_start) / SEND_QUANTITY);
    }

    MPI_Finalize();
    return 0;
}
