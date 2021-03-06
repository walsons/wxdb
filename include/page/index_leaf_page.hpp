#ifndef INDEX_LEAF_PAGE_HPP_
#define INDEX_LEAF_PAGE_HPP_

#include "fixed_page.hpp"

template <typename T>
class IndexLeafPage : public FixedPage<T>
{
public:
    using FixedPage<T>::FixedPage;
    void Init(int field_size);
    std::pair<int, IndexLeafPage> Split(int current_id);
};

template <typename T > inline
void IndexLeafPage<T>::Init(int field_size)
{
    FixedPage<T>::Init(field_size);
    this->page_type() = Page_Type::INDEX_LEAF_PAGE;
}

template <typename T > inline
std::pair<int, IndexLeafPage<T>> IndexLeafPage<T>::Split(int current_id)
{
    auto res = FixedPage<T>::Split(current_id);
    res.second.page_type() = Page_Type::INDEX_LEAF_PAGE;
    return { res.first, *reinterpret_cast<IndexLeafPage *>(&res.second)};
}

#endif