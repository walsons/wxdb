#ifndef EXPRESSION_H_
#define EXPRESSION_H_

#include "../defs.h"
#include "token.h"
#include <memory>
#include <sstream>
#include <unordered_map>

// enum class Func_Type
// {
//     FUNC_MAX,
//     FUNC_MIN,
//     FUNC_COUNT,
//     FUNC_AVG,
//     FUNC_SUM
// };

struct ColumnRef
{
    ColumnRef() = default;
    ColumnRef(const std::string &col_name, const std::string &tname = "")
        : table_name(tname), column_name(col_name) {}
    ~ColumnRef() = default;
    std::string all_name() const { return table_name.empty() ? column_name : (table_name + "." + column_name); }
    std::string table_name;
    std::string column_name;
};

// class Func
// {
// public:
//     Func() = default;
//     ~Func() = default;
//     // Function type
//     Func_Type type_;
//     // std::shared_ptr<ExprNode> expr;
//     std::shared_ptr<ColumnRef> col_ref_;
// };

enum class Term_Type : char
{
    TERM_INT,
    TERM_DOUBLE,
    TERM_BOOL,
    TERM_DATE,
    TERM_COL_REF,
    TERM_STRING,
    TERM_NULL
};

class TermExpr
{
public:
    TermExpr();
    TermExpr(const int &ival);
    TermExpr(const double &dval);
    TermExpr(const bool &bval);
    TermExpr(const Date &tval);
    TermExpr(const char *sval);
    TermExpr(const std::string &sval);
    TermExpr(const ColumnRef &ref);
    TermExpr(const TermExpr &term);

    void set_null();
    TermExpr &operator=(const int &ival);
    TermExpr &operator=(const double &dval);
    TermExpr &operator=(const bool &bval);
    TermExpr &operator=(const Date &tval);
    TermExpr &operator=(const char *sval);
    TermExpr &operator=(const std::string &sval);
    TermExpr &operator=(const ColumnRef &ref);
    TermExpr &operator=(const TermExpr &term);

    ~TermExpr();

    Term_Type term_type_;
    union
    {
        int ival_;
        double dval_;
        bool bval_;
        Date tval_;  // TERM_DATE
        std::string sval_;  // TERM_STRING
        ColumnRef ref_;
    };

private:
    void destory_class_member();
};

std::ostream &operator<<(std::ostream &os, const TermExpr &term);


class ExprNode
{
public:
    ExprNode(Operator_Type operator_type, 
             std::shared_ptr<TermExpr> term = nullptr, 
             ExprNode *next_expr = nullptr)
        : operator_type_(operator_type), term_(term), next_expr_(next_expr) {}
    ~ExprNode() = default;

    Operator_Type operator_type_;
    std::shared_ptr<TermExpr> term_;
    // Linking expression via link list
    ExprNode *next_expr_;
};

class Expression
{
public:
    Expression(ExprNode *expr, std::unordered_map<std::string, std::shared_ptr<TermExpr>> col_real_term = std::unordered_map<std::string, std::shared_ptr<TermExpr>>{});
    //Evaluate Reverse Polish Notation 
    void Eval(ExprNode *expr, std::unordered_map<std::string, std::shared_ptr<TermExpr>> col_real_term = std::unordered_map<std::string, std::shared_ptr<TermExpr>>{});
    // Store expr_node linklist in ostringstream
    static void DumpExprNode(std::ostringstream &os, ExprNode *expr);
    // Load expr_node linklist
    static ExprNode *LoadExprNode(std::istringstream &is);
    // Free expr_node linklist
    static void FreeExprNode(ExprNode *expr);

    TermExpr term_;
private:
    std::shared_ptr<TermExpr> EvalOperator(ExprNode *op, std::shared_ptr<TermExpr> term1,
                                                   std::shared_ptr<TermExpr> term2 = nullptr);
    // ExprNode *EvalOperator(ExprNode *op, ExprNode *expr1, ExprNode *expr2 = nullptr);
};

#endif