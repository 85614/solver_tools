#include "multithreading.h"
#include "backend.h"
#include <pthread.h>
#include <malloc.h>
#include <string.h>

const int BACKEND_MULTITHREADING_LOG_BUFFER_SIZE = 1024; // 日志记录缓冲最大大小

static int NUM_THREADS = -1;                   // 计算线程的数量
static pthread_t *THREADS = NULL;              // 计算线程的指针
static pthread_barrier_t *TASK_BARRIER = NULL; // 任务调度同步屏障
static void *TASK_DATA = NULL;                // 任务数据指针
static int *THREAD_IDS = NULL;
static BackendMultiThreadingTaskFunc CURRENT_TASK_FUNC = NULL; // 用于记录当前请求的任务（相当于大小只有1的任务队列）
static char **THREAD_LOG_BUFFER = NULL;                        // 日志记录缓冲，主要用于sprintf，每个线程（包括请求线程）对应一个缓冲

/**
 * 初始化后端多线程资源
 */
void backend_initialize_multithreading(int num_threads)
{
    // TODO: malloc error check
    int i;
    NUM_THREADS = num_threads;
    TASK_BARRIER = (pthread_barrier_t *)malloc(sizeof(pthread_barrier_t));
    pthread_barrier_init(TASK_BARRIER, NULL, num_threads + 1); // 1 is the request thread
    THREAD_IDS = (int *)malloc(sizeof(int) * num_threads);
    THREADS = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
    THREAD_LOG_BUFFER = (char **)malloc(sizeof(char *) * (num_threads + 1));
    for (i = 0; i <= num_threads; ++i)
    {
        THREAD_LOG_BUFFER[i] = (char *)malloc(sizeof(char) * BACKEND_MULTITHREADING_LOG_BUFFER_SIZE);
    }
    for (i = 0; i < num_threads; ++i)
    {
        THREAD_IDS[i] = i;
        pthread_create(THREADS + i, NULL, backend_multithreading_compute_thread_main, THREAD_IDS + i);
    }
}

/**
 * 回收后端多线程资源
 */
void backend_finalize_multithreading()
{
    int res, *res_ptr = &res, i;
    backend_multithreading_request(NULL, NULL);
    for (i = 0; i < NUM_THREADS; ++i)
    {
        pthread_join(THREADS[i], (void **)&res_ptr);
        // TODO: may be need some return values
    }
    pthread_barrier_destroy(TASK_BARRIER);
    for (i = 0; i <= NUM_THREADS; ++i)
    {
        free(THREAD_LOG_BUFFER[i]);
    }
    free(THREAD_LOG_BUFFER);
    free(THREADS);
    free(THREAD_IDS);
    free(TASK_BARRIER);
}

/**
 * Fortran请求后端多线程计算处理函数
 */
void backend_multithreading_request(BackendMultiThreadingTaskFunc task, void *data)
{
#if BACKEND_MUTITHREADING_DEBUG
    char *log_buffer = backend_multithreading_get_request_thread_log_buffer();
    snprintf(log_buffer, BACKEND_MULTITHREADING_LOG_BUFFER_SIZE,
             "[Thread-F] received new task %d request", task);
    backend_log_str_with_time(log_buffer);
#endif
    CURRENT_TASK_FUNC = task;
    TASK_DATA = data;
#if BACKEND_MUTITHREADING_DEBUG
    snprintf(log_buffer, BACKEND_MULTITHREADING_LOG_BUFFER_SIZE,
             "[Thread-F] assigned new task %d and wait for all threads to get the task", task);
    backend_log_str_with_time(log_buffer);
#endif
    pthread_barrier_wait(TASK_BARRIER);

#if BACKEND_MUTITHREADING_DEBUG
    backend_log_str_with_time("[Thread-F] notify all threads get task, reset task info");
#endif
    pthread_barrier_wait(TASK_BARRIER);
#if BACKEND_MUTITHREADING_DEBUG
    snprintf(log_buffer, BACKEND_MULTITHREADING_LOG_BUFFER_SIZE, "[Thread-F] finish task %d", task);
    backend_log_str_with_time(log_buffer);
#endif
}

/**
 * 计算线程主函数
 */
