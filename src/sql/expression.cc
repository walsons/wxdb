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

void Expression::DumpExprNode(std::ostringstream &os, ExprNode *expr)
{
    // is_operator Term_Type value is_operator Token_Type placeholder...
    // TODO: currently support column_ref > >= == <= < int double bool
    while (expr != nullptr)
    {
        if (expr->term_ == nullptr)
        {
            os << 1 << " " 
               << static_cast<int>(expr->operator_type_) 
               << " " << 0 << " ";
        }
        else
        {
            os << 0 << " "
               << static_cast<int>(expr->term_->term_type_) << " ";
            switch (expr->term_->term_type_)
            {
            case Term_Type::TERM_COL_REF:
                if (expr->term_->ref_->table_name_.size() != 0)
                {
                    os << 2 << " " 
                       << expr->term_->ref_->table_name_ << " "
                       << expr->term_->ref_->column_name_ << " ";
                }
                else
                {
                    os << 1 << " " 
                       << expr->term_->ref_->column_name_ << " ";
                }
                break;
            case Term_Type::TERM_INT:
                os << expr->term_->ival_ << " ";
                break;
            case Term_Type::TERM_DOUBLE:
                os << expr->term_->ival_ << " ";
                break;
            case Term_Type::TERM_BOOL:
                os << expr->term_->ival_ << " ";
                break;
            default:
                break;
            } 
        }
        expr = expr->next_expr_;
    }
}

ExprNode *Expression::LoadExprNode(std::istringstream &is)
{
    int is_operator;
    ExprNode *head = new ExprNode(Token_Type::TOKEN_NULL);
    ExprNode *expr = head;
    while (is >> is_operator)
    {
        if (is_operator)
        {
            int type = 0;
            is >> type;
            expr->next_expr_ = new ExprNode(static_cast<Token_Type>(type));
            // Remove placeholder
            is >> type;
        }
        else
        {
            int type = 0;
            is >> type;
            switch (static_cast<Term_Type>(type))
            {
            case Term_Type::TERM_COL_REF:
            {
                auto term = std::make_shared<TermExpr>(Term_Type::TERM_COL_REF);
                int count = 0;
                is >> count;
                if (count == 2)
                {
                    std::string table_name, column_name;
                    is >> table_name >> column_name;
                    term->ref_ = new ColumnRef(table_name + "." + column_name);
                }
                else
                {
                    std::string column_name;
                    is >> column_name;
                    term->ref_ = new ColumnRef(column_name);
                }
                expr->next_expr_ = new ExprNode(Token_Type::TOKEN_NULL, term);
                break;
            }
            case Term_Type::TERM_INT:
            {
                auto term = std::make_shared<TermExpr>(Term_Type::TERM_INT);
                is >> term->ival_;
                expr->next_expr_ = new ExprNode(Token_Type::TOKEN_NULL, term);
                break;
            }
            case Term_Type::TERM_DOUBLE:
            {
                auto term = std::make_shared<TermExpr>(Term_Type::TERM_DOUBLE);
                is >> term->dval_;
                expr->next_expr_ = new ExprNode(Token_Type::TOKEN_NULL, term);
                break;
            }
            case Term_Type::TERM_BOOL:
            {
                auto term = std::make_shared<TermExpr>(Term_Type::TERM_BOOL);
                is >> term->bval_;
                expr->next_expr_ = new ExprNode(Token_Type::TOKEN_NULL, term);
                break;
            }
            default:
                // TODO: other type processing
                break;
            }
        }
        expr = expr->next_expr_;
    }
    auto tmp = head->next_expr_;
    delete head;
    return tmp;
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