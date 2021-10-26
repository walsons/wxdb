#include <iostream>
#include "meta_commands.h"

int main() {
    MetaCommands meta_commands;
    std::cout << "wxdb version 1.0" << std::endl;
    std::cout << "Enter \".help\" for usage hints." << std::endl;
    std::cout << "Connected to a transient in-memory database." << std::endl;
    std::cout << "Use \".open FILENAME\" to reopen on a persistent database." << std::endl;
    bool flag = true;
    while (flag)
    {
        std::cout << "wxdb> " << std::flush;
        std::string user_input;
        std::cin >> user_input;
        if (meta_commands.data_->find(user_input) == meta_commands.data_->end()) {
            std::cout << "unknown command" << std::endl;
            continue;
        }
        switch((*meta_commands.data_)[user_input]) {
            case 0: 
                flag = false;
                break;
            default:
                break;
        }
    }
    
    return 0;
}