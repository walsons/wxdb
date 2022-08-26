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
        return ParseError<std::shared_ptr<TableInfo>>("invalid SQL: missing table name");
    ParseEatAndNextToken();
    // ( 
    if (!MatchToken(Token_Type::TOKEN_OPEN_PARENTHESIS, "("))
        return ParseError<std::shared_ptr<TableInfo>>("invalid SQL: missing \"(\"");
    // (contents)
    while (true)
    {
        token = ParseNextToken();
        switch (token->type_)
        {
        case Token_Type::TOKEN_WORD:  // Field info
        {
            auto field = parse_field_expr();
            if (field == nullptr) { return nullptr; }
            else { table_info->field_info.push_back(*field); }
            break;
        }
        case Token_Type::TOKEN_RESERVED_WORD:  // Constraint info
        {
            auto constraint = parse_constraint_expr();
            if (constraint == nullptr) { return nullptr; }
            else { table_info->constraint_info.push_back(*constraint); }
            break;
        }
        default:
            return ParseError<std::shared_ptr<TableInfo>>("invalid SQL: missing field name or constraint");
            break;
        }
        token = ParseNextToken();
        if (!MatchToken(Token_Type::TOKEN_COMMA, ",")) { break; }
    }
    // )
    token = this->ParseNextToken();
    if (!MatchToken(Token_Type::TOKEN_CLOSE_PARENTHESIS, ")"))
        return ParseError<std::shared_ptr<TableInfo>>("invalid SQL: missing \")\"");
    // ;
    if (!MatchToken(Token_Type::TOKEN_SEMICOLON, ";"))
        return ParseError<std::shared_ptr<TableInfo>>("invalid SQL: missing \";\"");
    return table_info;
}

std::shared_ptr<FieldInfo> TableParser::parse_field_expr()
{
    auto field_info = std::make_shared<FieldInfo>();
    std::shared_ptr<Token> token = ParseNextToken();
    field_info->field_name = token->text_; 
    token = ParseEatAndNextToken();
    if (token->type_ == Token_Type::TOKEN_RESERVED_WORD)
    {
        if (token->text_ == "int" || token->text_ == "integer") 
        {
            field_info->type = Col_Type::COL_TYPE_INT;
            field_info->length = sizeof(int);
            token = ParseEatAndNextToken();
        }
        else if (token->text_ == "double")
        {
            field_info->type = Col_Type::COL_TYPE_DOUBLE;
            field_info->length = sizeof(double);
            token = ParseEatAndNextToken();
        }
        else if (token->text_ == "bool")
        {
            field_info->type = Col_Type::COL_TYPE_BOOL;
            field_info->length = sizeof(bool);
            token = ParseEatAndNextToken();
        }
        else if (token->text_ == "date")
        {
            field_info->type = Col_Type::COL_TYPE_DATE;
            field_info->length = sizeof(Date);
            token = ParseEatAndNextToken();
        }
        else if (token->text_ == "varchar" || token->text_ == "char")
        {
            if (token->text_ == "varchar")
            {
                field_info->type = Col_Type::COL_TYPE_VARCHAR;
            }
            else if (token->text_ == "char")
            {
                field_info->type = Col_Type::COL_TYPE_CHAR;
            }
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
                        return ParseError<std::shared_ptr<FieldInfo>>("invalid SQL: missing \")\"");
                }
                else
                    return ParseError<std::shared_ptr<FieldInfo>>("invalid SQL: missing char length");
            }
            else
                return ParseError<std::shared_ptr<FieldInfo>>("invalid SQL: missing \"(\"");
        }
        else
            return ParseError<std::shared_ptr<FieldInfo>>("invalid SQL: wrong data type: " + token->text_ + "");
        // not null, default
        token = ParseNextToken();
        if (token->type_ == Token_Type::TOKEN_COMMA || token->type_ == Token_Type::TOKEN_CLOSE_PARENTHESIS) {  }  // line end, Nothing to do
        else if (MatchToken(Token_Type::TOKEN_NOT, "not"))
        {
            if (MatchToken(Token_Type::TOKEN_NULL, "null"))
                field_info->is_not_null = true;
            else
                return ParseError<std::shared_ptr<FieldInfo>>("invalid SQL: wrong constraint");
        }
        else if (MatchToken(Token_Type::TOKEN_RESERVED_WORD, "default"))
        {
            field_info->has_default = true;
            field_info->expr = ParseExpressionRD();
        }
        else 
            return ParseError<std::shared_ptr<FieldInfo>>("invalid SQL: wrong constraint");
    }
    else
        return ParseError<std::shared_ptr<FieldInfo>>("invalid SQL: missing field type");
    return field_info;
}

