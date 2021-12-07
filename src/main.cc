#include <iostream>
#include "hello_interface.h"
#include "meta_command.h"
#include "sql_statement.h"

int main() 
{
    HelloInterface();
    MetaCommand metaCommand;
    SQLStatement sqlStatement;
    Database database;
    bool mainLoopFlag = true;
    while (mainLoopFlag)
    {
        std::cout << "wxdb> " << std::flush;
        std::string userInput;
        std::getline(std::cin, userInput);

        if (userInput.empty())
        {
            continue;   
        }
        else if (userInput[0] == '.') 
        {
            // Meta commands
            metaCommand.Parse(userInput, mainLoopFlag);
        } 
        else 
        {
            // SQL statements
            sqlStatement.parse(database, userInput);       
        }
    }
    
    return 0;
}