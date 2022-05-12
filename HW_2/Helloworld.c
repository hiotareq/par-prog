#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define ARGUMENT_ERROR   0x0001

#define M_THREAD_SUCCESS 0x0000
#define E_THREAD_SUCCESS 0x0000
#define NUM_THREADS      4

void * HelloThread(void * args) 
{
    int arg = *(int *) args;
 
    if (&arg == NULL) 
    {
        printf("[Error] Argument can\'t be taken by thread\n");
        exit(ARGUMENT_ERROR);
    }

    printf("Hello World from %d thread!\n", arg);
    return E_THREAD_SUCCESS;
}

int main (int argc, char ** argv)
{
    int status, status_addr, threads_number, i, j, k;
        
    if (argc < 2)
    {
        printf("[Error] No arguments to execute :c\n");
        exit(ARGUMENT_ERROR);
    }
    else
    {
        threads_number = atoi(argv[1]);
    }

    int *     n_threads = (int *)malloc(threads_number * sizeof(int));
    pthread_t * threads = (pthread_t *)malloc(threads_number * sizeof(pthread_t));
    
    for (i = 0; i < threads_number; i++)
    {
        n_threads[i] = i;
    }

    for (j = 0; j < threads_number; j++) 
    {
        status = pthread_create(&threads[j], NULL, HelloThread, (void*) &n_threads[j]);
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

    free(n_threads);
    free(threads);

    return M_THREAD_SUCCESS;
}