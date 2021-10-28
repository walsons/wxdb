#ifndef DATABASE_H_
#define DATABASE_H_

#include <vector>
#include <string>
#include <unordered_map>

struct Row {
    std::vector<void *> item;
};

class Table {
public:
    Table() = default;
    ~Table() = default;
public:
    std::string table_name_;
    std::vector<std::string> columns_name_;
    std::vector<std::string> columns_type_;
    std::vector<Row> row_;
};

class Database {
public:
    Database() = default;
    ~Database() = default;
public:
    std::unordered_map<std::string, Table *> tables_;
};

#endif