std::shared_ptr<ConstraintInfo> TableParser::parse_constraint_expr()
{
    auto constraint_info = std::make_shared<ConstraintInfo>();
    std::shared_ptr<Token> token = ParseNextToken();
    // unique
    if (MatchToken(Token_Type::TOKEN_RESERVED_WORD, "unique"))
    {
        if (!MatchToken(Token_Type::TOKEN_OPEN_PARENTHESIS, "("))
            return ParseError<std::shared_ptr<ConstraintInfo>>("invalid SQL: missing \"(\"");
        token = ParseNextToken();
        if (token->type_ == Token_Type::TOKEN_WORD)
        {
            constraint_info->type = Constraint_Type::CONS_UNIQUE;
            constraint_info->col_ref.column_name = token->text_;
            ParseEatAndNextToken();
        }
        else
            return ParseError<std::shared_ptr<ConstraintInfo>>("invalid SQL: expect a column name");
        if (!MatchToken(Token_Type::TOKEN_CLOSE_PARENTHESIS, ")"))
            return ParseError<std::shared_ptr<ConstraintInfo>>("invalid SQL: missing \")\"");
    }
    // primary 
    else if (MatchToken(Token_Type::TOKEN_RESERVED_WORD, "primary"))
    {
        if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "key"))
            return ParseError<std::shared_ptr<ConstraintInfo>>("invalid SQL: missing \"key\"");
        if (!MatchToken(Token_Type::TOKEN_OPEN_PARENTHESIS, "("))
            return ParseError<std::shared_ptr<ConstraintInfo>>("invalid SQL: missing \"(\"");
        token = ParseNextToken();
        if (token->type_ == Token_Type::TOKEN_WORD)
        {
            constraint_info->type = Constraint_Type::CONS_PRIMARY_KEY;
            constraint_info->col_ref.column_name = token->text_;
            ParseEatAndNextToken();
        }
        else
            return ParseError<std::shared_ptr<ConstraintInfo>>("invalid SQL: expect a column name");
        if (!MatchToken(Token_Type::TOKEN_CLOSE_PARENTHESIS, ")"))
            return ParseError<std::shared_ptr<ConstraintInfo>>("invalid SQL: missing \")\"");
    }
    // foreign key
    else if (MatchToken(Token_Type::TOKEN_RESERVED_WORD, "foreign"))
    {
        if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "key"))
            return ParseError<std::shared_ptr<ConstraintInfo>>("invalid SQL: missing \"key\"");
        if (!MatchToken(Token_Type::TOKEN_OPEN_PARENTHESIS, "("))
            return ParseError<std::shared_ptr<ConstraintInfo>>("invalid SQL: missing \"(\"");
        token = ParseNextToken();
        if (token->type_ == Token_Type::TOKEN_WORD)
        {
            constraint_info->type = Constraint_Type::CONS_FOREIGN_KEY;
            constraint_info->col_ref.column_name = token->text_;
            ParseEatAndNextToken();
        }
        else
            return ParseError<std::shared_ptr<ConstraintInfo>>("invalid SQL: missing a column name");
        if (!MatchToken(Token_Type::TOKEN_CLOSE_PARENTHESIS, ")"))
            return ParseError<std::shared_ptr<ConstraintInfo>>("invalid SQL: missing \")\"");
        if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "references"))
            return ParseError<std::shared_ptr<ConstraintInfo>>("invalid SQL: missing \"references\"");
        token = ParseNextToken();
        if (token->type_ == Token_Type::TOKEN_WORD)
        {
            constraint_info->fk_ref.table_name = token->text_;
            ParseEatAndNextToken();
            if (!MatchToken(Token_Type::TOKEN_OPEN_PARENTHESIS, "("))
                return ParseError<std::shared_ptr<ConstraintInfo>>("invalid SQL: missing \"(\"");
            token = ParseNextToken();
            if (token->type_ == Token_Type::TOKEN_WORD)
            {
                constraint_info->fk_ref.column_name = token->text_;
                ParseEatAndNextToken();
            }
            else 
                return ParseError<std::shared_ptr<ConstraintInfo>>("invalid SQL: missing a column name");
            if (!MatchToken(Token_Type::TOKEN_CLOSE_PARENTHESIS, ")"))
                return ParseError<std::shared_ptr<ConstraintInfo>>("invalid SQL: missing \")\"");
        }
        else 
            return ParseError<std::shared_ptr<ConstraintInfo>>("invalid SQL: missing a table name");
    }
    // check
    else if (MatchToken(Token_Type::TOKEN_RESERVED_WORD, "check"))
    {
        if (!MatchToken(Token_Type::TOKEN_OPEN_PARENTHESIS, "("))
            return ParseError<std::shared_ptr<ConstraintInfo>>("invalid SQL: missing \"(\"");
        constraint_info->type = Constraint_Type::CONS_CHECK;
        constraint_info->expr = ParseExpressionRD();
        if (!MatchToken(Token_Type::TOKEN_CLOSE_PARENTHESIS, ")"))
            return ParseError<std::shared_ptr<ConstraintInfo>>("invalid SQL: missing \")\"");
    }
    return constraint_info;
}

