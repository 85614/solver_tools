#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include "fortran_types.h"

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

template <typename _Ty>
static bool greater(_Ty a, _Ty b)
{
    if (!std::isunordered(a, b))
        return a > b;
    else
        return std::isfinite(b);
}

struct comparer_t
{
    // using _Ty = float;
    // using float_t = typename std::conditional<std::is_floating_point<_Ty>::value, _Ty, double>::type;
    using float_t = double;

    float atol = 1e-8;
    float rtol = 1e-5;
    bool equal_nan = true;
    int max_print = 5;

    template <typename _Ty>
    _Ty cal_atol(_Ty a, _Ty b)
    {
        if (equal_nan && std::isnan(a) && std::isnan(b))
            return 0;
        return std::abs(a - b);
    }

    float_t cal_rtol(float_t a, float_t b)
    {
        if (equal_nan && std::isnan(a) && std::isnan(b))
            return 0;
        return std::abs((a - b) / (std::max(std::abs(a), std::abs(b)) + std::numeric_limits<float_t>::min()));
#if 0
        if (a == 0 && b == 0)
        {
            return 0;
        }
        else
        {
            return std::abs((a - b) / std::max(std::abs(a), std::abs(b)));
            return std::abs((a - b) / (b));
            return std::abs((a - b) / (b + std::numeric_limits<float_t>::denorm_min())); // 加了也是 x/0 = inf
        }
#endif
    }

    template <typename _Ty, typename std::enable_if<std::is_integral<_Ty>::value, int>::type = 0>
    bool is_close(_Ty a, _Ty b)
    {
        // 整型
        return a == b;
    }

    template <typename _Ty, typename std::enable_if<std::is_floating_point<_Ty>::value, int>::type = 0>
    bool is_close(_Ty a, _Ty b)
    {
        // 浮点
        if (equal_nan && std::isnan(a) && std::isnan(b))
            return true;
        return std::abs(a - b) <= (atol + rtol * std::abs(b));
    }

    template <typename _Ty>
    int count_close(_Ty *a, _Ty *b, int N)
    {
        int res = 0;
        for (int i = 0; i < N; ++i)
        {
            if (is_close(a[i], b[i]))
                ++res;
        }
        return res;
    }

    template <typename _Ty>
    std::vector<_Ty> cal_atol(_Ty *a, _Ty *b, int N)
    {
        std::vector<_Ty> res(N);
        for (int i = 0; i < N; ++i)
        {
            res[i] = cal_atol(a[i], b[i]);
        }
        return res;
    }

    template <typename _Ty>
    std::vector<float_t> cal_rtol(_Ty *a, _Ty *b, int N)
    {
        std::vector<float_t> res(N);
        for (int i = 0; i < N; ++i)
        {
            res[i] = cal_rtol(a[i], b[i]);
        }
        return res;
    }

    template <typename _Ty>
    bool allclose(_Ty *a, _Ty *b, int N)
    {
        for (int i = 0; i < N; ++i)
        {
            if (!is_close(a[i], b[i]))
                return false;
        }
        return true;
    }

    template <typename _Ty>
    void compare(_Ty *a, _Ty *b, int N)
    {
        int n_close = count_close(a, b, N);
        if (n_close == N)
        {
            std::cout << "allclose" << std::endl;
        }
        else
        {
            int n_not_close = N - n_close;
            int n_print_top = std::min(max_print, n_not_close);
            std::cout << n_not_close << "/" << N << " not close" << std::endl;
            auto atol = cal_atol(a, b, N);
            auto rtol = cal_rtol(a, b, N);
            std::vector<int> index_atol(N), index_rtol;
            for (int i = 0; i < N; ++i)
            {
                index_atol[i] = i;
            }
            index_rtol = index_atol;

            std::partial_sort(index_atol.begin(), index_atol.begin() + n_print_top, index_atol.end(), [&atol](int a, int b)
                              { return greater(atol[a], atol[b]); });
            std::partial_sort(index_rtol.begin(), index_rtol.begin() + n_print_top, index_rtol.end(), [&rtol](int a, int b)
                              { return greater(rtol[a], rtol[b]); });

            std::cout << "max atol:\n";
            for (int i = 0; i < std::min(max_print, n_not_close); ++i)
            {
                int idx = index_atol[i];
                std::cout << "index: " << idx << ", ab: [" << a[idx] << ", " << b[idx] << "], atol: " << atol[idx] << ", rtol: " << rtol[idx] << std::endl;
            }

            std::cout << "max rtol:\n";
            for (int i = 0; i < std::min(max_print, n_not_close); ++i)
            {
                int idx = index_rtol[i];
                std::cout << "index: " << idx << ", ab: [" << a[idx] << ", " << b[idx] << "], atol: " << atol[idx] << ", rtol: " << rtol[idx] << std::endl;
            }
            std::cout << std::endl;
        }
    }
};

static comparer_t comparer;

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
