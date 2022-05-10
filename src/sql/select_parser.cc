#include "../../include/sql/select_parser.h"
#include <iostream>

SelectParser::SelectParser(std::shared_ptr<Tokenizer> tokenizer)
    : Parser(tokenizer)
{
}

SelectParser::~SelectParser() = default;

std::shared_ptr<SRA> SelectParser::ParseSQLStmtSelect()
{
    auto token = ParseNextToken();
    /*** select ***/
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "select")) { return nullptr; }
    /*** Expression list ***/
    auto field_exprs = ParseFieldsExpr();
    if (parser_state_type_ == Parser_State_Type::PARSER_WRONG) { return nullptr; }
    /*** from ***/
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "from"))
    {
        ParseError("invalid SQL: missing \"from\"!");
        return nullptr;
    }
    /*** table ***/
    token = ParseNextToken();
    std::shared_ptr<SRA> table_expr = ParseTablesExpr();
    if (parser_state_type_ == Parser_State_Type::PARSER_WRONG) { return nullptr; }
    // If select statement ends, transform to relation algebra and return
    token = ParseNextToken();
    if (MatchToken(Token_Type::TOKEN_SEMICOLON, ";"))
    {
        auto project = SRAOfProject(table_expr, field_exprs);
        return project;
    }
    // TODO: If isn't end, then to match where sub-statement
    return nullptr;

}
    
std::vector<ExprNode *> SelectParser::ParseFieldsExpr()
{
    std::vector<ExprNode *> exprs;
    // Parsing statement:
    // table_name.column_name
    // * (wildcard)
    // table_name.*
    // column_name
    auto expr0 = ParseExpressionRD();
    exprs.push_back(expr0);
    auto token = ParseNextToken();
    while (token != nullptr && token->type_ == Token_Type::TOKEN_COMMA)
    {
        token = ParseEatAndNextToken();
        auto expr1 = ParseExpressionRD();
        exprs.push_back(expr1);
        token = ParseNextToken();
    }
    return exprs;
}

std::shared_ptr<SRA> SelectParser::ParseTablesExpr()
{
    auto token = ParseNextToken();
    if (token->type_ == Token_Type::TOKEN_WORD)
    {
        std::string table_name = token->text_;
        TableRef *table_ref = new TableRef(table_name);
        auto sra1 = SRAOfTable(table_ref);
        token = ParseEatAndNextToken();
        while (token != nullptr && token->type_ == Token_Type::TOKEN_COMMA)
        {
            token = ParseEatAndNextToken();
            table_name = token->text_;
            table_ref = new TableRef(table_name);
            auto sra2 = SRAOfTable(table_ref);
            // Cartesian product
            sra1 = SRAOfJoin(sra1, sra2, nullptr);
            token = ParseEatAndNextToken();
        }
        return sra1;
    }
    return nullptr;
}