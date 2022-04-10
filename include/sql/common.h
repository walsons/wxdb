#ifndef common_h_
#define common_h_

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

enum class SQL_Statement_Type
{
    SQL_CREATE_DATABASE,
    SQL_CREATE_TABLE,
    SQL_CREATE_VIEW,
    SQL_CREATE_INDEX,
    SQL_CREATE_USER,
    // ......
};

enum class Data_Type
{
    DATA_TYPE_INT,
    DATA_TYPE_DOUBLE,
    DATA_TYPE_CHAR,
    DATA_TYPE_TEXT,
    DATA_TYPE_BOOLEAN,
    DATA_TYPE_NULL
};

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

class FieldInfo
{
public:
    FieldInfo(Data_Type type, int length, const std::string &field_name);
    ~FieldInfo();

    std::string field_name_;
    unsigned int hash_code_;
    Data_Type type_;
    int length_;
};

class TableInfo
{
public:
    TableInfo(const std::string &table_name, std::vector<std::string> fields_name,
              std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<FieldInfo>>> fields);
    ~TableInfo();

    std::string table_name_;
    std::vector<std::string> fields_name_;
    std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<FieldInfo>>> fields_;
    std::shared_ptr<std::unordered_map<unsigned int, int>> offsets_;
    int record_length_;
};

class ForeignKeyRef
{
public:
    std::string column_name_, table_name_, table_column_name_;
    ForeignKeyRef(const std::string &column_name);
    ForeignKeyRef(const std::string &table_name, const std::string &column_name);
    ~ForeignKeyRef();
};

struct Constraint
{
    Constraint_Type type_;
    union
    {
        
    };
    // Perhaps more than 2 column have the same constraints,
    // using link list to connect these constrains to save space
    Constraint *next_;
};

#endif