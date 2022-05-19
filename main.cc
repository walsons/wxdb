/******************** old implement ********************
#include <iostream>
#include "include/db_old/hello_interface.h"
#include "include/db_old/meta_command.h"
#include "include/db_old/a_sql_statement.h"

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
 ******************** old implement ********************/

#include "include/db_old/hello_interface.h"
#include "include/db/dbms.h"
#include <cctype>
#include <algorithm>
#include <string>
#include "./include/sql/parser.h"
#include "./include/sql/database_parser.h"
#include "./include/sql/table_parser.h"
#include "./include/db/table_header.h"

std::string tolower(const std::string &str)
{
    std::string res;
    std::for_each(str.begin(), str.end(), [&](char c) { res.push_back(std::tolower(c)); });
    return res;
}

int main()
{
    HelloInterface();

    while (true)
    {
        std::string one_sql;
        std::getline(std::cin, one_sql);
        auto t = std::make_shared<Tokenizer>(one_sql);

        std::istringstream in(one_sql);
        std::string tmp;
        in >> tmp;
        if (tolower(tmp) == "create")
        {
            in >> tmp;
            tmp = tolower(tmp);
            if (tmp == "database") 
            {
                auto parser = std::make_shared<DatabaseParser>(t);
                auto info = parser->CreateDatabase();
                DBMS::GetInstance().CreateDatabase(info->database_name);
            }
            else if (tmp == "table")
            {
                auto parser = std::make_shared<TableParser>(t);
                auto info = parser->CreateTable();
                auto table_header = std::make_shared<TableHeader>();
                fill_table_header(table_header, *info);               
                DBMS::GetInstance().CreateTable(table_header);
            }
        }
    }
}