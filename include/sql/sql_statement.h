#ifndef SQL_STATEMENT_H_
#define SQL_STATEMENT_H_

#include "common.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

class SQLStatement
{
public:
    SQLStatement(SQL_Statement_Type type);
    virtual ~SQLStatement();

    SQL_Statement_Type type_;
};

class SQLStmtCreate : public SQLStatement
{
public:
    SQLStmtCreate(SQL_Statement_Type type,
                  std::shared_ptr<TableInfo> table_info, 
                  std::shared_ptr<Constraint> constraint);
    ~SQLStmtCreate();
    std::shared_ptr<TableInfo> table_info_;
    std::shared_ptr<Constraint> constraint_;
};

#endif