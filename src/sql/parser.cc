#include "../../include/sql/parser.h"
#include <memory>

Parser::Parser(std::shared_ptr<Tokenizer> tokenizer)
    : tokenizer_(tokenizer)
    , curr_token_(nullptr)
{
}

Parser::~Parser() = default;

void Parser::Reset()
{
    tokenizer_->Reset();
    curr_token_ = nullptr;
}

// Get next token, if current token doesn't be eat, return current token
std::shared_ptr<Token> Parser::ParseNextToken()
{
    // Beginning state of curr_token_ is nullptr
    if (curr_token_ == nullptr)
        curr_token_ = tokenizer_->GetNextToken();
    return curr_token_;
}

// Eat current token then return next token
std::shared_ptr<Token> Parser::ParseEatAndNextToken()
{
    curr_token_ = tokenizer_->GetNextToken();
    return curr_token_;
}

bool Parser::MatchToken(Token_Type type)
{
    std::shared_ptr<Token> token = ParseNextToken();
    if (token && token->type_ == type)
    {
        ParseEatAndNextToken();
        return true;
    }
    return false;
}

bool Parser::MatchToken(Token_Type type, const std::string &text)
{
    std::shared_ptr<Token> token = ParseNextToken();
    if (token && token->type_ == type && token->text_ == text)
    {
        ParseEatAndNextToken();
        return true;
    }
    return false;
}

/******************************************************
 * Parsing expression by using recursive descent approach
 * 
 * or 
 * and 
 * equality: equal | not equal 
 * comparison: GE | LE | GT | LT
 * expr: plus | minus
 * term: mutiply | divide
 * power
 * unary: not | plus | minus (positive negative)
 * paren: open_paren | close_paren ------
 * ---> or
 * ---> read_builtin: word | func | identifier | mutiply (wildcard)
 *      literal: float | decimal | exp_float | string | char 
 ******************************************************/ 

ExprNode *Parser::ParseExpressionRD()
{
    auto expr = ParseReadBooleanOr();
    return expr;
}

ExprNode *Parser::ParseReadBooleanOr()
{
    ExprNode *expr1 = nullptr, *expr2 = nullptr, *expr_op = nullptr;
    auto token = ParseNextToken();
    expr1 = ParseReadBooleanAnd();
    token = ParseNextToken();
    while (token != nullptr && token->type_ == Token_Type::TOKEN_OR)
    {
        ParseEatAndNextToken();
        expr2 = ParseReadBooleanAnd();
        expr1 = concatenate_expr_node(expr1, expr2);
        expr_op = new ExprNode(Operator_Type::OR);
        expr1 = concatenate_expr_node(expr1, expr_op);
        token = ParseNextToken();
    }
    return expr1;
}

ExprNode *Parser::ParseReadBooleanAnd()
{
    ExprNode *expr1 = nullptr, *expr2 = nullptr, *expr_op = nullptr;
    auto token = ParseNextToken();
    expr1 = ParseReadBooleanEquality();
    token = ParseNextToken();
    while (token != nullptr && token->type_ == Token_Type::TOKEN_AND)
    {
        ParseEatAndNextToken();
        expr2 = ParseReadBooleanEquality();
        expr1 = concatenate_expr_node(expr1, expr2);
        expr_op = new ExprNode(Operator_Type::AND);
        expr1 = concatenate_expr_node(expr1, expr_op);
        token = ParseNextToken();
    }
    return expr1;
}

ExprNode *Parser::ParseReadBooleanEquality()
{
    ExprNode *expr1 = nullptr, *expr2 = nullptr, *expr_op = nullptr;
    auto token = ParseNextToken();
    expr1 = ParseReadBooleanComparison();
    token = ParseNextToken();
    if (token != nullptr && (token->type_ == Token_Type::TOKEN_EQ ||
                             token->type_ == Token_Type::TOKEN_NOT_EQ))
    {
        ParseEatAndNextToken();
        expr2 = ParseReadBooleanComparison();
        expr1 = concatenate_expr_node(expr1, expr2);
        switch (token->type_)
        {
        case Token_Type::TOKEN_EQ:
            expr_op = new ExprNode(Operator_Type::EQ);
            break;
        case Token_Type::TOKEN_NOT_EQ:
            expr_op = new ExprNode(Operator_Type::NOT_EQ);
            break;
        }
        expr1 = concatenate_expr_node(expr1, expr_op);
        token = ParseNextToken();
    }
    return expr1;
}

