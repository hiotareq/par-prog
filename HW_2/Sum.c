#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define ARGUMENT_ERROR   0x0001

#define M_THREAD_SUCCESS 0x0000
#define E_THREAD_SUCCESS 0x0000
#define NUM_THREADS      4

pthread_mutex_t lock_x;
double result_sum = 0;

typedef struct pthread_sum_part
{
    int    thread_num;
    int    left_limit;
    int    right_limit;
    double thread_sum;
} pthread_s;

void * SumThread(void * args) 
{
    int n;
    double sum = 0;

    pthread_s arg = *(pthread_s *) args;
 
    if (&arg == NULL) 
    {
        printf("[Error] Argument can\'t be taken by thread\n");
        exit(ARGUMENT_ERROR);
    }

    for(n = arg.left_limit; n <= arg.right_limit; n++) 
    {
        pthread_mutex_lock(&lock_x);
        result_sum += (double) 1/n;
        pthread_mutex_unlock(&lock_x);
    }
    return E_THREAD_SUCCESS;
}

int main (int argc, char ** argv)
{
    int status, status_addr, threads_number, i, j, k, N;
        
    if (argc < 3)
    {
        printf("[Error] Not enough arguments to execute :c\n");
        exit(ARGUMENT_ERROR);
    }
    else
    {
        N = atoi(argv[1]);
        threads_number = atoi(argv[2]);
    }

    pthread_s * s_threads = (pthread_s *)malloc(threads_number * sizeof(pthread_s));
    pthread_t * threads   = (pthread_t *)malloc(threads_number * sizeof(pthread_t));
    
    for (i = 0; i < threads_number; i++)
    {
        s_threads[i].thread_num  = i + 1;
        s_threads[i].left_limit  = N * i       / threads_number + 1;
        s_threads[i].right_limit = N * (i + 1) / threads_number;
    }

    for (j = 0; j < threads_number; j++) 
    {
        status = pthread_create(&threads[j], NULL, SumThread, (void*) &s_threads[j]);
        if (status != M_THREAD_SUCCESS) 
        {
            printf("[Error] Can't create thread, status = %d\n", status);
            exit(1);
        }
    }

    for (k = 0; k < threads_number; k++) 
    {
        status = pthread_join(threads[k], (void**)&status_addr);
        if (status != M_THREAD_SUCCESS) 
        {
            printf("[Error] Can't join thread, status = %d\n", status);
            exit(0);
        }
    }

    free(s_threads);
    free(threads);

    printf("[Result] Sum is %lf\n", result_sum);
    return M_THREAD_SUCCESS;
}