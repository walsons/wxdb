#ifndef DATABASE_MANAGER_H_
#define DATABASE_MANAGER_H_

#include <string>
#include <vector>
#include <memory>

#include "../../include/defs.h"
#include "../../include/db/table_manager.h"

class DatabaseManager
{
public:
    DatabaseManager() = default;
    ~DatabaseManager() = default;
    void CreateDatabase(const std::string &db_name);
    void Open(const std::string &db_name);
    void Close();
    void CreateTable(const std::shared_ptr<TableHeader> table_header);

private:
    // These infomation about database need to be stored in "database_name.db" file:
    // 2. database name (char[]: 255 bytes + 1 bytes '\0')
    // 3. number of table (unsigned int: 4 bytes)
    // 4. table_name (char[]: n * (255 bytes + 1 bytes '\0'))
    std::string db_name_;
    unsigned num_table_;
    std::vector<std::string> table_name_;
    std::vector<std::shared_ptr<TableManager>> table_manager_;
private:
    bool is_open_;

};

#endif