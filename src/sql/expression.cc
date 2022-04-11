#include "../../include/sql/expression.h"

TermExpr::TermExpr(Term_Type term_type) : term_type_(term_type)
{
}

TermExpr::~TermExpr()
{
    if (term_type_ == Term_Type::TERM_ID) { id.~basic_string(); }
}