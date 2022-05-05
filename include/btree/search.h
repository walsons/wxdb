#ifndef SEARCH_H_
#define SEARCH_H_

namespace Search
{

// [0 0 0 1 1], find the first position which p(pos) is true
// if not found, return end
template <typename Predicator>
int upper_bound(int beg, int end, Predicator p)
{
    if (beg == end) { return end; }
    int rmost = end--;
    while (beg < end)
    {
        int mid = beg + (end - beg) / 2;
        if (!p(mid)) { beg = mid + 1; }
        else { end = mid; }
    }
    return p(beg) ? beg : rmost;
}

// [0 0 0 1 1], find the last position which p(pos) is false
// if not found, return end
template <typename Predicator>
int lower_bound(int beg, int end, Predicator p)
{
    if (beg == end) { return end; }
    int rmost = end--;
    while (beg < end)
    {
        int mid = beg + (end - beg + 1) / 2;
        if (!p(mid)) { beg = mid; }
        else { end = mid - 1; }
    }
    return !p(beg) ? beg : rmost;
}
    
}  // namespace Search

#endif