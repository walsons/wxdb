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
    if (word == "") {}
    else if (word == ".exit") 
    { 
        DBMS::GetInstance().CloseDatabase();
        exit = true; 
    }
    else
    {
        /**********          Database          **********/
        auto database_parser = std::make_shared<DatabaseParser>(tokenizer);
        std::shared_ptr<DatabaseInfo> database_info;
        // CREATE DATABASE
        database_parser->Reset();
        database_info = database_parser->CreateDatabase();
        if (database_info)
        {
            DBMS::GetInstance().CreateDatabase(database_info->database_name);
            return;
        }
        if (database_parser->PrintError())
            return;
        // USE [database]
        database_parser->Reset();
        database_info = database_parser->UseDatabase();
        if (database_info)
        {
            DBMS::GetInstance().UseDatabase(database_info->database_name);
            return;
        }
        if (database_parser->PrintError())
            return;

        /**********           Table            **********/
        auto table_parser = std::make_shared<TableParser>(tokenizer);
        // CREATE TABLE
        table_parser->Reset();
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
        table_parser->Reset();
        auto insert_info = table_parser->InsertTable();
        if (insert_info)
        {
            DBMS::GetInstance().InsertRow(insert_info);
            return;
        }
        if (table_parser->PrintError())
            return;
        // SELECT [column] FROM [table]
        table_parser->Reset();
        auto select_info = table_parser->SelectTable();
        if (select_info)
        {
            DBMS::GetInstance().SelectTable(select_info);
            return;
        }
        if (table_parser->PrintError())
            return;
        // DELETE FROM [table]
        table_parser->Reset();
        auto delete_info = table_parser->DeleteTable();
        if (delete_info)
        {
            DBMS::GetInstance().DeleteTable(delete_info);
            return;
        }
        if (table_parser->PrintError())
            return;

        /**********         Other case         **********/
        std::cout << "Unknown command" << std::endl;
    }
}

#endif