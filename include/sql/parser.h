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
#include <iostream>

class Parser
{
public:
    Parser(std::shared_ptr<Tokenizer> tokenizer);
    virtual ~Parser();
    void Reset();
    std::shared_ptr<Token> ParseNextToken();
    std::shared_ptr<Token> ParseEatAndNextToken();
    template <typename T>
    T ParseError(const std::string &message)
    {
        error_message_ = message;
        return nullptr;
    }
    bool PrintError()
    {
        if (!error_message_.empty())
        {
            std::cout << error_message_ << std::endl;
            error_message_.clear();
            return true;
        }
        return false;
    }
    bool MatchToken(Token_Type type);
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
    std::string error_message_;

private:
    ExprNode *concatenate_expr_node(ExprNode *expr1, ExprNode *expr2);
};

#endif