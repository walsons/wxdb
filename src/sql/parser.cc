#include "../../include/sql/parser.h"
#include <memory>

Parser::Parser(std::shared_ptr<Tokenizer> tokenizer)
    : tokenizer_(tokenizer)
    , curr_token_(nullptr)
    , parser_state_type_(Parser_State_Type::PARSER_CORRECT)
{
}

Parser::~Parser() = default;

// Get next token, if current token doesn't be eat, return current token
std::shared_ptr<Token> Parser::ParseNextToken()
{
    if (parser_state_type_ == Parser_State_Type::PARSER_WRONG) { return nullptr; }
    if (curr_token_ == NULL) 
    {
        curr_token_ = tokenizer_->GetNextToken();
    }
    return curr_token_;
}

// Eat current token
std::shared_ptr<Token> Parser::ParseEatToken()
{
    if (parser_state_type_ == Parser_State_Type::PARSER_WRONG) { return nullptr; }
    curr_token_ = nullptr;
    return nullptr;
}

// Eat current token then return next token
std::shared_ptr<Token> Parser::ParseEatAndNextToken()
{
    if (parser_state_type_ == Parser_State_Type::PARSER_WRONG) { return nullptr; }
    curr_token_ = tokenizer_->GetNextToken();
    return curr_token_;
}

void Parser::ParseError(const std::string &message)
{
    parser_state_type_ = Parser_State_Type::PARSER_WRONG;
    parser_message_ = message;
}

bool Parser::MatchToken(Token_Type type, const std::string &text)
{
    std::shared_ptr<Token> token = ParseNextToken();
    if (token && token->type_ == type && token->text_ == text)
    {
        ParseEatAndNextToken();
        return true;
    }
    return false;
}

/******************************************************
 * Parsing expression by using recursive descent approach
 * 
 * or 
 * and 
 * equality: equal | not equal 
 * comparison: GE | LE | GT | LT
 * expr: plus | mnus
 * term: mutiply | divide
 * power
 * unary: not | plus | minus (positive negative)
 * paren: open_paren | close_paren ------
 * ---> or
 * ---> read_builtin: word | func | identifier | mutiply (wildcard)
 *      literal: float | decimal | exp_float | string | char 
 ******************************************************/ 

Expression *Parser::ParseExpressionRD()
{
    Expression *expr = ParseReadBooleanOr();
}

Expression *Parser::ParseReadBooleanOr()
{

}