ExprNode *Parser::ParseReadBooleanComparison()
{
    ExprNode *expr1 = nullptr, *expr2 = nullptr, *expr_op = nullptr;
    auto token = ParseNextToken();
    expr1 = ParseReadExpr();
    token = ParseNextToken();
    if (token != nullptr && (token->type_ == Token_Type::TOKEN_GT ||
                             token->type_ == Token_Type::TOKEN_GE ||
                             token->type_ == Token_Type::TOKEN_LT ||
                             token->type_ == Token_Type::TOKEN_LE))
    {
        ParseEatAndNextToken();
        expr2 = ParseReadExpr();
        expr1 = concatenate_expr_node(expr1, expr2);
        switch (token->type_)
        {
        case Token_Type::TOKEN_GT:
            expr_op = new ExprNode(Operator_Type::GT);
            break;
        case Token_Type::TOKEN_GE:
            expr_op = new ExprNode(Operator_Type::GE);
            break;
        case Token_Type::TOKEN_LT:
            expr_op = new ExprNode(Operator_Type::LT);
            break;
        case Token_Type::TOKEN_LE:
            expr_op = new ExprNode(Operator_Type::LE);
            break;
        }
        expr1 = concatenate_expr_node(expr1, expr_op);
        token = ParseNextToken();
    }
    return expr1;
}

ExprNode *Parser::ParseReadExpr()
{
    ExprNode *expr1 = nullptr, *expr2 = nullptr, *expr_op = nullptr;
    auto token = ParseNextToken();
    expr1 = ParseReadTerm();
    token = ParseNextToken();
    while (token != nullptr && (token->type_ == Token_Type::TOKEN_PLUS ||
                                token->type_ == Token_Type::TOKEN_MINUS))
    {
        ParseEatAndNextToken();
        expr2 = ParseReadTerm();
        expr1 = concatenate_expr_node(expr1, expr2);
        switch (token->type_)
        {
        case Token_Type::TOKEN_PLUS:
            expr_op = new ExprNode(Operator_Type::PLUS);
            break;
        case Token_Type::TOKEN_MINUS:
            expr_op = new ExprNode(Operator_Type::MINUS);
            break;
        }
        expr1 = concatenate_expr_node(expr1, expr_op);
        token = ParseNextToken();
    }
    return expr1;
}

ExprNode *Parser::ParseReadTerm()
{
    ExprNode *expr1 = nullptr, *expr2 = nullptr, *expr_op = nullptr;
    auto token = ParseNextToken();
    expr1 = ParseReadPower();
    token = ParseNextToken();
    while (token != nullptr && (token->type_ == Token_Type::TOKEN_MULTIPLY ||
                                token->type_ == Token_Type::TOKEN_DIVIDE))
    {
        ParseEatAndNextToken();
        expr2 = ParseReadPower();
        expr1 = concatenate_expr_node(expr1, expr2);
        switch (token->type_)
        {
        case Token_Type::TOKEN_MULTIPLY:
            expr_op = new ExprNode(Operator_Type::MULTIPLY);
            break;
        case Token_Type::TOKEN_DIVIDE:
            expr_op = new ExprNode(Operator_Type::DIVIDE);
            break;
        }
        expr1 = concatenate_expr_node(expr1, expr_op);
        token = ParseNextToken();
    }
    return expr1;
}

ExprNode *Parser::ParseReadPower()
{
    ExprNode *expr1 = nullptr, *expr2 = nullptr, *expr_op = nullptr;
    auto token = ParseNextToken();
    expr1 = ParseReadUnary();
    token = ParseNextToken();
    while (token != nullptr && (token->type_ == Token_Type::TOKEN_POWER))
    {
        ParseEatAndNextToken();
        expr2 = ParseReadUnary();
        expr1 = concatenate_expr_node(expr1, expr2);
        expr_op = new ExprNode(Operator_Type::POWER);
        expr1 = concatenate_expr_node(expr1, expr_op);
        token = ParseNextToken();
    }
    return expr1;
}

ExprNode *Parser::ParseReadUnary()
{
    ExprNode *expr = nullptr, *expr_op = nullptr;
    auto token = ParseNextToken();
    if (token == nullptr)
        return ParseError<ExprNode *>("Syntax error!");
    if (token != nullptr && (token->type_ == Token_Type::TOKEN_NOT ||
                             token->type_ == Token_Type::TOKEN_PLUS ||
                             token->type_ == Token_Type::TOKEN_MINUS))
    {
        ParseEatAndNextToken();
        expr = ParseReadParen();
        switch (token->type_)
        {
        case Token_Type::TOKEN_NOT:
            expr_op = new ExprNode(Operator_Type::NOT);
            break;
        case Token_Type::TOKEN_PLUS:
            expr_op = new ExprNode(Operator_Type::POSITIVE);
            break;
        case Token_Type::TOKEN_MINUS:
            expr_op = new ExprNode(Operator_Type::NEGATIVE);
            break;
        }
        expr = concatenate_expr_node(expr, expr_op);
        token = ParseNextToken();
    }
    else
    {
        expr = ParseReadParen();
    }
    return expr;
}

