#include "../../include/sql/insert_parser.h"
#include <cstring>

InsertParser::InsertParser(std::shared_ptr<Tokenizer> tokenizer) : Parser(tokenizer)
{
}

InsertParser::~InsertParser() = default;

std::shared_ptr<SQLStmtInsert> InsertParser::ParseSQLStmtInsert()
{
    std::string table_name;
    std::vector<std::string> fields_name;
    std::shared_ptr<std::vector<DataValue>> values;

    /*** select ***/
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "insert")) { return nullptr; }
    /*** into ***/
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "into")) 
    {
        ParseError("invalid SQL: should be \"into\"!");
        return nullptr;
    }
    /*** table name ***/
    auto token = ParseNextToken();
    if (token->type_ == Token_Type::TOKEN_WORD) { table_name = token->text_; }
    else
    {
        ParseError("invalid SQL: missing table name!");
        return nullptr;
    }
    /*** column ***/
    token = ParseEatAndNextToken();
    if (MatchToken(Token_Type::TOKEN_OPEN_PARENTHESIS, "("))
    {
        token = ParseNextToken();
        if (token->type_ == Token_Type::TOKEN_WORD)
        {
            while (token->type_ == Token_Type::TOKEN_WORD)
            {
                fields_name.push_back(token->text_);
                token = ParseEatAndNextToken();
                if (!MatchToken(Token_Type::TOKEN_COMMA, ","))
                {
                    break;
                }
            }
        }
        else
        {
            ParseError("invalid SQL: missing field name!");
            return nullptr;
        }
        if (!MatchToken(Token_Type::TOKEN_CLOSE_PARENTHESIS, ")"))
        {
            ParseError("invalid SQL: missing \")\"!");
            return nullptr;
        }
        if (MatchToken(Token_Type::TOKEN_RESERVED_WORD, "values"))
        {
            values = ParseDataValueExpr();
        }
    }
    else if (MatchToken(Token_Type::TOKEN_RESERVED_WORD, "values"))
    {
        values = ParseDataValueExpr();
    }
    else
    {
        ParseError("invalid SQL: missing values!");
        return nullptr;
    }
    if (values == nullptr) { return nullptr; }
    // If fields_name is empty, which means must provide all field with values respectively
    auto sql_stmt_insert = std::make_shared<SQLStmtInsert>(SQL_Stmt_Type::SQL_INSERT_TABLE, 
                                                           table_name,
                                                           fields_name,
                                                           *values);
    return sql_stmt_insert;
}

std::shared_ptr<std::vector<DataValue>> InsertParser::ParseDataValueExpr()
{
    auto values = std::make_shared<std::vector<DataValue>>(std::vector<DataValue>());
    if (!MatchToken(Token_Type::TOKEN_OPEN_PARENTHESIS, "("))   
    {
        ParseError("invalid SQL: missing \"(\"!");
        return nullptr;
    }
    auto token = ParseNextToken();
    if (token->type_ == Token_Type::TOKEN_STRING ||
        token->type_ == Token_Type::TOKEN_DECIMAL ||
        token->type_ == Token_Type::TOKEN_ZERO ||
        token->type_ == Token_Type::TOKEN_FLOAT || 
        token->type_ == Token_Type::TOKEN_NULL)
    {
        while (token->type_ == Token_Type::TOKEN_STRING ||
               token->type_ == Token_Type::TOKEN_DECIMAL ||
               token->type_ == Token_Type::TOKEN_ZERO ||
               token->type_ == Token_Type::TOKEN_FLOAT || 
               token->type_ == Token_Type::TOKEN_NULL)
        {
            if (token->type_ == Token_Type::TOKEN_STRING)
            {
                DataValue data_value(Data_Type::DATA_TYPE_CHAR, token->text_.size());
                std::memcpy(data_value.char_value_, token->text_.c_str(), token->text_.size() + 1);
                values->push_back(data_value);
            }
            else if (token->type_ == Token_Type::TOKEN_DECIMAL ||
                     token->type_ == Token_Type::TOKEN_ZERO)
            {
                DataValue data_value(Data_Type::DATA_TYPE_INT);
                data_value.int_value_ = std::stoi(token->text_);
                values->push_back(data_value);
            }
            else if (token->type_ == Token_Type::TOKEN_FLOAT ||
                     token->type_ == Token_Type::TOKEN_EXP_FLOAT)
            {
                DataValue data_value(Data_Type::DATA_TYPE_DOUBLE);
                data_value.double_value_ = std::stod(token->text_);
                values->push_back(data_value);
            }
            else if (token->type_ == Token_Type::TOKEN_NULL)
            {
                DataValue data_value(Data_Type::DATA_TYPE_NULL);
                data_value.int_value_ = 0;
                values->push_back(data_value);
            }
            // TODO: support other type
            if (!MatchToken(Token_Type::TOKEN_COMMA, ","))
            {
                break;
            }
        }
    }
    else
    {
        ParseError("invalid SQL: missing a value!");
        return nullptr;
    }
    if (!MatchToken(Token_Type::TOKEN_CLOSE_PARENTHESIS, ")"))
    {
        ParseError("invalid SQL: missing \")\"!");
        return nullptr;
    }
    return values;
}