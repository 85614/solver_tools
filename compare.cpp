#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>

// 相对误差都是相对于 b

struct comparer_t
{
    using _Ty = float;
    using float_t = typename std::conditional<std::is_floating_point<_Ty>::value, _Ty, double>::type;

    float atol = 1e-5;
    float rtol = 1e-5;
    bool equal_nan = true;
    int max_print = 5;

    _Ty cal_atol(_Ty a, _Ty b)
    {
        if (equal_nan && std::isnan(a) && std::isnan(b))
            return 0;
        return std::abs(a - b);
    }

    _Ty cal_rtol(_Ty a, _Ty b)
    {
        if (equal_nan && std::isnan(a) && std::isnan(b))
            return 0;
        return std::abs((a - b) / (std::max(std::abs(a), std::abs(b)) + std::numeric_limits<_Ty>::denorm_min()));
#if 0
        if (a == 0 && b == 0)
        {
            return 0;
        }
        else
        {
            // return std::abs((a - b) / std::max(std::abs(a), std::abs(b)));
            return std::abs((a - b) / (b));
            return std::abs((a - b) / (b + std::numeric_limits<_Ty>::denorm_min())); // 加了也是 x/0 = inf
        }
#endif
    }

    bool is_close(_Ty a, _Ty b)
    {
        if (equal_nan && std::isnan(a) && std::isnan(b))
            return true;
        return std::abs(a - b) <= (atol + rtol * std::abs(b));
    }

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

    std::vector<_Ty> cal_atol(_Ty *a, _Ty *b, int N)
    {
        std::vector<_Ty> res(N);
        for (int i = 0; i < N; ++i)
        {
            res[i] = cal_atol(a[i], b[i]);
        }
        return res;
    }

    std::vector<_Ty> cal_rtol(_Ty *a, _Ty *b, int N)
    {
        std::vector<_Ty> res(N);
        for (int i = 0; i < N; ++i)
        {
            res[i] = cal_rtol(a[i], b[i]);
        }
        return res;
    }

    bool allclose(_Ty *a, _Ty *b, int N)
    {
        for (int i = 0; i < N; ++i)
        {
            if (!is_close(a[i], b[i]))
                return false;
        }
        return true;
    }

    static bool greater(_Ty a, _Ty b)
    {
        if (!std::isunordered(a, b))
            return a > b;
        else
            return std::isfinite(b);
    }
    int compare(_Ty *a, _Ty *b, int N)
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
        }
    }
};

void test_compare()
{
    float a[10]{NAN, INFINITY, 0, 4, NAN, 6, 7, 0, 9.1, 10};
    float b[10]{NAN, 2, 0, 4, 5, 6, 7, 8, 9, 0};
    comparer_t().compare(a, b, 10);
}

// int main()
// {
//    test_compare();
//     return 0;
// }