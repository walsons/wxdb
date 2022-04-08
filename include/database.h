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
            in.read((char*)table_, sizeof(Table) - sizeof(Pager *));
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
            // When fisrt insert, create an internal node and a leaf node, root node must be an internal node
            unsigned unused_page_num = table_->pager->GetUnusedPageNum();
            InternalNode *root_node = table_->pager->GetInternalNode(unused_page_num);
            root_node->SetNode(true, unused_page_num, std::numeric_limits<unsigned>::max(), 0);
            table_->root_page_num = unused_page_num;
            // Get leaf node
            unused_page_num = table_->pager->GetUnusedPageNum();
            LeafNode *leaf_node = table_->pager->GetLeafNode(unused_page_num);
            leaf_node->cell_[0].key = rowForInsert.id;
            leaf_node->cell_[0].value = rowForInsert;
            leaf_node->SetNode(unused_page_num, root_node->page_num_, 1);
            // Bind child and set key to 0
            root_node->clue_[0].key = rowForInsert.id;
            root_node->clue_[0].child = leaf_node->page_num_;
            ++root_node->num_clues_;
            std::cout << "Insert record into table \"" << table_->table_name << "\" successfully" << std::endl;
            return EXECUTE_SUCCESS;
        }
        // If insert key larger than max key, need to update all most right internal node 
        LeafNode *leaf_node = nullptr;
        InternalNode *root_node = table_->pager->GetInternalNode(table_->root_page_num);
        if (rowForInsert.id > root_node->clue_[root_node->num_clues_ - 1].key)
        {
            unsigned node_page_num = table_->root_page_num;
            while (table_->pager->GetPageType(node_page_num) == NODE_TYPE::INTERNAL_NODE)
            {
                InternalNode *node = table_->pager->GetInternalNode(node_page_num);
                node->clue_[node->num_clues_ - 1].key = rowForInsert.id;
                node_page_num = node->clue_[node->num_clues_ - 1].child;
            }
            leaf_node = table_->pager->GetLeafNode(node_page_num);
            ++leaf_node->num_cells_;
            leaf_node->cell_[leaf_node->num_cells_ - 1].key = rowForInsert.id;
            leaf_node->cell_[leaf_node->num_cells_ - 1].value = rowForInsert;
        }
        // Other case
        else
        {
            unsigned key = rowForInsert.id;
            Cursor cursor = InsertPosition(key);
            leaf_node = table_->pager->GetLeafNode(cursor.page_num);
            // Move later element a step for insert
            for (unsigned i = leaf_node->num_cells_; i > cursor.cell_num; --i)
            {
                leaf_node->cell_[i] = leaf_node->cell_[i - 1];
            }
            // Assignment
            leaf_node->cell_[cursor.cell_num].key = rowForInsert.id;
            leaf_node->cell_[cursor.cell_num].value = rowForInsert;
            ++leaf_node->num_cells_;
        }
        // If page is full after insert, split the page
        if (leaf_node->num_cells_ >= CELL_CAPACITY)
        {
            unsigned unused_page_num = table_->pager->GetUnusedPageNum();
            LeafNode *new_leaf_node = table_->pager->GetLeafNode(unused_page_num);
            // Copy half of lower cell to the new node
            unsigned new_leaf_node_num_cells = (leaf_node->num_cells_ + 1) / 2;
            std::memcpy(&new_leaf_node->cell_[0], &leaf_node->cell_[0], (new_leaf_node_num_cells) * sizeof(Cell));
            new_leaf_node->SetNode(unused_page_num, leaf_node->parent_page_num_, new_leaf_node_num_cells);
            // Move the element of leaf node
            leaf_node->num_cells_ -= new_leaf_node_num_cells;
            for (int i = 0; i < leaf_node->num_cells_; ++i) 
            {
                leaf_node->cell_[i] = leaf_node->cell_[i + new_leaf_node_num_cells];
            }
            // Insert key to parent node and add correspondent pointer to child
            table_->pager->InternalNodeInsert(new_leaf_node->parent_page_num_, new_leaf_node->page_num_, new_leaf_node->cell_[new_leaf_node->num_cells_ - 1].key);
        }
        // TODO: exception check
        // std::cout << "Table \"" << table_->table_name << "\" is full" << std::endl;
        // return EXECUTE_TABLE_FULL;
        std::cout << "Insert record into table \"" << table_->table_name << "\" successfully" << std::endl;
        return EXECUTE_SUCCESS;
    }

    ExecuteResult Select(char tableNameForSelect[], unsigned bufferSize)
    {
        std::cout << "\tid\tuser name\temail\t" << std::endl;
        if (table_->root_page_num != std::numeric_limits<unsigned>::max())
        {
            ShowAllLeafNode(table_->root_page_num);
        }
        return EXECUTE_SUCCESS;
    }

    // Create a cursor indicate table insert position
    Cursor InsertPosition(const unsigned &key)
    {
        // Currently, we assume insert will not excess node max size.
        Cursor cursor;
        cursor.table = table_;
        InternalNode *root_node = table_->pager->GetInternalNode(table_->root_page_num);
        // Search on all B+tree.
        auto pos = table_->pager->SearchBPlusTree(root_node->page_num_, key);
        cursor.page_num = pos.first;
        cursor.cell_num = pos.second;
        return cursor;
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
    void ShowAllLeafNode(unsigned page_num)
    {
        InternalNode *node = table_->pager->GetInternalNode(page_num);
        for (unsigned i = 0; i < node->num_clues_; ++i)
        {
            unsigned child_page_num = node->clue_[i].child;
            if (table_->pager->GetPageType(child_page_num) == NODE_TYPE::INTERNAL_NODE)
            {
                ShowAllLeafNode(child_page_num);
            }
            else
            {
                LeafNode *leaf_node = table_->pager->GetLeafNode(child_page_num);
                for (unsigned j = 0; j < leaf_node->num_cells_; ++j)
                {
                    Row &oneRow = leaf_node->cell_[j].value;
                    std::cout << "\t" << oneRow.id << "\t" << oneRow.user_name << "\t" << oneRow.email << "\t" << std::endl;
                }
            }
        }
    }

    // Flush buffer to disk
    void FlushToDisk()
    {
        if (table_ != nullptr)
        {
            // Flush page to disk
            table_->pager->FlushPageToDisk();
            // Write table name and row number to file
            std::ofstream out("table_attribute.db", std::ofstream::out | std::ofstream::binary);
            out.write((char*)table_, sizeof(Table));
            out.close();
        }
    }
};

#endif
