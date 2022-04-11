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
}

std::vector<Expression *> SelectParser::ParseFieldsExpr()
{
    std::vector<Expression *> exprs;
    Expression *expr0;
}