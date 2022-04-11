#include "../../include/sql/literal.h"

Literal::Literal(Data_Type data_type, std::string original_value)
    : data_type_(data_type), original_value_(original_value)
{
}

Literal::~Literal() = default;

IntLiteral::IntLiteral(Data_Type data_type, std::string original_value)
    : Literal(data_type, original_value), value(std::stoi(original_value))
{
}

IntLiteral::~IntLiteral() = default;

DoubleLiteral::DoubleLiteral(Data_Type data_type, std::string original_value)
    : Literal(data_type, original_value), value(std::stod(original_value))
{
}

DoubleLiteral::~DoubleLiteral() = default;