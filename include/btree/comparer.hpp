#ifndef COMPARER_HPP_
#define COMPARER_HPP_

#include <cstring>

template <typename T>
inline int basic_type_comparer(const T &x, const T &y)
{
    if (x < y) return -1;
    return x == y ? 0 : 1;
}

inline int integer_comparer(const int &x, const int &y)
{
    return basic_type_comparer(x, y);
}

inline int double_comparer(const double &x, const double &y)
{
    return basic_type_comparer(x, y);
}

inline int integer_cast_comparer(const char *x, const char *y)
{
    return basic_type_comparer(*reinterpret_cast<const int *>(x), *reinterpret_cast<const int *>(y));
}

inline int double_cast_comparer(const char *x, const char *y)
{
    return basic_type_comparer(*reinterpret_cast<const double *>(x), *reinterpret_cast<const double *>(y));
}

inline int string_comparer(const char *x, const char *y)
{
    return std::strcmp(x, y);
}

#endif