std::shared_ptr<InsertInfo> TableParser::InsertTable()
{
    auto insert_info = std::make_shared<InsertInfo>();
    // insert
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "insert")) { return nullptr; }
    // into 
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "into")) 
        return ParseError<std::shared_ptr<InsertInfo>>("invalid SQL: should be \"into\"");
    // table name
    auto token = ParseNextToken();
    if (token->type_ == Token_Type::TOKEN_WORD) { insert_info->table_name = token->text_; }
    else
        return ParseError<std::shared_ptr<InsertInfo>>("invalid SQL: missing table name");
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
            return ParseError<std::shared_ptr<InsertInfo>>("invalid SQL: missing field name");
        if (!MatchToken(Token_Type::TOKEN_CLOSE_PARENTHESIS, ")"))
            return ParseError<std::shared_ptr<InsertInfo>>("invalid SQL: missing \")\"");
        if (MatchToken(Token_Type::TOKEN_RESERVED_WORD, "values"))
        {
            auto value = parse_value_expr();
            if (value == nullptr) { return nullptr; }
            insert_info->col_val = std::move(*value);
        }
    }
    // values
    else if (MatchToken(Token_Type::TOKEN_RESERVED_WORD, "values"))
    {
        auto value = parse_value_expr();
        if (value == nullptr) { return nullptr; }
        insert_info->col_val = std::move(*value);
    }
    else
        return ParseError<std::shared_ptr<InsertInfo>>("invalid SQL: missing \"values\"");
    if (!MatchToken(Token_Type::TOKEN_SEMICOLON, ";"))
        return ParseError<std::shared_ptr<InsertInfo>>("invalid SQL: missing \";\"");
    // If field_name is empty, which means must provide all field with values respectively
    return insert_info;
}

std::shared_ptr<std::vector<ColVal>> TableParser::parse_value_expr()
{
    auto value = std::make_shared<std::vector<ColVal>>();
    if (!MatchToken(Token_Type::TOKEN_OPEN_PARENTHESIS, "("))   
        return ParseError<std::shared_ptr<std::vector<ColVal>>>("invalid SQL: missing \"(\"");
    auto token = ParseNextToken();
    if (token->type_ == Token_Type::TOKEN_STRING ||
        token->type_ == Token_Type::TOKEN_DECIMAL ||
        token->type_ == Token_Type::TOKEN_ZERO ||
        token->type_ == Token_Type::TOKEN_FLOAT || 
        token->type_ == Token_Type::TOKEN_NULL ||
        token->type_ == Token_Type::TOKEN_RESERVED_WORD)
    {
        while (token->type_ == Token_Type::TOKEN_STRING ||
               token->type_ == Token_Type::TOKEN_DECIMAL ||
               token->type_ == Token_Type::TOKEN_ZERO ||
               token->type_ == Token_Type::TOKEN_FLOAT || 
               token->type_ == Token_Type::TOKEN_NULL ||
               token->type_ == Token_Type::TOKEN_RESERVED_WORD)
        {
            if (token->type_ == Token_Type::TOKEN_STRING)
            {
                ColVal data_value(token->text_);
                data_value.type_ = Col_Type::COL_TYPE_VARCHAR;
                value->push_back(data_value);
            }
            else if (token->type_ == Token_Type::TOKEN_DECIMAL ||
                     token->type_ == Token_Type::TOKEN_ZERO)
            {
                ColVal data_value(std::stoi(token->text_));
                value->push_back(data_value);
            }
            else if (token->type_ == Token_Type::TOKEN_FLOAT ||
                     token->type_ == Token_Type::TOKEN_EXP_FLOAT)
            {
                ColVal data_value(std::stod(token->text_));
                value->push_back(data_value);
            }
            else if (token->type_ == Token_Type::TOKEN_NULL)
            {
                ColVal data_value;
                value->push_back(data_value);
            }
            else if (token->type_ == Token_Type::TOKEN_RESERVED_WORD)
            {
                ColVal data_value;
                if (token->text_ == "true")
                    data_value = true;
                else if (token->text_ == "false")
                    data_value = false;
                else
                    return ParseError<std::shared_ptr<std::vector<ColVal>>>("invalid SQL: value is wrong");
                value->push_back(data_value);
            }
            ParseEatAndNextToken();
            // TODO: support other type
            if (!MatchToken(Token_Type::TOKEN_COMMA, ","))
            {
                break;
            }
            token = ParseNextToken();
        }
    }
    else
        return ParseError<std::shared_ptr<std::vector<ColVal>>>("invalid SQL: missing a value");
    if (!MatchToken(Token_Type::TOKEN_CLOSE_PARENTHESIS, ")"))
        return ParseError<std::shared_ptr<std::vector<ColVal>>>("invalid SQL: missing \")\"");
    return value;
}

