/******************** Database Management System ********************/
#ifndef DBMS_H_
#define DBMS_H_

#include <string>
#include <memory>
#include "database_manager.h"

class DBMS
{
public:
    DBMS();
    ~DBMS() = default;

    void CreateDatabase(const std::string &db_name);
    void UseDatabase(const std::string &db_name);
    void CreateTable(const std::shared_ptr<TableHeader> table_header);

private:
    std::shared_ptr<DatabaseManager> db_manager_;
};

inline
DBMS::DBMS() : db_manager_(std::make_shared<DatabaseManager>())
{
}

#endif