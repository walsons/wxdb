#ifndef TYPE_CAST_H_
#define TYPE_CAST_H_

#include "col_val.h"

class TypeCast
{
public:
    static bool check_type_compatible(Col_Type t1, Col_Type t2);
};

inline bool TypeCast::check_type_compatible(Col_Type t1, Col_Type t2)
{
    // TODO: date can match varchar, int can match float
    return t1 == t2;
} 

#endif