#include "../../include/page/fixed_page.h"
#include <cstring>

template <typename T> inline
bool FixedPage<T>::Insert(int pos, const T &key, int child)
{
    if (Full()) { return false; }
    for (int i = size() - 1; i >= pos; ++i)
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
    keys(pos) = key;
    return true;
}

template <typename T> inline
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
std::pair<int, FixedPage<T>> FixedPage<T>::Split(int current_id)
{
    int page_id = pg_->NewPage();
    if (page_id == 0) { return {0, FixedPage<T>{nullptr, nullptr}}; }
    FixedPage upper_page{pg_->ReadForWrite(page_id), pg_};
    upper_page.Init(field_size());
    if (next_page())
    {
        FixedPage page{pg_->ReadForWrite(next_page()), pg_};
        page.prev_page() = page_id;
    }
    upper_page.next_page() = next_page();
    upper_page.prev_page() = current_id;
    next_page() = page_id;
    int lower_size = size() >> 1;
    int upper_size = size() - lower_size;
    for (int i = lower_size; i < size(); ++i)
    {
        upper_page.children(i - lower_size) = children(i);
    }
    std::memcpy(
        upper_page.end() - upper_size * field_size(),
        end() - upper_size * field_size(),
        upper_size * field_size()
    );
    upper_page.size() = upper_size;
    std::memmove(
        end() - lower_size * field_size(),
        end() - size() * field_size(),
        lower_size
    );
    size() = lower_size;
    return {page_id, upper_page};
}

template <typename T> inline
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
    Insert(des_pos, keys(src_pos), children(src_pos));
    Erase(src_pos);
}