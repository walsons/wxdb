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
            // fd.seekg(0, std::ios::end);
            // file_length = fd.tellg();
            // size_t row_num = file_length / sizeof(Row);
            // size_t page_num = row_num / kRowNumberPerPage;
            // fd.seekg(0, std::ios::beg);
            // // Initialization
            // for (size_t i = 0; i < MAX_PAGE_NUMBER; ++i)
            // {
            //     pages[i] = nullptr;
            // }
            // // Allocate
            // for (size_t i = 0; i < page_num; ++i)
            // {
            //     pages[i] = new char[PAGE_SIZE];
            //     fd.read(pages[i], PAGE_SIZE);
            // }
            // // rest of buffer in file
            // pages[page_num] = new char[PAGE_SIZE];
            // fd.read(pages[page_num], (row_num - page_num * kRowNumberPerPage) * sizeof(Row));
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

    // Get unused page
    void *GetUnusedPage(unsigned &unused_page_num)
    {
        unused_page_num = num_pages_;
        ++num_pages_;
        file_length_ += PAGE_SIZE;
        pages_[unused_page_num] = new char[PAGE_SIZE];
        return pages_[unused_page_num];
    }

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
            fd_.read(pages_[page_num], PAGE_SIZE);
        }
        return pages_[page_num];
    }

    // Convert page to leaf node
    LeafNode *PageToLeafNode(void *page)
    {
        return reinterpret_cast<LeafNode *>(page);
    }
    // Convert page to internal node
    InternalNode *PageToInternalNode(void *page)
    {
        return reinterpret_cast<InternalNode *>(page);
    }
    // Get page/node type
    NODE_TYPE GetPageType(void *page)
    {
        return *reinterpret_cast<NODE_TYPE *>(page);
    }

    unsigned FindInternalNodeClue(unsigned page_num, unsigned key)
    {
        // Binary search for insert position of key
        void *page = GetPage(page_num);
        InternalNode *internal_node = PageToInternalNode(page);   
        if (internal_node->clue_[internal_node->num_keys_ - 1].key == key)
        {
            return internal_node->num_keys_ - 1;
        }
        int beg = 0, end = internal_node->num_keys_ - 1;
        while (beg <= end)
        {
            int mid = beg + (end - beg) / 2;
            if (internal_node->clue_[mid].key <= key)
            {
                beg = mid + 1;
            }
            else
            {
                end = mid;
            }
        } // Requiring key is exist in page
        return end - 1;
    }

    void InternalNodeInsert(unsigned page_num, unsigned child_page_num, unsigned key)
    {
        // Binary search for insert position of key that larger than key
        void *page = GetPage(page_num);
        InternalNode *internal_node = PageToInternalNode(page);   
        int beg = 0, end = internal_node->num_keys_ = 1;
        while (beg <= end)
        {
            int mid = beg + (end - beg) / 2;
            if (internal_node->clue_[mid].key <= key)
            {
                beg = mid + 1;
            }
            else
            {
                end = mid;
            }
        } // Insert key is impossible larger than the end key
        // move later element
        for (int i = internal_node->num_keys_ - 1; i >= end; --i)
        {
            internal_node->clue_[i + 1] = internal_node->clue_[i];
        }
        internal_node->clue_[end].key = key;
        internal_node->clue_[end].child = child_page_num;
        // modify internal node attribute
        ++internal_node->num_keys_;
    }


private:
    std::fstream fd_;
    unsigned int file_length_;
    unsigned num_pages_;
    char *pages_[MAX_PAGE_NUMBER];
};

#endif