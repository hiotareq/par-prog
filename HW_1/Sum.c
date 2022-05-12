#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

double CountFunc(int rank, int size, int N)
{
    double result   = 0;
    double step     = (rank)     * N / size;
    double endpoint = (rank + 1) * N / size - 1;

    if (rank == 0)        step      = 1;
    if (rank == size - 1) endpoint = N;

    for (step; step <= endpoint; step++)
    {
        result += 1 / step;
    }

    printf("[EXECUTOR %d] My result is %lf\n", rank, result);
    return result;    
}

int main(int argc, char ** argv)
{
    if (argc > 2)
    {
        printf("Invalid number of params\n");
        exit(-1);
    }

    MPI_Init(&argc, &argv);
    int proc_num, world_rank, world_size;

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int N = atoi(argv[1]);

    if (N < world_size)
    {
        printf("Won\'t be counted :c\n");
        exit(-1);
    }

    double my_result = CountFunc(world_rank, world_size, N);

    if (world_rank == 0)
    {
        double recv_number = 0;
        double summ = my_result;

        for (proc_num = 1; proc_num < world_size; proc_num++)
        {
            MPI_Recv(&recv_number, 1, MPI_DOUBLE, proc_num, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            summ += recv_number;
        }

        printf("\x1b[31m[RESULT]\x1b[0m Seq result is %f\n", summ);
    }
    else
    {
        MPI_Send(&my_result, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
