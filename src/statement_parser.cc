#include "../include/statement_parser.h"
#include <string>
#include <vector>
#include <unordered_map>

CreateParser::CreateParser(Tokenizer *tokenizer) : Parser(tokenizer)
{
}

CreateParser::~CreateParser()
{
}

SQLStmtCreate *CreateParser::ParseSQLStmtCreate()
{
    std::string table_name;
    std::vector<std::string> fields_name;
    auto *fields = new std::unordered_map<std::string, FieldInfo *>();
    /*** create ***/
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "create")) { return nullptr; }
    /*** table ***/
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "table")) 
    { 
        parser_message_ = "invalid sql: should be table!";
        return nullptr; 
    }
    /*** table name ***/
    Token *token = ParseNextToken();
    if (token->type_ == Token_Type::TOKEN_WORD) { table_name = token->text_; }
    else
    {
        parser_message_ = "invalid sql: missing table name!";
        return nullptr;
    }
    token = ParseEatToken();
    /*** ( ***/
    if (!MatchToken(Token_Type::TOKEN_OPEN_PARENTHESIS, "("))
    {
        parser_message_ = "invalid sql: missing \"(\"!";
        return nullptr;
    }
    /*** column expression ***/
    token = ParseNextToken();
    while (token->type_ != Token_Type::TOKEN_CLOSE_PARENTHESIS)
    {
        FieldInfo *field = ParseSQLStmtColumnExpr();
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
    token = this->ParseNextToken();
    if (!MatchToken(Token_Type::TOKEN_CLOSE_PARENTHESIS, ")"))
    {
        parser_message_ = "invalid sql: missing \")\"!";
        return nullptr;
    }
    TableInfo *table_info = new TableInfo(table_name, fields_name, fields);
    Constraint_t *constraints = nullptr;
    SQLStmtCreate *sql_stmt_create = new SQLStmtCreate(SQL_Statement_Type::SQL_CREATE_TABLE,
                                                       table_info, constraints);
    return sql_stmt_create;
}

FieldInfo *CreateParser::ParseSQLStmtColumnExpr()
{
    Token *token = ParseNextToken();
    std::string column_name;
    Data_Type data_type;
    int length;
    if (token->type_ == Token_Type::TOKEN_WORD) { column_name = token->text_; }
    else
    {
        parser_message_ = "invalid sql: missing field name!";
        return nullptr;
    }
    token = ParseEatAndNextToken();
    if (token->type_ == Token_Type::TOKEN_RESERVED_WORD)
    {
        if (token->text_ == "int") 
        {
            data_type = Data_Type::DATA_TYPE_INT;
            length = sizeof(INT_SIZE);
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
                        parser_message_ = "invalid sql: missing \")\"!";
                        return nullptr;
                    }
                }
                else
                {
                    parser_message_ = "invalid sql: missing char length!";
                    return nullptr;
                }
            }
            else
            {
                parser_message_ = "invalid sql: missing \"(\"!";
                return nullptr;
            }
        }
        // TODO: support other data type
        else
        {
            parser_message_ = "invalid sql: wrong data type: ";
            parser_message_ += token->text_;
            parser_message_ += "!";
            return nullptr;
        }
    }
    else
    {
        parser_message_ = "invalid sql: missing field name!";
        return nullptr;
    }
    FieldInfo *field = new FieldInfo(data_type, length, column_name);
    return field;
}