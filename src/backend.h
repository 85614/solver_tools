#include "fortran_types.h"
#include "stdio.h"

inline void backend_log_str_with_time(const char *str)
{
    printf("%s\n", str);
}