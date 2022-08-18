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
        auto database_info = std::make_shared<DatabaseInfo>();
        if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "create")) { return nullptr; }
        if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "database")) { return nullptr; }
        std::shared_ptr<Token> token = ParseNextToken();
        if (token->type_ == Token_Type::TOKEN_WORD)
        {
            database_info->database_name = token->text_;
            ParseEatAndNextToken();
        }
        else
            return ParseError<std::shared_ptr<DatabaseInfo>>("invalid SQL: missing database name!");
        if (!MatchToken(Token_Type::TOKEN_SEMICOLON))
            return ParseError<std::shared_ptr<DatabaseInfo>>("invalid SQL: expect \";\"");
        return database_info;
    }
    
    std::shared_ptr<DatabaseInfo> UseDatabase()
    {
        auto database_info = std::make_shared<DatabaseInfo>();
        if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "use")) { return nullptr; }
        std::shared_ptr<Token> token = ParseNextToken();
        if (token->type_ == Token_Type::TOKEN_WORD)
        {
            database_info->database_name = token->text_;
            ParseEatAndNextToken();
        }
        else
            return ParseError<std::shared_ptr<DatabaseInfo>>("invalid SQL: missing database name!");
        if (!MatchToken(Token_Type::TOKEN_SEMICOLON))
            return ParseError<std::shared_ptr<DatabaseInfo>>("invalid SQL: expect \";\"");
        return database_info;
    }
};

#endif