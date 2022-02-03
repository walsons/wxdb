#ifndef B_TREE_H_
#define B_TREE_H_

#include "database.h"

enum class NODE_TYPE : char
{
    INTERNAL_NODE,
    LEAF_NODE
};

// The common header of internal node and leaf node
class NodeHeader
{
public:
    NodeHeader(NODE_TYPE node_type, bool is_root = false, void *parent_pointer = nullptr)
        : node_type_(node_type), is_root_(is_root), parent_pointer_(parent_pointer) { } 
    NODE_TYPE node_type_;
    bool is_root_;
    /* Waste 2 bytes */
    void *parent_pointer_;
};

// Cell is key/value pair
struct Cell
{
    unsigned key;
    // In this project, the value is Row
    Row value;
};

const int CELL_CAPACITY = (PAGE_SIZE - sizeof(NodeHeader) - sizeof(unsigned)) / sizeof(Cell);
class LeafNode : public NodeHeader
{
public:
    LeafNode(unsigned num_cells) 
        : NodeHeader(NODE_TYPE::LEAF_NODE), num_cells_(num_cells) {}
    unsigned num_cells_;
    Cell cell_[CELL_CAPACITY];
};

#endif