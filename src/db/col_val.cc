#include "../../include/db/col_val.h"

ColVal::ColVal() : type_(Col_Type::COL_TYPE_NULL) {}

ColVal::ColVal(const int &ival) 
    : type_(Col_Type::COL_TYPE_INT), ival_(ival) {}

ColVal::ColVal(const double &dval) 
    : type_(Col_Type::COL_TYPE_DOUBLE), dval_(dval) {}

ColVal::ColVal(const bool &bval) 
    : type_(Col_Type::COL_TYPE_BOOL), bval_(bval) {}

ColVal::ColVal(const Date &tval) 
    : type_(Col_Type::COL_TYPE_DATE), tval_(tval) {}

ColVal::ColVal(const std::string &sval) 
    : type_(Col_Type::COL_TYPE_VARCHAR)
{
    new (&sval_) std::string(sval);
}

ColVal::ColVal(const ColVal &col_val)
    : type_(col_val.type_)
{
    if (this != &col_val)
    {
        switch (col_val.type_)
        {
        case Col_Type::COL_TYPE_NULL:
            // Nothing need to do
        case Col_Type::COL_TYPE_INT:
            ival_ = col_val.ival_;
            break;
        case Col_Type::COL_TYPE_DOUBLE:
            dval_ = col_val.dval_;
            break;
        case Col_Type::COL_TYPE_BOOL:
            bval_ = col_val.bval_;
            break;
        case Col_Type::COL_TYPE_DATE:
            tval_ = col_val.tval_;
            break;
        case Col_Type::COL_TYPE_CHAR:
        case Col_Type::COL_TYPE_VARCHAR:
            new (&sval_) std::string(col_val.sval_);
            break;
        }
    }
}

void ColVal::set_null()
{
    destory_class_member();
    type_ = Col_Type::COL_TYPE_NULL;
}

ColVal &ColVal::operator=(const int &ival)
{
    destory_class_member();
    type_ = Col_Type::COL_TYPE_INT;
    ival_ = ival;
    return *this;
}

ColVal &ColVal::operator=(const double &dval)
{
    destory_class_member();
    type_ = Col_Type::COL_TYPE_DOUBLE;
    dval_ = dval;
    return *this;
}

ColVal &ColVal::operator=(const bool &bval)
{
    destory_class_member();
    type_ = Col_Type::COL_TYPE_BOOL;
    bval_ = bval;
    return *this;
}

ColVal &ColVal::operator=(const Date &tval)
{
    destory_class_member();
    type_ = Col_Type::COL_TYPE_DATE;
    tval_ = tval;
    return *this;
}

ColVal &ColVal::operator=(const std::string &sval)
{
    destory_class_member();
    type_ = Col_Type::COL_TYPE_VARCHAR;
    new (&sval_) std::string(sval);
    return *this;
}

ColVal &ColVal::operator=(const ColVal &col_val)
{
    if (this != &col_val)
    {
        switch (col_val.type_)
        {
        case Col_Type::COL_TYPE_NULL:
            // Nothing need to do
        case Col_Type::COL_TYPE_INT:
            *this = col_val.ival_;
            break;
        case Col_Type::COL_TYPE_DOUBLE:
            *this = col_val.dval_;
            break;
        case Col_Type::COL_TYPE_BOOL:
            *this = col_val.bval_;
            break;
        case Col_Type::COL_TYPE_DATE:
            *this = col_val.tval_;
            break;
        case Col_Type::COL_TYPE_CHAR:
            *this = col_val.sval_;
            this->type_ = Col_Type::COL_TYPE_CHAR;
            break;
        case Col_Type::COL_TYPE_VARCHAR:
            *this = col_val.sval_;
            break;
        }
    }
    return *this;
}

ColVal::~ColVal()
{
    destory_class_member();
}

void ColVal::destory_class_member()
{
    switch (type_)
    {
    case Col_Type::COL_TYPE_CHAR:
    case Col_Type::COL_TYPE_VARCHAR:
        sval_.~basic_string();
        break;
    default:
        break;
    }
}