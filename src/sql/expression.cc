#include "../../include/sql/expression.h"
#include <cmath>
#include <cassert>
#include <stack>
#include <unordered_map>
#include <iostream>

TermExpr::TermExpr() : term_type_(Term_Type::TERM_NULL) {}

TermExpr::TermExpr(const int &ival) 
    : term_type_(Term_Type::TERM_INT), ival_(ival) {}

TermExpr::TermExpr(const double &dval)
    : term_type_(Term_Type::TERM_DOUBLE), dval_(dval) {}

TermExpr::TermExpr(const bool &bval)
    : term_type_(Term_Type::TERM_BOOL), bval_(bval) {}

TermExpr::TermExpr(const Date &tval)
    : term_type_(Term_Type::TERM_DATE), tval_(tval) {}

TermExpr::TermExpr(const char *sval)
    : term_type_(Term_Type::TERM_STRING)
{
    new (&sval_) std::string(sval);
}

TermExpr::TermExpr(const std::string &sval)
    : term_type_(Term_Type::TERM_STRING)
{
    new (&sval_) std::string(sval);
}

TermExpr::TermExpr(const ColumnRef &ref)
    : term_type_(Term_Type::TERM_COL_REF)
{
    new (&ref_) ColumnRef(ref.column_name, ref.table_name);
}

TermExpr::TermExpr(const TermExpr &term)
{
    if (this != &term)
    {
        this->term_type_ = term.term_type_;
        switch (term.term_type_)
        {
        case Term_Type::TERM_NULL:
            // Nothing need to do
            break;
        case Term_Type::TERM_INT:
            ival_ = term.ival_;
            break;
        case Term_Type::TERM_DOUBLE:
            dval_ = term.dval_;
            break;
        case Term_Type::TERM_BOOL:
            bval_ = term.bval_;
            break;
        case Term_Type::TERM_DATE:
            tval_ = term.tval_;
            break;
        case Term_Type::TERM_STRING:
            new (&this->sval_) std::string(term.sval_);
            break;
        case Term_Type::TERM_COL_REF:
            new (&this->ref_) ColumnRef(term.ref_.column_name, term.ref_.table_name);
            break;
        }
    }
}

void TermExpr::set_null()
{
    destory_class_member();
    term_type_ = Term_Type::TERM_NULL;
}

TermExpr &TermExpr::operator=(const int &ival)
{
    destory_class_member();
    term_type_ = Term_Type::TERM_INT;
    ival_ = ival;
    return *this;
}

TermExpr &TermExpr::operator=(const double &dval)
{
    destory_class_member();
    term_type_ = Term_Type::TERM_DOUBLE;
    dval_ = dval;
    return *this;
}

TermExpr &TermExpr::operator=(const bool &bval)
{
    destory_class_member();
    term_type_ = Term_Type::TERM_BOOL;
    bval_ = bval;
    return *this;
}

TermExpr &TermExpr::operator=(const Date &tval)
{
    destory_class_member();
    term_type_ = Term_Type::TERM_DATE;
    tval_ = tval;
    return *this;
}

TermExpr &TermExpr::operator=(const char *sval)
{
    destory_class_member();
    term_type_ = Term_Type::TERM_STRING;
    new (&sval_) std::string(sval);
    return *this;
}

TermExpr &TermExpr::operator=(const std::string &sval)
{
    destory_class_member();
    term_type_ = Term_Type::TERM_STRING;
    new (&sval_) std::string(sval);
    return *this;
}

TermExpr &TermExpr::operator=(const ColumnRef &ref)
{
    destory_class_member();
    term_type_ = Term_Type::TERM_COL_REF;
    new (&ref_) ColumnRef(ref.column_name, ref.table_name);
    return *this;
}

TermExpr &TermExpr::operator=(const TermExpr &term)
{
    if (this != &term)
    {
        switch (term.term_type_)
        {
        case Term_Type::TERM_NULL:
            // Nothing need to do
            break;
        case Term_Type::TERM_INT:
            *this = term.ival_;
            break;
        case Term_Type::TERM_DOUBLE:
            *this = term.dval_;
            break;
        case Term_Type::TERM_BOOL:
            *this = term.bval_;
            break;
        case Term_Type::TERM_DATE:
            *this = term.tval_;
            break;
        case Term_Type::TERM_STRING:
            *this = term.sval_;
            break;
        case Term_Type::TERM_COL_REF:
            *this = term.ref_;
            break;
        }
    }
    return *this;
}

