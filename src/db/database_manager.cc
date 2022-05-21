#include "../../include/db/database_manager.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include "../../include/db/type_cast.h"
#include "../../include/db/col_val.h"

void DatabaseManager::CreateDatabase(const std::string &db_name)
{
    std::strncpy(info_.db_name, db_name.c_str(), MAX_LENGTH_NAME);
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

void DatabaseManager::InsertRow(const std::shared_ptr<InsertInfo> insert_info)
{
    std::shared_ptr<TableManager> table;
    for (auto & t : table_manager_)
    {
        if (t->table_name() == insert_info->table_name)
        {
            table = t;
        }
    }
    if (table == nullptr) 
    {
        std::cerr << "table " << insert_info->table_name << " not found!" << std::endl;
        return;
    }
    if (insert_info->field_name.empty())
    {
        if (insert_info->col_val.size() != table->number_of_column())
        {
            std::cerr << "column size not equal!" << std::endl;
            return;
        }
        for (size_t i = 0; i < table->number_of_column(); ++i)
        {
            if (!TypeCast::check_type_compatible(insert_info->col_val[i].type_, table->column_type(i)))
            {
                std::cerr << "incompatible type!" << std::endl;
                return;
            }
            table->SetTempRecord(i, insert_info->col_val[i]);
        }
    }
    else
    {
        auto &field_map = insert_info->field_name;
        ColVal null_value;
        for (size_t i = 0; i < table->number_of_column(); ++i)
        {
            if (field_map.find(table->column_name(i)) != field_map.end())   
            {
                table->SetTempRecord(i, insert_info->col_val[field_map[table->column_name(i)]]);
            }
            else
            {
                table->SetTempRecord(i, null_value);
            }
        }
    }
    table->InsertRecord();
}