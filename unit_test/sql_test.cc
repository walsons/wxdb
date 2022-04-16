#include "catch.hpp"

#include <iostream>
#include <memory>

#include "../include/sql/token.h"
#include "../include/sql/tokenizer.h"
#include "../include/sql/create_parser.h"
#include "../include/sql/insert_parser.h"
#include "../include/sql/select_parser.h"

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

void same_insert_stmt_test(const std::string &statement)
{
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

        // # SQL_Stmt_Type
        CHECK(sql_stmt_create->type_ == SQL_Stmt_Type::SQL_CREATE_TABLE);

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
    }

    SECTION("insert parser")
    {
        std::string statement = "insert into customers \
                                 values \
                                 ( 1, \
                                   'Jack', \
                                   'Shanghai' \
                                 );";
        auto tokenizer = std::make_shared<Tokenizer>(statement);
        InsertParser insert_parser(tokenizer);
        auto sql_stmt_insert = insert_parser.ParseSQLStmtInsert();
        REQUIRE(sql_stmt_insert != nullptr);

        // # SQL_Stmt_Type
        CHECK(sql_stmt_insert->type_ == SQL_Stmt_Type::SQL_INSERT);
        // # Table name
        CHECK(sql_stmt_insert->table_name_ == "customers");
        // # field name
        CHECK(sql_stmt_insert->fields_name_.empty());
        // # values
        DataValue value(Data_Type::DATA_TYPE_INT);
        value.SetIntValue(1);
        CHECK(sql_stmt_insert->values_[0].GetDataType() == value.GetDataType());
        CHECK(sql_stmt_insert->values_[0].int_value() == value.int_value());
        value.SetCharValue("Jack");
        CHECK(sql_stmt_insert->values_[1].GetDataType() == value.GetDataType());
        CHECK(sql_stmt_insert->values_[1].char_value() == value.char_value());
        value.SetCharValue("Shanghai");
        CHECK(sql_stmt_insert->values_[2].GetDataType() == value.GetDataType());
        CHECK(sql_stmt_insert->values_[2].char_value() == value.char_value());

        statement = "insert into customers \
                     ( cust_id, \
                       cust_name, \
                       cust_address \
                     ) \
                     values \
                     ( 1, \
                       \"Jack\", \
                       \"Shanghai\" \
                     );";
        tokenizer = std::make_shared<Tokenizer>(statement);
        insert_parser = InsertParser(tokenizer);
        sql_stmt_insert = insert_parser.ParseSQLStmtInsert();
        REQUIRE(sql_stmt_insert != nullptr);

        // # SQL_Stmt_Type
        CHECK(sql_stmt_insert->type_ == SQL_Stmt_Type::SQL_INSERT);
        // # Table name
        CHECK(sql_stmt_insert->table_name_ == "customers");
        // # field name
        CHECK(sql_stmt_insert->fields_name_[0] == "cust_id");
        CHECK(sql_stmt_insert->fields_name_[1] == "cust_name");
        CHECK(sql_stmt_insert->fields_name_[2] == "cust_address");
        // # values
        value.SetIntValue(1);
        CHECK(sql_stmt_insert->values_[0].GetDataType() == value.GetDataType());
        CHECK(sql_stmt_insert->values_[0].int_value() == value.int_value());
        value.SetCharValue("Jack");
        CHECK(sql_stmt_insert->values_[1].GetDataType() == value.GetDataType());
        CHECK(sql_stmt_insert->values_[1].char_value() == value.char_value());
        value.SetCharValue("Shanghai");
        CHECK(sql_stmt_insert->values_[2].GetDataType() == value.GetDataType());
        CHECK(sql_stmt_insert->values_[2].char_value() == value.char_value());
    }

    SECTION("select parser")
    {
        std::string statement = "selec cust_id, cust_address \
                                 from customers;";
        auto tokenizer = std::make_shared<Tokenizer>(statement);
        SelectParser select_parser(tokenizer);
        auto sra = select_parser.ParseSQLStmtSelect();
        REQUIRE(sra != nullptr);
        REQUIRE(sra->type_ == SRA_Type::SRA_PROJECT);
    }
}