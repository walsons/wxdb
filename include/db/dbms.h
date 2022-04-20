/******************** Database Management System ********************/
#ifndef DBMS_H_
#define DBMS_H_

#include <string>
#include "database_manager.h"

class DBMS
{
public:
    DBMS() = default;
    ~DBMS() = default;
    void CreateDatabase(const std::string &db_name);

    DatabaseManager db_manager_;
};

#endif