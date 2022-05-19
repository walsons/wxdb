#ifndef EXPRESSION_H_
#define EXPRESSION_H_

#include "token.h"
#include "literal.h"
#include "common.h"
#include <memory>
#include <sstream>

enum class Func_Type
{
    FUNC_MAX,
    FUNC_MIN,
    FUNC_COUNT,
    FUNC_AVG,
    FUNC_SUM
};

class ColumnRef
{
public:
    ColumnRef(const std::string &all_name);
    ~ColumnRef();
    std::string table_name_;
    std::string column_name_;
    std::string column_alias_;
    std::string all_name_;
};

class Func
{
public:
    Func() = default;
    ~Func() = default;
    // Function type
    Func_Type type_;
    // std::shared_ptr<ExprNode> expr;
    std::shared_ptr<ColumnRef> col_ref_;
};

enum class Term_Type : char
{
    TERM_INT,
    TERM_DOUBLE,
    TERM_BOOL,
    TERM_DATE,
    TERM_COL_REF,
    TERM_STRING,
    TERM_NULL
    // TERM_ID,
    // TERM_LITERAL,
    // TERM_FUNC
};

class TermExpr
{
public:
    TermExpr(Term_Type term_type = Term_Type::TERM_NULL) : term_type_(term_type) {}
    ~TermExpr();
    Term_Type term_type_;
    union
    {
        int ival_;
        double dval_;
        bool bval_;
        int tval_;  // TERM_DATE
        std::string sval_;  // TERM_STRING
        // Identifier, table name...
        // std::string id_;
        // Literal
        // Literal *literal_;
        // Field variable
        ColumnRef *ref_;
        void *null_;
        // Function
        // Func func_;
    };
};

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
    Expression(ExprNode *expr);
    //Evaluate Reverse Polish Notation 
    void Eval(ExprNode *expr);
    // Store expr_node linklist in ostringstream
    static void DumpExprNode(std::ostringstream &os, ExprNode *expr);
    // Load expr_node linklist
    static ExprNode *LoadExprNode(std::istringstream &is);
    // Free expr_node linklist
    static void FreeExprNode(ExprNode *expr);

    // DataValue value;
    std::shared_ptr<TermExpr> term_;
private:
    std::shared_ptr<TermExpr> EvalOperator(ExprNode *op, std::shared_ptr<TermExpr> term1,
                                                   std::shared_ptr<TermExpr> term2 = nullptr);
    // ExprNode *EvalOperator(ExprNode *op, ExprNode *expr1, ExprNode *expr2 = nullptr);
};

#endif