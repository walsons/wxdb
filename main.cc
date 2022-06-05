#include "include/db/hello_interface.h"
#include "include/main_loop.h"

int main()
{
    HelloInterface();

    bool exit = false;
    while (!exit)
    {
        main_loop(exit);
    }
}