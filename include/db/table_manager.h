#ifndef TABLE_MANAGER_H_
#define TABLE_MANAGER_H_

#include "../sql/table_info.h"
#include "table_header.h"
#include "../btree/btree.hpp"
#include "../page/pager.h"
#include "index_manager.h"

class TableManager
{
    TableHeader table_header_;
    std::string table_name_;
    bool is_open_ = false, is_mirror_ = false;
    std::shared_ptr<IntBTree> btr_;
    std::shared_ptr<Pager> pg_;
    int tmp_record_size_;
    char *tmp_record_, *tmp_cache_, *tmp_index_; 
    int *tmp_null_mark_;
    std::shared_ptr<IndexManager> indices_[MAX_NUM_COLUMN];
    ExprNode *check_constraint_[MAX_NUM_CHECK_CONSTRAINT];

public:
    TableManager() = default;
    TableManager(const std::string &table_name) : table_name_(table_name) {}
    ~TableManager() = default;
    bool CreateTable(const std::shared_ptr<TableHeader> table_header);
    bool OpenTable(const std::string &table_name);
    void DropTable();
    void CloseTable();
    std::shared_ptr<TableManager> Mirror(const std::string &alias);

    int InsertRecord();
    bool SetTempRecord(int column_number, ColVal value);

    std::pair<int, int> GetRowPosition(int row_id);

private:
    void allocate_temp_record();
    void load_indices();
    void free_indices();
    void load_check_constraints();
    void free_check_constraints();
    bool validate_constraints();

public:
    const TableHeader &table_header() const { return table_header_; }
    std::string table_name() const { return table_name_; }
    bool is_open() const { return is_open_; }
    unsigned number_of_column() const { return table_header_.num_column; }
    Col_Type column_type(int index) const { return table_header_.column_type[index]; }
    std::string column_name(int index) const { return table_header_.column_name[index]; }
    std::shared_ptr<Pager> pg() { return pg_; }
    std::shared_ptr<IntBTree> btr() { return btr_; }
};

#endif