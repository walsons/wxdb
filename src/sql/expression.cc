#include "../../include/sql/expression.h"
#include <cmath>
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

TermExpr::~TermExpr()
{
    // switch (term_type_)
    // {
    // TODO
    // case Term_Type::TERM_ID:
    //     id_.~basic_string();
    //     break;
    // case Term_Type::TERM_LITERAL:
    //     delete literal_;
    //     break;
    // case Term_Type::TERM_COL_REF:
    //     delete ref_;
    //     break;
    // case Term_Type::TERM_FUNC:
    //     func_.~Func();
    //     break;
    // default:
    //     break;
    // }
}

Expression::Expression(ExprNode *expr)
{
    Eval(expr);
}

void Expression::Eval(ExprNode *expr)
{
    // TODO: add type check
    assert(expr != nullptr);
    std::stack<std::shared_ptr<TermExpr>> term_stack;
    term_stack.push(expr->term_);
    expr = expr->next_expr_;
    while (expr != nullptr)
    {
        // If not a operator exprnode
        if (expr->operator_type_ == Operator_Type::NONE)
        {
            term_stack.push(expr->term_);
            expr = expr->next_expr_;
        }
        else
        {
            auto cur_expr = expr;
            expr = expr->next_expr_;
            auto term2 = term_stack.top();
            term_stack.pop();
            // Unary operator
            if (cur_expr->operator_type_ > Operator_Type::UNARY_DELIMETER)
            {
                auto term = EvalOperator(cur_expr, term2);
                term_stack.push(term);
            }
            // Binary operator
            else
            {
                auto term1 = term_stack.top();
                term_stack.pop();
                auto term = EvalOperator(cur_expr, term1, term2);
                term_stack.push(term);
            }
        }
    }
    term_ = term_stack.top();
}

void Expression::DumpExprNode(std::ostringstream &os, ExprNode *expr)
{
    // is_operator Term_Type value is_operator Token_Type placeholder...
    // Operator_Type(if not a operator) Term_Type value Operator_Type(if is a operator) 
    // TODO: currently support column_ref > >= == <= < int double bool
    while (expr != nullptr)
    {
        os << static_cast<int>(expr->operator_type_) << " ";
        if (expr->operator_type_ == Operator_Type::NONE)
        {
            os << static_cast<int>(expr->term_->term_type_) << " ";
            switch (expr->term_->term_type_)
            {
            case Term_Type::TERM_INT:
                os << expr->term_->ival_ << " ";
                break;
            case Term_Type::TERM_DOUBLE:
                os << expr->term_->ival_ << " ";
                break;
            case Term_Type::TERM_BOOL:
                os << expr->term_->ival_ << " ";
                break;
            case Term_Type::TERM_DATE:
                os << expr->term_->tval_ << " ";
                break;
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
            case Term_Type::TERM_STRING:
                os << expr->term_->sval_ << " ";
                break;
            case Term_Type::TERM_NULL:
                // No need add any bytes
                break;
            }
        }
        expr = expr->next_expr_;
    }
}

ExprNode *Expression::LoadExprNode(std::istringstream &is)
{
    int operator_type = 0;
    ExprNode *head = new ExprNode(Operator_Type::NONE);
    ExprNode *expr = head;
    while (is >> operator_type)
    {
        if (static_cast<Operator_Type>(operator_type) == Operator_Type::NONE)
        {
            expr->next_expr_ = new ExprNode(Operator_Type::NONE);
            expr = expr->next_expr_;
            int term_type = 0;
            is >> term_type;
            expr->term_->term_type_ = static_cast<Term_Type>(term_type);
            switch (expr->term_->term_type_)
            {
            case Term_Type::TERM_INT:
                is >> expr->term_->ival_;
                break;
            case Term_Type::TERM_DOUBLE:
                is >> expr->term_->ival_;
                break;
            case Term_Type::TERM_BOOL:
                is >> expr->term_->ival_;
                break;
            case Term_Type::TERM_DATE:
                is >> expr->term_->tval_;
                break;
            case Term_Type::TERM_COL_REF:
            {
                int tmp = 0;
                is >> tmp;
                if (tmp == 2)
                {
                    is >> expr->term_->ref_->table_name_;
                    is >> expr->term_->ref_->column_name_;
                }
                else
                {
                    is >> expr->term_->ref_->column_name_;
                }
                break;
            }
            case Term_Type::TERM_STRING:
                is >> expr->term_->sval_;
                break;
            case Term_Type::TERM_NULL:
                // No need add any bytes
                break;
            }
        }
        else
        {
            expr->operator_type_ = static_cast<Operator_Type>(operator_type);
        }
    }
    auto tmp = head->next_expr_;
    delete head;
    return tmp;
}

