#include "catch.hpp"

#ifdef _WIN32
#include <direct.h>
#elif __linux__
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "../include/sql/database_parser.h"
#include "../include/db/dbms.h"
#include "../include/db/database_manager.h"
#include "../include/sql/table_parser.h"
#include "../include/page/general_page.h"
#include "../include/page/data_page.hpp"
#include "../include/page/index_leaf_page.hpp"
#include <fstream>
#include <memory>

TEST_CASE( "TC-DATABASE", "[database test]" ) 
{
    // below 3 section need to run respectively
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
            auto test_term1 = std::unordered_map<std::string, std::shared_ptr<TermExpr>>{{"age", std::make_shared<TermExpr>(18)}};
            auto test_term2 = std::unordered_map<std::string, std::shared_ptr<TermExpr>>{{"age", std::make_shared<TermExpr>(60)}};
            auto test_term3 = std::unordered_map<std::string, std::shared_ptr<TermExpr>>{{"age", std::make_shared<TermExpr>(12)}};
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
    }

    SECTION("insert into")
    {
        std::string statement = "INSERT INTO users (id, name, email, age, height, country, sign_up) \
                                 VALUES (1, \"Walson\", \"walsons@163.com\", 18, 180, \"China\", \"2020-01-03\");";
        auto tokenizer = std::make_shared<Tokenizer>(statement);
        TableParser table_parser(tokenizer);
        auto insert_info = table_parser.InsertTable();
        REQUIRE(insert_info != nullptr);
        DBMS::GetInstance().UseDatabase("mydb");
        DBMS::GetInstance().InsertRow(insert_info);
        DBMS::GetInstance().CloseDatabase();

        std::ifstream ifs(DB_DIR + "users.tdata");
        REQUIRE(ifs.is_open());
        
        ifs.seekg(1 * PAGE_SIZE, std::ios::beg);
        char page[PAGE_SIZE];              
        ifs.read(page, PAGE_SIZE);
        if (ifs.eof()) { ifs.clear(); }
        DataPage<int> data_page{page, nullptr};
        CHECK(data_page.size() == 1);
        auto header_and_buf = data_page.GetBlock(0);
        int record_length = sizeof(VariantPage::BlockHeader) + 8 + sizeof(int) + 32 + 255 + sizeof(int) + sizeof(double) + 32 + sizeof(Date);
        CHECK(header_and_buf.first.size == record_length);
        CHECK(header_and_buf.first.overflow_page == 0);
        REQUIRE(data_page.slots(0) == PAGE_SIZE - record_length);

        char *buf = header_and_buf.second;
        int pos = 0;
        // vaildate each column
        CHECK(*reinterpret_cast<int *>(buf + pos) == 1);  // __rowid__
        pos += sizeof(int);
        CHECK(*reinterpret_cast<int *>(buf + pos) == 0);  // null_mark
        pos += sizeof(int);
        CHECK(*reinterpret_cast<int *>(buf + pos) == 1);  // id
        pos += sizeof(int);
        CHECK(std::string(buf + pos) == "Walson");  // name
        pos += 32;
        CHECK(std::string(buf + pos) == "walsons@163.com");  // email
        pos += 255;
        CHECK(*reinterpret_cast<int *>(buf + pos) == 18);  // age
        pos += sizeof(int);
        CHECK(*reinterpret_cast<double *>(buf + pos) == 180);  // height
        pos += sizeof(double);
        CHECK(std::string(buf + pos) == "China");  // country
        pos += 32;
        Date date{*reinterpret_cast<time_t *>(buf + pos)};
        CHECK(date.timestamp2str() == "2020-01-03");  // sign_up

        // index id
        {
            ifs.seekg(2 * PAGE_SIZE, std::ios::beg);
            ifs.read(page, PAGE_SIZE);
            if (ifs.eof()) { ifs.clear(); }
            IndexLeafPage<const char*> index_leaf_page{page, nullptr};
            CHECK(index_leaf_page.size() == 1);
            const char *index_buf = index_leaf_page.get_key(0);
            int record_length = index_leaf_page.field_size();
            CHECK(record_length == sizeof(int) + 1 + sizeof(int));
            CHECK(*reinterpret_cast<const int *>(index_buf + 0) == 1);
            CHECK(*(index_buf + 4) == 0);
            CHECK(*reinterpret_cast<const int *>(index_buf + 5) == 1);
        }
        // index email
        {
            ifs.seekg(3 * PAGE_SIZE, std::ios::beg);
            ifs.read(page, PAGE_SIZE);
            if (ifs.eof()) { ifs.clear(); }
            IndexLeafPage<const char*> index_leaf_page{page, nullptr};
            CHECK(index_leaf_page.size() == 1);
            const char *index_buf = index_leaf_page.get_key(0);
            int record_length = index_leaf_page.field_size();
            CHECK(record_length == sizeof(int) + 1 + 255);
            CHECK(*reinterpret_cast<const int *>(index_buf + 0) == 1);
            CHECK(*(index_buf + 4) == 0);
            CHECK(std::string(index_buf + 5) == "walsons@163.com");
        }
    }

    // Delete the .db directory before run this section
    SECTION("insert into multiple row")
    {
        // create database
        {
            std::string statement = "create database mydb";
            auto t = std::make_shared<Tokenizer>(statement);
            auto parser = std::make_shared<DatabaseParser>(t);
            auto database_info = parser->CreateDatabase();
            DBMS::GetInstance().CreateDatabase(database_info->database_name);
        }

        // create table
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
        }

        DBMS::GetInstance().UseDatabase("mydb");
        std::vector<int> ids{3,6,7,13,1,4,21,11,5,89,33,2,20,30,66,34,14,58,61,46};  // 20 numbers
        // In this case, 4 layers btree need 7126 numbers, 5 layers need 3121206 numbers,
        // test case is insert 100000 rows, which take up disk about 128MB
        for (int i = 100; i < 100000; ++i) { ids.push_back(i); }  
        for (auto i : ids)
        {
            std::string statement = "INSERT INTO users (id, name, email, age, height, country, sign_up) \
                                     VALUES (" + std::to_string(i) + ", \"Walson\", \"walsons@163.com\", 18, 180, \"China\", \"2020-01-03\");";
            auto tokenizer = std::make_shared<Tokenizer>(statement);
            TableParser table_parser(tokenizer);
            auto insert_info = table_parser.InsertTable();
            REQUIRE(insert_info != nullptr);
            DBMS::GetInstance().InsertRow(insert_info);
        }
        DBMS::GetInstance().CloseDatabase();
        
        std::string header_path = DB_DIR + "users.thead";
        std::string data_path = DB_DIR + "users.tdata";
        std::ifstream ifs(header_path, std::ios::binary);
        assert(ifs.good());
        TableHeader header;
        ifs.read(reinterpret_cast<char *>(&header), sizeof(header));
        auto pg = std::make_shared<Pager>(data_path);

        // Validate main data
        auto btr = std::make_shared<IntBTree>(pg, header.index_root_page[header.main_index]);
        int root_page_id = btr->root_page_id();
        char *buf = pg->Read(root_page_id);
        Page_Type page_type = *reinterpret_cast<Page_Type *>(buf);
        while (page_type == Page_Type::FIXED_PAGE)
        {
            IntBTree::interior_page page{buf, pg};
            int first_child_id =  page.children(0);
            buf = pg->Read(first_child_id);
            page_type = *reinterpret_cast<Page_Type *>(buf);
        }
        IntBTree::leaf_page data_page{buf, pg};
        int cnt = 0;
        char *pos = nullptr;
        for (int i = 0; i < ids.size(); ++i)
        {
            if (cnt == data_page.size())
            {
                data_page = IntBTree::leaf_page{pg->Read(data_page.next_page()), pg};
                cnt = 0;
            }
            pos = data_page.GetBlock(cnt++).second;
            CHECK(*reinterpret_cast<int *>(pos + 8) == ids[i]);
        }
        
        // Validate index id
        {
            auto btr = std::make_shared<IndexBTree>(pg, header.index_root_page[0], sizeof(int) + 5, 
                                                    IndexManager::GetIndexComparer(Col_Type::COL_TYPE_INT));
            int root_page_id = btr->root_page_id();
            char *buf = pg->Read(root_page_id);
            Page_Type page_type = *reinterpret_cast<Page_Type *>(buf);
            while (page_type == Page_Type::FIXED_PAGE)
            {
                IndexBTree::interior_page page{buf, pg};
                int first_child_id =  page.children(0);
                buf = pg->Read(first_child_id);
                page_type = *reinterpret_cast<Page_Type *>(buf);
            }
            IndexLeafPage<const char*> index_page{buf, pg};
            int cnt = 0;
            char *pos = nullptr;
            std::sort(ids.begin(), ids.end());
            for (int i = 0; i < ids.size(); ++i)
            {
                if (cnt == index_page.size())
                {
                    index_page = IndexLeafPage<const char*>{pg->Read(index_page.next_page()), pg};
                    cnt = 0;
                }
                pos = index_page.begin() + index_page.field_size() * (cnt++);
                CHECK(*reinterpret_cast<int *>(pos + 5) == ids[i]);
            }
        }

        // Validate index email
        {
            auto btr = std::make_shared<IndexBTree>(pg, header.index_root_page[2], 255 + 5,
                                                    IndexManager::GetIndexComparer(Col_Type::COL_TYPE_VARCHAR));
            int root_page_id = btr->root_page_id();
            char *buf = pg->Read(root_page_id);
            Page_Type page_type = *reinterpret_cast<Page_Type *>(buf);
            while (page_type == Page_Type::FIXED_PAGE)
            {
                IndexBTree::interior_page page{buf, pg};
                int first_child_id =  page.children(0);
                buf = pg->Read(first_child_id);
                page_type = *reinterpret_cast<Page_Type *>(buf);
            }
            IndexLeafPage<const char*> index_page{buf, pg};
            int cnt = 0;
            char *pos = nullptr;
            // std::sort(ids.begin(), ids.end());
            for (int i = 0; i < ids.size(); ++i)
            {
                if (cnt == index_page.size())
                {
                    index_page = IndexLeafPage<const char*>{pg->Read(index_page.next_page()), pg};
                    cnt = 0;
                }
                pos = index_page.begin() + index_page.field_size() * (cnt++);
                CHECK(std::string(pos + 5) == "walsons@163.com");
            }
        }
    }

    SECTION("select")
    {
        // create database
        {
            std::string statement = "create database mydb";
            auto t = std::make_shared<Tokenizer>(statement);
            auto parser = std::make_shared<DatabaseParser>(t);
            auto database_info = parser->CreateDatabase();
            DBMS::GetInstance().CreateDatabase(database_info->database_name);
        }
        // create table
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
        }
        // insert table
        {
            DBMS::GetInstance().UseDatabase("mydb");
            std::vector<std::string> statements;
            statements.emplace_back("INSERT INTO users (id, name, email, age, height, country, sign_up) \
                                     VALUES (1, \"Walson\", \"walsons@163.com\", 18, 180, \"China\", \"2020-01-03\");");
            statements.emplace_back("INSERT INTO users (id, name, email, age, height, country, sign_up) \
                                     VALUES (2, \"John\", \"John123@163.com\", 20, 175, \"China\", \"2020-03-12\");");
            std::vector<int> ids{3,6,7,13,4,21,11,5,89,33,20,30,66,34,14,58,61,46};  // 20 numbers
            for (auto i : ids)
            {
                statements.emplace_back("INSERT INTO users (id, name, email, age, height, country, sign_up) \
                                        VALUES (" + std::to_string(i) + ", \"Walson\", \"walsons@163.com\", 18, 180, \"China\", \"2020-01-03\");");
            }
            for (auto statement : statements)
            {
                auto tokenizer = std::make_shared<Tokenizer>(statement);
                TableParser table_parser(tokenizer);
                auto insert_info = table_parser.InsertTable();
                REQUIRE(insert_info != nullptr);
                DBMS::GetInstance().InsertRow(insert_info);
            }
        }
        // select
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
        }

        DBMS::GetInstance().CloseDatabase();
    }
}
