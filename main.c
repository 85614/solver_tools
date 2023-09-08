
#include "multithreading.h"
    void backend_push__(const char *func);
    void backend_pop__();

int main(int argc, char **argv)
{
    backend_initialize_multithreading(6);
    backend_push__("hello");
    backend_request_hello_world__();
    backend_pop__();
    backend_finalize_multithreading();
    return 0;
}