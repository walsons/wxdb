#include <iostream>
#include <string>

int main() {
    std::cout << "wxdb version 1.0" << std::endl;
    std::cout << "Enter \".help\" for usage hints." << std::endl;
    std::cout << "Connected to a transient in-memory database." << std::endl;
    std::cout << "Use \".open FILENAME\" to reopen on a persistent database." << std::endl;
    while (true)
    {
        std::cout << "wxdb> " << std::flush;
        std::string user_input;
        std::cin >> user_input;
        std::cout << "receive" << std::endl;
        /* code */
    }
    
    return 0;
}