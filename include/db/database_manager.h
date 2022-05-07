#ifndef DATABASE_MANAGER_H_
#define DATABASE_MANAGER_H_

#include <cstring>
#include <string>
#include <vector>
#include <memory>

#include "../../include/defs.h"
#include "../../include/db/table_manager.h"

class DatabaseManager
{
public:
    DatabaseManager();
    ~DatabaseManager() = default;
    void CreateDatabase(const std::string &db_name);
    void Open(const std::string &db_name);
    void Close();
    void CreateTable(const std::shared_ptr<TableHeader> table_header);

private:
    // These infomation about database need to be stored in "database_name.db" file:
    // 1. Database name (char[]: 255 bytes + 1 bytes '\0')
    // 2. The number of tables (unsigned int: 4 bytes)
    // 3. Table name array (char[]: n * (255 bytes + 1 bytes '\0'))
    struct Info
    {
        char db_name[MAX_LENGTH_NAME];
        unsigned num_table;
        char table_name[MAX_NUM_TABLE][MAX_LENGTH_NAME];
    };
    Info info_;
    std::vector<std::shared_ptr<TableManager>> table_manager_;
private:
    bool is_open_;

};

inline DatabaseManager::DatabaseManager() : is_open_(false)
{
    std::memset(&info_, 0, sizeof(info_));
}

#endif