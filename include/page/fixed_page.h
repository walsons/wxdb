#ifndef FIXED_PAGE_H_
#define FIXED_PAGE_H_

#include "general_page.h"

// Page structure
/*
 * name          offset                                size          comment
 * page_type     0                                     2             page type
 * field_size    2                                     2             field size
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
    T &keys(int index);

    char *begin();
    char *end();

    void Init(int field_size);
    int Capacity() const;
    bool Full() const;
    bool Empty() const;
    bool Underflow() const;
    bool UnderflowIfRemove() const;
    bool Insert(int pos, const T &key, int child);
    bool Erase(int pos);
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
int &FixedPage<T>::children(int index) { return *reinterpret_cast<int *>(buf_ + 16 + index * sizeof(int)); }

template <typename T> inline
T &FixedPage<T>::keys(int index) { return *reinterpret_cast<T *>(begin() + index * field_size()); }

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
int FixedPage<T>::Capacity() const { return (PAGE_SIZE - header_size()) / (sizeof(T) + 4); }

template <typename T> inline
bool FixedPage<T>::Full() const { return size() == Capacity(); }

template <typename T> inline
bool FixedPage<T>::Empty() const { return size() == 0; }

// B+Tree must satisfied the condition that the number of children of each 
// internal node(except root node) greater or equal to round up (m / 2)
template <typename T> inline
bool FixedPage<T>::Underflow() const { return size() < (Capacity() + 1) / 2; }

template <typename T> inline
bool FixedPage<T>::UnderflowIfRemove() const { return size() < (Capacity() + 1) / 2 + 1; }

#endif