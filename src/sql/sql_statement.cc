#include "../../include/sql/sql_statement.h"

SQLStatement::SQLStatement(SQL_Statement_Type type) : type_(type)
{
}

SQLStatement::~SQLStatement()
{
}

SQLStmtCreate::SQLStmtCreate(SQL_Statement_Type type,
                             std::shared_ptr<TableInfo> table_info, 
                             std::shared_ptr<Constraint> constraint)
    : SQLStatement(type)
    , table_info_(table_info)
    , constraint_(constraint)
{
}

SQLStmtCreate::~SQLStmtCreate()
{
}