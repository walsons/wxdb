#include "common_header.h"

TEST_CASE( "TC-CreateTable", "[create table test]" ) 
{
    SECTION("create table example 1")
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
            using column2term_t = std::unordered_map<std::string, std::shared_ptr<TermExpr>>;
            auto test_term1 = column2term_t{{"age", std::make_shared<TermExpr>(18)}};
            auto test_term2 = column2term_t{{"age", std::make_shared<TermExpr>(60)}};
            auto test_term3 = column2term_t{{"age", std::make_shared<TermExpr>(12)}};
            Expression expression(expr_node, test_term1);
            CHECK(expression.term_.bval_ == true);
            expression.Eval(expr_node, test_term2);
            CHECK(expression.term_.bval_ == true);
            expression.Eval(expr_node, test_term3);
            CHECK(expression.term_.bval_ == false);
        }

        std::ifstream ifs(DB_DIR + "users.tdata");
        Page_Type page_type;
        REQUIRE(ifs.is_open());
        
        ifs.seekg(1 * PAGE_SIZE, std::ios::beg);
        ifs.read(reinterpret_cast<char*>(&page_type), sizeof(Page_Type));
        if (ifs.eof()) { ifs.clear(); }
        CHECK(page_type == Page_Type::VARIANT_PAGE);

        ifs.seekg(2 * PAGE_SIZE, std::ios::beg);
        ifs.read(reinterpret_cast<char*>(&page_type), sizeof(Page_Type));
        if (ifs.eof()) { ifs.clear(); }
        CHECK(page_type == Page_Type::INDEX_LEAF_PAGE);

        ifs.seekg(3 * PAGE_SIZE, std::ios::beg);
        ifs.read(reinterpret_cast<char*>(&page_type), sizeof(Page_Type));
        if (ifs.eof()) { ifs.clear(); }
        CHECK(page_type == Page_Type::INDEX_LEAF_PAGE);

        // Validate database file change
        {
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
            CHECK(info.num_table == 1);
        }
    }

    SECTION("create table example 2")
    {
        // namespace fs = std::filesystem;
        // // Create database
        // {
        //     fs::remove_all(DB_DIR.c_str());
        //     std::string statement = "create database mydb";
        //     auto t = std::make_shared<Tokenizer>(statement);
        //     auto database_parser = std::make_shared<DatabaseParser>(t);
        //     auto database_info = database_parser->CreateDatabase();
        //     DBMS::GetInstance().CreateDatabase(database_info->database_name);
        // }

        // DBMS::GetInstance().UseDatabase("mydb");
        // std::string statement = "CREATE TABLE comments (                           \
        //                              id          INT,                              \
        //                              user_id     INT        NOT NULL,              \
        //                              time        DATE       NOT NULL,              \
        //                              contents    VARCHAR(255),                     \
        //                              PRIMARY KEY (id),                             \
        //                              FOREIGN KEY (user_id) REFERENCES users (id)   \
        //                          );";
        // auto t = std::make_shared<Tokenizer>(statement);
        // auto table_parser = std::make_shared<TableParser>(t);
        // auto table_info = table_parser->CreateTable();
        // REQUIRE(table_info != nullptr);
        // auto table_header = std::make_shared<TableHeader>();
        // fill_table_header(table_header, *table_info);               
        // DBMS::GetInstance().CreateTable(table_header);
        // DBMS::GetInstance().CloseDatabase();
        // TODO: validate
    }
}