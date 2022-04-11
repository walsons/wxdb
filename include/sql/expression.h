#ifndef EXPRESSION_H_
#define EXPRESSION_H_

#include "token.h"
#include "literal.h"

enum class Func_Type
{
    FUNC_MAX,
    FUNC_MIN,
    FUNC_COUNT,
    FUNC_AVG,
    FUNC_SUM
};

class Expression;

class Func
{
public:
    // Function type
    Func_Type type_;
    Expression *expr;
};

enum class Term_Type
{
    TERM_UNKNOWN,
    TERM_LITERAL,
    TERM_ID,
    TERM_NULL,
    TERM_COL_REF,
    TERM_FUNC
};

class ColumnRef
{
public:
    ColumnRef(const std::string &table_name, const std::string &column_name);
    ColumnRef(const std::string &all_name);
    ~ColumnRef();
    std::string table_name_;
    std::string column_name_;
    std::string column_alias_;
    std::string all_name_;
};

class TermExpr
{
public:
    TermExpr(Term_Type term_type);
    ~TermExpr();
    Term_Type term_type_;
    union
    {
        // Identifier, table name...
        std::string id;
        // Literal
        Literal *val;
        // Field variable
        ColumnRef *ref;
        // Function
        Func func;
    };
};

class Expression
{
public:
    Expression(Token_Type operator_type, Expression *next_expr);
    ~Expression();

    Token_Type operator_type_;
    Term_Type *term_;
    // It might have alias when appears in select statement
    std::string alias_;
    // Linking expression via link list
    Expression *next_expr_;
};

#endif