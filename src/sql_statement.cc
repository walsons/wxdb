#include "../include/sql_statement.h"

SQLStatement::SQLStatement(SQL_Statement_Type type) : type_(type)
{
}

SQLStatement::~SQLStatement()
{
}

FieldInfo::FieldInfo(Data_Type type, int length, const std::string &field_name)
{
}

TableInfo::TableInfo(const std::string &table_name, std::vector<std::string> fields_name,
                     std::unordered_map<std::string, FieldInfo *> *fields)
    : table_name_(table_name), fields_name_(fields_name), fields_(fields)
{
}

SQLStmtCreate::SQLStmtCreate(SQL_Statement_Type type,
                             TableInfo * table_info, 
                             Constraint_t *constraints)
    : SQLStatement(type)
    , table_info_(table_info)
    , constraints_(constraints)
{
}

SQLStmtCreate::~SQLStmtCreate()
{
}