/******************** Database Management System ********************/
#ifndef DBMS_H_
#define DBMS_H_

#include <direct.h>
#include <string>
#include <memory>
#include <cassert>
#include "../defs.h"
#include "database_manager.h"

class DBMS
{
    DBMS();
public:
    static DBMS &GetInstance();
    ~DBMS() = default;

    void CreateDatabase(const std::string &db_name);
    void UseDatabase(const std::string &db_name);
    void CreateTable(const std::shared_ptr<TableHeader> table_header);
    void InsertRow(const std::shared_ptr<InsertInfo> insert_info);

private:
    std::shared_ptr<DatabaseManager> db_manager_;
};

inline DBMS::DBMS() : db_manager_(std::make_shared<DatabaseManager>())
{
    int exists = access(DB_DIR.c_str(), F_OK);
    if (exists != 0)
    {
        int res = mkdir(DB_DIR.c_str());
        assert(res == 0);
    }
}

inline DBMS &DBMS::GetInstance()
{
    static DBMS dbms;
    return dbms;
}

#endif