#ifndef DATABASE_PARSER_H_
#define DATABASE_PARSER_H_

#include "parser.h"

class DatabaseParser : public Parser
{
public:
    using Parser::Parser;
    ~DatabaseParser() = default;

    std::string CreateDatabase()
    {
        if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "create")) { return nullptr; }
        if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "database")) { return nullptr; }
        std::shared_ptr<Token> token = ParseNextToken();
        if (token->type_ == Token_Type::TOKEN_WORD)
            return token->text_;
        else
            ParseError("invalid SQL: missing database name!");
        return nullptr;
    }
    
    std::string UseDatabase()
    {
        if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "use")) { return nullptr; }
        std::shared_ptr<Token> token = ParseNextToken();
        if (token->type_ == Token_Type::TOKEN_WORD)
            return token->text_;
        else
            ParseError("invalid SQL: missing database name!");
        return nullptr;
    }
};

#endif