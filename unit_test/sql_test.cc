#include "catch.hpp"

#include <iostream>
#include <memory>

#include "../include/sql/token.h"
#include "../include/sql/tokenizer.h"
#include "../include/sql/create_parser.h"

TEST_CASE( "TC-TOKENIZER", "[tokenizer test]" ) 
{
    SECTION("tokenizer")
    {
        Tokenizer tokenizer("select name age from users;");
        std::shared_ptr<Token> token = nullptr;
        token = tokenizer.GetNextToken();
        CHECK(token->text_ == "select");
        CHECK(token->type_ == Token_Type::TOKEN_RESERVED_WORD);
        token = tokenizer.GetNextToken();
        CHECK(token->text_ == "name");
        CHECK(token->type_ == Token_Type::TOKEN_WORD);
        token = tokenizer.GetNextToken();
        CHECK(token->text_ == "age");
        CHECK(token->type_ == Token_Type::TOKEN_WORD);
        token = tokenizer.GetNextToken();
        CHECK(token->text_ == "from");
        CHECK(token->type_ == Token_Type::TOKEN_RESERVED_WORD);
        token = tokenizer.GetNextToken();
        CHECK(token->text_ == "users");
        CHECK(token->type_ == Token_Type::TOKEN_WORD);
        token = tokenizer.GetNextToken();
        CHECK(token->text_ == ";");
        CHECK(token->type_ == Token_Type::TOKEN_SEMICOLON);
        token = tokenizer.GetNextToken();
        CHECK(token == nullptr);
    }
}

TEST_CASE("TC-PARSER", "[parser test]")
{
    SECTION("create parser")
    {
        std::string statement = "create table customers \
                                 ( cust_id int, \
                                   cust_name char(50), \
                                   cust_address char(50) \
                                 );";
        auto tokenizer = std::make_shared<Tokenizer>(statement);
        CreateParser create_parser(tokenizer);
        auto sql_stmt_create = create_parser.ParseSQLStmtCreate();
        REQUIRE(sql_stmt_create != nullptr);

        // # TableInfo
        auto table_info = sql_stmt_create->table_info_;
        CHECK(table_info->table_name_ == "customers");
        CHECK(table_info->fields_name_[0] == "cust_id");
        CHECK(table_info->fields_name_[1] == "cust_name");
        CHECK(table_info->fields_name_[2] == "cust_address");

        // ## FieldInfo
        auto field_info = (*table_info->fields_)["cust_id"];
        CHECK(field_info->field_name_ == "cust_id");
        CHECK(field_info->type_ == Data_Type::DATA_TYPE_INT);
        CHECK(field_info->length_ == sizeof(int));
        field_info = (*table_info->fields_)["cust_name"];
        CHECK(field_info->field_name_ == "cust_name");
        CHECK(field_info->type_ == Data_Type::DATA_TYPE_CHAR);
        CHECK(field_info->length_ == 50);
        field_info = (*table_info->fields_)["cust_address"];
        CHECK(field_info->field_name_ == "cust_address");
        CHECK(field_info->type_ == Data_Type::DATA_TYPE_CHAR);
        CHECK(field_info->length_ == 50);

        // # Constraint
        auto constraint = sql_stmt_create->constraint_;
        
    }
}