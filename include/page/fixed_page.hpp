#ifndef FIXED_PAGE_HPP_
#define FIXED_PAGE_HPP_

#include "general_page.h"

// Page structure
/*
 * name          offset                                size          comment
 * page_type     0                                     2             page type
 * field_size    2                                     2             size of key for BTree
 * size          4                                     4             number of pairs of key and children
 * next_page     8                                     4             next page id
 * prev_page     12                                    4             previous page id
 * children[i]   16                                    4             i th children
 * ----------------------------  unused space  --------------------------------
 * keys[i]       PAGE_SIZE - field_size * (size - i)   field_size    i th key
 */

template <typename T>
class FixedPage : public GeneralPage
{
public:
    using GeneralPage::GeneralPage;
    uint16_t &field_size();
    int &size();
    int &next_page();
    int &prev_page();
    static constexpr int header_size();
    int &children(int index);
    T get_key(int index);
    void set_key(int index, const T &data);

    char *begin();
    char *end();

    void Init(int field_size);
    int Capacity();
    bool Full();
    bool Empty();
    bool Underflow();
    bool UnderflowIfRemove();
    bool Insert(int pos, const T &key, int child);
    bool Erase(int pos);
    bool Update(int pos, const T &key, int child);
    // <page_id, page>
    std::pair<int, FixedPage> Split(int current_id);
    bool Merge(FixedPage page, int current_id);
    void MoveFrom(FixedPage src_page, int src_pos, int des_pos);
};

template <typename T> inline
uint16_t &FixedPage<T>::field_size() { return *reinterpret_cast<uint16_t *>(buf_ + 2); }

template <typename T> inline
int &FixedPage<T>::size() { return *reinterpret_cast<int *>(buf_ + 4); }

template <typename T> inline
int &FixedPage<T>::next_page() { return *reinterpret_cast<int *>(buf_ + 8); }

template <typename T> inline
int &FixedPage<T>::prev_page() { return *reinterpret_cast<int *>(buf_ + 12); }

template <typename T> inline
constexpr int FixedPage<T>::header_size() { return 16; }

template <typename T> inline
int &FixedPage<T>::children(int index) { return *reinterpret_cast<int *>(buf_ + header_size() + index * sizeof(int)); }

template <typename T> inline
T FixedPage<T>::get_key(int index) { return *reinterpret_cast<T *>(begin() + index * field_size()); }

template <typename T> inline
void FixedPage<T>::set_key(int index, const T &data) { *reinterpret_cast<T *>(begin() + index * field_size()) = data; }

// Specialized for const char *
template <> inline
const char *FixedPage<const char *>::get_key(int index) { return begin() + index * field_size(); }

template <> inline
void FixedPage<const char *>::set_key(int index, const char * const &data) 
{
    std::memcpy(begin() + index * field_size(), data, field_size());
}

template <typename T> inline
char *FixedPage<T>::begin() { return end() - size() * field_size(); }

template <typename T> inline
char *FixedPage<T>::end() { return buf_ + PAGE_SIZE; }

template <typename T> inline
void FixedPage<T>::Init(int fsize)
{
    page_type() = Page_Type::FIXED_PAGE;
    field_size() = fsize;
    size() = 0;
    next_page() = prev_page() = 0;
}

template <typename T> inline
int FixedPage<T>::Capacity() { return (PAGE_SIZE - header_size()) / (field_size() + 4); }

template <typename T> inline
bool FixedPage<T>::Full() { return size() == Capacity(); }

template <typename T> inline
bool FixedPage<T>::Empty() { return size() == 0; }

// B+Tree must satisfied the condition that the number of children of each 
// internal node(except root node) greater or equal to round up (m / 2)
template <typename T> inline
bool FixedPage<T>::Underflow() { return size() < (Capacity() + 1) / 2; }

template <typename T> inline
bool FixedPage<T>::UnderflowIfRemove() { return size() < (Capacity() + 1) / 2 + 1; }

template <typename T>
bool FixedPage<T>::Insert(int pos, const T &key, int child)
{
    if (Full()) { return false; }
    for (int i = size() - 1; i >= pos; --i)
    {
        children(i + 1) = children(i);
    }
    children(pos) = child;
    std::memmove(
        reinterpret_cast<char *>(end() - (size() + 1) * field_size()),
        reinterpret_cast<char *>(end() - size() * field_size()),
        pos * field_size()
    );
    ++size();
    set_key(pos, key);
    return true;
}

template <typename T>
bool FixedPage<T>::Erase(int pos)
{
    if (Empty()) { return false; }
    for (int i = pos + 1; i < size(); ++i)
    {
        children(i - 1) = children(i);
    }
    std::memmove(
        reinterpret_cast<char *>(end() - (size() - 1) * field_size()),
        reinterpret_cast<char *>(end() - size() * field_size()),
        pos * field_size()
    );
    --size();
}

template <typename T> inline
bool FixedPage<T>::Update(int pos, const T &key, int child)
{
    set_key(pos, key);
    return true;
}

template <typename T>
std::pair<int, FixedPage<T>> FixedPage<T>::Split(int current_id)
{
    int page_id = pg_->NewPage();
    if (page_id == 0) { return {0, FixedPage<T>{nullptr, nullptr}}; }
    FixedPage upper_page{pg_->ReadForWrite(page_id), pg_};
    upper_page.Init(field_size());
    if (prev_page())
    {
        FixedPage page{pg_->ReadForWrite(prev_page()), pg_};
        page.next_page() = page_id;
    }
    upper_page.prev_page() = prev_page();
    upper_page.next_page() = current_id;
    prev_page() = page_id;
    int lower_size = size() >> 1;
    int upper_size = size() - lower_size;
    for (int i = 0; i < upper_size; ++i)
    {
        upper_page.children(i) = children(i);
    }
    for (int i = 0; i < lower_size; ++i)
    {
        children(i) = children(i + upper_size);
    }
    std::memcpy(
        upper_page.end() - upper_size * field_size(),
        begin(),
        upper_size * field_size()
    );
    upper_page.size() = upper_size;
    size() = lower_size;
    return {page_id, upper_page};
}

template <typename T>
bool FixedPage<T>::Merge(FixedPage page, int current_id)
{
    int merge_size = size() + page.size();
    if (merge_size > Capacity()) { return false; }
    next_page() = page.next_page();
    if (next_page())
    {
        FixedPage page{pg_->ReadForWrite(next_page()), pg_};
        page.prev_page() = current_id;
    }
    for (int i = size(); i < merge_size; ++i)
    {
        children(i) = page.children(i - size());
    }
    std::memmove(
        begin() - page.size() * field_size(),
        begin(),
        page.size() * field_size()
    );
    std::memcpy(
        end() - page.size() * field_size(),
        page.begin(),
        page.size() * field_size()
    );
    return true;
}

template <typename T> inline
void FixedPage<T>::MoveFrom(FixedPage src_page, int src_pos, int des_pos)
{
    Insert(des_pos, get_key(src_pos), children(src_pos));
    Erase(src_pos);
}

#endif