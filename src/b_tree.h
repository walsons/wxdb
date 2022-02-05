#ifndef B_TREE_H_
#define B_TREE_H_

#include "wxdb_define.h"
#include "row.h"

enum class NODE_TYPE : char
{
    INTERNAL_NODE,
    LEAF_NODE
};

// The common header of internal node and leaf node
class NodeHeader
{
public:
    NodeHeader(NODE_TYPE node_type, bool is_root = false, short page_num = 0, unsigned parent_page = 0)
        : node_type_(node_type), is_root_(is_root), page_num_(page_num), parent_page_num_(parent_page) {} 
    NODE_TYPE node_type_;
    bool is_root_;
    short page_num_;
    unsigned parent_page_num_;
};

// Cell is key/value pair
struct Cell
{
    unsigned key;
    // In this project, the value is Row
    Row value;
};

/********************
 member         bytes
---------------------
node_type_        1 
is_root_          1 
page_num_         2
parent_page_      4
num_cells_        4
cell_[]         rest
 ********************/
const unsigned CELL_CAPACITY = (PAGE_SIZE - sizeof(NodeHeader) - sizeof(unsigned)) / sizeof(Cell);
class LeafNode : public NodeHeader
{
public:
    LeafNode(unsigned num_cells = 0) 
        : NodeHeader(NODE_TYPE::LEAF_NODE), num_cells_(num_cells) {}
    void InitForPage(bool is_root, unsigned page_num, unsigned parent_page_num)
    {
        this->node_type_ = NODE_TYPE::LEAF_NODE;
        this->is_root_ = is_root;
        this->page_num_ = page_num;
        this->parent_page_num_ = parent_page_num;
        this->num_cells_ = 0;
    }
    unsigned num_cells_;
    Cell cell_[CELL_CAPACITY];
};

struct Clue
{
    unsigned key;
    unsigned child;
};
const unsigned CLUE_CAPACITY = (PAGE_SIZE - sizeof(NodeHeader) - sizeof(unsigned)) / sizeof(Clue);
class InternalNode : public NodeHeader
{
public:
    InternalNode(unsigned num_keys = 0)
        : NodeHeader(NODE_TYPE::INTERNAL_NODE), num_keys_(num_keys) {} 
    unsigned num_keys_;
    Clue clue_[CLUE_CAPACITY]; 
};


/* 
 * Binary Search 
 * Condition: each key is not equal
 */

// The key is in the array, find the index of the key

// The key is not in the array, find the index of the first key that larger than given key
unsigned BinarySearchLargerKey(Cell cell[], int n, unsigned key)
{
    cell[n].key = std::numeric_limits<unsigned>::max();
    int beg = 0, end = n;
    while (beg < end)
    {
        int mid = beg + (end - beg) / 2;
        if (cell[mid].key <= key)
        {
            beg = mid + 1;
        }
        else
        {
            end = mid;
        }
    }
    return end;
}


#endif