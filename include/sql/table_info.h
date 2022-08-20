#ifndef TABLE_INFO_H_
#define TABLE_INFO_H_

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include "expression.h"
#include "../db/col_val.h"
#include "../defs.h"

enum class Constraint_Type
{
    CONS_UNIQUE,
    CONS_PRIMARY_KEY,
    CONS_FOREIGN_KEY,
    CONS_CHECK,
};

struct ForeignKeyRef
{
    std::string table_name;
    std::string column_name;
};

struct FieldInfo
{
    std::string field_name;
    Col_Type type;
    int length;
    bool is_not_null;
    bool has_default;
    ExprNode *expr;
};

struct ConstraintInfo
{
    Constraint_Type type;
    ColumnRef col_ref;
    ForeignKeyRef fk_ref;
    ExprNode *expr;
};

struct TableInfo
{
    std::string table_name;
    std::vector<FieldInfo> field_info;
    std::vector<ConstraintInfo> constraint_info;
};

struct InsertInfo
{
    std::string table_name;
    // The key is field name, the value is the index of value,
    // if the map is empty, which means value is stored according 
    // to column order
    std::unordered_map<std::string, int> field_name;
    std::vector<ColVal> col_val;
};

struct SelectInfo
{
    std::vector<ColumnRef> columns;
    std::vector<std::string> tables;
    ExprNode *where;
};

struct DeleteInfo
{
    std::string table_name;
    ExprNode *where;
};

#endif