TermExpr::~TermExpr()
{
    destory_class_member();
}

void TermExpr::destory_class_member()
{
    switch (term_type_)
    {
    case Term_Type::TERM_STRING:
        sval_.~basic_string();
        break;
    case Term_Type::TERM_COL_REF:
        ref_.~ColumnRef();
        break;
    default:
        break;
    }
}

std::ostream &operator<<(std::ostream &os, const TermExpr &term)
{
    switch (term.term_type_)
    {
    case Term_Type::TERM_NULL:
        os << "NULL";
        break;
    case Term_Type::TERM_INT:
        os << term.ival_;
        break;
    case Term_Type::TERM_DOUBLE:
        os << term.dval_;
        break;
    case Term_Type::TERM_BOOL:
        os << (term.bval_ ? "true" : "false");
        break;
    case Term_Type::TERM_DATE:
        os << term.tval_.timestamp2str();
        break;
    case Term_Type::TERM_STRING:
        os << term.sval_;
        break;
    case Term_Type::TERM_COL_REF:
        os << term.ref_.all_name();
        break;
    }
    return os;
}

bool operator==(const TermExpr &term1, const TermExpr &term2)
{
    // Type of double and int compatible cases
    if (term1.term_type_ == Term_Type::TERM_INT && term2.term_type_ == Term_Type::TERM_DOUBLE)
    {
        if (static_cast<double>(term1.ival_) == term2.dval_) { return true; }
        return false;
    }
    else if (term1.term_type_ == Term_Type::TERM_DOUBLE && term2.term_type_ == Term_Type::TERM_INT)
    {
        if (term1.dval_ == static_cast<double>(term2.ival_)) { return true; }
        return false;
    }
    // Other type cases
    if (term1.term_type_ != term2.term_type_) { return false; }  // exception case
    switch (term1.term_type_)
    {
    case Term_Type::TERM_INT:
        return term1.ival_ == term2.ival_;
        break;
    case Term_Type::TERM_DOUBLE:
        return term1.dval_ == term2.dval_;
        break;
    case Term_Type::TERM_NULL:
        return true;
        break;
    case Term_Type::TERM_BOOL:
        return term1.bval_ == term2.bval_;
        break;
    case Term_Type::TERM_DATE:
        return term1.tval_.timestamp == term2.tval_.timestamp;
        break;
    case Term_Type::TERM_STRING:
        return term1.sval_ == term2.sval_;
        break;
    case Term_Type::TERM_COL_REF:  // exception case
        return false;
        break;
    }
    return false;
}

bool operator>(const TermExpr &term1, const TermExpr &term2)
{
    // Type of double and int compatible cases
    if (term1.term_type_ == Term_Type::TERM_INT && term2.term_type_ == Term_Type::TERM_DOUBLE)
    {
        if (static_cast<double>(term1.ival_) > term2.dval_) { return true; }
        return false;
    }
    else if (term1.term_type_ == Term_Type::TERM_DOUBLE && term2.term_type_ == Term_Type::TERM_INT)
    {
        if (term1.dval_ > static_cast<double>(term2.ival_)) { return true; }
        return false;
    }
    // Other type cases
    if (term1.term_type_ != term2.term_type_) { return false; }  // exception case
    switch (term1.term_type_)
    {
    case Term_Type::TERM_INT:
        return term1.ival_ > term2.ival_;
        break;
    case Term_Type::TERM_DOUBLE:
        return term1.dval_ > term2.dval_;
        break;
    case Term_Type::TERM_NULL:
        return false;
        break;
    case Term_Type::TERM_BOOL:
        return false;  // exception case
        break;
    case Term_Type::TERM_DATE:
        return term1.tval_.timestamp > term2.tval_.timestamp;
        break;
    case Term_Type::TERM_STRING:
        return term1.sval_ > term2.sval_;
        break;
    case Term_Type::TERM_COL_REF:  // exception case
        return false;
        break;
    }
    return false;
}

