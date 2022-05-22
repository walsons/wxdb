#include "catch.hpp"

#include "direct.h"

#include "../include/sql/database_parser.h"
#include "../include/db/dbms.h"
#include "../include/db/database_manager.h"
#include "../include/sql/table_parser.h"
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

    SECTION("create table")
    {
        DBMS::GetInstance().UseDatabase("mydb");
        std::string statement = "CREATE TABLE users (                              \
                                     id          INT,                              \ 
                                     name        CHAR(32)       NOT NULL,          \
                                     email       VARCHAR(255),                     \
                                     age         INT,                              \
                                     height      DOUBLE,                           \
                                     country     CHAR(32)       DEFAULT \"China\", \
                                     sign_up     DATE,                             \
                                     UNIQUE (email),                               \
                                     PRIMARY KEY (id),                             \
                                     CHECK(age>=18 AND age<= 60)                   \
                                 );";
        auto t = std::make_shared<Tokenizer>(statement);
        auto parser = std::make_shared<TableParser>(t);
        auto info = parser->CreateTable();
        auto table_header = std::make_shared<TableHeader>();
        fill_table_header(table_header, *info);               
        DBMS::GetInstance().CreateTable(table_header);
        DBMS::GetInstance().CloseDatabase();
    }
}