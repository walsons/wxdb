#ifndef TABLE_MANAGER_H_
#define TABLE_MANAGER_H_

#include "../sql/table_info.h"
#include "table_header.h"
#include "../btree/btree.h"
#include "../page/pager.h"

class TableManager
{
    bool is_open_, is_mirror_;
    TableHeader table_header_;
    std::shared_ptr<IntBTree> btr_;
    std::shared_ptr<Pager> pg_;
    std::string table_name_;
    int tmp_record_size_;
    char *tmp_record_, *tmp_cache_, *tmp_index_; 
    int *tmp_null_mark_;

public:
    TableManager() = default;
    ~TableManager() = default;
    bool CreateTable(const std::shared_ptr<TableHeader> table_header);

private:
    void allocate_temp_record();
};

#endif