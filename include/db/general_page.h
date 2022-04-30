#ifndef GENERAL_PAGE_H_
#define GENERAL_PAGE_H_

#include <memory>
#include "pager.h"

class GeneralPage
{
public:
    char *buf_;
    std::shared_ptr<Pager> pg_;
    GeneralPage(char *buf, std::shared_ptr<Pager> pg);
};

inline
GeneralPage::GeneralPage(char *buf, std::shared_ptr<Pager> pg) 
    : buf_(buf), pg_(pg)
{
}

#endif