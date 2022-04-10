#ifndef SQL_STMT_H_
#define SQL_STMT_H_

#include "common.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

enum class SQL_Stmt_Type
{
    SQL_CREATE_DATABASE,
    SQL_CREATE_TABLE,
    SQL_CREATE_VIEW,
    SQL_CREATE_INDEX,
    SQL_CREATE_USER,

    SQL_INSERT_TABLE,
    // ......
};

class SQLStmt
{
public:
    SQLStmt(SQL_Stmt_Type type);
    virtual ~SQLStmt();

    SQL_Stmt_Type type_;
};

#endif