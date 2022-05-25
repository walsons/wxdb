#ifndef PAGER_H_
#define PAGER_H_

#include <fstream>
#include <string>

#include "wxdb_define.h"
#include "b_tree.h"

class Pager
{
public:
    Pager(const std::string &file_name) 
    {
        fd_.open(file_name, std::ios::binary | std::ios::in | std::ios::out);
        if (fd_)
        {
            fd_.seekg(0, std::ios::end);
            file_length_ = fd_.tellg();
            num_pages_ = file_length_ / PAGE_SIZE;
            fd_.seekg(0, std::ios::beg);
            // Initialization
            for (size_t i = 0; i < MAX_PAGE_NUMBER; ++i)
            {
                pages_[i] = nullptr;
            }
        }
        else
        {
            std::ofstream out_fd(file_name, std::ios::out);
            out_fd.close();
            fd_.open(file_name, std::ios::binary | std::ios::in | std::ios::out);
            file_length_ = 0;
            num_pages_ = 0;
            for (size_t i = 0; i < MAX_PAGE_NUMBER; ++i)
            {
                pages_[i] = nullptr;
            }
        }
    }
    ~Pager()
    {
        fd_.close();
        for (size_t i = 0; i < MAX_PAGE_NUMBER; ++i)
        {
            if (pages_[i] != nullptr)
            {
                delete[] pages_[i];
                pages_[i] = nullptr;
            }
        }
    }

    // Get unused page and allocate space
    unsigned GetUnusedPageNum()
    {
        unsigned unused_page_num = num_pages_;
        ++num_pages_;
        file_length_ += PAGE_SIZE;
        pages_[unused_page_num] = new char[PAGE_SIZE];
        return unused_page_num;
    }

    NODE_TYPE GetPageType(unsigned page_num)
    {
        void *page = GetPage(page_num);
        return *reinterpret_cast<NODE_TYPE *>(page);
    }

    // Get leaf node by page number
    LeafNode *GetLeafNode(unsigned page_num)
    {
        void *page = GetPage(page_num);
        return reinterpret_cast<LeafNode *>(page);
    }
    
    // Get internal node by page number
    InternalNode *GetInternalNode(unsigned page_num)
    {
        void *page = GetPage(page_num);
        return reinterpret_cast<InternalNode *>(page);
    }

    /* 
    * Search on tree until leaf node
    */
    std::pair<unsigned, unsigned> SearchBPlusTree(unsigned page_num, unsigned key)
    {
        if (GetPageType(page_num) == NODE_TYPE::LEAF_NODE)
        {
            LeafNode *node = GetLeafNode(page_num);
            unsigned index = BinarySearchLargerKey<Cell>(node->cell_, node->num_cells_, key);
            return {node->page_num_, index};
        }
        InternalNode *node = GetInternalNode(page_num);
        unsigned index = BinarySearchLargerKey<Clue>(node->clue_, node->num_clues_, key);
        unsigned child_page_num = node->clue_[index].child;
        return SearchBPlusTree(child_page_num, key);
    }

    // unsigned FindInternalNodeClue(unsigned page_num, unsigned key)
    // {
    //     // Binary search for insert position of key
    //     InternalNode *internal_node = GetInternalNode(page_num);
    //     if (internal_node->clue_[internal_node->num_keys_ - 1].key == key)
    //     {
    //         return internal_node->num_keys_ - 1;
    //     }
    //     int beg = 0, end = internal_node->num_keys_ - 1;
    //     while (beg <= end)
    //     {
    //         int mid = beg + (end - beg) / 2;
    //         if (internal_node->clue_[mid].key <= key)
    //         {
    //             beg = mid + 1;
    //         }
    //         else
    //         {
    //             end = mid;
    //         }
    //     } // Requiring key is exist in page
    //     return end - 1;
    // }

    void InternalNodeInsert(unsigned page_num, unsigned child_page_num, unsigned key)
    {
        // Binary search for insert position of key that larger than key
        InternalNode *internal_node = GetInternalNode(page_num);
        unsigned index = BinarySearchLargerKey<Clue>(internal_node->clue_, internal_node->num_clues_, key);
        // move later element
        for (int i = internal_node->num_clues_; i > index; --i)
        {
            internal_node->clue_[i] = internal_node->clue_[i - 1];
        }
        internal_node->clue_[index].key = key;
        internal_node->clue_[index].child = child_page_num;
        // modify internal node attribute
        ++internal_node->num_clues_;
    }

    // Flush page to disk
    void FlushPageToDisk()
    {
        for (unsigned i = 0; i < num_pages_; ++i)
        {
            if (pages_[i] != nullptr)
            {
                fd_.seekp(i * PAGE_SIZE, std::ios::beg);
                assert(fd_.good());
                fd_.write(pages_[i], PAGE_SIZE);
            }
        }
    }

private:
    std::fstream fd_;
    unsigned int file_length_;
    unsigned num_pages_;
    char *pages_[MAX_PAGE_NUMBER];

private:
    // Get page that record in file by page number
    void *GetPage(unsigned page_num)
    {
        if (page_num >= MAX_PAGE_NUMBER)
        {
            throw std::out_of_range("excess max page number");
        }
        // not implement delete action, so page number equal to pages[page_num].
        if (page_num >= num_pages_)
        {
            throw std::out_of_range("excess max page number");
        }
        if (pages_[page_num] == nullptr)
        {
            fd_.seekg(page_num * PAGE_SIZE, std::ios::beg);
            pages_[page_num] = new char[PAGE_SIZE];
            assert(fd_.good());
            fd_.read(pages_[page_num], PAGE_SIZE);
            if (fd_.eof()) { fd_.clear(); }
        }
        return pages_[page_num];
    }
};

#endif