#include <iostream>
#include "include/main_loop.h"

int main()
{
    std::cout << "wxdb version 1.0" << std::endl;
    std::cout << "Enter \".help\" for usage hints." << std::endl;

    bool exit = false;
    while (!exit)
    {
        main_loop(exit);
    }
}