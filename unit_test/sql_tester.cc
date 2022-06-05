#include "catch.hpp"

#include <iostream>
#include <memory>
#include <vector>
#include <string>

#include "../include/sql/token.h"
#include "../include/sql/tokenizer.h"
#include "../include/sql/table_info.h"
#include "../include/sql/table_parser.h"
#include "../include/sql/select_parser.h"
#include "../include/db/table_header.h"

TEST_CASE("TC-Parser", "[parser test]")
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
        // TODO: validate expr node
        CHECK(cons.expr != nullptr);
        auto test_term1 = std::unordered_map<std::string, std::shared_ptr<TermExpr>>{{"age", std::make_shared<TermExpr>(18)}};
        auto test_term2 = std::unordered_map<std::string, std::shared_ptr<TermExpr>>{{"age", std::make_shared<TermExpr>(60)}};
        auto test_term3 = std::unordered_map<std::string, std::shared_ptr<TermExpr>>{{"age", std::make_shared<TermExpr>(12)}};
        expression.Eval(cons.expr, test_term1);
        CHECK(expression.term_.bval_ == true);
        expression.Eval(cons.expr, test_term2);
        CHECK(expression.term_.bval_ == true);
        expression.Eval(cons.expr, test_term3);
        CHECK(expression.term_.bval_ == false);
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

    SECTION("insert parser")
    {
        std::string statement = "INSERT INTO users (id, name, email, age, height, country, sign_up) \
                                 VALUES (1, \"Walson\", \"walsons@163.com\", 18, 180, \"China\", \"2020-01-03\");";
        auto tokenizer = std::make_shared<Tokenizer>(statement);
        TableParser table_parser(tokenizer);
        auto insert_info = table_parser.InsertTable();
        REQUIRE(insert_info != nullptr);

        // table name
        CHECK(insert_info->table_name == "users");
        // field name map
        auto &m = insert_info->field_name;
        CHECK(m.find("id") != m.end());
        CHECK(m.find("name") != m.end());
        CHECK(m.find("email") != m.end());
        CHECK(m.find("age") != m.end());
        CHECK(m.find("height") != m.end());
        CHECK(m.find("country") != m.end());
        CHECK(m.find("sign_up") != m.end());
        CHECK(m["id"] == 0);
        CHECK(m["name"] == 1);
        CHECK(m["email"] == 2);
        CHECK(m["age"] == 3);
        CHECK(m["height"] == 4);
        CHECK(m["country"] == 5);
        CHECK(m["sign_up"] == 6);
        // Column value
        auto &vals = insert_info->col_val;
        CHECK(vals[0].type_ == Col_Type::COL_TYPE_INT);
        CHECK(vals[0].ival_ == ColVal(1).ival_);
        // Parser can't distinguish char and varchar only via literal, default is varchar
        CHECK(vals[1].type_ == Col_Type::COL_TYPE_VARCHAR);  
        CHECK(vals[1].sval_ == ColVal("Walson").sval_);
        CHECK(vals[2].type_ == Col_Type::COL_TYPE_VARCHAR);
        CHECK(vals[2].sval_ == ColVal("walsons@163.com").sval_);
        CHECK(vals[3].type_ == Col_Type::COL_TYPE_INT);
        CHECK(vals[3].ival_ == ColVal(18).ival_);
        CHECK(vals[4].type_ == Col_Type::COL_TYPE_INT);
        CHECK(vals[4].ival_ == ColVal(180).ival_);
        CHECK(vals[5].type_ == Col_Type::COL_TYPE_VARCHAR);
        CHECK(vals[5].sval_ == ColVal("China").sval_);
        // Parser also can't distinguish date and char and varchar, it will decide in the next steps.
        CHECK(vals[6].type_ == Col_Type::COL_TYPE_VARCHAR);
        CHECK(vals[6].sval_ == ColVal("2020-01-03").sval_);
    }
}