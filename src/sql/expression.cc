#include "../../include/sql/expression.h"

ColumnRef::ColumnRef(const std::string &all_name)
    : all_name_(all_name)
{
    std::string::size_type pos = all_name.find(".");
    if (pos != std::string::npos) 
    { 
        table_name_ = all_name.substr(0, pos);
        column_name_ = all_name.substr(pos + 1);
    }
    else
    {
        column_name_ = all_name;
    }
}

ColumnRef::~ColumnRef() = default;

TermExpr::TermExpr(Term_Type term_type) : term_type_(term_type)
{
}

TermExpr::~TermExpr()
{
    switch (term_type_)
    {
    case Term_Type::TERM_ID:
        id_.~basic_string();
        break;
    case Term_Type::TERM_LITERAL:
        delete val_;
        break;
    case Term_Type::TERM_COL_REF:
        delete ref_;
        break;
    case Term_Type::TERM_FUNC:
        func_.~Func();
        break;
    default:
        break;
    }
}

ExprNode::ExprNode(Token_Type operator_type, 
                   std::shared_ptr<TermExpr> term, 
                   ExprNode *next_expr)
    : operator_type_(operator_type), term_(nullptr), next_expr_(next_expr)
{
}

ExprNode::~ExprNode() = default;
