
#include "multithreading.h"

void backend_push__(const char *func);
void backend_pop__();
void backend_printf__(const char *fmt, ...);

void test_compare();
void test_multithreading();
void test_push_pop();
void test_clock_multithread();

void mul_f__();
void mul_c_O3();
void mul_f_para__();
void mul_f_openmp__();
void mul_c_O0();
int main(int argc, char **argv)
{
    mul_c_O3();
    mul_c_O0();
    mul_f__();
    mul_f_openmp__();
    backend_initialize_multithreading(4);
    mul_f_para__();
    backend_finalize_multithreading();
    // test_compare();
    // test_multithreading();
    // test_push_pop();
    // test_clock_multithread();
    return 0;
}

/////////////// test_clock_multithread ////////////////////////
#include <assert.h>
#include <stdio.h>
void do_nothing_request(void *data, int threadid)
{
    backend_push__("do_nothing");
    backend_pop__();
}

void test_clock_multithread()
{
    const int NUM_THREADS = 2, N = 1024 * 1024;
    backend_initialize_multithreading(NUM_THREADS);
    backend_push__("test_clock_multithread");
    for (int i = 0; i < N; ++i)
    {
        backend_push__("request");
        backend_multithreading_request(do_nothing_request, NULL);
        backend_pop__();
        backend_push__("do_nothing");
        backend_pop__();
    }
    backend_pop__();
    backend_finalize_multithreading();
}

/////////////// test_push_pop ////////////////////////

void test_push_pop()
{
    char msg[2] = {"a"};
    for (int n = 64; n <= 1024 * 1024; n *= 4, ++msg[0])
    {
        backend_push__(msg);
        for (int i = 0; i < n; ++i)
        {
            backend_push__("do_nothing");
            backend_pop__();
        }
        backend_pop__();
    }
}
/////////////// test_multithreading ////////////////////////
#include <assert.h>
#include <stdio.h>
void do_little_request(void *data, int threadid)
{
    int *arr = data;
    ++arr[threadid];
}

void test_multithreading()
{
    const int NUM_THREADS = 6, N = 1024 * 1024;
    backend_initialize_multithreading(NUM_THREADS);
    int arr[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        arr[i] = 0;
    }
    backend_push__("test_multithreading");
    for (int i = 0; i < N; ++i)
    {
        backend_push__("count");
        backend_multithreading_request(do_little_request, arr);
        backend_pop__();
        backend_push__("do_nothing");
        backend_pop__();
    }
    backend_pop__();
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        assert(arr[i] == N);
        printf("%d\n", arr[i]);
    }

    backend_finalize_multithreading();
}

/////////////// test_fprintf ////////////////////////
void test_fprintf()
{
    int a = 3;
    float b = 4;
    const char *s = "hello";
    backend_printf__("test f-printf %d %f %s \n", &a, &b, s);
}

/////////////// test_compare ////////////////////////
#include <math.h>
void backend_set_comparer(float rtol, float atol, int equal_nan);
void backend_compare_int(int *a, int *b, int N);
void backend_compare_float(float *a, float *b, int N);
void backend_compare_double(double *a, double *b, int N);
void backend_set_comparer(float rtol, float atol, int equal_nan);
void test_compare()
{
    {
        int a[10] = {1, 2, 3, 4, 5, 2, 3, 4, 5};
        int b[10] = {1, 3, 3, 4, 5, 2, 3, 4, 5};
        backend_compare_int(a, b, 10);
    }
    {
        float a[10] = {NAN, INFINITY, 0, 4, NAN, 6, 7, 0, 9.1, 10};
        float b[10] = {NAN, 2, 0, 4, 5, 6, 7, 8, 9, 0};
        backend_compare_float(a, b, 10);
    }

    {
        double a[10] = {NAN, INFINITY, 0, 4, NAN, 6, 7, 0, 9.1, 10};
        double b[10] = {NAN, 2, 0, 4, 5, 6, 7, 8, 9, 10 * (1e-5 + 1)};
        backend_compare_double(a, b, 10);
        backend_set_comparer(5e-6, 0, 1);
        backend_compare_double(a, b, 10);
    }
}