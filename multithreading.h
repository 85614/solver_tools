#ifndef BACKEND_MULTITHREADING_H
#define BACKEND_MULTITHREADING_H

#include <stddef.h>

typedef void (*BackendMultiThreadingTaskFunc)(void *, int);

extern const int BACKEND_MULTITHREADING_LOG_BUFFER_SIZE; // 日志记录缓冲最大大小

void backend_initialize_multithreading(int num_threads);

void backend_multithreading_request(BackendMultiThreadingTaskFunc task, void *data);

void *backend_multithreading_compute_thread_main(void *args);

void backend_finalize_multithreading();

// void backend_multithreading_hello_world(void *data, int thread_id);

// inline char *backend_multithreading_get_compute_thread_log_buffer(int thread_id);

// inline char *backend_multithreading_get_request_thread_log_buffer();

// inline int backend_get_num_threads();

// 非0值表示DEBUG模式，会在日志中输出每个线程的详细行为
#define BACKEND_MUTITHREADING_DEBUG 0

#endif