#include "../../include/db/dbms.h"

void DBMS::CreateDatabase(const std::string &db_name)
{
    db_manager_->CreateDatabase(db_name);
}

void DBMS::UseDatabase(const std::string &db_name)
{
    db_manager_->Open(db_name);
}

void DBMS::CreateTable(const std::shared_ptr<TableHeader> table_header)
{
    db_manager_->CreateTable(table_header);
}

void DBMS::InsertRow(const std::shared_ptr<InsertInfo> insert_info)
{
    db_manager_->InsertRow(insert_info);
}

void DBMS::SelectTable(const std::shared_ptr<SelectInfo> select_info)
{
    db_manager_->SelectTable(select_info);
}

void DBMS::CloseDatabase()
{
    db_manager_->Close();
}