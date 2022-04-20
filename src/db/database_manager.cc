#include "../../include/db/database_manager.h"

inline
bool DatabaseManager::is_open() { return is_open_; }

void DatabaseManager::Open(const std::string &db_name)
{
    if (!is_open())
    {
        file_manager_ = std::make_shared<FileManager>(".db_data", db_name);
        db_name_ = db_name;
    }
}

void DatabaseManager::Close()
{
    if (is_open())
    {
        // Store database info in the file
        // These infomation about database need to be stored:
        // 1. length of database name(exclude '\0')
        // 2. database name 
        std::string file_name = db_name_;
        std::ofstream ofd(file_name, std::ios::out | std::ios::binary);
        int db_name_length = db_name_.size();
        ofd.write(reinterpret_cast<char *>(&db_name_length), sizeof(int));
        ofd.write((db_name_.c_str()), db_name_length);
        is_open_ = false;
    }
    
}