#include "../../include/sql/table_parser.h"

std::shared_ptr<TableInfo> TableParser::CreateTable()
{
    auto table_info = std::make_shared<TableInfo>();
    // create
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "create")) { return nullptr; }
    // table
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "table")) { return nullptr; }
    // table name
    std::shared_ptr<Token> token = ParseNextToken();
    if (token->type_ == Token_Type::TOKEN_WORD) { table_info->table_name = token->text_; }
    else
    {
        ParseError("invalid SQL: missing table name!");
        return nullptr;
    }
    token = ParseEatToken();
    // ( 
    if (!MatchToken(Token_Type::TOKEN_OPEN_PARENTHESIS, "("))
    {
        ParseError("invalid SQL: missing \"(\"!");
        return nullptr;
    }
    // column expression
    token = ParseNextToken();
    while (token->type_ != Token_Type::TOKEN_CLOSE_PARENTHESIS)
    {
        auto field = parse_column_expr();
        if (field == nullptr) { return nullptr; }
        else 
        {
            table_info->field_info.push_back(*field);
        }
        token = ParseNextToken();
        if (!MatchToken(Token_Type::TOKEN_COMMA, ","))
        {
            break;
        }
    }
    // )
    token = this->ParseNextToken();
    if (!MatchToken(Token_Type::TOKEN_CLOSE_PARENTHESIS, ")"))
    {
        ParseError("invalid SQL: missing \")\"!");
        return nullptr;
    }
    // ;
    if (!MatchToken(Token_Type::TOKEN_SEMICOLON, ";"))
    {
        ParseError("invalid SQL: missing \";\"!");
        return nullptr;
    }
    return table_info;
}

