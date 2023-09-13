#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include "fortran_types.h"
#include "compare.h"

extern "C"
{
    void backend_set_comparer(float rtol, float atol, int equal_nan);
    void backend_compare_int(int *a, int *b, int N);
    void backend_compare_long(long *a, long *b, int N);
    void backend_compare_float(float *a, float *b, int N);
    void backend_compare_double(double *a, double *b, int N);

    void backend_compare_Fint(Fint *a, Fint *b, int N);
    void backend_compare_Freal(Freal *a, Freal *b, int N);
    void backend_compare_Fdouble(Fdouble *a, Fdouble *b, int N);

    // fortran 调需要全小写
    void backend_compare_fint__(Fint *a, Fint *b, int *N)
    {
        return backend_compare_Fint(a, b, *N);
    }
    void backend_compare_freal__(Freal *a, Freal *b, int *N)
    {
        return backend_compare_Freal(a, b, *N);
    }
    void backend_compare_fdouble__(Fdouble *a, Fdouble *b, int *N)
    {
        return backend_compare_Fdouble(a, b, *N);
    }
}

comparer_t comparer;

void backend_set_comparer(float rtol, float atol, int equal_nan)
{
    rtol = rtol < 0 ? 1e-5 : rtol;
    atol = atol < 0 ? 1e-8 : atol;
    comparer.rtol = rtol;
    comparer.atol = atol;
    comparer.equal_nan = equal_nan;
}

#define DEC_COMPARE(type) \
    void backend_compare_##type(type *a, type *b, int N) { comparer.compare(a, b, N); }

DEC_COMPARE(int);
DEC_COMPARE(long);
DEC_COMPARE(float);
DEC_COMPARE(double);
DEC_COMPARE(Fint);
DEC_COMPARE(Freal);
DEC_COMPARE(Fdouble);
