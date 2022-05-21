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

TEST_CASE("TC-PARSER", "[parser test]")
{
    SECTION("create parser example 1")
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
        auto tokenizer = std::make_shared<Tokenizer>(statement);
        TableParser table_parser(tokenizer);
        auto table_info = table_parser.CreateTable();
        REQUIRE(table_info != nullptr);

        // TableInfo
        CHECK(table_info->table_name == "users");
        CHECK(table_info->field_info.size() == 7);
        CHECK(table_info->constraint_info.size() == 3);
        
        // FieldInfo
        auto field = table_info->field_info[0];
        CHECK(field.field_name == "id");
        CHECK(field.type == Col_Type::COL_TYPE_INT);
        CHECK(field.length == sizeof(int));
        CHECK(field.is_not_null == false);  
        CHECK(field.has_default == false);

        field = table_info->field_info[1];
        CHECK(field.field_name == "name");
        CHECK(field.type == Col_Type::COL_TYPE_CHAR);
        CHECK(field.length == 32);
        CHECK(field.is_not_null == true);  
        CHECK(field.has_default == false);

        field = table_info->field_info[2];
        CHECK(field.field_name == "email");
        CHECK(field.type == Col_Type::COL_TYPE_VARCHAR);
        CHECK(field.length == 255);
        CHECK(field.is_not_null == false);  
        CHECK(field.has_default == false);

        field = table_info->field_info[3];
        CHECK(field.field_name == "age");
        CHECK(field.type == Col_Type::COL_TYPE_INT);
        CHECK(field.length == sizeof(int));
        CHECK(field.is_not_null == false);  
        CHECK(field.has_default == false);

        field = table_info->field_info[4];
        CHECK(field.field_name == "height");
        CHECK(field.type == Col_Type::COL_TYPE_DOUBLE);
        CHECK(field.length == sizeof(double));
        CHECK(field.is_not_null == false);  
        CHECK(field.has_default == false);

        field = table_info->field_info[5];
        CHECK(field.field_name == "country");
        CHECK(field.type == Col_Type::COL_TYPE_CHAR);
        CHECK(field.length == 32);
        CHECK(field.is_not_null == false);  
        CHECK(field.has_default == true);
        CHECK(field.expr != nullptr);
        Expression expression(field.expr);
        CHECK(expression.term_.sval_ == "China");

        field = table_info->field_info[6];
        CHECK(field.field_name == "sign_up");
        CHECK(field.type == Col_Type::COL_TYPE_DATE);
        CHECK(field.length == sizeof(Date));
        CHECK(field.is_not_null == false);  
        CHECK(field.has_default == false);

        // Constraint info
        auto cons = table_info->constraint_info[0];
        CHECK(cons.type == Constraint_Type::CONS_UNIQUE);
        CHECK(cons.col_ref.column_name == "email");

        cons = table_info->constraint_info[1];
        CHECK(cons.type == Constraint_Type::CONS_PRIMARY_KEY);
        CHECK(cons.col_ref.column_name == "id");

        cons = table_info->constraint_info[2];
        CHECK(cons.type == Constraint_Type::CONS_CHECK);
        CHECK(cons.expr != nullptr);
    }

    SECTION("create parser example 2")
    {
        std::string statement = "CREATE TABLE comments (                           \
                                     id          INT,                              \
                                     user_id     INT        NOT NULL,              \
                                     time        DATE       NOT NULL,              \
                                     contents    VARCHAR(255),                     \
                                     PRIMARY KEY (id),                             \
                                     FOREIGN KEY (user_id) REFERENCES users (id)   \
                                 );";
        auto tokenizer = std::make_shared<Tokenizer>(statement);
        TableParser table_parser(tokenizer);
        auto table_info = table_parser.CreateTable();
        REQUIRE(table_info != nullptr);

        // TableInfo
        CHECK(table_info->table_name == "comments");
        CHECK(table_info->field_info.size() == 4);
        CHECK(table_info->constraint_info.size() == 2);
        
        // FieldInfo
        auto field = table_info->field_info[0];
        CHECK(field.field_name == "id");
        CHECK(field.type == Col_Type::COL_TYPE_INT);
        CHECK(field.length == sizeof(int));
        CHECK(field.is_not_null == false);  
        CHECK(field.has_default == false);

        field = table_info->field_info[1];
        CHECK(field.field_name == "user_id");
        CHECK(field.type == Col_Type::COL_TYPE_INT);
        CHECK(field.length == sizeof(int));
        CHECK(field.is_not_null == true);  
        CHECK(field.has_default == false);

        field = table_info->field_info[2];
        CHECK(field.field_name == "time");
        CHECK(field.type == Col_Type::COL_TYPE_DATE);
        CHECK(field.length == sizeof(Date));
        CHECK(field.is_not_null == true);  
        CHECK(field.has_default == false);

        field = table_info->field_info[3];
        CHECK(field.field_name == "contents");
        CHECK(field.type == Col_Type::COL_TYPE_VARCHAR);
        CHECK(field.length == 255);
        CHECK(field.is_not_null == false);  
        CHECK(field.has_default == false);

        // Constraint info
        auto cons = table_info->constraint_info[0];
        CHECK(cons.type == Constraint_Type::CONS_PRIMARY_KEY);
        CHECK(cons.col_ref.column_name == "id");

        cons = table_info->constraint_info[1];
        CHECK(cons.type == Constraint_Type::CONS_FOREIGN_KEY);
        CHECK(cons.fk_ref.table_name == "users");
        CHECK(cons.fk_ref.column_name == "id");
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