#include "../../include/sql/common.h"
#include <cstring>

DataValue::DataValue(Data_Type type) : type_(type)
{
}

DataValue::DataValue(const DataValue &data_value)
{
    switch (data_value.type_)
    {
    case Data_Type::DATA_TYPE_INT:
        this->SetIntValue(data_value.int_value_);
        break;
    case Data_Type::DATA_TYPE_DOUBLE:
        this->SetDoubleValue(data_value.double_value_);
        break;
    case Data_Type::DATA_TYPE_CHAR:
        this->SetCharValue(data_value.char_value_);
        break;
    default:
        type_ = data_value.type_;
        break;
    }
}

DataValue &DataValue::operator=(const DataValue &data_value)
{
    switch (data_value.type_)
    {
    case Data_Type::DATA_TYPE_INT:
        this->SetIntValue(data_value.int_value_);
        break;
    case Data_Type::DATA_TYPE_DOUBLE:
        this->SetDoubleValue(data_value.double_value_);
        break;
    case Data_Type::DATA_TYPE_CHAR:
        this->SetCharValue(data_value.char_value_);
        break;
    default:
        type_ = data_value.type_;
        break;
    }
    return *this;
}

DataValue::~DataValue()
{
    if (type_ == Data_Type::DATA_TYPE_CHAR) { char_value_.~basic_string(); }
}

Data_Type DataValue::GetDataType() { return type_; }

void DataValue::SetIntValue(const int &value)
{
    if (type_ == Data_Type::DATA_TYPE_CHAR) { char_value_.~basic_string(); }
    if (type_ != Data_Type::DATA_TYPE_INT) 
    { 
        type_ = Data_Type::DATA_TYPE_INT;
    }
    int_value_ = value;
}

void DataValue::SetDoubleValue(const double &value)
{
    if (type_ == Data_Type::DATA_TYPE_CHAR) { char_value_.~basic_string(); }
    if (type_ != Data_Type::DATA_TYPE_DOUBLE) 
    { 
        type_ = Data_Type::DATA_TYPE_DOUBLE;
    }
    double_value_ = value;

}

void DataValue::SetCharValue(const std::string &value)
{
    type_ = Data_Type::DATA_TYPE_CHAR;
    new(&char_value_) std::string(value);
}

const int DataValue::int_value() { return int_value_; }

const double DataValue::double_value() { return double_value_; }

const std::string DataValue::char_value() { return char_value_; }
