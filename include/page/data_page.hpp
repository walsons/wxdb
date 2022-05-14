#ifndef DATA_PAGE_HPP_
#define DATA_PAGE_HPP_

#include "variant_page.h"

template <typename Key>
class DataPage : public VariantPage
{
public:
    using VariantPage::VariantPage;
    Key &keys(int id);
    std::pair<int, DataPage> Split(int currend_id);
};

template <typename Key> inline    
Key &DataPage<Key>::keys(int id)
{
    *reinterpret_cast<Key *>(GetBlock(id).second);
}

template <typename Key> inline    
std::pair<int, DataPage<Key>> DataPage<Key>::Split(int currend_id)
{
    auto res = VariantPage::Split(currend_id);
    return { res.first, *reinterpret_cast<DataPage *>(&res.second) };
}

#endif