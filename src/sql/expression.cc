#include "../../include/sql/expression.h"

ColumnRef::ColumnRef(const std::string &table_name, const std::string &column_name)
    : table_name_(table_name), column_name_(column_name)
{
}

ColumnRef::ColumnRef(const std::string &all_name)
    : all_name_(all_name)
{
    size_t pos = all_name.find(",");
    table_name_ = all_name.substr(0, pos);
    column_name_ = all_name.substr(pos + 1);
}

ColumnRef::~ColumnRef() = default;

TermExpr::TermExpr(Term_Type term_type) : term_type_(term_type)
{
}

TermExpr::~TermExpr()
{
    if (term_type_ == Term_Type::TERM_ID) { id.~basic_string(); }
}

Expression::Expression(Token_Type operator_type, Expression *next_expr)
    : operator_type_(operator_type), next_expr_(next_expr)
{
}

Expression::~Expression() = default;