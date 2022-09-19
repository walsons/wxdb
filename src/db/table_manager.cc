#include "../../include/db/table_manager.h"
#include <cassert>

bool TableManager::CreateTable(const std::shared_ptr<TableHeader> table_header)
{
    if (is_open_) { return false; }
    table_name_ = table_header->table_name;
    std::string data_path = DB_DIR + table_name_ + ".tdata";
    pg_ = std::make_shared<Pager>(data_path);
    btr_ = std::make_shared<IntBTree>(pg_, table_header->index_root_page[table_header->main_index]);

    table_header_ = *table_header;
    table_header_.index_root_page[table_header_.main_index] = btr_->root_page_id();

    allocate_temp_record();
    load_indices();
    load_check_constraints();
    is_mirror_ = false;
    is_open_ = true;

    // Save table header
    std::string header_path = DB_DIR + table_name_ + ".thead";
    std::ofstream ofs(header_path, std::ios::binary);
    ofs.write(reinterpret_cast<const char *>(&table_header_), sizeof(table_header_));
    
    return true;
}

bool TableManager::OpenTable(const std::string &table_name)
{
    if (is_open_) { return false; }
    table_name_ = table_name;
    std::string header_path = DB_DIR + table_name_ + ".thead";
    std::string data_path = DB_DIR + table_name_ + ".tdata";
    std::ifstream ifs(header_path, std::ios::binary);
    assert(ifs.good());
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

void TableManager::DropTable()
{
    if (!is_open_) { return; }
    CloseTable();
    std::string header_path = DB_DIR + table_name_ + ".thead";
    std::string data_path = DB_DIR + table_name_ + ".tdata";
    std::remove(header_path.c_str());
    std::remove(data_path.c_str());
}

void TableManager::CloseTable()
{
    if (!is_open_) { return; }
    if (!is_mirror_)
    {
        std::string header_path = DB_DIR + table_name_ + ".thead";
        table_header_.index_root_page[table_header_.main_index] = btr_->root_page_id();
        free_indices();
        free_check_constraints();
        std::ofstream ofs(header_path, std::ios::binary);
        ofs.write(reinterpret_cast<const char *>(&table_header_), sizeof(table_header_));
    }
    btr_ = nullptr;
    // Write contents in cache back to .tdata file(will call pg_->Close() in destructor)
    pg_ = nullptr;
    delete[] tmp_record_;
    delete[] tmp_cache_;
    delete[] tmp_index_;
    tmp_record_ = nullptr;
    tmp_cache_ = nullptr;
    tmp_index_ = nullptr;
    is_open_ = false;
    is_mirror_ = false;
}

std::shared_ptr<TableManager> TableManager::Mirror(const std::string &alias)
{
    auto table = std::make_shared<TableManager>();
    table->table_header_ = table_header_;
    std::strncpy(table->table_header_.table_name, alias.c_str(), MAX_LENGTH_NAME);
    table->table_name_ = alias;
    table->is_open_ = true; 
    table->is_mirror_ = true;
    // Share btr_ and pg_
    table->btr_ = btr_;
    table->pg_ = pg_;
    table->allocate_temp_record();
    // Share indices_, check_constraint_, 
    for (size_t i = 0; i < MAX_NUM_COLUMN; ++i)
    {
        table->indices_[i] = indices_[i];
    }
    std::memcpy(table->check_constraint_, check_constraint_, sizeof(check_constraint_));
    return table;
}

int TableManager::InsertRecord()
{
    int *row_id = reinterpret_cast<int *>(tmp_record_);   
    // Increment auto_inc after insert successfully
    if (table_header_.is_main_index_auto_inc) { *row_id = table_header_.auto_inc; }
    if (table_header_.num_check_constraint != 0)
    {
        std::memcpy(tmp_cache_, tmp_record_, tmp_record_size_);
    }
    // auto_inc starts from 1, return 0 if failed
    if (!validate_constraints()) { return 0; }
    btr_->Insert(*row_id, tmp_record_, tmp_record_size_);
    // Update other indices
    for (int i = 0; i < table_header_.num_column; ++i)
    {
        if (i != table_header_.main_index && ((1 << i) & table_header_.flag_index))
        {
            if (*tmp_null_mark_ & (1 << i))
            {
                indices_[i]->Insert(nullptr, *row_id);
            }
            else
            {
                indices_[i]->Insert(tmp_record_ + table_header_.column_offset[i], *row_id);
            }
        }
    }
    if (table_header_.is_main_index_auto_inc)
    {
        ++table_header_.num_record;
        ++table_header_.auto_inc;
    }
    return *row_id;
}

bool TableManager::SetTempRecord(int column_number, ColVal value)
{
    if (value.type_ == Col_Type::COL_TYPE_NULL)
    {
        *tmp_null_mark_ |= 1 << column_number;
        return true;
    }
    *tmp_null_mark_ &= ~(1 << column_number);
    switch (value.type_)
    {
    case Col_Type::COL_TYPE_INT:
        *reinterpret_cast<int *>(tmp_record_ + table_header_.column_offset[column_number]) = value.ival_;
        break;
    case Col_Type::COL_TYPE_DOUBLE:
        *reinterpret_cast<double *>(tmp_record_ + table_header_.column_offset[column_number]) = value.dval_;
        break;
    case Col_Type::COL_TYPE_BOOL:
        *reinterpret_cast<bool *>(tmp_record_ + table_header_.column_offset[column_number]) = value.bval_;
        break;
    case Col_Type::COL_TYPE_VARCHAR:
    case Col_Type::COL_TYPE_CHAR:
        std::strncpy(tmp_record_ + table_header_.column_offset[column_number], 
                     value.sval_.c_str(), 
                     table_header_.column_length[column_number]);
        break;
    case Col_Type::COL_TYPE_DATE:
        std::memcpy(tmp_record_ + table_header_.column_offset[column_number], &value.tval_,sizeof(Date));
        break;
    default:
        assert(false);
        break;
    }
    return true;
}

int TableManager::DeleteRecord(int row_id)
{
    btr_->Erase(row_id);
    // Update other indices
    for (int i = 0; i < table_header_.num_column; ++i)
    {
        if (i != table_header_.main_index && ((1 << i) & table_header_.flag_index))
        {
            if (*tmp_null_mark_ & (1 << i))
            {
                indices_[i]->Erase(nullptr, row_id);
            }
            else
            {
                indices_[i]->Erase(tmp_record_ + table_header_.column_offset[i], row_id);
            }
        }
    }
    if (table_header_.is_main_index_auto_inc)
    {
        --table_header_.num_record;
    }
}
    
int TableManager::UpdateRecord(int row_id)
{
    btr_->Update(row_id, tmp_record_, tmp_record_size_);  
    // Update other indices
    for (int i = 0; i < table_header_.num_column; ++i)
    {
        if (i != table_header_.main_index && ((1 << i) & table_header_.flag_index))
        {
            if (*tmp_null_mark_ & (1 << i))
            {
                indices_[i]->Update(nullptr, row_id);
            }
            else
            {
                indices_[i]->Update(tmp_record_ + table_header_.column_offset[i], row_id);
            }
        }
    }
    return row_id;
}

std::pair<int, int> TableManager::GetRowPosition(int row_id)
{
    auto pos = btr_->upper_bound(btr_->root_page_id(), row_id);
    return pos;
}

void TableManager::PrintCurrentTmpRecord() 
{

}

void TableManager::allocate_temp_record()
{
    if (tmp_record_) { delete[] tmp_record_; }
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
    *tmp_null_mark_ = 0;
}

void TableManager::load_indices()
{
    for (int i = 0; i < table_header_.num_column; ++i)
    {
        // It's index column except main index
        if (i != table_header_.main_index && ((1 << i) & table_header_.flag_index))
        {
            indices_[i] = std::make_shared<IndexManager>(table_header_.column_length[i],
                pg_, table_header_.index_root_page[i], IndexManager::GetIndexComparer(table_header_.column_type[i]));
        }
    }
}

void TableManager::free_indices()
{
    for (int i = 0; i < table_header_.num_column; ++i)
    {
        if (i != table_header_.main_index && ((1 << i) & table_header_.flag_index))
        {
            table_header_.index_root_page[i] = indices_[i]->root_page_id();
            indices_[i] = nullptr;
        }
    }
}

void TableManager::load_check_constraints()
{
    std::memset(check_constraint_, 0, sizeof(check_constraint_));
    for (size_t i = 0; i != table_header_.num_check_constraint; ++i)
    {
        std::istringstream in(table_header_.check_constraint[i]);
        check_constraint_[i] = Expression::LoadExprNode(in);
    }
}

void TableManager::free_check_constraints()
{
    for (size_t i = 0; i != table_header_.num_check_constraint; ++i)
    {
        Expression::FreeExprNode(check_constraint_[i]);
        check_constraint_[i] = nullptr;
    }
}

bool TableManager::validate_constraints()
{
    // TODO: validate all constraints
    return true;
}