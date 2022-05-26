#ifndef DATA_PAGE_HPP_
#define DATA_PAGE_HPP_

#include "variant_page.h"

template <typename Key>
class DataPage : public VariantPage
{
public:
    using VariantPage::VariantPage;
    Key get_key(int index);
    void set_key(int index, const Key &data);
    std::pair<int, DataPage> Split(int currend_id);
};

template <typename Key> inline
Key DataPage<Key>::get_key(int index) { return *reinterpret_cast<Key *>(GetBlock(index).second); }

template <typename Key> inline
void DataPage<Key>::set_key(int index, const Key &data) { *reinterpret_cast<Key *>(GetBlock(index).second) = data; }

template <typename Key> inline    
std::pair<int, DataPage<Key>> DataPage<Key>::Split(int currend_id)
{
    auto res = VariantPage::Split(currend_id);
    return { res.first, *reinterpret_cast<DataPage *>(&res.second) };
}

#endif