ExprNode *Parser::ParseReadParen()
{
    ExprNode *expr = nullptr;
    auto token = ParseNextToken();
    if (token == nullptr)
        return ParseError<ExprNode *>("Syntax error!");
    if (token != nullptr && (token->type_ == Token_Type::TOKEN_OPEN_PARENTHESIS))
    {
        ParseEatAndNextToken();
        expr = ParseReadBooleanOr();
        token = ParseNextToken();
        if (token == nullptr || (token->type_ != Token_Type::TOKEN_CLOSE_PARENTHESIS))
            return ParseError<ExprNode *>("Syntax error: missing \")\"!");
        else { ParseEatAndNextToken(); }
    }
    else
    {
        expr = ParseReadBuiltin();
    }
    return expr;
}

ExprNode *Parser::ParseReadBuiltin()
{
    ExprNode *expr = nullptr;
    auto token = ParseNextToken();
    if (token == nullptr)
        return ParseError<ExprNode *>("Syntax error!");
    // The difference between string and column ref is that string has ""
    // Currently word in expression must be column ref
    if (token->type_ == Token_Type::TOKEN_WORD)
    {
        std::string text = token->text_;
        token = ParseEatAndNextToken();
        if (token != nullptr && token->type_ == Token_Type::TOKEN_OPEN_PARENTHESIS)
        {
            // TODO: parse function
        }
        else 
        {
            // Identifier: there are three forms of column_ref:
            // student.*, student.sno, sno
            std::shared_ptr<ColumnRef> column_ref = nullptr;
            auto pos = text.find(".");
            if (pos != std::string::npos)
            {
                column_ref = std::make_shared<ColumnRef>(text.substr(pos + 1), text.substr(0, pos));
            }
            else
            {
                column_ref = std::make_shared<ColumnRef>(text);
            }
            auto term = std::make_shared<TermExpr>(*column_ref);
            expr = new ExprNode(Operator_Type::NONE, term);
        }
    }
    else if (token->type_ == Token_Type::TOKEN_MULTIPLY)
    {
        // Select statement wildcard *
        std::string text = token->text_;
        ColumnRef *column_ref = new ColumnRef(text);
        auto term = std::make_shared<TermExpr>(column_ref);
        expr = new ExprNode(Operator_Type::NONE, term);
    }
    else
    {
        expr = ParseReadLiteral();
    }
    return expr;
}

ExprNode *Parser::ParseReadLiteral()
{
    ExprNode *expr = nullptr;
    auto token = ParseNextToken();
    if (token == nullptr)
        return ParseError<ExprNode *>("Syntax error: missing number!");
    if (token->type_ == Token_Type::TOKEN_DECIMAL ||
        token->type_ == Token_Type::TOKEN_ZERO)
    {
        auto term = std::make_shared<TermExpr>(stoi(token->text_));
        expr = new ExprNode(Operator_Type::NONE, term);
        ParseEatAndNextToken();
        return expr;
    }
    else if (token->type_ == Token_Type::TOKEN_FLOAT ||
             token->type_ == Token_Type::TOKEN_EXP_FLOAT)
    {
        auto term = std::make_shared<TermExpr>(stod(token->text_));
        expr = new ExprNode(Operator_Type::NONE, term);
        ParseEatAndNextToken();
        return expr;
    }
    else if (token->type_ == Token_Type::TOKEN_STRING)
    {
        auto term = std::make_shared<TermExpr>(token->text_);
        expr = new ExprNode(Operator_Type::NONE, term);
        ParseEatAndNextToken();
        return expr;
    }
    return ParseError<ExprNode *>("Syntax error: unenabled data type: " + token->text_ + "!");
}

ExprNode *Parser::concatenate_expr_node(ExprNode *expr1, ExprNode *expr2)
{
    if (expr1 == nullptr) { return expr2; }
    ExprNode *p = expr1;
    while (p->next_expr_ != nullptr)
    {
        p = p->next_expr_;
    }
    p->next_expr_ = expr2;
    return expr1;
}