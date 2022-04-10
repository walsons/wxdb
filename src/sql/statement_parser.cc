#include "../../include/sql/statement_parser.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

CreateParser::CreateParser(Tokenizer *tokenizer) : Parser(tokenizer)
{
}

CreateParser::~CreateParser() = default;

std::shared_ptr<SQLStmtCreate> CreateParser::ParseSQLStmtCreate()
{
    std::string table_name;
    std::vector<std::string> fields_name;
    auto fields = std::make_shared<std::unordered_map<std::string, std::shared_ptr<FieldInfo>>>();
    /*** create ***/
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "create")) { return nullptr; }
    /*** table ***/
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "table")) 
    { 
        ParseError("invalid SQL: should be table!");
        return nullptr; 
    }
    /*** table name ***/
    std::shared_ptr<Token> token = ParseNextToken();
    if (token->type_ == Token_Type::TOKEN_WORD) { table_name = token->text_; }
    else
    {
        ParseError("invalid SQL: missing table name!");
        return nullptr;
    }
    token = ParseEatToken();
    /*** ( ***/
    if (!MatchToken(Token_Type::TOKEN_OPEN_PARENTHESIS, "("))
    {
        ParseError("invalid SQL: missing \"(\"!");
        return nullptr;
    }
    /*** column expression ***/
    token = ParseNextToken();
    while (token->type_ != Token_Type::TOKEN_CLOSE_PARENTHESIS)
    {
        auto field = ParseSQLStmtColumnExpr();
        if (field == nullptr) { return nullptr; }
        else 
        {
            fields->insert({field->field_name_, field});
            fields_name.push_back(field->field_name_);
        }
        token = ParseNextToken();
        if (!MatchToken(Token_Type::TOKEN_COMMA, ","))
        {
            break;
        }
    }
    /*** ) ***/
    token = this->ParseNextToken();
    if (!MatchToken(Token_Type::TOKEN_CLOSE_PARENTHESIS, ")"))
    {
        ParseError("invalid SQL: missing \")\"!");
        return nullptr;
    }
    auto table_info = std::make_shared<TableInfo>(table_name, fields_name, fields);
    // TODO: add constrains
    std::shared_ptr<Constraint> constraints = nullptr;
    auto sql_stmt_create = std::make_shared<SQLStmtCreate>(SQL_Statement_Type::SQL_CREATE_TABLE,
                                                           table_info, constraints);
    return sql_stmt_create;
}

std::shared_ptr<FieldInfo> CreateParser::ParseSQLStmtColumnExpr()
{
    std::shared_ptr<Token> token = ParseNextToken();
    std::string column_name;
    Data_Type data_type;
    int length;
    if (token->type_ == Token_Type::TOKEN_WORD) { column_name = token->text_; }
    else
    {
        ParseError("invalid SQL: missing field name!");
        return nullptr;
    }
    token = ParseEatAndNextToken();
    if (token->type_ == Token_Type::TOKEN_RESERVED_WORD)
    {
        if (token->text_ == "int" || token->text_ == "integer") 
        {
            data_type = Data_Type::DATA_TYPE_INT;
            length = sizeof(int);
            token = ParseEatAndNextToken();
        }
        else if (token->text_ == "double")
        {
            data_type = Data_Type::DATA_TYPE_DOUBLE;
            length = sizeof(double);
            token = ParseEatAndNextToken();
        }
        else if (token->text_ == "char")
        {
            data_type = Data_Type::DATA_TYPE_CHAR;
            token = ParseEatAndNextToken();
            if (MatchToken(Token_Type::TOKEN_OPEN_PARENTHESIS, "("))
            {
                token = ParseNextToken();
                if (token->type_ == Token_Type::TOKEN_DECIMAL)
                {
                    length = std::stoi(token->text_);
                    token = ParseEatAndNextToken();
                    if (MatchToken(Token_Type::TOKEN_CLOSE_PARENTHESIS, ")"))
                    {
                        token = ParseNextToken();
                    }
                    else
                    {
                        ParseError("invalid SQL: missing \")\"!");
                        return nullptr;
                    }
                }
                else
                {
                    ParseError("invalid SQL: missing char length!");
                    return nullptr;
                }
            }
            else
            {
                ParseError("invalid SQL: missing \"(\"!");
                return nullptr;
            }
        }
        // TODO: support other data type
        else
        {
            ParseError("invalid SQL: wrong data type: " + token->text_ + "!");
            return nullptr;
        }
    }
    else
    {
        ParseError("invalid SQL: missing field type!");
        return nullptr;
    }
    auto field = std::make_shared<FieldInfo>(data_type, length, column_name);
    return field;
}