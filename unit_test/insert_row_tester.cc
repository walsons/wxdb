#include "common_header.h"

TEST_CASE( "TC-InsertRow", "[insert row test]" ) 
{
    SECTION("insert row")
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
        // Create table
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
            auto table_parser = std::make_shared<TableParser>(t);
            auto table_info = table_parser->CreateTable();
            auto table_header = std::make_shared<TableHeader>();
            fill_table_header(table_header, *table_info);               
            DBMS::GetInstance().CreateTable(table_header);
        }
        
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
}
