#ifndef MAIN_LOOP_H_
#define MAIN_LOOP_H_

#include <cctype>
#include <algorithm>
#include <string>
#include <iostream>
#include "db/dbms.h"
#include "sql/parser.h"
#include "sql/database_parser.h"
#include "sql/table_parser.h"
#include "db/table_header.h"

std::string &ToLower(std::string &str)
{
    std::for_each(str.begin(), str.end(), [](char &c) { 
        c = std::tolower(c); 
    });
    return str;
};

void main_loop(bool &exit)
{
    std::string one_sql;
    std::getline(std::cin, one_sql);
    auto tokenizer = std::make_shared<Tokenizer>(one_sql);

    std::istringstream in(one_sql);
    std::string word;
    in >> word;
    ToLower(word);
    if (word == "") {}
    else if (word == ".exit") 
    { 
        DBMS::GetInstance().CloseDatabase();
        exit = true; 
    }
    else if (word == "use")
    {
        auto parser = std::make_shared<DatabaseParser>(tokenizer);
        auto database_info = parser->UseDatabase();
        if (database_info)
            DBMS::GetInstance().UseDatabase(database_info->database_name);
    }
    else if (word == "create")
    {
        in >> word;
        ToLower(word);
        if (word == "database") 
        {
            auto parser = std::make_shared<DatabaseParser>(tokenizer);
            auto database_info = parser->CreateDatabase();
            if (database_info)
                DBMS::GetInstance().CreateDatabase(database_info->database_name);
        }
        else if (word == "table")
        {
            auto parser = std::make_shared<TableParser>(tokenizer);
            auto info = parser->CreateTable();
            if (info)
            {
                auto table_header = std::make_shared<TableHeader>();
                fill_table_header(table_header, *info);               
                DBMS::GetInstance().CreateTable(table_header);
            }
        }
    }
    else if (word == "insert")
    {
        in >> word;
        ToLower(word);
        if (word == "into")
        {
            auto parser = std::make_shared<TableParser>(tokenizer);
            auto info = parser->InsertTable();
            if (info)
                DBMS::GetInstance().InsertRow(info);
        }
    }
    else 
    {
        std::cout << "Unknown command" << std::endl;
    }
}

#endif