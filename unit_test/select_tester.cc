#include "common_header.h"

TEST_CASE( "TC-Select", "[insert row test]" ) 
{
    namespace fs = std::filesystem;
    // Create database
    {
        fs::remove_all(DB_DIR.c_str());
        std::string statement = "create database mydb";
        auto t = std::make_shared<Tokenizer>(statement);
        auto database_parser = std::make_shared<DatabaseParser>(t);
        auto database_info = database_parser->CreateDatabase();
        DBMS::GetInstance().CreateDatabase(database_info->database_name);
    }
    DBMS::GetInstance().UseDatabase("mydb");
    // Create first table
    {
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
        auto table_parser = std::make_shared<TableParser>(t);
        auto table_info = table_parser->CreateTable();
        REQUIRE(table_info != nullptr);
        auto table_header = std::make_shared<TableHeader>();
        fill_table_header(table_header, *table_info);               
        DBMS::GetInstance().CreateTable(table_header);
    }
    // Create second table
    {
        std::string statement = "CREATE TABLE comments (                           \
                                     id          INT,                              \
                                     user_id     INT        NOT NULL,              \
                                     time        DATE       NOT NULL,              \
                                     contents    VARCHAR(255),                     \
                                     PRIMARY KEY (id),                             \
                                     FOREIGN KEY (user_id) REFERENCES users (id)   \
                                );";
        auto t = std::make_shared<Tokenizer>(statement);
        auto table_parser = std::make_shared<TableParser>(t);
        auto table_info = table_parser->CreateTable();
        REQUIRE(table_info != nullptr);
        auto table_header = std::make_shared<TableHeader>();
        fill_table_header(table_header, *table_info);               
        DBMS::GetInstance().CreateTable(table_header);
    }
    // Insert row
    {
        std::vector<std::string> statements;
        // users
        statements.emplace_back("INSERT INTO users (id, name, email, age, height, country, sign_up) \
                                    VALUES (1, \"Walson\", \"walsons@163.com\", 18, 180, \"China\", \"2020-01-03\");");
        statements.emplace_back("INSERT INTO users (id, name, email, age, height, country, sign_up) \
                                    VALUES (2, \"John\", \"John123@163.com\", 20, 175, \"China\", \"2020-03-12\");");
        statements.emplace_back("INSERT INTO users (id, name, email, age, height, country, sign_up) \
                                    VALUES (3, \"caler\", \"caler2000@gmail.com\", 22, 173, \"Canada\", \"2020-01-23\");");
        // comments
        statements.emplace_back("INSERT INTO comments (id, user_id, time, contents) \
                                    VALUES (1, 1, \"2021-06-03\", \"Yes!\");");
        statements.emplace_back("INSERT INTO comments (id, user_id, time, contents) \
                                    VALUES (2, 1, \"2021-06-03\", \"Cool!\");");
        for (auto statement : statements)
        {
            auto tokenizer = std::make_shared<Tokenizer>(statement);
            TableParser table_parser(tokenizer);
            auto insert_info = table_parser.InsertTable();
            REQUIRE(insert_info != nullptr);
            DBMS::GetInstance().InsertRow(insert_info);
        }
    }

    SECTION("select one table")
    {
        std::string statement = "SELECT id, name, sign_up FROM users WHERE id < 10;";
        auto tokenizer = std::make_shared<Tokenizer>(statement);
        TableParser table_parser(tokenizer);
        auto select_info = table_parser.SelectTable();
        REQUIRE(select_info != nullptr);
        // columns
        CHECK(select_info->columns[0].table_name.empty());
        CHECK(select_info->columns[0].column_name == "id");
        CHECK(select_info->columns[1].column_name == "name");
        CHECK(select_info->columns[2].column_name == "sign_up");
        // tables
        CHECK(select_info->tables[0] == "users");
        // where
        CHECK(select_info->where != nullptr);

        DBMS::GetInstance().SelectTable(select_info);
        DBMS::GetInstance().CloseDatabase();
    }

    SECTION("select one table using asterisk")
    {
        std::string statement = "SELECT * FROM users WHERE id < 10;";
        auto tokenizer = std::make_shared<Tokenizer>(statement);
        TableParser table_parser(tokenizer);
        auto select_info = table_parser.SelectTable();
        REQUIRE(select_info != nullptr);
        // columns
        CHECK(select_info->columns.empty());
        // tables
        CHECK(select_info->tables[0] == "users");
        // where
        CHECK(select_info->where != nullptr);

        DBMS::GetInstance().SelectTable(select_info);
        DBMS::GetInstance().CloseDatabase();
    }

    SECTION("select one table using index")
    {
        std::string statement = "SELECT id, name, sign_up FROM users WHERE id = 2;";
        auto tokenizer = std::make_shared<Tokenizer>(statement);
        TableParser table_parser(tokenizer);
        auto select_info = table_parser.SelectTable();
        REQUIRE(select_info != nullptr);
        // columns
        CHECK(select_info->columns[0].table_name.empty());
        CHECK(select_info->columns[0].column_name == "id");
        CHECK(select_info->columns[1].column_name == "name");
        CHECK(select_info->columns[2].column_name == "sign_up");
        // tables
        CHECK(select_info->tables[0] == "users");
        // where
        CHECK(select_info->where != nullptr);

        DBMS::GetInstance().SelectTable(select_info);
        DBMS::GetInstance().CloseDatabase();
    }

    SECTION("select many tables")
    {
        std::string statement = "SELECT comments.id, users.name, comments.time, comments.contents \
                                 FROM users, comments \
                                 WHERE users.id = comments.user_id;";
        auto tokenizer = std::make_shared<Tokenizer>(statement);
        TableParser table_parser(tokenizer);
        auto select_info = table_parser.SelectTable();
        REQUIRE(select_info != nullptr);
        // columns
        CHECK(select_info->columns[0].table_name == "comments");
        CHECK(select_info->columns[0].column_name == "id");
        CHECK(select_info->columns[1].table_name == "users");
        CHECK(select_info->columns[1].column_name == "name");
        CHECK(select_info->columns[2].column_name == "time");
        CHECK(select_info->columns[3].column_name == "contents");
        // tables
        CHECK(select_info->tables[0] == "users");
        CHECK(select_info->tables[1] == "comments");
        // where
        CHECK(select_info->where != nullptr);

        DBMS::GetInstance().SelectTable(select_info);
        DBMS::GetInstance().CloseDatabase();
    }

    SECTION("select many tables using asterisk")
    {
        std::string statement = "SELECT * \
                                 FROM users, comments \
                                 WHERE users.id = comments.user_id;";
        auto tokenizer = std::make_shared<Tokenizer>(statement);
        TableParser table_parser(tokenizer);
        auto select_info = table_parser.SelectTable();
        REQUIRE(select_info != nullptr);
        // columns
        CHECK(select_info->columns.empty());
        // tables
        CHECK(select_info->tables[0] == "users");
        CHECK(select_info->tables[1] == "comments");
        // where
        CHECK(select_info->where != nullptr);

        DBMS::GetInstance().SelectTable(select_info);
        DBMS::GetInstance().CloseDatabase();
    }

    SECTION("select many tables using index")
    {
        std::string statement = "SELECT * \
                                 FROM users, comments \
                                 WHERE users.email = \"walsons@163.com\" AND comments.user_id = 1;";
        auto tokenizer = std::make_shared<Tokenizer>(statement);
        TableParser table_parser(tokenizer);
        auto select_info = table_parser.SelectTable();
        REQUIRE(select_info != nullptr);
        // columns
        CHECK(select_info->columns.empty());
        // tables
        CHECK(select_info->tables[0] == "users");
        CHECK(select_info->tables[1] == "comments");
        // where
        CHECK(select_info->where != nullptr);

        DBMS::GetInstance().SelectTable(select_info);
        DBMS::GetInstance().CloseDatabase();
    }
}