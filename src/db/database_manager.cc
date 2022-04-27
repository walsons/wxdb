#include "../../include/db/database_manager.h"

#include <fstream>
#include <iostream>

void DatabaseManager::CreateDatabase(const std::string &db_name)
{
    db_name_ = db_name;
    num_table_ = 0;
    Close();
}

void DatabaseManager::Open(const std::string &db_name)
{
    if (db_name == db_name_) { return; }
    if (is_open_)
    {
        Close();
        is_open_ = false;
    }
    std::ifstream ifs(db_name + ".db", std::ios::in | std::ios::binary);
    if (!ifs.is_open()) { std::cout << "Database \"" << db_name << "\" doesn't exist!"; }
    else
    {
        char *name_buf = new char[NAME_MAX_LENGTH + 1];
        char *unsigned_buf = new char[4];

        ifs.read(name_buf, NAME_MAX_LENGTH + 1);
        db_name_ = std::string(name_buf);
        ifs.read(unsigned_buf, sizeof(num_table_));
        num_table_ = reinterpret_cast<unsigned>(unsigned_buf);
        table_name_.resize(num_table_);
        table_manager_.resize(num_table_);
        for (size_t i = 0; i < num_table_; ++i)
        {
            ifs.read(name_buf, NAME_MAX_LENGTH + 1);
            table_name_[i] = std::string(name_buf);
            table_manager_[i] = std::make_shared<TableManager>();
        }
        is_open_ = true;
        delete[] unsigned_buf;
        delete[] name_buf;
    }
}

void DatabaseManager::Close()
{
    std::ofstream ofs(db_name_ + ".db", std::ios::out | std::ios::binary);
    ofs.write(static_cast<const char *>(db_name_.c_str()), db_name_.size() + 1);
    ofs.seekp(NAME_MAX_LENGTH + 1, std::ios::beg);
    ofs.write(reinterpret_cast<const char *>(&num_table_), sizeof(num_table_));
    for (size_t i = 0; i < num_table_; ++i)
    {
        ofs.seekp(NAME_MAX_LENGTH + 1 + sizeof(num_table_) + i * (NAME_MAX_LENGTH + 1));
        ofs.write(table_name_[i].c_str(), table_name_.size() + 1);
    }
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
        for (size_t i = 0; i < num_table_; ++i)
        {
            if (table_name_[i] == table_header->table_name_)
            {
                std::cout << "Table \"" << table_header->table_name_ << "\" has exist!" << std::endl;
                return;
            }
        }
        ++num_table_;
        table_name_.push_back(table_header->table_name_);
        table_manager_.emplace_back(std::make_shared<TableManager>());
        table_manager_.back()->CreateTable(table_header);
    }
}

