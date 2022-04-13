#include "../../include/sql/select_parser.h"

SelectParser::SelectParser(std::shared_ptr<Tokenizer> tokenizer)
    : Parser(tokenizer)
{
}

SelectParser::~SelectParser() = default;

std::shared_ptr<SRA> *SelectParser::ParseSQLStmtSelect()
{
    auto token = ParseNextToken();
    /*** select ***/
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "select")) { return nullptr; }
    /*** Expression list ***/
    std::vector<Expression *> field_exprs = ParseFieldsExpr();
    /*** from ***/
    if (!MatchToken(Token_Type::TOKEN_RESERVED_WORD, "word"))
    {
        ParseError("invalid SQL: missing \"from\"!");
        return nullptr;
    }
    /*** table ***/
    token = ParseNextToken();
    SRA *table_expr = ParseTablesExpr();
}

std::vector<Expression *> SelectParser::ParseFieldsExpr()
{
    std::vector<Expression *> exprs;
    // Parsing statement:
    // table_name.column_name
    // * (wildcard)
    // table_name.*
    // column_name
    Expression *expr0 = ParseExpressionRD();
    exprs.push_back(expr0);
    auto token = ParseNextToken();
    while (token != nullptr && token->type_ == Token_Type::TOKEN_COMMA)
    {
        auto token = ParseEatAndNextToken();
        Expression *expr1 = ParseExpressionRD();
        exprs.push_back(expr1);
        token = ParseNextToken();
    }
    return exprs;
}

SRA *SelectParser::ParseTablesExpr()
{
    auto token = ParseNextToken();
    if (token->type_ == Token_Type::TOKEN_WORD)
    {
        std::string table_name = token->text_;
        TableRef *table_ref = new TableRef(table_name);
        SRA *sra = new SRA(SRA_Type::SRA_TABLE);
        sra->table_ = table_ref;
        token = ParseEatAndNextToken();
        while (token != nullptr && token->type_ == Token_Type::TOKEN_COMMA)
        {
            token = ParseEatAndNextToken();
            table_name = token->text_;
            table_ref = new TableRef(table_name);
            SRA *sra2 = new SRA(SRA_Type::SRA_TABLE);
            sra2->table_ = table_ref;
            // Cartesian product
            SRA *new_sra = new SRA(SRA_Type::SRA_JOIN);
            new_sra->join_ = SRAJoin(sra, sra2, nullptr);
            sra = new_sra;
            token = ParseEatAndNextToken();
        }
        return sra;
    }
    return nullptr;
}