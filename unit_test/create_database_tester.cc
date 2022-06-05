#include "common_header.h"

TEST_CASE( "TC-CreateDatabase", "[create database]" ) 
{
    namespace fs = std::filesystem;
    SECTION("create database implement")
    {
        fs::remove_all(DB_DIR.c_str());
        std::string statement = "create database mydb";
        auto t = std::make_shared<Tokenizer>(statement);
        auto database_parser = std::make_shared<DatabaseParser>(t);
        auto database_info = database_parser->CreateDatabase();
        DBMS::GetInstance().CreateDatabase(database_info->database_name);
        
        bool exist = fs::exists(DB_DIR.c_str());
        REQUIRE(exist);
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