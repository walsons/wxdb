#ifndef B_TREE_H_
#define B_TREE_H_

#include <utility>

#include "wxdb_define.h"
#include "row.h"

enum class NODE_TYPE : char
{
    INTERNAL_NODE,
    LEAF_NODE
};

/***********************
 *  member         bytes
 * ---------------------
 * node_type_        1 
 * is_root_          1 
 * page_num_         4
 * parent_page_      4
 ***********************/

// The common header of internal node and leaf node
class NodeHeader
{
public: 
    NODE_TYPE node_type_;
    bool is_root_;
    unsigned page_num_;
    unsigned parent_page_num_;
};

// Clue is key/pointer pair
struct Clue
{
    unsigned key;
    unsigned child;
};
const unsigned CLUE_CAPACITY = (PAGE_SIZE - sizeof(NodeHeader) - sizeof(unsigned)) / sizeof(Clue);
class InternalNode : public NodeHeader
{
public:
    void SetNode(bool is_root, unsigned page_num, unsigned parent_page_num, unsigned num_clues)
    {
        node_type_ = NODE_TYPE::INTERNAL_NODE;
        is_root_ = is_root;
        page_num_ = page_num;
        parent_page_num_ = parent_page_num;
        num_clues_ = num_clues;
    }
    unsigned num_clues_;
    Clue clue_[CLUE_CAPACITY]; 
};

// Cell is key/value pair
struct Cell
{
    unsigned key;
    Row value;
};
const unsigned CELL_CAPACITY = (PAGE_SIZE - sizeof(NodeHeader) - sizeof(unsigned)) / sizeof(Cell);
class LeafNode : public NodeHeader
{
public:
    void SetNode(unsigned page_num, unsigned parent_page_num, unsigned num_cells)
    {
        node_type_ = NODE_TYPE::LEAF_NODE;
        is_root_ = false;
        page_num_ = page_num;
        parent_page_num_ = parent_page_num;
        num_cells_ = num_cells;
    }
    unsigned num_cells_;
    Cell cell_[CELL_CAPACITY];
};

/* 
 * Binary Search 
 * Condition: each key is not equal
 */
/* TODO: The key is in the array, find the index of the key */

/* The key is not in the array, find the index of the first key that larger than given key */
template <typename T>
unsigned BinarySearchLargerKey(T structure_with_key[], int n, unsigned key)
{
    structure_with_key[n].key = std::numeric_limits<unsigned>::max();
    int beg = 0, end = n;
    while (beg < end)
    {
        int mid = beg + (end - beg) / 2;
        if (structure_with_key[mid].key <= key)
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