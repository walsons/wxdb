#include "../../include/db/dbms.h"
#include <iostream>

void DBMS::CreateDatabase(const std::string &db_name)
{
    if (db_manager_->CreateDatabase(db_name))
        std::cout << "Create database \"" << db_name << "\" successfully" << std::endl;
}

void DBMS::UseDatabase(const std::string &db_name)
{
    if (db_manager_->Open(db_name))
        std::cout << "Database changed to \"" << db_name << "\"" << std::endl;
}

void DBMS::CreateTable(const std::shared_ptr<TableHeader> table_header)
{
    if (db_manager_->CreateTable(table_header))
        std::cout << "Create database \"" << table_header->table_name << "\" successfully" << std::endl;
}

void DBMS::InsertRow(const std::shared_ptr<InsertInfo> insert_info)
{
    db_manager_->InsertRow(insert_info);
    std::cout << "Insert successfully" << std::endl;
}

void DBMS::SelectTable(const std::shared_ptr<SelectInfo> select_info)
{
    db_manager_->SelectTable(select_info);
}

void DBMS::CloseDatabase()
{
    db_manager_->Close();
}