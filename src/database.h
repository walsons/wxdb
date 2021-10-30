#ifndef DATABASE_H_
#define DATABASE_H_

#include <vector>
#include <string>
#include <unordered_map>

class Page {
public:
    Page() = default;
    ~Page() = default;
    char buffer_[4096];
};

struct Row {
    std::vector<void *> items_;
};

class Table {
public:
    Table() = default;
    ~Table() = default;
public:
    std::string table_name_;
    std::vector<std::string> column_names_;
    std::vector<std::string> column_types_;
    std::vector<Row *> rows_;
};

class Database {
public:
    Database() = default;
    ~Database() = default;
public:
    std::unordered_map<std::string, Table *> tables_;
};

#endif
