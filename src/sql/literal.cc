#include "../../include/sql/literal.h"

Literal::Literal(std::string original_value) : original_value_(original_value)
{
}

Literal::~Literal() = default;