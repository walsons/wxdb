#ifndef SQL_STATEMENT_H_
#define SQL_STATEMENT_H_

#include <string>
#include <vector>
#include <unordered_map>

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

class SQLStatement
{
public:
    SQLStatement(SQL_Statement_Type type);
    virtual ~SQLStatement();

    SQL_Statement_Type type_;
};

class FieldInfo
{
public:
    FieldInfo(Data_Type type, int length, const std::string &field_name);

    std::string field_name_;
    unsigned int hash_code_;
    Data_Type type_;
    int length_;
};

class TableInfo
{
public:
    TableInfo(const std::string &table_name, std::vector<std::string> fields_name,
              std::unordered_map<std::string, FieldInfo *> *fields);

    std::string table_name_;
    std::vector<std::string> fields_name_;
    std::unordered_map<std::string, FieldInfo *> *fields_;
    std::unordered_map<unsigned int, int> *offsets_;
    int record_length_;
};

struct Constraint_t
{
    Constraint_Type type_;
    union
    {
        // TODO
    };
    Constraint_t *next_;
};

class SQLStmtCreate : public SQLStatement
{
public:
    SQLStmtCreate(SQL_Statement_Type type,
                  TableInfo * table_info, 
                  Constraint_t *constraints);
    ~SQLStmtCreate();
    TableInfo *table_info_;
    Constraint_t *constraints_;
};

#endif