#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>

/*
Вариант на отл(10)
Программа должна реализовать численное интегрирование заданной
функции на заданном интервале с использованием распараллеливания с
общей памятью. Аргументы программы – число потоков, необходимая
точность интегрирования. Требуется обеспечить оптимальный шаг для
каждого из участков интегрирования и динамическую балансировку между
исполнителями, а также универсальность программы (последнее
подразумевает, что функция записана только в одном месте в программе,
оценка её поведения проводится численными методами, а не при помощи
заранее вычисленных производных разных степеней).
Требуется измерить ускорение и эффективность программы.
*/

#define ARGUMENT_ERROR   0x0010
#define M_THREAD_SUCCESS 0x0000
#define E_THREAD_SUCCESS 0x0000

#define STACK_FAILURE    0x0011

#define STACK_SIZE       100000

#define CONDITION_FALSE  0x0000
#define CONDITION_TRUE   0x0001

#define LEFT_LIMIT       0.00001
#define RIGHT_LIMIT      1.00000

typedef struct
{
    double a;
    double b;
    double fa;
    double fb;
    double sAB;
} data_fragment_t;

typedef struct
{
    data_fragment_t data[STACK_SIZE];
    int sp;
} data_stack_t;

typedef struct
{
    double s_all;
    int n_active;
    int maxtask;
    sem_t sem_sum;
    sem_t sem_list;
    sem_t sem_task_present;
} data_sdat_t;

int SPK = 35;
double error = 0;
data_stack_t global_stack = {};
data_sdat_t  sdat         = {};

