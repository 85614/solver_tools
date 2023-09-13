#include "fortran_types.h"
void backend_push__(const char *func);
void backend_pop__();

void backend_set_comparer(float rtol, float atol, int equal_nan);
void backend_compare_int(int *a, int *b, int N);
void backend_compare_long(long *a, long *b, int N);
void backend_compare_float(float *a, float *b, int N);
void backend_compare_double(double *a, double *b, int N);

void backend_compare_Fint(Fint *a, Fint *b, int N);
void backend_compare_Freal(Freal *a, Freal *b, int N);
void backend_compare_Fdouble(Fdouble *a, Fdouble *b, int N);
