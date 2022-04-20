#include "../../include/db/dbms.h"

void DBMS::CreateDatabase(const std::string &db_name)
{
    DatabaseManager db_manager;
    db_manager.Open(db_name);
    // db_manager.Close()
    
}