bool operator<(const TermExpr &term1, const TermExpr &term2)
{
    // Type of double and int compatible cases
    if (term1.term_type_ == Term_Type::TERM_INT && term2.term_type_ == Term_Type::TERM_DOUBLE)
    {
        if (static_cast<double>(term1.ival_) < term2.dval_) { return true; }
        return false;
    }
    else if (term1.term_type_ == Term_Type::TERM_DOUBLE && term2.term_type_ == Term_Type::TERM_INT)
    {
        if (term1.dval_ < static_cast<double>(term2.ival_)) { return true; }
        return false;
    }
    // Other type cases
    if (term1.term_type_ != term2.term_type_) { return false; }  // exception case
    switch (term1.term_type_)
    {
    case Term_Type::TERM_INT:
        return term1.ival_ < term2.ival_;
        break;
    case Term_Type::TERM_DOUBLE:
        return term1.dval_ < term2.dval_;
        break;
    case Term_Type::TERM_NULL:
        return false;
        break;
    case Term_Type::TERM_BOOL:
        return false;  // exception case
        break;
    case Term_Type::TERM_DATE:
        return term1.tval_.timestamp < term2.tval_.timestamp;
        break;
    case Term_Type::TERM_STRING:
        return term1.sval_ < term2.sval_;
        break;
    case Term_Type::TERM_COL_REF:  // exception case
        return false;
        break;
    }
    return false;
}

bool operator>=(const TermExpr &term1, const TermExpr &term2)
{
    // Type of double and int compatible cases
    if (term1.term_type_ == Term_Type::TERM_INT && term2.term_type_ == Term_Type::TERM_DOUBLE)
    {
        if (static_cast<double>(term1.ival_) >= term2.dval_) { return true; }
        return false;
    }
    else if (term1.term_type_ == Term_Type::TERM_DOUBLE && term2.term_type_ == Term_Type::TERM_INT)
    {
        if (term1.dval_ >= static_cast<double>(term2.ival_)) { return true; }
        return false;
    }
    // Other type cases
    if (term1.term_type_ != term2.term_type_) { return false; }  // exception case
    switch (term1.term_type_)
    {
    case Term_Type::TERM_INT:
        return term1.ival_ >= term2.ival_;
        break;
    case Term_Type::TERM_DOUBLE:
        return term1.dval_ >= term2.dval_;
        break;
    case Term_Type::TERM_NULL:
        return false;
        break;
    case Term_Type::TERM_BOOL:
        return false;  // exception case
        break;
    case Term_Type::TERM_DATE:
        return term1.tval_.timestamp >= term2.tval_.timestamp;
        break;
    case Term_Type::TERM_STRING:
        return term1.sval_ >= term2.sval_;
        break;
    case Term_Type::TERM_COL_REF:  // exception case
        return false;
        break;
    }
    return false;
}

bool operator<=(const TermExpr &term1, const TermExpr &term2)
{
    // Type of double and int compatible cases
    if (term1.term_type_ == Term_Type::TERM_INT && term2.term_type_ == Term_Type::TERM_DOUBLE)
    {
        if (static_cast<double>(term1.ival_) <= term2.dval_) { return true; }
        return false;
    }
    else if (term1.term_type_ == Term_Type::TERM_DOUBLE && term2.term_type_ == Term_Type::TERM_INT)
    {
        if (term1.dval_ <= static_cast<double>(term2.ival_)) { return true; }
        return false;
    }
    // Other type cases
    if (term1.term_type_ != term2.term_type_) { return false; }  // exception case
    switch (term1.term_type_)
    {
    case Term_Type::TERM_INT:
        return term1.ival_ <= term2.ival_;
        break;
    case Term_Type::TERM_DOUBLE:
        return term1.dval_ <= term2.dval_;
        break;
    case Term_Type::TERM_NULL:
        return false;
        break;
    case Term_Type::TERM_BOOL:
        return false;  // exception case
        break;
    case Term_Type::TERM_DATE:
        return term1.tval_.timestamp <= term2.tval_.timestamp;
        break;
    case Term_Type::TERM_STRING:
        return term1.sval_ <= term2.sval_;
        break;
    case Term_Type::TERM_COL_REF:  // exception case
        return false;
        break;
    }
    return false;
}

