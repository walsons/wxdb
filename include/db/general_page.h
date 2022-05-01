#ifndef GENERAL_PAGE_H_
#define GENERAL_PAGE_H_

#include <memory>
#include "pager.h"

enum class Page_Type : uint16_t
{
    FIXED_PAGE,
    INDEX_LEAF_PAGE,
    VARIANT_PAGE,
    OVERFLOW_PAGE
};

class GeneralPage
{
public:
    char *buf_;
    // All pages in the same file share a pagers
    std::shared_ptr<Pager> pg_;
    GeneralPage(char *buf, std::shared_ptr<Pager> pg);
    ~GeneralPage() = default;
    Page_Type &page_type();
};

inline
GeneralPage::GeneralPage(char *buf, std::shared_ptr<Pager> pg) 
    : buf_(buf), pg_(pg)
{
}

inline
Page_Type &GeneralPage::page_type()
{
    return *reinterpret_cast<Page_Type *>(buf_);
}

#endif