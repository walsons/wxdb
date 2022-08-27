#ifndef VARIANT_PAGE_H_
#define VARIANT_PAGE_H_

#include "general_page.h"

// Page structure
/*
 * name          offset                                size          comment
 * page_type     0                                     2             page type
 * free_block    2                                     2             pointer to the first free block
 * free_size     4                                     2             total free size of this page(include free block)
 * size          6                                     2             number of items(slot and block)
 * bottom_used   8                                     2             size of space that blocks(include free block) used
 * next_page     10                                    4             next page id
 * prev_page     14                                    4             previous page id
 * slots[i]      18                                    2             i th slot
 * ---------------------------- unused space  --------------------------------
 * blocks[i]     PAGE_SIZE - field_size * (size - i)   field_size    i th block
 */

class VariantPage : public GeneralPage
{
    // The header of block that is freed
    struct FreeBlockHeader
    {
        uint16_t size;  // The size of free block(include header itself)
        uint16_t next;  // Next free block
    };

public:
    // The header of block that store data
    struct BlockHeader
    {
        uint16_t size;
        int overflow_page;  // The overflow_page id if it has
    };

public:
    using GeneralPage::GeneralPage;
    uint16_t &free_block();  // free block doesn't have slot
    uint16_t &free_size();
    uint16_t &size();
    uint16_t &bottom_used();
    uint16_t &next_page();
    uint16_t &prev_page();
    uint16_t &slots(int index);
    static constexpr int header_size();
    BlockHeader &block_header(int offset);
    FreeBlockHeader &free_block_header(int offset);

private:
    char *allocate(int sz);
    void defragment();
    void set_free_block(int offset);

public:
    void Init(int = 0);
    bool Underflow();
    bool UnderflowIfRemove();
    bool Insert(int pos, const char *data, int data_size);
    void Erase(int pos, bool erase_overflow_page = true);
    bool Update(int pos, const char *data, int data_size);
    void MoveFrom(VariantPage src_page, int src_pos, int dest_pos);
    // <page_id, page>
    std::pair<int, VariantPage> Split(int current_id);
    bool Merge(VariantPage page, int current_id);
    std::pair<BlockHeader, char *> GetBlock(int index);
};

inline
uint16_t &VariantPage::free_block() { return *reinterpret_cast<uint16_t *>(buf_ + 2); }

inline
uint16_t &VariantPage::free_size() { return *reinterpret_cast<uint16_t *>(buf_ + 4); }

inline
uint16_t &VariantPage::size() { return *reinterpret_cast<uint16_t *>(buf_ + 6); }

inline
uint16_t &VariantPage::bottom_used() { return *reinterpret_cast<uint16_t *>(buf_ + 8); }

inline
uint16_t &VariantPage::next_page() { return *reinterpret_cast<uint16_t *>(buf_ + 10); }

inline
uint16_t &VariantPage::prev_page() { return *reinterpret_cast<uint16_t *>(buf_ + 14); }

inline
uint16_t &VariantPage::slots(int index) { return *reinterpret_cast<uint16_t *>(buf_ + 18 + index * 2); }

inline
constexpr int VariantPage::header_size() { return 18; }
    
inline
VariantPage::BlockHeader &VariantPage::block_header(int offset) 
{ 
    return *reinterpret_cast<BlockHeader *>(buf_ + offset); 
}

inline
VariantPage::FreeBlockHeader &VariantPage::free_block_header(int offset) 
{ 
    return *reinterpret_cast<FreeBlockHeader *>(buf_ + offset); 
}

#endif