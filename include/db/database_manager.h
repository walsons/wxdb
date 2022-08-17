#ifndef DATABASE_MANAGER_H_
#define DATABASE_MANAGER_H_

#include <cstring>
#include <string>
#include <vector>
#include <memory>

#include "../../include/defs.h"
#include "../../include/db/table_manager.h"
#include "../../include/db/col_val.h"
#include "../../include/btree/btree_iterator.hpp"
#include "../../include/db/record_manager.h"

class DatabaseManager
{
public:
    DatabaseManager();
    ~DatabaseManager() = default;
    void CloseDatabase();

    bool CreateDatabase(const std::string &db_name);
    bool OpenDatabase(const std::string &db_name);
    bool CreateTable(const std::shared_ptr<TableHeader> table_header);
    void InsertRow(const std::shared_ptr<InsertInfo> insert_info);
    void SelectTable(const std::shared_ptr<SelectInfo> select_info);

private:
    void update_column2term(const std::shared_ptr<TableManager> &tm,
                            BTreeIterator<VariantPage> &btit, 
                            RecordManager &rm,
                            std::unordered_map<std::string, std::shared_ptr<TermExpr>> &column2term);
    void update_column2term(const std::vector<std::shared_ptr<TableManager>> &tms,
                            std::vector<BTreeIterator<VariantPage>> &btits, 
                            std::vector<RecordManager> &rms,
                            std::unordered_map<std::string, std::shared_ptr<TermExpr>> &column2term);
    void iterate_one_table(const std::shared_ptr<TableManager> &tm, std::vector<ColumnRef> &columns, ExprNode *condition);
    void iterate_many_table(const std::vector<std::shared_ptr<TableManager>> &tms, std::vector<ColumnRef> &columns, ExprNode *condition);
    void iterate_one_table(const std::shared_ptr<TableManager> &tm, std::vector<ColumnRef> &columns, ExprNode *condition,
                           std::vector<std::shared_ptr<TermExpr>> cols, std::vector<std::shared_ptr<TermExpr>> vals);
    void iterate_many_table(const std::vector<std::shared_ptr<TableManager>> &tms, std::vector<ColumnRef> &columns, ExprNode *condition,
                            std::vector<std::shared_ptr<TermExpr>> cols, std::vector<std::shared_ptr<TermExpr>> vals);

private:
    // These infomation about database need to be stored in "database_name.db" file:
    // 1. Database name (char[]: 255 bytes + 1 bytes '\0')
    // 2. The number of tables (unsigned int: 4 bytes)
    // 3. Table name array (char[]: n * (255 bytes + 1 bytes '\0'))
    struct Info
    {
        char db_name[MAX_LENGTH_NAME];
        unsigned num_table;
        char table_name[MAX_NUM_TABLE][MAX_LENGTH_NAME];
    };
    Info info_;
    std::vector<std::shared_ptr<TableManager>> table_manager_;
    
private:
    bool is_open_;
};

inline DatabaseManager::DatabaseManager() : is_open_(false)
{
    std::memset(&info_, 0, sizeof(info_));
}

#endif