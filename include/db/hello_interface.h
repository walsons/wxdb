#ifndef HELLO_INTERFACE_H_
#define HELLO_INTERFACE_H_

#include <iostream>

class HelloInterface {
public:
    HelloInterface() {
        std::cout << "wxdb version 1.0" << std::endl;
        std::cout << "Enter \".help\" for usage hints." << std::endl;
    }
    ~HelloInterface() = default;
};

#endif
