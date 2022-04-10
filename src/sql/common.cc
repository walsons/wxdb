#include "../../include/sql/common.h"

DataValue::DataValue(Data_Type type, size_t string_length) : type_(type)
{
    if (type_ == Data_Type::DATA_TYPE_CHAR)
    {
        char_value_ = new char[string_length + 1];
    }
}

DataValue::~DataValue()
{
    if (type_ == Data_Type::DATA_TYPE_CHAR)
    {
        delete char_value_;
    }
}

FieldInfo::FieldInfo(Data_Type type, int length, const std::string &field_name)
{
}

FieldInfo::~FieldInfo() = default;

TableInfo::TableInfo(const std::string &table_name, std::vector<std::string> fields_name,
                     std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<FieldInfo>>> fields)
    : table_name_(table_name), fields_name_(fields_name), fields_(fields)
{
}

TableInfo::~TableInfo() = default;

ForeignKeyRef::ForeignKeyRef(const std::string &column_name)
    : column_name_(column_name)
{
}

ForeignKeyRef::ForeignKeyRef(const std::string &table_name, const std::string &column_name)
    : table_name_(table_name), column_name_(column_name)
{
}

ForeignKeyRef::~ForeignKeyRef() = default;