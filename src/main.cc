#include <iostream>
#include "hello_interface.h"
#include "meta_command.h"
#include "sql_statement.h"

int main() {
    HelloInterface();
    MetaCommand meta_command;
    SQLStatement sql_statement;
    Database database;
    bool main_loop_flag = true;
    while (main_loop_flag)
    {
        std::cout << "wxdb> " << std::flush;
        std::string user_input;
        std::getline(std::cin, user_input);
        
        if (user_input[0] == '.') {
            // Meta commands
            if (!meta_command.parse(user_input)) {
                main_loop_flag = false;
            }
        } else {
            // SQL statements
            sql_statement.parse(database, user_input);       
        }
    }
    
    return 0;
}