
#include <iostream>
#include <stdio.h>
#include <unordered_map>
#include <cstdarg>
#include <vector>
#include <cassert>
#include "fortran_types.h"

extern "C"
{
    void backend_printf__(const char *fmt, ...);
}

int count_ch(char ch, const char *begin, const char *end = nullptr)
{
    assert(begin);
    int count = 0;
    for (auto ptr = begin; ptr != end && *ptr; ++ptr)
        if (*ptr == ch)
            ++count;
    return count;
};

template <typename _Ty, typename std::enable_if<std::is_pointer<_Ty>::value, int>::type = 0>
void sprintf_helper(char *&pb, const char *fmt, size_t n_extra_int, va_list &args)
{
    assert(n_extra_int <= 2);
    switch (n_extra_int)
    {
    case 0:
        pb += sprintf(pb, fmt, va_arg(args, _Ty));
        break;
    case 1:
        pb += sprintf(pb, fmt, *va_arg(args, int *), va_arg(args, _Ty));
        break;
    case 2:
        pb += sprintf(pb, fmt, *va_arg(args, int *), *va_arg(args, int *), va_arg(args, _Ty));
        break;
    default:
        break;
    }
}
template <typename _Ty, typename std::enable_if<!std::is_pointer<_Ty>::value, int>::type = 0>
void sprintf_helper(char *&pb, const char *fmt, size_t n_extra_int, va_list &args)
{
    assert(n_extra_int <= 2);
    switch (n_extra_int)
    {
    case 0:
        pb += sprintf(pb, fmt, *va_arg(args, _Ty *));
        break;
    case 1:
        pb += sprintf(pb, fmt, *va_arg(args, int *), *va_arg(args, _Ty *));
        break;
    case 2:
        pb += sprintf(pb, fmt, *va_arg(args, int *), *va_arg(args, int *), *va_arg(args, _Ty *));
        break;
    default:
        break;
    }
}

void backend_printf__(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    std::vector<std ::string> args_buf;
    char buf[256]{};
    auto pb = buf;
    // static_assert(sizeof(Freal) == 4, ""); // withoutdouble 时 是8
    static_assert(sizeof(Fdouble) == 8, "");
    for (auto ptr = fmt; *ptr;)
    {
        if (*ptr == '%')
        {
            auto p_mod_op = ptr;
            bool long_length = false; // 只关心lf, ld之类的
            int extra_int_count = 0;
            for (++ptr; *ptr; ++ptr)
            {

                if (count_ch(*ptr, "dioxXu"))
                {
                    // int or  unsigned int
                    if (long_length)
                        sprintf_helper<int64_t>(pb, std::string(p_mod_op, ptr + 1).c_str(), extra_int_count, args);
                    else
                        sprintf_helper<Fint>(pb, std::string(p_mod_op, ptr + 1).c_str(), extra_int_count, args);
                }
                else if (count_ch(*ptr, "fFeEaAgG"))
                {
                    // float
                    if (long_length)
                        sprintf_helper<double>(pb, std::string(p_mod_op, ptr + 1).c_str(), extra_int_count, args);
                    else
                        sprintf_helper<Freal>(pb, std::string(p_mod_op, ptr + 1).c_str(), extra_int_count, args);
                }
                else if (*ptr == 'c')
                {
                    // char
                    sprintf_helper<char>(pb, std::string(p_mod_op, ptr + 1).c_str(), extra_int_count, args);
                }
                else if (count_ch(*ptr, "ps"))
                {
                    // void* or str
                    sprintf_helper<void *>(pb, std::string(p_mod_op, ptr + 1).c_str(), extra_int_count, args);
                }
                // else if (*ptr == 'n')
                // {
                //     pb += sprintf(pb, std::string(p_mod_op, ptr + 1).c_str());
                // }
                else if (*ptr == '%')
                {
                    pb += sprintf(pb, std::string(p_mod_op, ptr + 1).c_str());
                }
                else if (*ptr == 'l')
                {
                    long_length = true;
                    continue;
                }
                else if (*ptr == '*')
                {
                    ++extra_int_count;
                    continue;
                }

                else
                {
                    // 不管 %n
                    continue;
                }
                ptr++;
                break; // 已找到%x
            }
        }
        else
        {
            *pb++ = *ptr++;
        }
    }
    va_end(args);
    std::cout << buf;
}