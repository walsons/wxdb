#ifndef TABLE_MANAGER_H_
#define TABLE_MANAGER_H_

#include "../sql/common.h"
#include "table_header.h"

class TableManager
{
public:
    TableManager() = default;
    ~TableManager() = default;
    void CreateTable(const std::shared_ptr<TableHeader> table_header);

};

#endif