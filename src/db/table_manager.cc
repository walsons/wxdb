#include "../../include/db/table_manager.h"

bool TableManager::CreateTable(const std::shared_ptr<TableHeader> table_header)
{
    if (is_open_) { return false; }
    table_name_ = table_header->table_name;
    std::string file_path = DB_DIR + table_name_ + ".tdata";
    pg_ = std::make_shared<Pager>(file_path);
    btr_ = std::make_shared<IntBTree>(pg_);

    table_header_ = *table_header;
    table_header_.index_root_page[table_header->main_index] = btr_->root_page_id();

    allocate_temp_record();
    // TODO
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