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

    std::istringstream in(one_sql);
    std::string word;
    in >> word;
    if (word == "") {}
    else if (word == ".exit") 
    { 
        DBMS::GetInstance().CloseDatabase();
        exit = true; 
    }
    else
    {
        std::shared_ptr<Tokenizer> tokenizer;
        /**********          Database          **********/
        std::shared_ptr<DatabaseParser> database_parser;
        std::shared_ptr<DatabaseInfo> database_info;
        // CREATE DATABASE
        tokenizer = std::make_shared<Tokenizer>(one_sql);
        database_parser = std::make_shared<DatabaseParser>(tokenizer);
        database_info = database_parser->CreateDatabase();
        if (database_info)
        {
            DBMS::GetInstance().CreateDatabase(database_info->database_name);
            return;
        }
        if (database_parser->PrintError())
            return;
        // USE [database]
        tokenizer = std::make_shared<Tokenizer>(one_sql);
        database_parser = std::make_shared<DatabaseParser>(tokenizer);
        database_info = database_parser->UseDatabase();
        if (database_info)
        {
            DBMS::GetInstance().UseDatabase(database_info->database_name);
            return;
        }
        if (database_parser->PrintError())
            return;

        /**********           Table            **********/
        std::shared_ptr<TableParser> table_parser;
        // CREATE TABLE
        tokenizer = std::make_shared<Tokenizer>(one_sql);
        table_parser = std::make_shared<TableParser>(tokenizer);
        auto table_info = table_parser->CreateTable();
        if (table_info)
        {
            auto table_header = std::make_shared<TableHeader>();
            fill_table_header(table_header, *table_info);               
            DBMS::GetInstance().CreateTable(table_header);
            return;
        }
        if (table_parser->PrintError())
            return;
        // INSERT INTO 
        tokenizer = std::make_shared<Tokenizer>(one_sql);
        table_parser = std::make_shared<TableParser>(tokenizer);
        auto insert_info = table_parser->InsertTable();
        if (insert_info)
        {
            DBMS::GetInstance().InsertRow(insert_info);
            return;
        }
        if (table_parser->PrintError())
            return;
        // SELECT [column] FROM [table]
        tokenizer = std::make_shared<Tokenizer>(one_sql);
        table_parser = std::make_shared<TableParser>(tokenizer);
        auto select_info = table_parser->SelectTable();
        if (select_info)
        {
            DBMS::GetInstance().SelectTable(select_info);
            return;
        }
        if (table_parser->PrintError())
            return;

        /**********         Other case         **********/
        std::cout << "Unknown command" << std::endl;
    }
}

#endif