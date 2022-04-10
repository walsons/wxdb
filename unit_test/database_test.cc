#include "catch.hpp"

#include <iostream>
#include <string>
#include <vector>

#include "../include/db/meta_command.h"
#include "../include/db/a_sql_statement.h"

MetaCommand meta_command;
TEST_CASE( "META_COMMAND", "[meta command test]" ) {
    meta_command.ExecuteMetaCommand(".unrecognized command such as fjslkjkef");
    meta_command.ExecuteMetaCommand(".exit");
    CHECK(meta_command.MainLoop() == false);
    meta_command.ExecuteMetaCommand(".help");
};

ASQLStatement sql_statement;
Database database;
TEST_CASE( "SQL_STATEMENT", "[SQL statement test]" ) {
    std::string user_input = "create user";
    PREPARE_RESULT res = sql_statement.PrepareStatement(database, user_input);
    REQUIRE(res == PREPARE_RESULT::SUCCESS);
    sql_statement.ExecuteStatement(database, user_input);

    user_input = "select user"; 
    res = sql_statement.PrepareStatement(database, user_input);
    REQUIRE(res == PREPARE_RESULT::SUCCESS);
    sql_statement.ExecuteStatement(database, user_input);

    std::vector<int> ids{1,5,4,8,7,2,3,9,12,16,23,21,20,6,10,22,19,11,18,14,13,15,17,28};
    for (const auto &id : ids)
    {
        user_input = "insert user " + std::to_string(id) +  " walsons walsons@163.com";
        res = sql_statement.PrepareStatement(database, user_input);
        REQUIRE(res == PREPARE_RESULT::SUCCESS);
        sql_statement.ExecuteStatement(database, user_input);
    }

    user_input = "select user"; 
    res = sql_statement.PrepareStatement(database, user_input);
    REQUIRE(res == PREPARE_RESULT::SUCCESS);
    sql_statement.ExecuteStatement(database, user_input);

    // Print tree structure
    Pager *pager = database.table()->pager;
    unsigned root_page_num = database.table()->root_page_num;
    if (pager->GetPageType(root_page_num) == NODE_TYPE::INTERNAL_NODE)
    {

    }
    else
    {
        LeafNode *leaf_node = pager->GetLeafNode(root_page_num);
        std::cout << CELL_CAPACITY << std::endl;
    }
};