#define STACK_IS_FREE(stack) (stack.sp == 0)
#define PUT_INTO_STACK(stack, a, b, fa, fb, sab)                       \
    {                                                                  \
        if (stack.sp == STACK_SIZE)                                    \
        {                                                              \
            printf("[Error] Stack (%s) is full.\n", #stack);           \
            exit(STACK_FAILURE);                                       \
        }                                                              \
        stack.data[stack.sp++] = (data_fragment_t){a, b, fa, fb, sab}; \
    }

#define GET_FROM_STACK(stack, a, b, fa, fb, sab)                       \
    {                                                                  \
        if (stack.sp == 0)                                             \
        {                                                              \
            printf("[Error] Stack (%s) is empty.\n", #stack);          \
            exit(STACK_FAILURE);                                       \
        }                                                              \
        data_fragment_t temp = stack.data[--stack.sp];                 \
        a   = temp.a;                                                  \
        b   = temp.b;                                                  \
        fa  = temp.fa;                                                 \
        fb  = temp.fb;                                                 \
        sab = temp.sAB;                                                \
    }

#define MOVE_STACK_ELEM(stack_1, stack_2) memcpy(stack_1.data + stack_1.sp++, stack_2.data + --stack_2.sp, sizeof(data_fragment_t));
#define BREAK_CONDITION(sAB, sACB, a, b)  fabs(sAB - sACB) < error * fabs(sACB) ? CONDITION_TRUE : CONDITION_FALSE

double function(double x)
{
    double y = 1 / x;
    double sin_y = sin(y);
    return y * y * sin_y * sin_y;
}

void * IntegralThread(void *args)
{
    int thread_num = *(int *)args;
    int i;
    int seg_counter = 0;

    double a, b, c, fa, fb, fc, sac, sab, scb, sacb;
    double s = 0;

    clock_t cpu_time = clock();
    time_t wall_time = time(NULL);

    data_stack_t local_stack = {};
    local_stack.sp = 0;

    while (1)
    {
        sem_wait(&sdat.sem_task_present);
        sem_wait(&sdat.sem_list);

        GET_FROM_STACK(global_stack, a, b, fa, fb, sab);

        if (!STACK_IS_FREE(global_stack)) sem_post(&sdat.sem_task_present);

        if (a <= b)
        {
            sdat.n_active++;
        }

        sem_post(&sdat.sem_list);

        if (a > b)
        {
            break;
        }

        while (1)
        {
            //Initialization
            c    = (a + b) * 0.5;
            fc   = function(c);
            sac  = (fa + fc) * (c - a) * 0.5;
            scb  = (fc + fb) * (b - c) * 0.5;
            sacb = sac + scb;
            seg_counter++;

            //Has the necessary precision been achieved?
            if (BREAK_CONDITION(sab, sacb, a, b))
            {
                s += sacb;

                if (STACK_IS_FREE(local_stack))
                {
                    break;
                }
                else
                {
                    GET_FROM_STACK(local_stack, a, b, fa, fb, sab);
                }
            }
            else
            {
                PUT_INTO_STACK(local_stack, a, c, fa, fc, sac);
                a   = c;
                fa  = fc;
                sab = scb;
            }

            //Add segments to the global stack?
            if ((local_stack.sp > SPK) && (STACK_IS_FREE(global_stack)))
            {
                sem_wait(&sdat.sem_list);

                while ((local_stack.sp > 1) && (global_stack.sp < STACK_SIZE)) // sdat.maxtask
                {
                    MOVE_STACK_ELEM(global_stack, local_stack);
                }

                if (!STACK_IS_FREE(global_stack)) sem_post(&sdat.sem_task_present);
                sem_post(&sdat.sem_list);
            }
        }

        sem_wait(&sdat.sem_list);
        sdat.n_active--;

        if (sdat.n_active == 0 && STACK_IS_FREE(global_stack))
        {
            for (i = 0; i < sdat.maxtask; i++)
            {
                PUT_INTO_STACK(global_stack, 2.0, 1.0, 0, 0, 0);
            }

            sem_post(&sdat.sem_task_present);
        }

        sem_post(&sdat.sem_list);
    }

    sem_wait(&sdat.sem_sum);
    sdat.s_all += s;
    printf("[Thread %d] Adding %lf to the total amount (total amount = %lf)\n", thread_num, s, sdat.s_all);

    cpu_time  = clock()    - cpu_time;
    wall_time = time(NULL) - wall_time;

    printf("%*sCalculated %d segments\n", 11, " ", seg_counter);
	printf("%*sCPU time: %dm %02ds, Wall time: %ldm %02lds\n", 11, " ", (int) ((cpu_time / CLOCKS_PER_SEC / 60)), (int) ((cpu_time / CLOCKS_PER_SEC) % 60), wall_time / 60, wall_time % 60);

    sem_post(&sdat.sem_sum);
    return E_THREAD_SUCCESS;
}

int main(int argc, char **argv)
{
    int status, status_addr, threads_number, i, j, k;

    if (argc < 3)
    {
        printf("[Error] Not enough arguments to execute :c\n");
        exit(ARGUMENT_ERROR);
    }
    else
    {
        threads_number = atoi(argv[1]);
        error          = atof(argv[2]);
    }

    global_stack.sp = 0;
    sdat.n_active = 0;
    sdat.maxtask = threads_number;

    sem_init(&sdat.sem_sum, 0, 1);
    sem_init(&sdat.sem_list, 0, 1);
    sem_init(&sdat.sem_task_present, 0, 0);

    sdat.s_all = 0;

    double m_A, m_B, m_fA, m_fB, m_sAB;
    m_A   = LEFT_LIMIT;
    m_B   = RIGHT_LIMIT;
    m_fA  = function(m_A);
    m_fB  = function(m_B);
    m_sAB = 0.5 * (m_fA + m_fB) * (m_B - m_A);

    printf("[  Info  ] Numerical integration of a function(x) on a segment (%1.6lf, %1.6lf)\n", m_A, m_B);
    printf("[  Info  ] Precision: %1.6lf; Number of threads: %d\n", error, sdat.maxtask);

    PUT_INTO_STACK(global_stack, m_A, m_B, m_fA, m_fB, m_sAB);
    sem_post(&sdat.sem_task_present);

    int *n_threads = (int *)malloc(threads_number * sizeof(pthread_t));
    pthread_t *threads = (pthread_t *)malloc(threads_number * sizeof(pthread_t));

    for (i = 0; i < threads_number; i++)
    {
        n_threads[i] = i;
    }

    for (j = 0; j < threads_number; j++)
    {
        status = pthread_create(&threads[j], NULL, IntegralThread, (void *)&n_threads[j]);
        if (status != M_THREAD_SUCCESS)
        {
            printf("[Error] Can't create thread, status = %d\n", status);
            exit(1);
        }
    }

    for (k = 0; k < threads_number; k++)
    {
        status = pthread_join(threads[k], (void **)&status_addr);
        if (status != M_THREAD_SUCCESS)
        {
            printf("[Error] Can't join thread, status = %d\n", status);
            exit(0);
        }
    }

    printf("[ Result ] J = %lf\n", sdat.s_all);
    return M_THREAD_SUCCESS;
}

