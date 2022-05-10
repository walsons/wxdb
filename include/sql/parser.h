/**************************************
 ********** semantic parsing **********
 **************************************/
#ifndef PARSER_H_
#define PARSER_H_

#include "token.h"
#include "tokenizer.h"
#include "expression.h"
#include <string>
#include <memory>

enum class Parser_State_Type
{
    PARSER_WRONG,
    PARSER_CORRECT,
    PARSER_RESERVED_WORD
};

class Parser
{
public:
    Parser(std::shared_ptr<Tokenizer> tokenizer);
    virtual ~Parser();
    std::shared_ptr<Token> ParseNextToken();
    std::shared_ptr<Token> ParseEatToken();
    std::shared_ptr<Token> ParseEatAndNextToken();
    void ParseError(const std::string &message);
    bool MatchToken(Token_Type type, const std::string &text);

    // Parsing expression by using recursive descent approach
    ExprNode *ParseExpressionRD();
    ExprNode *ParseReadBooleanOr();
    ExprNode *ParseReadBooleanAnd();
    ExprNode *ParseReadBooleanEquality();
    ExprNode *ParseReadBooleanComparison();
    ExprNode *ParseReadExpr();
    ExprNode *ParseReadTerm();
    ExprNode *ParseReadPower();
    ExprNode *ParseReadUnary();
    ExprNode *ParseReadParen();
    ExprNode *ParseReadBuiltin();
    ExprNode *ParseReadLiteral();

public:
    std::shared_ptr<Tokenizer> tokenizer_;
    std::shared_ptr<Token> curr_token_;
    Parser_State_Type parser_state_type_;
    std::string parser_message_;

private:
    ExprNode *concatenate_expr_node(ExprNode *expr1, ExprNode *expr2);
};

#endif