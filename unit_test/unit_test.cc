#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2.hpp"

#include <string>

#include "../src/meta_command.h"
#include "../src/sql_statement.h"

MetaCommand meta_command;
TEST_CASE( "META_COMMAND", "[meta command test]" ) {
    meta_command.ExecuteMetaCommand(".unrecognized command such as fjslkjkef");
    meta_command.ExecuteMetaCommand(".exit");
    CHECK(meta_command.MainLoop() == false);
    meta_command.ExecuteMetaCommand(".help");
};

SQLStatement sql_statement;
Database database;
TEST_CASE( "SQL_STATEMENT", "[SQL statement test]" ) {
    std::string user_input = "create user";
    PREPARE_RESULT res = sql_statement.PrepareStatement(database, user_input);
    REQUIRE(res == PREPARE_RESULT::SUCCESS);
    sql_statement.ExecuteStatement(database, user_input);

    user_input = "insert user 1 walsons walsons@163.com";
    res = sql_statement.PrepareStatement(database, user_input);
    REQUIRE(res == PREPARE_RESULT::SUCCESS);
    sql_statement.ExecuteStatement(database, user_input);

    user_input = "select user"; 
    res = sql_statement.PrepareStatement(database, user_input);
    REQUIRE(res == PREPARE_RESULT::SUCCESS);
    sql_statement.ExecuteStatement(database, user_input);
};
