
#include "multithreading.h"

void backend_push__(const char *func);
void backend_pop__();
void backend_printf__(const char *fmt, ...);

void test_compare();
int main(int argc, char **argv)
{
    test_compare();
    {
        int a = 3;
        float b = 4;
        const char *s = "hello";
        backend_printf__("test f-printf %d %f %s \n", &a, &b, s);
    }
    backend_initialize_multithreading(6);
    backend_push__("hello");
    backend_request_hello_world__();
    backend_pop__();
    backend_finalize_multithreading();
    return 0;
}

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