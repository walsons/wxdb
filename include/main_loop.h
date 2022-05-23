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

std::string tolower(const std::string &str)
{
    std::string res;
    std::for_each(str.begin(), str.end(), [&](char c) { res.push_back(std::tolower(c)); });
    return res;
}

void main_loop(bool &exit)
{
    std::string one_sql;
    std::getline(std::cin, one_sql);
    auto t = std::make_shared<Tokenizer>(one_sql);

    std::istringstream in(one_sql);
    std::string tmp;
    in >> tmp;
    if (tolower(tmp) == ".exit") { exit = true; }
    else if (tolower(tmp) == "create")
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
    else if (tolower(tmp) == "insert")
    {

    }
}

#endif