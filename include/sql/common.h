#ifndef common_h_
#define common_h_

#include <string>

enum class Data_Type : char
{
    DATA_TYPE_INT,
    DATA_TYPE_DOUBLE,
    DATA_TYPE_DATE,
    DATA_TYPE_CHAR,
    DATA_TYPE_VARCHAR,
    DATA_TYPE_TEXT,
    DATA_TYPE_BOOLEAN,
    DATA_TYPE_NULL
};

class DataValue
{
public:
    DataValue(Data_Type type);
    DataValue(const DataValue &data_value);
    DataValue &operator=(const DataValue &data_value);
    ~DataValue();
    Data_Type GetDataType();
    void SetIntValue(const int &value);
    void SetDoubleValue(const double &value);
    void SetCharValue(const std::string &value);
    const int int_value();
    const double double_value();
    const std::string char_value();

private:
    Data_Type type_;
    union
    {
        int int_value_;
        double double_value_;
        std::string char_value_;
    };
};

#endif