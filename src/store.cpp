#include <iostream>
#include "tools.h"
#include "compare.h"
#include "unordered_map"

struct buffer_t
{
    const void *data;
    size_t bytes;
};

template <typename _Ty>
void backend_check(const char *tag, const _Ty *data, int N)
{
    static std::unordered_map<std::string, buffer_t> map;
    auto it = map.find(tag);
    if (it != map.end())
    {
        std::cout << "compare " << tag << std::endl;
        backend_compare(data, static_cast<const _Ty *>(it->second.data), N);
    }
    else
    {
        std::cout << "save " << tag << std::endl;
        map.insert(it, std::make_pair(std::string(tag), buffer_t{data, N * sizeof(_Ty)}));
    }
}
#define DEC_CHECK(type) \
    void backend_check_##type(const char *tag, const type *data, int N) { backend_check(tag, data, N); }
extern "C"
{
    DEC_CHECK(int)
    DEC_CHECK(long)
    DEC_CHECK(float)
    DEC_CHECK(Freal)
    DEC_CHECK(Fdouble)

    void backend_check_freal__(const char *tag, const Freal *data, int *N) { backend_check_Freal(tag, data, *N); }
    void backend_check_fdouble__(const char *tag, const Fdouble *data, int *N) { backend_check_Fdouble(tag, data, *N); }
}