bool operator!=(const TermExpr &term1, const TermExpr &term2)
{
    // Type of double and int compatible cases
    if (term1.term_type_ == Term_Type::TERM_INT && term2.term_type_ == Term_Type::TERM_DOUBLE)
    {
        if (static_cast<double>(term1.ival_) != term2.dval_) { return true; }
        return false;
    }
    else if (term1.term_type_ == Term_Type::TERM_DOUBLE && term2.term_type_ == Term_Type::TERM_INT)
    {
        if (term1.dval_ != static_cast<double>(term2.ival_)) { return true; }
        return false;
    }
    // Other type cases
    if (term1.term_type_ != term2.term_type_) { return false; }  // exception case
    switch (term1.term_type_)
    {
    case Term_Type::TERM_INT:
        return term1.ival_ != term2.ival_;
        break;
    case Term_Type::TERM_DOUBLE:
        return term1.dval_ != term2.dval_;
        break;
    case Term_Type::TERM_NULL:
        return false;
        break;
    case Term_Type::TERM_BOOL:
        return term1.bval_ != term2.bval_;
        break;
    case Term_Type::TERM_DATE:
        return term1.tval_.timestamp != term2.tval_.timestamp;
        break;
    case Term_Type::TERM_STRING:
        return term1.sval_ != term2.sval_;
        break;
    case Term_Type::TERM_COL_REF:  // exception case
        return false;
        break;
    }
    return false;
}

Expression::Expression(ExprNode *expr, std::unordered_map<std::string, std::shared_ptr<TermExpr>> col_real_term)
{
    Eval(expr, col_real_term);
}

