#ifndef DATABASE_H_
#define DATABASE_H_

#include "wxdb_define.h"

#include <vector>
#include <string>
#include <cstring>
#include <unordered_map>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <limits>

#include "row.h"
#include "pager.h"
#include "b_tree.h"

/***********
 *  Table  *
 ***********/
#define TABLE_NAME_MAX_LENGTH 25
class Table
{
public:
    // If root_page_num = std::numeric_limits<unsigned>::max(), which means not have any node.
    Table(const std::string &file_name = "wxdb.db")
        : table_name(""), root_page_num(std::numeric_limits<unsigned>::max()), pager(new Pager(file_name)) { }
    ~Table()
    {
        if (pager != nullptr)
        {
            delete pager;
            pager = nullptr;
        }
    }
    char table_name[TABLE_NAME_MAX_LENGTH + 1];
    unsigned root_page_num;
    Pager *pager;
};

/************
 *  Cursor  *
 ************/
struct Cursor
{
    Table *table;
    unsigned page_num;
    unsigned cell_num;
    bool end_of_table;
};

/************
 * Database *
 ************/
class Database {
public:
    Database() : table_(nullptr)
    { 
        std::ifstream in("table_attribute.db", std::ifstream::binary | std::ifstream::in);
        if (in)
        {
            table_ = new Table();
            in.read((char*)table_, TABLE_NAME_MAX_LENGTH + 1);
            in.close();
        }
    }
    ~Database() 
    {
        FlushToDisk();
        if (table_ != nullptr)
        {
            delete table_;
            table_ = nullptr;
        }
    }

    ExecuteResult CreateTable(char tableNameForCreate[], unsigned bufferSize)
    {
        if (table_ == nullptr)
        {
            table_ = new Table();
            std::memcpy(table_->table_name, tableNameForCreate, bufferSize);
            std::cout << "Create table " << "\"" << table_->table_name << "\" successfully" << std::endl;
        }
        else
        {
            std::cout << "You can only create one table" << std::endl;
        }
        return EXECUTE_SUCCESS;
    }

    ExecuteResult InsertRow(Row rowForInsert)
    {
        // If table doesn't have any page/node
        if (table_->root_page_num == std::numeric_limits<unsigned>::max())
        {
            Pager *pager = table_->pager;
            unsigned unused_page_num = 0;
            void *unused_page = pager->GetUnusedPage(unused_page_num);
            table_->root_page_num = unused_page_num;
            LeafNode *leaf_node = pager->PageToLeafNode(unused_page);
            leaf_node->InitForPage(true, unused_page_num, std::numeric_limits<unsigned>::max());
        }
        unsigned key = rowForInsert.id;
        Cursor cursor = InsertPosition(key);

        Pager *pager = table_->pager;
        void *page = pager->GetPage(cursor.page_num);
        LeafNode *leaf_node = pager->PageToLeafNode(page);

        // move later element
        if (leaf_node->num_cells_ > 0)
        {
            for (int i = leaf_node->num_cells_ - 1; i >= cursor.cell_num; --i)
            {
                leaf_node->cell_[i + 1] = leaf_node->cell_[i];
            }
        }
        // The function will increase the number of cells
        serialize_row(&rowForInsert, cursor);

        // Need to check whether to update parent key if have parent, 
        // if the insert position is end, need to update
        if (cursor.cell_num = leaf_node->num_cells_ - 1)
        {
            if (leaf_node->parent_page_num_ != std::numeric_limits<unsigned>::max())
            {
                // find the max key before insert which index is (leaf_node->num_cells_ - 2)
                unsigned index = pager->FindInternalNodeClue(leaf_node->parent_page_num_, leaf_node->cell_[leaf_node->num_cells_ - 2].key);
                // update key
                void *parent_page = pager->GetPage(leaf_node->parent_page_num_);           
                InternalNode *parent_node = pager->PageToInternalNode(parent_page);
                parent_node->clue_[index].key = leaf_node->cell_[leaf_node->num_cells_ - 1].key;
            }
        }
        // If page is full after insert, split the page
        if (leaf_node->num_cells_ >= CELL_CAPACITY)
        {
            unsigned unused_page_num = 0;
            void *unused_page = pager->GetUnusedPage(unused_page_num);
            LeafNode *new_leaf_node = pager->PageToLeafNode(unused_page);
            // Copy half of cell to the new node
            unsigned mid_index = (leaf_node->num_cells_ + 1) / 2;
            unsigned new_leaf_node_num_cells = leaf_node->num_cells_ - mid_index; 
            std::memcpy(&new_leaf_node->cell_[0], &leaf_node->cell_[mid_index], (new_leaf_node_num_cells) * sizeof(Cell));
            // Full leaf node have parent
            if (leaf_node->parent_page_num_ != std::numeric_limits<unsigned>::max())
            {
                // Modify both node attribute
                new_leaf_node->InitForPage(false, unused_page_num, leaf_node->parent_page_num_);
                new_leaf_node->num_cells_ = new_leaf_node_num_cells;
                leaf_node->num_cells_ -= new_leaf_node_num_cells;
                pager->InternalNodeInsert(leaf_node->parent_page_num_, leaf_node->page_num_, leaf_node->cell_[leaf_node->num_cells_ - 1].key);
            }
            // Full leaf node doesn't have parent





        }




        //     std::cout << "Table \"" << table_->table_name << "\" is full" << std::endl;
        //     return EXECUTE_TABLE_FULL;
        std::cout << "Insert record into table \"" << table_->table_name << "\" successfully" << std::endl;
        return EXECUTE_SUCCESS;
    }

