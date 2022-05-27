#ifndef TYPE_CAST_H_
#define TYPE_CAST_H_

#include "col_val.h"

class TypeCast
{
public:
    // make column value to the given type if compatible
    static bool make_type_compatible(ColVal &val, Col_Type type);
};

inline bool TypeCast::make_type_compatible(ColVal &val, Col_Type type)
{
    if (val.type_ == type) { return true; }
    else if (type == Col_Type::COL_TYPE_DOUBLE)
    {
        if (val.type_ == Col_Type::COL_TYPE_INT)
        {
            val = static_cast<double>(val.ival_);
            return true;
        }
    }
    else if (type == Col_Type::COL_TYPE_DATE)
    {
        if (val.type_ == Col_Type::COL_TYPE_VARCHAR)
        {
            Date date{val.sval_};
            if (date.timestamp == -1) { return false; }
            val = date;
            return true;
        }
    }
    else if (type == Col_Type::COL_TYPE_CHAR && val.type_ == Col_Type::COL_TYPE_VARCHAR)
    {
        val.type_ = Col_Type::COL_TYPE_CHAR;
        return true;
    }
    return false;
} 

#endif