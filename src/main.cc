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
    while (metaCommand.MainLoop())
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
            metaCommand.ExecuteMetaCommand(userInput);
        } 
        else 
        {
            // SQL statements
            if (sqlStatement.PrepareStatement(database, userInput) == PREPARE_SUCCESS)
            {
                sqlStatement.ExecuteStatement(database, userInput);
            }
        }
    }
    
    return 0;
}