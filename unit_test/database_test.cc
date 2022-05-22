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

        TableManager tm;
        tm.OpenTable("users");
        // table header
        auto header = std::make_shared<TableHeader>(tm.table_header());
        CHECK(std::string(header->table_name) == "users");
        REQUIRE(header->num_column == 8);  // extra column is __rowid__
        std::vector<std::string> col_names{"id", "name", "email", "age", "height", "country", "sign_up", "__rowid__"};
        std::vector<Col_Type> col_types{Col_Type::COL_TYPE_INT, Col_Type::COL_TYPE_CHAR, 
            Col_Type::COL_TYPE_VARCHAR, Col_Type::COL_TYPE_INT, Col_Type::COL_TYPE_DOUBLE,
            Col_Type::COL_TYPE_CHAR, Col_Type::COL_TYPE_DATE, Col_Type::COL_TYPE_INT};
        std::vector<unsigned> col_lens{sizeof(int), 32, 255, sizeof(int), sizeof(double), 32, sizeof(Date), sizeof(int)};
        std::vector<unsigned> col_offsets{8, 12, 44, 299, 303, 311, 343, 0};
        for (int i = 0; i < header->num_column; ++i)
        {
            CHECK(std::string(header->column_name[i]) == col_names[i]);
            CHECK(header->column_type[i] == col_types[i]);
            CHECK(header->column_length[i] == col_lens[i]);
            CHECK(header->column_offset[i] == col_offsets[i]);
        }
        CHECK(header->auto_inc == 1);
        CHECK(header->main_index == 7);
        CHECK(header->is_main_index_auto_inc == true);
        CHECK(header->num_record == 0);
        // flag
        CHECK(header->flag_not_null == 0b10000011);
        CHECK(header->flag_unique == 0b10000101);
        CHECK(header->flag_primary == 0b1);
        CHECK(header->flag_foreign == 0);
        CHECK(header->flag_default == 0b100000);
        CHECK(header->flag_index == 0b10000101);  // __rowid__, email, id
        // default value
        {
            std::istringstream is(header->default_value[5]);
            ExprNode *expr_node = Expression::LoadExprNode(is);
            Expression expression(expr_node);
            CHECK(expression.term_.sval_ == "China");
        }
        // check constraint
        {
            CHECK(header->num_check_constraint == 1);
            std::istringstream is(header->check_constraint[0]);
            ExprNode *expr_node = Expression::LoadExprNode(is);
            // TODO: validate expr node
            CHECK(expr_node != nullptr);
            auto test_term1 = std::make_shared<TermExpr>(18);
            auto test_term2 = std::make_shared<TermExpr>(60);
            auto test_term3 = std::make_shared<TermExpr>(12);
            Expression expression(expr_node, test_term1);
            CHECK(expression.term_.bval_ == true);
            expression.Eval(expr_node, test_term2);
            CHECK(expression.term_.bval_ == true);
            expression.Eval(expr_node, test_term3);
            CHECK(expression.term_.bval_ == false);
        }
    }
}