std::shared_ptr<SelectInfo> TableParser::SelectTable()
{
    auto select_info = std::make_shared<SelectInfo>();
    select_info->where = nullptr;
    // select
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "select")) { return nullptr; }
    // column name
    auto token = ParseNextToken();
    if (MatchToken(Token_Type::TOKEN_MULTIPLY, "*"))
    {
        //  Nothing need to do, remain columns to empty
    }
    else if (token->type_ == Token_Type::TOKEN_WORD)
    {
        while (token->type_ == Token_Type::TOKEN_WORD)
        {
            std::string col_name = token->text_;
            size_t pos = 0;
            ColumnRef ref;
            if ((pos = col_name.find(".")) != std::string::npos)
            {
                ref.table_name = col_name.substr(0, pos);
                ref.column_name = col_name.substr(pos + 1);
            }
            else
            {
                ref.column_name = col_name;
            }
            ParseEatAndNextToken();
            select_info->columns.push_back(ref);
            if (!MatchToken(Token_Type::TOKEN_COMMA, ","))
            {
                break;
            }
            token = ParseNextToken();
        }
    }
    else 
        return ParseError<std::shared_ptr<SelectInfo>>("Invalid SQL: should be column name");
    // from
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "from")) 
        return ParseError<std::shared_ptr<SelectInfo>>("Invalid SQL: should be \"from\"");
    // table name
    token = ParseNextToken();
    if (token->type_ == Token_Type::TOKEN_WORD)
    {
        while (token->type_ == Token_Type::TOKEN_WORD)
        {
            select_info->tables.push_back(token->text_);
            token = ParseEatAndNextToken();
            if (!MatchToken(Token_Type::TOKEN_COMMA, ","))
            {
                break;
            }
            token = ParseNextToken();
        }
    }
    else 
        return ParseError<std::shared_ptr<SelectInfo>>("Invalid SQL: should be table name");
    if (MatchToken(Token_Type::TOKEN_SEMICOLON, ";")) 
    {
        return select_info;
    }
    // where if has
    if (MatchToken(Token_Type::TOKEN_RESERVED_WORD, "where")) 
    {
        ExprNode *node = ParseExpressionRD();      
        select_info->where = node;
    }
    else
        return ParseError<std::shared_ptr<SelectInfo>>("Invalid SQL: missing \";\" or syntax error");
    if (!MatchToken(Token_Type::TOKEN_SEMICOLON, ";")) 
        return ParseError<std::shared_ptr<SelectInfo>>("Invalid SQL: missing \";\"");
    return select_info;
}

