#include "../../include/sql/expression.h"
#include <cassert>
#include <stack>

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
        delete literal_;
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

Expression::Expression(ExprNode *expr)
{
    Eval(expr);
}

void Expression::Eval(ExprNode *expr)
{
    // TODO: add type check
    assert(expr != nullptr);
    std::stack<ExprNode *> expr_stack;
    expr_stack.push(expr);
    expr = expr->next_expr_;
    while (expr != nullptr)
    {
        // If not a operator exprnode
        if (expr->term_ != nullptr)
        {
            expr_stack.push(expr);
            expr = expr->next_expr_;
        }
        else
        {
            auto cur_expr = expr;
            expr = expr->next_expr_;
            auto expr2 = expr_stack.top();
            expr_stack.pop();
            // Unary operator
            if (cur_expr->operator_type_ == Token_Type::TOKEN_NOT ||
                cur_expr->operator_type_ == Token_Type::TOKEN_POSITIVE ||
                cur_expr->operator_type_ == Token_Type::TOKEN_NEGATIVE)
            {
                cur_expr = EvalOperator(cur_expr, expr2);
            }
            // Binary operator
            else
            {
                auto expr1 = expr_stack.top();
                expr_stack.pop();
                cur_expr = EvalOperator(cur_expr, expr1, expr2);
            }
            expr_stack.push(cur_expr);
        }
    }
    expr = expr_stack.top();
    switch (expr->term_->term_type_)
    {
    case Term_Type::TERM_INT:
        value.SetIntValue(expr->term_->ival_);
        break;
    case Term_Type::TERM_DOUBLE:
        value.SetDoubleValue(expr->term_->dval_);
        break;
    case Term_Type::TERM_BOOL:
        value.SetBoolValue(expr->term_->bval_);
        break;
    default:
        // TODO: other type processing
        break;
    }
}

ExprNode *Expression::EvalOperator(ExprNode *op, ExprNode *expr1, ExprNode *expr2)
{
    switch (op->operator_type_)
    {
    // Unary operator
    case Token_Type::TOKEN_NOT:
        break;
    case Token_Type::TOKEN_POSITIVE:
        break;
    case Token_Type::TOKEN_NEGATIVE:
        break;
    // Binary operator
    case Token_Type::TOKEN_OR:
        break;
    case Token_Type::TOKEN_AND:
        break;
    case Token_Type::TOKEN_EQ:
        break;
    case Token_Type::TOKEN_NOT_EQ:
        break;
    case Token_Type::TOKEN_GE:
        break;
    case Token_Type::TOKEN_LE:
        break;
    case Token_Type::TOKEN_GT:
        break;
    case Token_Type::TOKEN_LT:
        break;
    case Token_Type::TOKEN_PLUS:
        break;
    case Token_Type::TOKEN_MINUS:
        break;
    case Token_Type::TOKEN_MULTIPLY:
        break;
    case Token_Type::TOKEN_DIVIDE:
        break;
    case Token_Type::TOKEN_POWER:
        break;
    default:
        break;
    }
    return nullptr;
}