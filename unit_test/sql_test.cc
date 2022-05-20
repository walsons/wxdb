#include "catch.hpp"

#include <iostream>
#include <memory>

#include "../include/sql/token.h"
#include "../include/sql/tokenizer.h"
#include "../include/sql/table_info.h"
#include "../include/sql/table_parser.h"
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
        TableParser table_parser(tokenizer);
        auto table_info = table_parser.CreateTable();
        REQUIRE(table_info != nullptr);

        // # TableInfo
        CHECK(table_info->table_name == "customers");

        // ## FieldInfo
        CHECK(table_info->field_info[0].field_name == "cust_id");
        CHECK(table_info->field_info[1].field_name == "cust_name");
        CHECK(table_info->field_info[2].field_name == "cust_address");

        auto field_info = table_info->field_info[0];
        CHECK(field_info.field_name == "cust_id");
        CHECK(field_info.type == Data_Type::DATA_TYPE_INT);
        CHECK(field_info.length == sizeof(int));
        field_info = table_info->field_info[1];
        CHECK(field_info.field_name == "cust_name");
        CHECK(field_info.type == Data_Type::DATA_TYPE_VARCHAR);
        CHECK(field_info.length == 50);
        field_info = table_info->field_info[2];
        CHECK(field_info.field_name == "cust_address");
        CHECK(field_info.type == Data_Type::DATA_TYPE_VARCHAR);
        CHECK(field_info.length == 50);
    }

    SECTION("create parser with constraints")
    {
        std::string statement = "CREATE TABLE customers ( \            
                                 id          int, \ 
                                 name        char(32)       NOT NULL , \
                                 email       VARCHAR(255)   UNIQUE, \
                                 age         int, \
                                 height      DOUBLE, \
                                 country     Char(32)       DEFAULT \"China\", \
                                 sign_up     Date, \
                                 PRIMARY KET (id), \
                                 CHECK(age>=18 AND age<= 60) \
                                 );";
        auto tokenizer = std::make_shared<Tokenizer>(statement);
        TableParser table_parser(tokenizer);
        auto table_info = table_parser.CreateTable();
        REQUIRE(table_info != nullptr);

        // # TableInfo
        CHECK(table_info->table_name == "customers");
        CHECK(table_info->field_info.size() == 9);
        
        // ## FieldInfo
        CHECK(table_info->field_info[0].field_name == "id");
        CHECK(table_info->field_info[1].field_name == "name");
        CHECK(table_info->field_info[1].constraint[0] == Constraint_Type::CONS_NOT_NULL);
        CHECK(table_info->field_info[2].field_name == "email");
        CHECK(table_info->field_info[2].constraint[0] == Constraint_Type::CONS_UNIQUE);
        CHECK(table_info->field_info[3].field_name == "age");
        CHECK(table_info->field_info[4].field_name == "height");
        CHECK(table_info->field_info[5].field_name == "country");
        CHECK(table_info->field_info[5].constraint[0] == Constraint_Type::CONS_DEFAULT);
        CHECK(table_info->field_info[6].field_name == "sign_up");
        CHECK(table_info->field_info[7].constraint[0] == Constraint_Type::CONS_PRIMARY_KEY);
        CHECK(table_info->field_info[8].constraint[0] == Constraint_Type::CONS_CHECK);

        // auto field_info = table_info->field_info[0];
        // CHECK(field_info.field_name == "cust_id");
        // CHECK(field_info.type == Data_Type::DATA_TYPE_INT);
        // CHECK(field_info.length == sizeof(int));
        // field_info = table_info->field_info[1];
        // CHECK(field_info.field_name == "cust_name");
        // CHECK(field_info.type == Data_Type::DATA_TYPE_VARCHAR);
        // CHECK(field_info.length == 50);
        // field_info = table_info->field_info[2];
        // CHECK(field_info.field_name == "cust_address");
        // CHECK(field_info.type == Data_Type::DATA_TYPE_VARCHAR);
        // CHECK(field_info.length == 50);
    }

    // SECTION("insert parser")
    // {
    //     std::string statement = "insert into customers \
    //                              values \
    //                              ( 1, \
    //                                'Jack', \
    //                                'Shanghai' \
    //                              );";
    //     auto tokenizer = std::make_shared<Tokenizer>(statement);
    //     InsertParser insert_parser(tokenizer);
    //     auto sql_stmt_insert = insert_parser.ParseSQLStmtInsert();
    //     REQUIRE(sql_stmt_insert != nullptr);

    //     // # SQL_Stmt_Type
    //     CHECK(sql_stmt_insert->type_ == SQL_Stmt_Type::SQL_INSERT);
    //     // # Table name
    //     CHECK(sql_stmt_insert->table_name_ == "customers");
    //     // # field name
    //     CHECK(sql_stmt_insert->fields_name_.empty());
    //     // # values
    //     DataValue value(Data_Type::DATA_TYPE_INT);
    //     value.SetIntValue(1);
    //     CHECK(sql_stmt_insert->values_[0].GetDataType() == value.GetDataType());
    //     CHECK(sql_stmt_insert->values_[0].int_value() == value.int_value());
    //     value.SetCharValue("Jack");
    //     CHECK(sql_stmt_insert->values_[1].GetDataType() == value.GetDataType());
    //     CHECK(sql_stmt_insert->values_[1].char_value() == value.char_value());
    //     value.SetCharValue("Shanghai");
    //     CHECK(sql_stmt_insert->values_[2].GetDataType() == value.GetDataType());
    //     CHECK(sql_stmt_insert->values_[2].char_value() == value.char_value());

    //     statement = "insert into customers \
    //                  ( cust_id, \
    //                    cust_name, \
    //                    cust_address \
    //                  ) \
    //                  values \
    //                  ( 1, \
    //                    \"Jack\", \
    //                    \"Shanghai\" \
    //                  );";
    //     tokenizer = std::make_shared<Tokenizer>(statement);
    //     insert_parser = InsertParser(tokenizer);
    //     sql_stmt_insert = insert_parser.ParseSQLStmtInsert();
    //     REQUIRE(sql_stmt_insert != nullptr);

    //     // # SQL_Stmt_Type
    //     CHECK(sql_stmt_insert->type_ == SQL_Stmt_Type::SQL_INSERT);
    //     // # Table name
    //     CHECK(sql_stmt_insert->table_name_ == "customers");
    //     // # field name
    //     CHECK(sql_stmt_insert->fields_name_[0] == "cust_id");
    //     CHECK(sql_stmt_insert->fields_name_[1] == "cust_name");
    //     CHECK(sql_stmt_insert->fields_name_[2] == "cust_address");
    //     // # values
    //     value.SetIntValue(1);
    //     CHECK(sql_stmt_insert->values_[0].GetDataType() == value.GetDataType());
    //     CHECK(sql_stmt_insert->values_[0].int_value() == value.int_value());
    //     value.SetCharValue("Jack");
    //     CHECK(sql_stmt_insert->values_[1].GetDataType() == value.GetDataType());
    //     CHECK(sql_stmt_insert->values_[1].char_value() == value.char_value());
    //     value.SetCharValue("Shanghai");
    //     CHECK(sql_stmt_insert->values_[2].GetDataType() == value.GetDataType());
    //     CHECK(sql_stmt_insert->values_[2].char_value() == value.char_value());
    // }

    SECTION("select parser")
    {
        std::string statement = "select cust_id, cust_address \
                                 from customers;";
        auto tokenizer = std::make_shared<Tokenizer>(statement);
        SelectParser select_parser(tokenizer);
        auto sra = select_parser.ParseSQLStmtSelect();
        REQUIRE(sra != nullptr);
        REQUIRE(sra->type_ == SRA_Type::SRA_PROJECT);
    }
}