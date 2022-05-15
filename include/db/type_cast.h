#ifndef TYPE_CAST_H_
#define TYPE_CAST_H_

#include "../sql/common.h"

class TypeCast
{
public:
    static bool check_type_compatible(Data_Type t1, Data_Type t2);
};

inline bool TypeCast::check_type_compatible(Data_Type t1, Data_Type t2)
{
    // TODO: date can match varchar, int can match float
    return t1 == t2;
} 

#endif