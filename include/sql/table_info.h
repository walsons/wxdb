#ifndef TABLE_INFO_H_
#define TABLE_INFO_H_

#include <vector>
#include <string>
#include <memory>
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
    // TODO
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
};

struct TableInfo
{
    std::string table_name;
    std::vector<FieldInfo> field_info;
    uint32_t flag_not_null, flag_primary, flag_index, flag_unique, flag_default;
    std::vector<CheckConstraint> check_constraint;
    // Which column is foreign key, have default value
    std::vector<std::pair<int, ForeignKeyRef>> foreign_key_ref;
    std::vector<std::pair<int, Expression *>> default_value;
};

struct InsertInfo
{
    std::string table_name;
    std::vector<std::string> field_name;
    std::vector<DataValue> value;
};

#endif