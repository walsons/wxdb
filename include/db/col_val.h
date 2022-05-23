#ifndef COL_VAL_H_
#define COL_VAL_H_

#include <string>
#include "../defs.h"

// Column type
enum class Col_Type
{
    COL_TYPE_INT,
    COL_TYPE_DOUBLE,
    COL_TYPE_BOOL,
    COL_TYPE_DATE,
    COL_TYPE_CHAR,
    COL_TYPE_VARCHAR,
    COL_TYPE_NULL
};

// column value
class ColVal
{
public:
    ColVal();
    ColVal(const int &ival);
    ColVal(const double &dval);
    ColVal(const bool &bval);
    ColVal(const Date &tval);
    // default is varchar
    ColVal(const char *sval);
    ColVal(const std::string &sval);
    ColVal(const ColVal &col_val);

    void set_null();
    ColVal &operator=(const int &ival);
    ColVal &operator=(const double &dval);
    ColVal &operator=(const bool &bval);
    ColVal &operator=(const Date &tval);
    // default is varchar
    ColVal &operator=(const char *sval);
    ColVal &operator=(const std::string &sval);
    ColVal &operator=(const ColVal &col_val);

    ~ColVal();

    Col_Type type_;
    union
    {
        void *null_;
        int ival_;
        double dval_;
        bool bval_;
        Date tval_;  
        std::string sval_;  // char or varchar
    };

private:
    void destory_class_member();
};

#endif