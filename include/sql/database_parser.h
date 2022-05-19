#ifndef DATABASE_PARSER_H_
#define DATABASE_PARSER_H_

#include "parser.h"

struct DatabaseInfo
{
    std::string database_name;
};

class DatabaseParser : public Parser
{
public:
    using Parser::Parser;
    ~DatabaseParser() = default;
    std::shared_ptr<DatabaseInfo> CreateDatabase()
    {
        auto info = std::make_shared<DatabaseInfo>();
        if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "create")) { return nullptr; }
        if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "database")) { return nullptr; }
        std::shared_ptr<Token> token = ParseNextToken();
        if (token->type_ == Token_Type::TOKEN_WORD) { info->database_name = token->text_; }
        else
        {
            ParseError("invalid SQL: missing database name!");
            return nullptr;
        }
        return info;
    }
};

#endif