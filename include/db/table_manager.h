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
    bool is_open_, is_mirror_;
    std::shared_ptr<IntBTree> btr_;
    std::shared_ptr<Pager> pg_;
    int tmp_record_size_;
    char *tmp_record_, *tmp_cache_, *tmp_index_; 
    int *tmp_null_mark_;
    std::shared_ptr<IndexManager> indices_[MAX_NUM_COLUMN];
    ExprNode *check_constraint_[MAX_NUM_CHECK_CONSTRAINT];

public:
    TableManager() = default;
    ~TableManager() = default;
    bool CreateTable(const std::shared_ptr<TableHeader> table_header);
    bool OpenTable(const std::string &table_name);
    void DropTable();
    void CloseTable();
    std::shared_ptr<TableManager> Mirror(const std::string &alias);

private:
    void allocate_temp_record();
    void load_indices();
    void free_indices();
    void load_check_constraints();
    void free_check_constraints();
};

#endif