std::shared_ptr<DeleteInfo> TableParser::DeleteTable()
{
    auto delete_info = std::make_shared<DeleteInfo>();
    delete_info->where = nullptr;
    // delete
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "delete"))
        return nullptr;
    // from
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD))
        return ParseError<std::shared_ptr<DeleteInfo>>("invalid SQL: should be \"FROM\"");
    // table name
    auto token = ParseNextToken();
    if (token && token->type_ == Token_Type::TOKEN_WORD)
    {
        delete_info->table_name = token->text_;
        ParseEatAndNextToken();
    }
    else
        return ParseError<std::shared_ptr<DeleteInfo>>("invalid SQL: missing table name");
    // ;
    if (MatchToken(Token_Type::TOKEN_SEMICOLON))
        return delete_info;
    // where if has
    if (MatchToken(Token_Type::TOKEN_RESERVED_WORD, "where"))
    {
        ExprNode *node = ParseExpressionRD();
        delete_info->where = node;
    }
    else if ((token = ParseEatAndNextToken()) != nullptr)
        return ParseError<std::shared_ptr<DeleteInfo>>("invalid SQL: missing \";\" or syntax error");
    // ;
    if (!MatchToken(Token_Type::TOKEN_SEMICOLON))
        return ParseError<std::shared_ptr<DeleteInfo>>("invalid SQL: missing \";\"");
    return delete_info;
}

std::shared_ptr<UpdateInfo> TableParser::UpdateTable()
{
    auto update_info = std::make_shared<UpdateInfo>();
    update_info->where = nullptr;
    // update
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "update"))
        return nullptr;
    // table name
    auto token = ParseNextToken();
    if (token && token->type_ == Token_Type::TOKEN_WORD)
    {
        update_info->table_name = token->text_;
        ParseEatAndNextToken();
    }
    else
        return ParseError<std::shared_ptr<UpdateInfo>>("invalid SQL: missing table name");
    // set
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "set"))
        return ParseError<std::shared_ptr<UpdateInfo>>("invalid SQL: expect \"SET\"");
    // column=value
    auto col2val = parse_set_expr();
    if (col2val == nullptr)
        return nullptr;
    update_info->column_ref2value = std::move(*col2val);
    // where
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "where"))
        return ParseError<std::shared_ptr<UpdateInfo>>("invalid SQL: expect \"WHERE\"");
    // expression
    ExprNode *node = ParseExpressionRD();
    if (node == nullptr)
        return ParseError<std::shared_ptr<UpdateInfo>>("invalid SQL: condition expression syntax error");
    update_info->where = node;
    // ;
    if (!MatchToken(Token_Type::TOKEN_SEMICOLON))
        return ParseError<std::shared_ptr<UpdateInfo>>("invalid SQL: missing \";\"");
    return update_info;
}

std::shared_ptr<std::unordered_map<std::string, ColVal>> TableParser::parse_set_expr()
{
    auto res = std::make_shared<std::unordered_map<std::string, ColVal>>();
    std::unordered_map<std::string, ColVal> col2val;
    auto token = ParseNextToken();
    while (true)
    {
        auto token = ParseNextToken();
        ColumnRef column_ref;
        if (token && token->type_ == Token_Type::TOKEN_WORD) 
            column_ref.column_name = token->text_;
        else
            return ParseError<std::shared_ptr<std::unordered_map<std::string, ColVal>>>("invalid SQL: missing column name");
        ParseEatAndNextToken();
        if (!MatchToken(Token_Type::TOKEN_EQ))
            return ParseError<std::shared_ptr<std::unordered_map<std::string, ColVal>>>("invalid SQL: expect \"=\"");
        token = ParseNextToken();
        ColVal column_value;
        if (token)
        {
            switch (token->type_)
            {
            case Token_Type::TOKEN_NULL:
                break;
            case Token_Type::TOKEN_ZERO:
            case Token_Type::TOKEN_DECIMAL:
                column_value = stoi(token->text_);
                break;
            case Token_Type::TOKEN_FLOAT:
            case Token_Type::TOKEN_EXP_FLOAT:
                column_value = stod(token->text_);
                break;
            case Token_Type::TOKEN_RESERVED_WORD:
                if (token->text_ == "true")
                    column_value = true;
                else if (token->text_ == "false")
                    column_value = false;
                else
                    return ParseError<std::shared_ptr<std::unordered_map<std::string, ColVal>>>("invalid SQL: value is wrong");
                break;
            case Token_Type::TOKEN_STRING:
                column_value = token->text_;
                break;
            default:
                return ParseError<std::shared_ptr<std::unordered_map<std::string, ColVal>>>("invalid SQL: value is wrong");
                break;
            }
        }
        else
            return ParseError<std::shared_ptr<std::unordered_map<std::string, ColVal>>>("invalid SQL: syntax error");
        ParseEatAndNextToken();
        if (!MatchToken(Token_Type::TOKEN_COMMA))
            break;
    }
    *res = std::move(col2val);
    return res;
}