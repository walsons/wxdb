#ifndef OVERFLOW_PAGE_H_
#define OVERFLOW_PAGE_H_

#include "general_page.h"

// Page structure
/*
 * name         offset    size    comment
 * page_type    0         2       page type
 * size         2         2       number of bytes of data in this page
 * next         4         4       next page id
 * block        8         4       data start address
 */

class OverflowPage : public GeneralPage
{
public:
    using GeneralPage::GeneralPage;
    uint16_t &size();
    int &next();
    char *block(); 
    static constexpr int header_size();
    static constexpr int block_size();
    void Init();
};

inline
uint16_t &OverflowPage::size() { return *reinterpret_cast<uint16_t *>(buf_ + 2); }

inline
int &OverflowPage::next() { return *reinterpret_cast<int *>(buf_ + 4); }

inline
char *OverflowPage::block() { return reinterpret_cast<char *>(buf_ + 8); } 

inline
constexpr int OverflowPage::header_size() { return 8; }

inline
constexpr int OverflowPage::block_size() { return PAGE_SIZE - header_size(); }

inline
void OverflowPage::Init()
{
    page_type() = Page_Type::OVERFLOW_PAGE;
    size() = 0;
    next() = 0;
}

#endif