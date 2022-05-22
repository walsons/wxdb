#include "catch.hpp"

#include "direct.h"

#include "../include/sql/database_parser.h"
#include "../include/db/dbms.h"
#include "../include/db/database_manager.h"
#include <fstream>
#include <memory>

TEST_CASE( "TC-DATABASE", "[database test]" ) 
{
    SECTION("create database")
    {
        std::string statement = "create database mydb";
        auto t = std::make_shared<Tokenizer>(statement);
        auto parser = std::make_shared<DatabaseParser>(t);
        auto database_info = parser->CreateDatabase();
        DBMS::GetInstance().CreateDatabase(database_info->database_name);

        int exists = access(DB_DIR.c_str(), F_OK);
        REQUIRE(exists == 0);
        std::ifstream ifs(DB_DIR + "mydb.db");
        REQUIRE(ifs.is_open());
        struct Info
        {
            char db_name[MAX_LENGTH_NAME];
            unsigned num_table;
            char table_name[MAX_NUM_TABLE][MAX_LENGTH_NAME];
        };
        Info info;
        ifs.read(reinterpret_cast<char *>(&info), sizeof(Info));
        CHECK(std::string(info.db_name) == "mydb");
        CHECK(info.num_table == 0);
    }
}