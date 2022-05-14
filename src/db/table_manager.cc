#include "../../include/db/table_manager.h"

bool TableManager::CreateTable(const std::shared_ptr<TableHeader> table_header)
{
    if (is_open_) { return false; }
    table_name_ = table_header->table_name;
    std::string data_path = DB_DIR + table_name_ + ".tdata";
    pg_ = std::make_shared<Pager>(data_path);
    btr_ = std::make_shared<IntBTree>(pg_);

    table_header_ = *table_header;
    table_header_.index_root_page[table_header->main_index] = btr_->root_page_id();

    allocate_temp_record();
    load_indices();
    load_check_constraints();
    is_mirror_ = false;
    is_open_ = true;
    return true;
}

bool TableManager::OpenTable(const std::string &table_name)
{
    if (is_open_) { return false; }
    table_name_ = table_name;
    std::string header_path = DB_DIR + table_name_ + ".thead";
    std::string data_path = DB_DIR + table_name_ + ".tdata";
    std::ifstream ifs(header_path, std::ios::binary);
    ifs.read(reinterpret_cast<char *>(&table_header_), sizeof(table_header_));
    pg_ = std::make_shared<Pager>(data_path);
    btr_ = std::make_shared<IntBTree>(pg_, table_header_.index_root_page[table_header_.main_index]);

    allocate_temp_record();
    load_indices();
    load_check_constraints();
    is_mirror_ = false;
    is_open_ = true;
    return true;
}

void TableManager::allocate_temp_record()
{
    if (tmp_record_) delete[] tmp_record_;
    int total_length = 4;  // 4 bytes for not null
    for (int i = 0; i < table_header_.num_column; ++i)
    {
        total_length += table_header_.column_length[i];
    }
    tmp_record_size_ = total_length;
    tmp_record_ = new char[tmp_record_size_];
    tmp_cache_ = new char[tmp_record_size_];
    tmp_index_ = new char[tmp_record_size_];
    tmp_null_mark_ = reinterpret_cast<int *>(tmp_record_ + 4);
}

void TableManager::load_indices()
{
    for (int i = 0; i < table_header_.num_column; ++i)
    {
        // It's index column except main index
        if (i != table_header_.main_index && ((1 << i) & table_header_.flag_index))
        {
            indices_[i] = std::make_shared<IndexManager>(pg_,
                table_header_.column_length[i], 
                table_header_.index_root_page[i],
                IndexManager::GetIndexComparer(table_header_.column_type[i]));
        }
    }
}

void TableManager::load_check_constraints()
{
    std::memset(check_constraint_, 0, sizeof(check_constraint_));
    for (int i = 0; i != table_header_.num_check_constraint; ++i)
    {
        std::istringstream in(table_header_.check_constraint[i]);
        check_constraint_[i] = Expression::LoadExprNode(in);
    }
}