void Expression::Eval(ExprNode *expr, std::unordered_map<std::string, std::shared_ptr<TermExpr>> col_real_term)
{
    // TODO: add type check
    assert(expr != nullptr);
    std::stack<std::shared_ptr<TermExpr>> term_stack;
    bool is_col_ref = false;
    std::shared_ptr<TermExpr> tmp_store;
    auto set_col = [&]() {
        if (expr->term_->term_type_ == Term_Type::TERM_COL_REF)
        {
            tmp_store = expr->term_;
            expr->term_ = col_real_term[expr->term_->ref_.all_name()];
            is_col_ref = true;
        }
    };
    auto restore_col = [&]() {
        if (is_col_ref)
        {
            expr->term_ = tmp_store;
            is_col_ref = false;
        }
    };
    while (expr != nullptr)
    {
        // If not a operator exprnode
        if (expr->operator_type_ == Operator_Type::NONE)
        {
            set_col();
            term_stack.push(expr->term_);
            restore_col();
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
    term_ = *term_stack.top();
}

void Expression::DumpExprNode(std::ostringstream &os, ExprNode *expr)
{
    // Operator_Type(if not a operator) Term_Type value 
    // Operator_Type(if is a operator) 
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
                os << expr->term_->tval_.timestamp << " ";
                break;
            case Term_Type::TERM_COL_REF:
                if (expr->term_->ref_.table_name.size() != 0)
                {
                    os << 2 << " " 
                       << expr->term_->ref_.table_name << " "
                       << expr->term_->ref_.column_name << " ";
                }
                else
                {
                    os << 1 << " " 
                       << expr->term_->ref_.column_name << " ";
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
            expr->next_expr_ = new ExprNode(Operator_Type::NONE, std::make_shared<TermExpr>());
            expr = expr->next_expr_;
            int term_type = 0;
            is >> term_type;
            // expr->term_->term_type_ = static_cast<Term_Type>(term_type);
            switch (static_cast<Term_Type>(term_type))
            {
            case Term_Type::TERM_INT:
            {
                int tmp;
                is >> tmp;
                *expr->term_ = tmp;
                break;
            }
            case Term_Type::TERM_DOUBLE:
            {
                double tmp;
                is >> tmp;
                *expr->term_ = tmp;
                break;
            }
            case Term_Type::TERM_BOOL:
            {
                bool tmp;
                is >> tmp;
                *expr->term_ = tmp;
                break;
            }
            case Term_Type::TERM_DATE:
            {
                Date tmp;
                is >> tmp.timestamp;
                *expr->term_ = tmp;
                break;
            }
            case Term_Type::TERM_COL_REF:
            {
                ColumnRef ref;
                int tmp = 0;
                is >> tmp;
                if (tmp == 2) { is >> ref.table_name >> ref.column_name; }
                else { is >> ref.column_name; }
                *expr->term_ = ref;
                break;
            }
            case Term_Type::TERM_STRING:
            {
                std::string tmp;
                is >> tmp;
                *expr->term_ = tmp;
                break;
            }
            case Term_Type::TERM_NULL:
                // No need add any bytes
                break;
            }
        }
        else
        {
            expr->next_expr_ = new ExprNode(static_cast<Operator_Type>(operator_type));
            expr = expr->next_expr_;
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
        res = std::make_shared<TermExpr>(!term1->bval_);
        break;
    }
    case Operator_Type::POSITIVE:
    {
        if (term1->term_type_ == Term_Type::TERM_INT)
        {
            res = std::make_shared<TermExpr>(term1->ival_);
        }
        else if (term1->term_type_ == Term_Type::TERM_DOUBLE)
        {
            res = std::make_shared<TermExpr>(res->dval_);
        }
        break;
    }
    case Operator_Type::NEGATIVE:
    {
        if (term1->term_type_ == Term_Type::TERM_INT)
        {
            res = std::make_shared<TermExpr>(-term1->ival_);
        }
        else if (term1->term_type_ == Term_Type::TERM_DOUBLE)
        {
            res = std::make_shared<TermExpr>(-res->dval_);
        }
        break;
    }
    // Binary operator
    case Operator_Type::OR:
        res = std::make_shared<TermExpr>(term1->bval_ || term2->bval_);
        break;
    case Operator_Type::AND:
        res = std::make_shared<TermExpr>(term1->bval_ && term2->bval_);
        break;
    case Operator_Type::EQ:
        res = std::make_shared<TermExpr>(*term1 == *term2);
        break;
    case Operator_Type::NOT_EQ:
        res = std::make_shared<TermExpr>(*term1 != *term2);
        break;
    case Operator_Type::GE:
        res = std::make_shared<TermExpr>(*term1 >= *term2);
        break;
    case Operator_Type::LE:
        res = std::make_shared<TermExpr>(*term1 <= *term2);
        break;
    case Operator_Type::GT:
        res = std::make_shared<TermExpr>(*term1 > *term2);
        break;
    case Operator_Type::LT:
        res = std::make_shared<TermExpr>(*term1 < *term2);
        break;
    case Operator_Type::PLUS:
        if (term1->term_type_ == Term_Type::TERM_INT || term2->term_type_ == Term_Type::TERM_INT)
        {
            res = std::make_shared<TermExpr>(term1->ival_ + term2->ival_);
        }
        else if (term1->term_type_ == Term_Type::TERM_DOUBLE || term2->term_type_ == Term_Type::TERM_DOUBLE)
        {
            res = std::make_shared<TermExpr>(term1->dval_ + term2->dval_);
        }
        // TODO: int plus double
        break;
    case Operator_Type::MINUS:
        if (term1->term_type_ == Term_Type::TERM_INT || term2->term_type_ == Term_Type::TERM_INT)
        {
            res = std::make_shared<TermExpr>(term1->ival_ - term2->ival_);
        }
        else if (term1->term_type_ == Term_Type::TERM_DOUBLE || term2->term_type_ == Term_Type::TERM_DOUBLE)
        {
            res = std::make_shared<TermExpr>(term1->dval_ - term2->dval_);
        }
        break;
    case Operator_Type::MULTIPLY:
        if (term1->term_type_ == Term_Type::TERM_INT || term2->term_type_ == Term_Type::TERM_INT)
        {
            res = std::make_shared<TermExpr>(term1->ival_ * term2->ival_);
        }
        else if (term1->term_type_ == Term_Type::TERM_DOUBLE || term2->term_type_ == Term_Type::TERM_DOUBLE)
        {
            res = std::make_shared<TermExpr>(term1->dval_ * term2->dval_);
        }
        break;
    case Operator_Type::DIVIDE:
        if (term1->term_type_ == Term_Type::TERM_INT || term2->term_type_ == Term_Type::TERM_INT)
        {
            res = std::make_shared<TermExpr>(term1->ival_ / term2->ival_);
        }
        else if (term1->term_type_ == Term_Type::TERM_DOUBLE || term2->term_type_ == Term_Type::TERM_DOUBLE)
        {
            res = std::make_shared<TermExpr>(term1->dval_ / term2->dval_);
        }
        break;
    case Operator_Type::MOD:
        if (term1->term_type_ == Term_Type::TERM_INT || term2->term_type_ == Term_Type::TERM_INT)
        {
            res = std::make_shared<TermExpr>(term1->ival_ % term2->ival_);
        }
        break;
    case Operator_Type::POWER:
        if (term1->term_type_ == Term_Type::TERM_INT || term2->term_type_ == Term_Type::TERM_INT)
        {
            res = std::make_shared<TermExpr>(std::pow(term1->ival_, term2->ival_));
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