std::shared_ptr<TermExpr> Expression::EvalOperator(ExprNode *op, std::shared_ptr<TermExpr> term1,
                                                   std::shared_ptr<TermExpr> term2)
{
    // TODO: check type is match
    std::shared_ptr<TermExpr> res = nullptr;
    switch (op->operator_type_)
    {
    // Unary operator
    case Operator_Type::NOT:
    {
        res = std::make_shared<TermExpr>(Term_Type::TERM_BOOL);
        res->bval_ = !term1->bval_;
        break;
    }
    case Operator_Type::POSITIVE:
    {
        if (term1->term_type_ == Term_Type::TERM_INT)
        {
            res = std::make_shared<TermExpr>(Term_Type::TERM_INT);
            res->ival_ = term1->ival_;
        }
        else if (term1->term_type_ == Term_Type::TERM_DOUBLE)
        {
            res = std::make_shared<TermExpr>(Term_Type::TERM_DOUBLE);
            res->dval_ = res->dval_;
        }
        break;
    }
    case Operator_Type::NEGATIVE:
    {
        if (term1->term_type_ == Term_Type::TERM_INT)
        {
            res = std::make_shared<TermExpr>(Term_Type::TERM_INT);
            res->ival_ = -term1->ival_;
        }
        else if (term1->term_type_ == Term_Type::TERM_DOUBLE)
        {
            res = std::make_shared<TermExpr>(Term_Type::TERM_DOUBLE);
            res->dval_ = -res->dval_;
        }
        break;
    }
    // Binary operator
    case Operator_Type::OR:
        res = std::make_shared<TermExpr>(Term_Type::TERM_BOOL);
        res->bval_ = (term1->bval_ || term2->bval_);
        break;
    case Operator_Type::AND:
        res = std::make_shared<TermExpr>(Term_Type::TERM_BOOL);
        res->bval_ = (term1->bval_ && term2->bval_);
        break;
    case Operator_Type::EQ:
        res = std::make_shared<TermExpr>(Term_Type::TERM_BOOL);
        res->bval_ = (term1->bval_ == term2->bval_);
        break;
    case Operator_Type::NOT_EQ:
        res = std::make_shared<TermExpr>(Term_Type::TERM_BOOL);
        res->bval_ = (term1->bval_ != term2->bval_);
        break;
    case Operator_Type::GE:
        res = std::make_shared<TermExpr>(Term_Type::TERM_BOOL);
        res->bval_ = (term1->bval_ >= term2->bval_);
        break;
    case Operator_Type::LE:
        res = std::make_shared<TermExpr>(Term_Type::TERM_BOOL);
        res->bval_ = (term1->bval_ <= term2->bval_);
        break;
    case Operator_Type::GT:
        res = std::make_shared<TermExpr>(Term_Type::TERM_BOOL);
        res->bval_ = (term1->bval_ > term2->bval_);
        break;
    case Operator_Type::LT:
        res = std::make_shared<TermExpr>(Term_Type::TERM_BOOL);
        res->bval_ = (term1->bval_ < term2->bval_);
        break;
    case Operator_Type::PLUS:
        if (term1->term_type_ == Term_Type::TERM_INT || term2->term_type_ == Term_Type::TERM_INT)
        {
            res = std::make_shared<TermExpr>(Term_Type::TERM_INT);
            res->ival_ = term1->ival_ + term2->ival_;
        }
        else if (term1->term_type_ == Term_Type::TERM_DOUBLE || term2->term_type_ == Term_Type::TERM_DOUBLE)
        {
            res = std::make_shared<TermExpr>(Term_Type::TERM_DOUBLE);
            res->dval_ = term1->dval_ + term2->dval_;
        }
        // TODO: int plus double
        break;
    case Operator_Type::MINUS:
        if (term1->term_type_ == Term_Type::TERM_INT || term2->term_type_ == Term_Type::TERM_INT)
        {
            res = std::make_shared<TermExpr>(Term_Type::TERM_INT);
            res->ival_ = term1->ival_ - term2->ival_;
        }
        else if (term1->term_type_ == Term_Type::TERM_DOUBLE || term2->term_type_ == Term_Type::TERM_DOUBLE)
        {
            res = std::make_shared<TermExpr>(Term_Type::TERM_DOUBLE);
            res->dval_ = term1->dval_ - term2->dval_;
        }
        break;
    case Operator_Type::MULTIPLY:
        if (term1->term_type_ == Term_Type::TERM_INT || term2->term_type_ == Term_Type::TERM_INT)
        {
            res = std::make_shared<TermExpr>(Term_Type::TERM_INT);
            res->ival_ = term1->ival_ * term2->ival_;
        }
        else if (term1->term_type_ == Term_Type::TERM_DOUBLE || term2->term_type_ == Term_Type::TERM_DOUBLE)
        {
            res = std::make_shared<TermExpr>(Term_Type::TERM_DOUBLE);
            res->dval_ = term1->dval_ * term2->dval_;
        }
        break;
    case Operator_Type::DIVIDE:
        if (term1->term_type_ == Term_Type::TERM_INT || term2->term_type_ == Term_Type::TERM_INT)
        {
            res = std::make_shared<TermExpr>(Term_Type::TERM_INT);
            res->ival_ = term1->ival_ / term2->ival_;
        }
        else if (term1->term_type_ == Term_Type::TERM_DOUBLE || term2->term_type_ == Term_Type::TERM_DOUBLE)
        {
            res = std::make_shared<TermExpr>(Term_Type::TERM_DOUBLE);
            res->dval_ = term1->dval_ / term2->dval_;
        }
        break;
    case Operator_Type::POWER:
        if (term1->term_type_ == Term_Type::TERM_INT || term2->term_type_ == Term_Type::TERM_INT)
        {
            res = std::make_shared<TermExpr>(Term_Type::TERM_INT);
            res->ival_ = std::pow(term1->ival_, term2->ival_);
        }
        break;
    }
    return res;
}

void Expression::FreeExprNode(ExprNode *expr)
{
    if (expr == nullptr) { return; }
    while (expr->next_expr_ != nullptr)
    {
        auto tmp = expr;
        expr = expr->next_expr_;
        delete tmp;
    }
    delete expr;
}