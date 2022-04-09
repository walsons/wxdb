#include <iostream>
#include "include/hello_interface.h"
#include "include/meta_command.h"
#include "include/a_sql_statement.h"

int main() 
{
    HelloInterface();
    MetaCommand metaCommand;
    ASQLStatement sqlStatement;
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
            if (sqlStatement.PrepareStatement(database, userInput) == PREPARE_RESULT::SUCCESS)
            {
                sqlStatement.ExecuteStatement(database, userInput);
            }
        }
    }
    
    return 0;
}