void *backend_multithreading_compute_thread_main(void *args)
{
    int tid = *(int *)args;
    void *task_data;
    BackendMultiThreadingTaskFunc current_task;
    char runing = 1;

#if BACKEND_MUTITHREADING_DEBUG
    char *log_buffer = backend_multithreading_get_compute_thread_log_buffer(tid);
    snprintf(log_buffer, BACKEND_MULTITHREADING_LOG_BUFFER_SIZE, "[Thread-%d] started and is waiting for task", tid);
    backend_log_str_with_time(log_buffer);
#endif

    while (runing)
    {
        pthread_barrier_wait(TASK_BARRIER);
        current_task = CURRENT_TASK_FUNC;
        task_data = TASK_DATA;

#if BACKEND_MUTITHREADING_DEBUG
        snprintf(log_buffer, BACKEND_MULTITHREADING_LOG_BUFFER_SIZE, "[Thread-%d] got task %d", tid, current_task);
        backend_log_str_with_time(log_buffer);
#endif

        if (current_task == NULL)
            runing = 0;
        else
            current_task(task_data, tid);
#if BACKEND_MUTITHREADING_DEBUG
        snprintf(log_buffer, BACKEND_MULTITHREADING_LOG_BUFFER_SIZE,
                 "[Thread-%d] finshed task %d and waits other threads to finish", tid, current_task);
        backend_log_str_with_time(log_buffer);
#endif
        pthread_barrier_wait(TASK_BARRIER);
#if BACKEND_MUTITHREADING_DEBUG
        snprintf(log_buffer, BACKEND_MULTITHREADING_LOG_BUFFER_SIZE,
                 "[Thread-%d] finshed task %d and go on", tid, current_task);
        backend_log_str_with_time(log_buffer);
#endif
    }
#if BACKEND_MUTITHREADING_DEBUG
    snprintf(log_buffer, BACKEND_MULTITHREADING_LOG_BUFFER_SIZE,
             "[Thread-%d] about to exit", tid);
    backend_log_str_with_time(log_buffer);
#endif
    return args;
}

// /**
//  * Fortran请求后端每个计算线程输出hello world，无实际意义，测试用
//  */
// void backend_request_hello_world__()
// {
//     backend_multithreading_request(backend_multithreading_hello_world, NULL);
// }

// /**
//  * 计算线程示例函数：Hello world
//  */
// void backend_multithreading_hello_world(void *data, int thread_id)
// {
//     int num_threads = backend_get_num_threads();
//     Fchar *buffer = backend_multithreading_get_compute_thread_log_buffer(thread_id);
//     snprintf(buffer, BACKEND_MULTITHREADING_LOG_BUFFER_SIZE,
//              "[Thread-%d] hello world, total threads: %d", thread_id, num_threads);
//     backend_log_str_with_time(buffer);
// }

// /**
//  * 统一接口获取每个计算线程对应的日志的buffer
//  */
// inline char *backend_multithreading_get_compute_thread_log_buffer(int thread_id)
// {
//     return THREAD_LOG_BUFFER[thread_id + 1];
// }

// /*
//  * 统一接口获取每个请求线程（也即Fortran主程序线程）对应的日志的buffer
//  */
// inline char *backend_multithreading_get_request_thread_log_buffer()
// {
//     return THREAD_LOG_BUFFER[0];
// }

// /**
//  * 获取后端所有计算线程数
//  */
// inline int backend_get_num_threads()
// {
//     return NUM_THREADS;
// }

#include "defs.h"
#include <assert.h>

void mul_f_para_inner__(void *A, void *B, void *C, int *I_BEGIN, int *I_END);

void mul_f_para_request(void *data, int id)
{
    float *A, *B, *C;
    float **ptr = data;
    A = *ptr++;
    B = *ptr++;
    C = *ptr++;

    assert(A[1] == 1);
    assert(B[1] == 1);
    int I_BEGIN, I_END;
    I_BEGIN = N_DIM * ((double)id / NUM_THREADS) + 1;
    I_END = N_DIM * ((double)(id + 1) / NUM_THREADS);
    // printf("[%d,%d]", I_BEGIN, I_END);
    mul_f_para_inner__(A, B, C, &I_BEGIN, &I_END);
}

void mul_f_para_request__(void *A, void *B, void *C)
{
    void *data[] = {A, B, C};
    void **ptr = data;
    backend_multithreading_request(mul_f_para_request, ptr);
}