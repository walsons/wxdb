#ifndef EXPRESSION_H_
#define EXPRESSION_H_

#include "token.h"
#include "literal.h"

enum class Term_Type
{
    TERM_UNKNOWN,
    TERM_LITERAL,
    TERM_ID,
    TERM_NULL,
    TERM_COL_REF,
    TERM_FUNC
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


    };
};

class Expression
{
public:
    Token_Type operator_type;

};

#endif