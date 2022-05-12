#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define ARGUMENT_ERROR   0x0001

#define M_THREAD_SUCCESS 0x0000
#define E_THREAD_SUCCESS 0x0000
#define NUM_THREADS      4

int result_sum = 0;
pthread_mutex_t lock_x;

typedef struct pthread_round_part
{
    int thread_num;
} pthread_r;

void * RoundThread(void * args) 
{
    pthread_r arg = *(pthread_r *) args;
 
    if (&arg == NULL) 
    {
        printf("[Error] Argument can\'t be taken by thread\n");
        exit(ARGUMENT_ERROR);
    }

    while(result_sum + 1 != arg.thread_num) {}
    pthread_mutex_lock(&lock_x);
    result_sum += 1;
    printf("[THREAD %02d] Result_Sum is %d\n", arg.thread_num, result_sum);
    pthread_mutex_unlock(&lock_x);

    return E_THREAD_SUCCESS;
}

int main (int argc, char ** argv)
{
    int status, status_addr, threads_number, i, j, k, N;
        
    if (argc < 2)
    {
        printf("[Error] Not enough arguments to execute :c\n");
        exit(ARGUMENT_ERROR);
    }
    else
    {
        threads_number = atoi(argv[1]);
    }

    pthread_r * s_threads = (pthread_r *)malloc(threads_number * sizeof(pthread_r));
    pthread_t * threads   = (pthread_t *)malloc(threads_number * sizeof(pthread_t));
    
    for (i = 0; i < threads_number; i++) s_threads[i].thread_num = i + 1;
    for (j = 0; j < threads_number; j++) 
    {
        status = pthread_create(&threads[j], NULL, RoundThread, (void*) &s_threads[j]);
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

    printf("Sum is %d\n", result_sum);
    return M_THREAD_SUCCESS;
}