std::shared_ptr<FieldInfo> TableParser::parse_column_expr()
{
    auto field_info = std::make_shared<FieldInfo>();
    std::shared_ptr<Token> token = ParseNextToken();
    if (token->type_ == Token_Type::TOKEN_WORD) 
    { 
        field_info->field_name = token->text_; 
        token = ParseEatAndNextToken();
        if (token->type_ == Token_Type::TOKEN_RESERVED_WORD)
        {
            if (token->text_ == "int" || token->text_ == "integer") 
            {
                field_info->type = Data_Type::DATA_TYPE_INT;
                field_info->length = sizeof(int);
                token = ParseEatAndNextToken();
            }
            else if (token->text_ == "double")
            {
                field_info->type = Data_Type::DATA_TYPE_DOUBLE;
                field_info->length = sizeof(double);
                token = ParseEatAndNextToken();
            }
            else if (token->text_ == "varchar" || token->text_ == "char")
            {
                field_info->type = Data_Type::DATA_TYPE_VARCHAR;
                token = ParseEatAndNextToken();
                if (MatchToken(Token_Type::TOKEN_OPEN_PARENTHESIS, "("))
                {
                    token = ParseNextToken();
                    if (token->type_ == Token_Type::TOKEN_DECIMAL)
                    {
                        field_info->length = std::stoi(token->text_);
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
            // not null, unique, default
            if (MatchToken(Token_Type::TOKEN_RESERVED_WORD, "unique"))
            {
                field_info->constraint.push_back(Constraint_Type::CONS_UNIQUE);
            }
            else if (MatchToken(Token_Type::TOKEN_RESERVED_WORD, "not"))
            {
                if (MatchToken(Token_Type::TOKEN_RESERVED_WORD, "null"))
                {
                    field_info->constraint.push_back(Constraint_Type::CONS_NOT_NULL);
                }
                else
                {
                    ParseError("invalid SQL: wrong constraint!");
                }
            }
            else if (MatchToken(Token_Type::TOKEN_RESERVED_WORD, "default"))
            {
                field_info->expr = ParseExpressionRD();
            }
            else 
            {
                ParseError("invalid SQL: wrong constraint!");
            }
        }
        else
        {
            ParseError("invalid SQL: missing field type!");
            return nullptr;
        }
    }
    // primary 
    else if (MatchToken(Token_Type::TOKEN_RESERVED_WORD, "primary"))
    {
        if (MatchToken(Token_Type::TOKEN_OPEN_PARENTHESIS, "("))
        {
            field_info->constraint.push_back(Constraint_Type::CONS_PRIMARY_KEY);
            field_info->expr = ParseExpressionRD();
            if (!MatchToken(Token_Type::TOKEN_OPEN_PARENTHESIS, ")"))
            {
                ParseError("invalid SQL: missing \")\"!");
            }
        }
        else 
        {
            ParseError("invalid SQL: missing \"(\"!");
        }
    }
    // check
    else if (MatchToken(Token_Type::TOKEN_RESERVED_WORD, "constraint"))
    {
        if (MatchToken(Token_Type::TOKEN_OPEN_PARENTHESIS, "("))
        {
            field_info->constraint.push_back(Constraint_Type::CONS_CHECK);
            field_info->expr = ParseExpressionRD();
            if (!MatchToken(Token_Type::TOKEN_OPEN_PARENTHESIS, ")"))
            {
                ParseError("invalid SQL: missing \")\"!");
            }
        }
        else 
        {
            ParseError("invalid SQL: missing \"(\"!");
        }
    }
    else
    {
        ParseError("invalid SQL: missing field name or constraint!");
        return nullptr;
    }
    return field_info;
}

std::shared_ptr<InsertInfo> TableParser::InsertTable()
{
    auto insert_info = std::make_shared<InsertInfo>();
    // insert
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "insert")) { return nullptr; }
    // into 
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "into")) 
    {
        ParseError("invalid SQL: should be \"into\"!");
        return nullptr;
    }
    // table name
    auto token = ParseNextToken();
    if (token->type_ == Token_Type::TOKEN_WORD) { insert_info->table_name = token->text_; }
    else
    {
        ParseError("invalid SQL: missing table name!");
        return nullptr;
    }
    // column
    token = ParseEatAndNextToken();
    if (MatchToken(Token_Type::TOKEN_OPEN_PARENTHESIS, "("))
    {
        token = ParseNextToken();
        if (token->type_ == Token_Type::TOKEN_WORD)
        {
            while (token->type_ == Token_Type::TOKEN_WORD)
            {
                insert_info->field_name.insert({token->text_, insert_info->field_name.size()});
                token = ParseEatAndNextToken();
                if (!MatchToken(Token_Type::TOKEN_COMMA, ","))
                {
                    break;
                }
                token = ParseNextToken();
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
            auto value = parse_value_expr();
            if (value == nullptr) { return nullptr; }
            insert_info->value = std::move(*value);
        }
    }
    // values
    else if (MatchToken(Token_Type::TOKEN_RESERVED_WORD, "values"))
    {
        auto value = parse_value_expr();
        if (value == nullptr) { return nullptr; }
        insert_info->value = std::move(*value);
    }
    else
    {
        ParseError("invalid SQL: missing \"values\"!");
        return nullptr;
    }
    if (!MatchToken(Token_Type::TOKEN_SEMICOLON, ";"))
    {
        ParseError("invalid SQL: missing \";\"!");
        return nullptr;
    }
    // If field_name is empty, which means must provide all field with values respectively
    return insert_info;
}

std::shared_ptr<std::vector<DataValue>> TableParser::parse_value_expr()
{
    auto value = std::shared_ptr<std::vector<DataValue>>();
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
                DataValue data_value(Data_Type::DATA_TYPE_CHAR);
                data_value.SetCharValue(token->text_);
                value->push_back(data_value);
            }
            else if (token->type_ == Token_Type::TOKEN_DECIMAL ||
                     token->type_ == Token_Type::TOKEN_ZERO)
            {
                DataValue data_value(Data_Type::DATA_TYPE_INT);
                data_value.SetIntValue(std::stoi(token->text_));
                value->push_back(data_value);
            }
            else if (token->type_ == Token_Type::TOKEN_FLOAT ||
                     token->type_ == Token_Type::TOKEN_EXP_FLOAT)
            {
                DataValue data_value(Data_Type::DATA_TYPE_DOUBLE);
                data_value.SetDoubleValue(std::stod(token->text_));
                value->push_back(data_value);
            }
            else if (token->type_ == Token_Type::TOKEN_NULL)
            {
                DataValue data_value(Data_Type::DATA_TYPE_NULL);
                value->push_back(data_value);
            }
            token = ParseEatToken();
            // TODO: support other type
            if (!MatchToken(Token_Type::TOKEN_COMMA, ","))
            {
                break;
            }
            token = ParseNextToken();
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
    return value;

}