#ifndef DATABASE_MANAGER_H_
#define DATABASE_MANAGER_H_

#include <string>
#include <memory>
#include "file_manager.h"
#include "buffer_manager.h"

class DatabaseManager
{
public:
    DatabaseManager() = default;
    ~DatabaseManager() = default;
    bool is_open();
    void Open(const std::string &db_name);
    void Close();

    // These infomation about database need to be stored:
    // 1. length of database name(exclude '\0')
    // 2. database name 
    std::string db_name_;
    bool is_open_;
    std::shared_ptr<FileManager> file_manager_;
    std::shared_ptr<BufferManager> buffer_manager_;
};

#endif