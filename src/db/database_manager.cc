#include "../../include/db/database_manager.h"
#include <cstring>
#include <fstream>
#include <iostream>

void DatabaseManager::CreateDatabase(const std::string &db_name)
{
    std::memcpy(&info_.db_name, db_name.c_str(), MAX_LENGTH_NAME);
    info_.db_name[MAX_LENGTH_NAME - 1] = '\0';
    info_.num_table = 0;
    Close();
}

void DatabaseManager::Open(const std::string &db_name)
{
    if (std::strcmp(db_name.c_str(), info_.db_name) == 0) { return; }
    if (is_open_)
    {
        Close();
        is_open_ = false;
    }
    std::ifstream ifs(DB_DIR + db_name + ".db", std::ios::in | std::ios::binary);
    if (!ifs.is_open()) { std::cout << "Database \"" << db_name << "\" doesn't exist!"; }
    else
    {
        ifs.read(reinterpret_cast<char *>(&info_), sizeof(info_));
        is_open_ = true;
    }
}

void DatabaseManager::Close()
{
    std::ofstream ofs(DB_DIR + info_.db_name + ".db", std::ios::out | std::ios::binary);
    ofs.write(reinterpret_cast<const char *>(&info_), sizeof(info_));
    is_open_ = false;
}

void DatabaseManager::CreateTable(const std::shared_ptr<TableHeader> table_header)
{
    if (!is_open_) 
    {
        std::cout << "Haven't choose a database!" << std::endl;
    }
    else 
    {
        for (size_t i = 0; i < info_.num_table; ++i)
        {
            if (std::strcpy(info_.table_name[i], table_header->table_name) == 0)
            {
                std::cout << "Table \"" << table_header->table_name << "\" has exist!" << std::endl;
                return;
            }
        }
        std::strncpy(info_.table_name[info_.num_table], table_header->table_name, MAX_LENGTH_NAME);
        ++info_.num_table;
        table_manager_.emplace_back(std::make_shared<TableManager>());
        table_manager_.back()->CreateTable(table_header);
    }
}
