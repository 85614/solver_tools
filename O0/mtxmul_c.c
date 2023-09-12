typedef float _Ty;
#include "tools.h"
#include "defs.h"
#include <stdio.h>

_Ty a[N_DIM][N_DIM];
_Ty b[N_DIM][N_DIM];
_Ty c[N_DIM][N_DIM];


void CAT(mul_c_, TAG)()
{

    _Ty tmp = 0;
    for (int i = 0; i < N_DIM; ++i)
    {
        for (int j = 0; j < N_DIM; ++j)
        {
            a[i][j] = tmp;
            b[i][j] = tmp;
            tmp += 1;
        }
    }

    for (int l = 0; l < N_LOOP; ++l)
    {
        backend_push__("c-mul-" STR(TAG));
        for (int i = 0; i < N_DIM; ++i)
        {
            for (int j = 0; j < N_DIM; ++j)
            {
                c[i][j] = 0;
                for (int k = 0; k < N_DIM; ++k)
                {
                    c[i][j] += a[i][k] * b[k][j];
                }
            }
        }
        backend_pop__();
    }
    printf("hello%f\n", c[3][3]);
}