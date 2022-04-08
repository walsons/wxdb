/**************************************
 ********** semantic parsing **********
 **************************************/
#ifndef PARSER_H_
#define PARSER_H_

#include "token.h"
#include "tokenizer.h"
#include <string>

enum class Parser_State_Type
{
    PARSER_WRONG,
    PARSER_CORRECT,
    PARSER_RESERVED_WORD
};

class Parser
{
public:
    Parser(Tokenizer *tokenizer);
    virtual ~Parser();
    Token *ParseNextToken();
    Token *ParseEatToken();
    Token *ParseEatAndNextToken();
    void ParseError(const std::string &message);
    bool MatchToken(Token_Type type, const std::string &text);

public:
    Tokenizer *tokenizer_;
    Token *curr_token_;
    Parser_State_Type parser_state_type_;
    std::string parser_message_;
};

#endif