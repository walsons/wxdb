#ifndef TABLE_INFO_H_
#define TABLE_INFO_H_

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include "expression.h"
#include "common.h"

enum class Constraint_Type
{
    CONS_NOT_NULL,
    CONS_UNIQUE,
    CONS_PRIMARY_KEY,
    CONS_FOREIGN_KEY,
    CONS_DEFAULT,
    OONS_AUTO_INCREMENT,
    CONS_CHECK,
    CONS_SIZE
};

struct CheckConstraint
{
    ExprNode *check_condition;
};

struct ForeignKeyRef
{
    std::string table_name;
    std::string column_name;
};

struct FieldInfo
{
    std::string field_name;
    Data_Type type;
    int length;
    std::vector<Constraint_Type> constraint;
    ExprNode* expr;  // for default(default value) or primary(column reference) or check(expression)
};

struct TableInfo
{
    std::string table_name;
    std::vector<FieldInfo> field_info;
    uint32_t flag_not_null, flag_primary, flag_index, flag_unique, flag_default;
    std::vector<CheckConstraint> check_constraint;
    // Which column is foreign key, have default value
    std::vector<std::pair<int, ForeignKeyRef>> foreign_key_ref;
    std::vector<std::pair<int, ExprNode *>> default_value;
};

struct InsertInfo
{
    std::string table_name;
    // The key is field name, the value is the index of DataValue,
    // if the map is empty, which means DataValue is stored according 
    // to column order
    std::unordered_map<std::string, int> field_name;
    std::vector<DataValue> value;
};

#endif