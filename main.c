
#include "multithreading.h"
    void backend_push__(const char *func);
    void backend_pop__();
void backend_printf__(const char *fmt, ...);

int main(int argc, char **argv)
{
    {
    int a = 3;
    float b = 4;
    const char * s= "hello";
    backend_printf__("test f-printf %d %f %s \n", &a, &b, s);
    }
    backend_initialize_multithreading(6);
    backend_push__("hello");
    backend_request_hello_world__();
    backend_pop__();
    backend_finalize_multithreading();
    return 0;
}