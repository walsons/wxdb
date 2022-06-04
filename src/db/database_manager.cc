#include "../../include/db/database_manager.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include "../../include/db/type_cast.h"
#include "../../include/db/col_val.h"
#include "../../include/btree/btree_iterator.hpp"
#include "../../include/db/record_manager.h"

void DatabaseManager::CreateDatabase(const std::string &db_name)
{
    std::strncpy(info_.db_name, db_name.c_str(), MAX_LENGTH_NAME);
    info_.num_table = 0;
    Close();
}

void DatabaseManager::Open(const std::string &db_name)
{
    if (std::strcmp(db_name.c_str(), info_.db_name) == 0) 
    { 
        is_open_ = true;
        return; 
    }
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
        for (size_t i = 0; i < info_.num_table; ++i)
        {
            table_manager_.push_back(std::make_shared<TableManager>(info_.table_name[i]));
        }
    }
}

void DatabaseManager::Close()
{
    std::ofstream ofs(DB_DIR + info_.db_name + ".db", std::ios::out | std::ios::binary);
    ofs.write(reinterpret_cast<const char *>(&info_), sizeof(info_));
    is_open_ = false;
    // Close table
    for (size_t i = 0; i < info_.num_table; ++i)
    {
        table_manager_[i]->CloseTable();
    }
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
    for (auto &t : table_manager_)
    {
        if (t->table_name() == insert_info->table_name)
        {
            table = t;
            break;
        }
    }
    if (table == nullptr) 
    {
        std::cerr << "table " << insert_info->table_name << " not found!" << std::endl;
        return;
    }
    table->OpenTable(table->table_name());
    if (insert_info->field_name.empty())
    {
        if (insert_info->col_val.size() != table->number_of_column())
        {
            std::cerr << "column size not equal!" << std::endl;
            return;
        }
        for (size_t i = 0; i < table->number_of_column(); ++i)
        {
            if (!TypeCast::make_type_compatible(insert_info->col_val[i], table->column_type(i)))
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
        for (int i = 0; i < static_cast<int>(table->number_of_column()) - 1; ++i)
        {
            // insert_info don't have __rowid__, and we will set __rowid__ in InsertRecord()
            if (!TypeCast::make_type_compatible(insert_info->col_val[i], table->column_type(i)))
            {
                std::cerr << "incompatible type!" << std::endl;
                return;
            }
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

void DatabaseManager::SelectTable(const std::shared_ptr<SelectInfo> select_info)
{
    // Get table map
    std::vector<std::shared_ptr<TableManager>> tms;
    tms.reserve(select_info->tables.size());
    std::unordered_map<std::string, std::shared_ptr<TableManager>> table_map;
    for (const auto &item : select_info->tables)
    {
        table_map[item] = nullptr;
    }
    for (size_t i = 0; i < table_manager_.size(); ++i)
    {
        if (table_map.find(table_manager_[i]->table_name()) != table_map.end())
        {
            table_map[table_manager_[i]->table_name()] = table_manager_[i];
        }
    }
    for (const auto &item : table_map)
    {
        if (item.second == nullptr)
        {
            std::cout << "Error: no table named \"" << item.first << "\"!" << std::endl;
            return;
        }
    }
    // Get Columns
    auto columns = select_info->columns;
    // Get where expression
    auto condition = select_info->where;

    // Iterator records
    if (table_map.size() == 1)
    {
        iterate_one_table(table_map, columns, condition);
    }
    else
    {
        iterate_many_table();
    }
}

void DatabaseManager::iterate_one_table(const std::unordered_map<std::string, std::shared_ptr<TableManager>> &table_map,
                                        const std::vector<ColumnRef> &columns, ExprNode *condition)
{
    auto tm = (*table_map.begin()).second;
    // Find the first record
    int row_id = 1;
    auto file_page = tm->GetRowPosition(row_id);
    BTreeIterator<VariantPage> btit{tm->pg(), file_page};   
    RecordManager record_manager{tm->pg()};
    std::vector<int> col_index;
    for (const auto &col : columns)
    {
        bool exist_flag = false;
        for (int i = 0; i < tm->table_header().num_column; ++i)
        {
            if (tm->column_name(i) == col.column_name)
            {
                col_index.push_back(i);
                exist_flag = true;
                break;
            }
        }
        if (!exist_flag)
        {
            std::cout << "Error: no column named \"" << col.column_name << "\"!" << std::endl;
            return;
        }
    }
    // TODO: process * case
    // Print header
    for (auto it = columns.begin(); it != columns.end(); ++it)
    {
        if (it != columns.begin()) { std::cout << "\t"; }
        std::cout << it->column_name;
    }
    std::cout << std::endl;
    // Print rows
    for (; !btit.IsEnd(); btit.next())
    {
        record_manager.Open(*btit, false);
        for (auto it = col_index.begin(); it != col_index.end(); ++it)
        {
            if (it != col_index.begin()) { std::cout << "\t"; }
            record_manager.Seek(tm->table_header().column_offset[*it]);
            int col_length = tm->table_header().column_length[*it];
            switch (tm->table_header().column_type[*it])
            {
            case Col_Type::COL_TYPE_INT:
            {
                int val;
                record_manager.Read(&val, col_length);
                std::cout << val;
                break;
            }
            case Col_Type::COL_TYPE_DOUBLE:
            {
                double val;
                record_manager.Read(&val, col_length);
                std::cout << val;
                break;
            }
            case Col_Type::COL_TYPE_BOOL:
            {
                bool val;
                record_manager.Read(&val, col_length);
                std::cout << (val ? "true" : "false");
                break;
            }
            case Col_Type::COL_TYPE_DATE:
            {
                Date val;
                record_manager.Read(&val, col_length);
                std::cout << (val.timestamp2str());
                break;
            }
            case Col_Type::COL_TYPE_CHAR:
            case Col_Type::COL_TYPE_VARCHAR:
            {
                char *val = new char[col_length + 1];
                val[col_length] = '\0';
                record_manager.Read(val, col_length);
                std::cout << val;
                delete[] val;
                break;
            }
            case Col_Type::COL_TYPE_NULL:
            {
                std::cout << "NULL";
            }
            default:
                break;
            }
        }
        std::cout << std::endl;
    }
}

void DatabaseManager::iterate_many_table()
{
    // TODO
}