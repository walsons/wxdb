#ifndef HELLO_INTERFACE_H_
#define HELLO_INTERFACE_H_

class HelloInterface {
public:
    HelloInterface() {
        std::cout << "wxdb version 1.0" << std::endl;
        std::cout << "Enter \".help\" for usage hints." << std::endl;
        std::cout << "Connected to a transient in-memory database." << std::endl;
        std::cout << "Use \".open FILENAME\" to reopen on a persistent database." << std::endl;
    }
    ~HelloInterface() = default;
};

#endif
