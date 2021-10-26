#include <iostream>
#include "meta_command.h"
#include "sql_statement.h"

int main() {
    MetaCommand meta_command;
    std::cout << "wxdb version 1.0" << std::endl;
    std::cout << "Enter \".help\" for usage hints." << std::endl;
    std::cout << "Connected to a transient in-memory database." << std::endl;
    std::cout << "Use \".open FILENAME\" to reopen on a persistent database." << std::endl;
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
            StatementType statement_type = OTHER_TYPE;
            if (user_input.substr(0, 6) == "insert") {
                statement_type = INSERT_TYPE;
            } else if (user_input.substr(0, 6) == "select") {
                statement_type = SELECT_TYPE;
            }
            switch (statement_type) {
                case INSERT_TYPE:
                    std::cout << "do insert" << std::endl;
                    break;
                case SELECT_TYPE:
                    std::cout << "do select" << std::endl;
                    break;
                default:
                    std::cout << "unknown command" << std::endl;
                    break;
            }
            
        }
    }
    
    return 0;
}