    ExecuteResult Select(char tableNameForSelect[], unsigned bufferSize)
    {
        std::cout << "\tid\tuser name\temail\t" << std::endl;
        Row oneRow;
        Cursor cursor = SelectPosition();
        while (!cursor.end_of_table)
        {
            deserialize_row(&oneRow, cursor);
            std::cout << "\t" << oneRow.id << "\t" << oneRow.user_name << "\t" << oneRow.email << "\t" << std::endl;
            CursorAdvance(cursor);
        }
        return EXECUTE_SUCCESS;
    }

    // Create a cursor indicate table select position
    Cursor SelectPosition() const
    {
        Cursor cursor;
        cursor.table = table_;
        cursor.page_num = table_->root_page_num;
        cursor.cell_num = 0;
        cursor.end_of_table = false;
        if (table_->root_page_num == std::numeric_limits<unsigned>::max())
        {
            cursor.end_of_table = true;
        }
        return cursor;
    }
    // Create a cursor indicate table insert position
    Cursor InsertPosition(unsigned key)
    {
        // Currently, we assume insert will not excess node max size.
        Cursor cursor;
        cursor.table = table_;
        cursor.end_of_table = true;
        Pager *pager = table_->pager;
        void *page = pager->GetPage(table_->root_page_num);
        if (pager->GetPageType(page) == NODE_TYPE::LEAF_NODE)
        {
            LeafNode *root_node = pager->PageToLeafNode(page);
            unsigned insert_index = BinarySearchLargerKey(root_node->cell_, root_node->num_cells_, key);
            cursor.page_num = root_node->page_num_;
            cursor.cell_num = insert_index;
        }
        else
        {

        }
        return cursor;
    }

    // Get the position of the cursor
    void *CursorValue(const Cursor &cursor)
    {
        // Cursor must in the leaf node
        Pager *pager = table_->pager;
        void *page = pager->GetPage(cursor.page_num);
        LeafNode *leaf_node = pager->PageToLeafNode(page);
        return &(leaf_node->cell_[cursor.cell_num]);
        // unsigned pageNumber = cursor.row_num / kRowNumberPerPage;
        // if (pageNumber >= MAX_PAGE_NUMBER)
        // {
        //     std::cout << "Row number beyond max range" << std::endl;
        //     return nullptr;
        // }
        // unsigned rowNumberInPage = cursor.row_num % kRowNumberPerPage;
        // return cursor.table->pager->pages[pageNumber] + rowNumberInPage * sizeof(Row);
    }

    // Push the cursor advance one step
    void CursorAdvance(Cursor &cursor) const
    {
        Pager *pager = table_->pager;
        void *page = pager->GetPage(cursor.page_num);
        LeafNode *leaf_node = pager->PageToLeafNode(page);
        if (++cursor.cell_num == leaf_node->num_cells_)
        {
            cursor.end_of_table = true;
        }
    }


    Table *table() const
    {
        return table_;
    }

// member
private: 
    Table *table_;

// function
private: 
    // copy row buffer to destination.
    void serialize_row(Row *row, const Cursor &cursor)
    {
        void *destination = CursorValue(cursor);
        std::memcpy(destination + kIdOffset, &row->id, kIdSize);
        std::memcpy(destination + kUserNameOffset, &row->user_name, kUserNameSize);
        std::memcpy(destination + kEmailOffset, &row->email, kEmailSize);
        // num_cells_ addition 
        Pager *pager = table_->pager;
        void *page = pager->GetPage(cursor.page_num);
        LeafNode *leaf_node = pager->PageToLeafNode(page);
        ++leaf_node->num_cells_;
    }

    // copy destination buffer to row.
    void deserialize_row(Row *row, const Cursor &cursor)
    {
        void *destination = CursorValue(cursor);
        std::memcpy(&row->id, destination + kIdOffset, kIdSize);
        std::memcpy(&row->user_name, destination + kUserNameOffset, kUserNameSize);
        std::memcpy(&row->email, destination + kEmailOffset, kEmailSize);
    }



    // Flush buffer to disk
    void FlushToDisk()
    {
        if (table_ != nullptr)
        {
    //         size_t nums_of_pages = table_->row_number / kRowNumberPerPage;
    //         for (size_t i = 0; i < nums_of_pages; ++i)
    //         {
    //             if (table_->pager->pages[i] != nullptr)
    //             {
    //                 table_->pager->fd.seekp(i * PAGE_SIZE, std::ios::beg);
    //                 table_->pager->fd.write(table_->pager->pages[i], PAGE_SIZE);
    //             }
    //         }
    //         // last page not full
    //         size_t rest_row_num = table_->row_number - kRowNumberPerPage * nums_of_pages;
    //         if (rest_row_num > 0)
    //         {
    //             table_->pager->fd.seekp(nums_of_pages * PAGE_SIZE, std::ios::beg);
    //             table_->pager->fd.write(table_->pager->pages[nums_of_pages], rest_row_num * sizeof(Row));
    //         }
            // Write table name and row number to file
            std::ofstream out("table_attribute.db", std::ofstream::out | std::ofstream::binary);
            out.write((char*)table_, sizeof(Table));
            out.close();
        }
    }
};

#endif
