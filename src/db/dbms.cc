#include "../../include/db/dbms.h"

void DBMS::CreateDatabase(const std::string &db_name)
{
    DatabaseManager db_manager;
    db_manager.CreateDatabase(db_name);
}

void DBMS::UseDatabase(const std::string &db_name)
{
    db_manager_->Open(db_name);
}

void DBMS::CreateTable(const std::shared_ptr<TableHeader> table_header)
{
    db_manager_->CreateTable(table_header);
}