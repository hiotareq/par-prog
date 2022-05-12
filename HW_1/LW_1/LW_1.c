#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double ** Array2dCreate(double ** array2d, int col, int row);
void      Array2dPrint (double ** array2d, int col, int row);
void      Array2dDelete(double ** array2d, int col, int row);

double * AxisCreate(double * axis, int size, double step);
void     AxisDelete(double * axis, int size);

void EquationInitCond(double ** u, double h, double tau, long int M, long int K, int world_rank, int world_size);
void ExecutorAction  (double ** u, double h, double tau, long int M, long int K, int world_rank, int world_size);


double AnalyticSolution (double x, double t);
double a  (double x, double t);
double f  (double x, double t);
double phi(double x);
double ksi(double t);

double ** Array2dCreate(double ** array2d, int col, int row)
{
    int c;
    array2d = (double **)malloc(col * sizeof(double *));

    for (c = 0; c < col; c++)
    {
        array2d[c] = (double *)calloc(row, sizeof(double));
    }

    return array2d;
}

void Array2dPrint(double ** array2d, int col, int row) //(double ** sol, double x_size, double t_size)
{
    int r,c;

    for (c = 0; c < col; c++)
    {
        for (r = 0; r < row; r++)
        {
            printf("%lf ", array2d[c][r]);
        }
        printf("\n");
    }
}

void Array2dPrintT(double ** array2d, int col, int row) //(double ** sol, double x_size, double t_size)
{
    int r,c;

    for (r = 0; r < row; r++)
    {
        for (c = 0; c < col; c++)
        {
            printf("%lf ", array2d[c][r]);
        }
        printf("\n");
    }
}

void Array2dDelete(double ** array2d, int col, int row)
{
    int c;

    for (c = 0; c < col; c++)
    {
        free(array2d[c]);
    }

    free(array2d);
}

double * AxisCreate(double * axis, int size, double step)
{
    int i;
    axis = (double *)malloc(size * sizeof(double));

    for (i = 0; i < size; i++)
    {
        axis[i] = i * step;
    }

    return axis;
}

void AxisDelete(double * axis, int size)
{
    free(axis);
}

double a(double x, double t)
{
    return 2;
}

double phi(double x)
{
    return cos(M_PI * x);
}

double ksi(double t)
{
    return exp(-t);
}

double f(double x, double t)
{
    return x + t;
}

double AnalyticSolution(double x, double t)
{
    if (2 * t <= x)  return x * t - t * t * 0.5 + cos(M_PI * (2 * t - x));
    else             return x * t - t * t * 0.5 + (2 * t - x) * (2 * t - x) * 0.125 + exp(-(t - x * 0.5));
}


void ExecutorAction(double ** u, double h, double tau, long int M, long int K, int world_rank, int world_size)
{
    int m, k;

    double x_m, t_k;
    double value[2];

    unsigned int m_s = M * (world_rank) / (world_size);
    unsigned int M_p = (int) (M * (world_rank + 1)/ world_size - M * (world_rank)/ world_size);

    for (k = 0; k < K - 1; k++)
    {
        t_k = k * tau;

        if (world_rank != 0)
        {
            MPI_Recv(&value, 2, MPI_DOUBLE, world_rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            u[0][k + 1] = 2 * h * tau / (h + a(m_s * h, k * tau) * tau) * f((m_s + 0.5) * h, (k + 0.5) * tau) + (h - a(m_s * h, k * tau) * tau) / (h + a(m_s * h, k * tau) * tau) * (u[0][k] - value[1]) + value[0];
        }

        for (m = 1; m < M_p; m++)
        {
            x_m = m * h;
            u[m][k + 1] = 2 * h * tau / (h + a((m_s + m) * h, k * tau) * tau) * f((m_s + m + 0.5) * h, (k + 0.5) * tau) + (h - a((m_s + m) * h, k * tau) * tau) / (h + a((m_s + m) * h, k * tau) * tau) * (u[m][k] - u[m - 1][k + 1]) + u[m - 1][k];
        }

        if (world_rank != world_size - 1)
        {
            value[0] = u[M_p - 1][k];
            value[1] = u[M_p - 1][k + 1];

            MPI_Send(&value, 2, MPI_DOUBLE, world_rank + 1, 0, MPI_COMM_WORLD);
        }
    }
}

void EquationInitCond(double ** u, double h, double tau, long int M, long int K, int world_rank, int world_size)
{
    unsigned int k;

    if (world_rank == 0)
    {
        for (k = 0; k < K; k++)
        {
            u[0][k] = ksi(k * tau);
        }
    }

    unsigned int m;
    unsigned int M_gl = M * (world_rank) / (world_size);
    unsigned int M_p  = (int) (M * (world_rank + 1)/ world_size - M * (world_rank)/ world_size);

    for (m = 0; m < M_p; m++)
    {
        u[m][0] = phi((M_gl + m) * h);
    }
}

int main (int argc, char ** argv)
{
    double tau, h;

    if (argc < 2)
    {
        printf("[Error ] Not engough arguments :c\n");
        exit(-1);
    }
    else
    {
        tau = atof(argv[1]);
        h   = 2 * tau;
    }

    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    double final_error;
    double time_start = MPI_Wtime();

    unsigned long int M = 1 / h;
    unsigned long int K = 1 / tau;

    unsigned int M_p = (int) (M * (world_rank + 1) / world_size - M * (world_rank)/ world_size);

    if (world_rank == 0)
    {
        printf("[ Info ] Test for %d executors\n", world_size);
        printf("[ Info ] dt: %lf; dx: %lf; dots quantity: %ld\n", tau, h, M * K);
    }

    double ** u = Array2dCreate(u, M_p, K);
    EquationInitCond(u, h, tau, M, K, world_rank, world_size);
    ExecutorAction  (u, h, tau, M, K, world_rank, world_size);


    if (world_size > 1)
    {
        if (world_rank == 0)
        {
            /*
            int m, k, exec;
            double ** u_r = Array2dCreate(u_r, M, K);

            for (k = 0; k < K;   k++)
            {
                for (m = 0; m < M_p; m++)
                {
                    u_r[m][k] = u[m][k];
                }
            }

            for (exec = 1; exec < world_size; exec++)
            {
                for (m = M * exec / world_size; m < M * (exec + 1) / world_size; m++)
                {
                    MPI_Recv(&u_r[m][0], K, MPI_DOUBLE, exec, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
            }
            */
            double time_finish = MPI_Wtime();
            printf("[Exec %d] Time %lf\n", world_rank, time_finish - time_start);

            /*
            double err = 0;
            double prev_err = 0;

            for (k = 0; k < K; k++)
            {
                for (m = 0; m < M; m++)
                {
                    prev_err = fabs(u_r[m][k] - AnalyticSolution(m * h, k * tau));
                    if (err < prev_err) err = prev_err;
                }
            }
            final_error = err;
            Array2dDelete(u_r, M, K);
            */
        }
        else
        {

            int m;
            /*
            for (m = 0; m < M_p; m++)
            {
                MPI_Send(&u[m][0], K, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
            }
            */

            double time_finish = MPI_Wtime();
            printf("[Exec %d] Time %lf\n", world_rank, time_finish - time_start);
        }
    }

    if (world_rank == 0)
    {
        if (world_size == 1)
        {
            double time_finish = MPI_Wtime();
            printf("[Exec %d] Time %lf\n", world_rank, time_finish - time_start);
        }
        //printf("[Result] Error = %1.16lf\n---\n", final_error);
    }

    Array2dDelete(u, M_p, K);
    MPI_Finalize();
    return 0;
}
