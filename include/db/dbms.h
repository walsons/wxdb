/******************** Database Management System ********************/
#ifndef DBMS_H_
#define DBMS_H_

#ifdef _WIN32
#include <direct.h>
#elif __linux__
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif
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
    // About SQL
    void CreateDatabase(const std::string &db_name);
    void UseDatabase(const std::string &db_name);
    void CreateTable(const std::shared_ptr<TableHeader> table_header);
    void InsertRow(const std::shared_ptr<InsertInfo> insert_info);
    void SelectTable(const std::shared_ptr<SelectInfo> select_info);
    // call when exit
    void CloseDatabase();

private:
    std::shared_ptr<DatabaseManager> db_manager_;
};

inline DBMS::DBMS() : db_manager_(std::make_shared<DatabaseManager>())
{
#ifdef _WIN32
    int exists = access(DB_DIR.c_str(), F_OK);
    if (exists != 0)
    {
        int res = mkdir(DB_DIR.c_str());
        assert(res == 0);
    }
#elif __linux__
    int exists = access(DB_DIR.c_str(), F_OK);
    if (exists != 0)
    {
        int res = mkdir(DB_DIR.c_str(), 0775);
        assert(res == 0);
    }
#endif
}

inline DBMS &DBMS::GetInstance()
{
    static DBMS dbms;
    return dbms;
}

#endif