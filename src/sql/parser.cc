#include "../../include/sql/parser.h"
#include <memory>

Parser::Parser(std::shared_ptr<Tokenizer> tokenizer)
    : tokenizer_(tokenizer)
    , curr_token_(nullptr)
    , parser_state_type_(Parser_State_Type::PARSER_CORRECT)
{
}

Parser::~Parser() = default;

// Get next token, if current token doesn't be eat, return current token
std::shared_ptr<Token> Parser::ParseNextToken()
{
    if (parser_state_type_ == Parser_State_Type::PARSER_WRONG) { return nullptr; }
    if (curr_token_ == NULL) 
    {
        curr_token_ = tokenizer_->GetNextToken();
    }
    return curr_token_;
}

// Eat current token
std::shared_ptr<Token> Parser::ParseEatToken()
{
    if (parser_state_type_ == Parser_State_Type::PARSER_WRONG) { return nullptr; }
    curr_token_ = nullptr;
    return nullptr;
}

// Eat current token then return next token
std::shared_ptr<Token> Parser::ParseEatAndNextToken()
{
    if (parser_state_type_ == Parser_State_Type::PARSER_WRONG) { return nullptr; }
    curr_token_ = tokenizer_->GetNextToken();
    return curr_token_;
}

void Parser::ParseError(const std::string &message)
{
    parser_state_type_ = Parser_State_Type::PARSER_WRONG;
    parser_message_ = message;
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
        ParseEatToken();
        expr2 = ParseReadBooleanAnd();
        expr1 = concatenate_expr_node(expr1, expr2);
        expr_op = new ExprNode(token->type_);
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
        ParseEatToken();
        expr2 = ParseReadBooleanEquality();
        expr1 = concatenate_expr_node(expr1, expr2);
        expr_op = new ExprNode(token->type_);
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
        ParseEatToken();
        expr2 = ParseReadBooleanComparison();
        expr1 = concatenate_expr_node(expr1, expr2);
        expr_op = new ExprNode(token->type_);
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
        ParseEatToken();
        expr2 = ParseReadExpr();
        expr1 = concatenate_expr_node(expr1, expr2);
        expr_op = new ExprNode(token->type_);
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
        ParseEatToken();
        expr2 = ParseReadTerm();
        expr1 = concatenate_expr_node(expr1, expr2);
        expr_op = new ExprNode(token->type_);
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
                                token->type_ == Token_Type::TOKEN_MULTIPLY))
    {
        ParseEatToken();
        expr2 = ParseReadPower();
        expr1 = concatenate_expr_node(expr1, expr2);
        expr_op = new ExprNode(token->type_);
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
        ParseEatToken();
        expr2 = ParseReadUnary();
        expr1 = concatenate_expr_node(expr1, expr2);
        expr_op = new ExprNode(token->type_);
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
    {
        ParseError("Syntax error!");
        return nullptr;
    }
    if (token != nullptr && (token->type_ == Token_Type::TOKEN_NOT ||
                             token->type_ == Token_Type::TOKEN_PLUS ||
                             token->type_ == Token_Type::TOKEN_MINUS))
    {
        switch (token->type_)
        {
        case Token_Type::TOKEN_PLUS:
            token->type_ = Token_Type::TOKEN_POSITIVE;
            break;
        case Token_Type::TOKEN_MINUS:
            token->type_ = Token_Type::TOKEN_NEGATIVE;
            break;
        default:
            break;
        }
        ParseEatToken();
        expr = ParseReadParen();
        expr_op = new ExprNode(token->type_);
        expr = concatenate_expr_node(expr, expr_op);
        token = ParseNextToken();
    }
    return expr;
}

ExprNode *Parser::ParseReadParen()
{
    ExprNode *expr = nullptr;
    auto token = ParseNextToken();
    if (token == nullptr)
    {
        ParseError("Syntax error!");
        return nullptr;
    }
    if (token != nullptr && (token->type_ == Token_Type::TOKEN_OPEN_PARENTHESIS))
    {
        ParseEatToken();
        expr = ParseReadBooleanOr();
        token = ParseNextToken();
        if (token == nullptr || (token->type_ != Token_Type::TOKEN_CLOSE_PARENTHESIS))
        {
            ParseError("Syntax error: missing \")\"!");
            return nullptr;
        }
        else { ParseEatToken(); }
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
    {
        ParseError("Syntax error!");
        return nullptr;
    }
    // The difference between string and column ref is that string has ""
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
            auto column_ref = new ColumnRef(text);
            auto term = std::make_shared<TermExpr>(Term_Type::TERM_COL_REF);
            term->ref_ = column_ref;
            expr = new ExprNode(Token_Type::TOKEN_WORD, term);
        }
    }
    else if (token->type_ == Token_Type::TOKEN_MULTIPLY)
    {
        // Select statement wildcard *
        std::string text = token->text_;
        ColumnRef *column_ref = new ColumnRef(text);
        auto term = std::make_shared<TermExpr>(Term_Type::TERM_COL_REF);
        term->ref_ = column_ref;
        expr = new ExprNode(Token_Type::TOKEN_WORD, term, nullptr);
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
    {
        ParseError("Syntax error: missing number!");
        return nullptr;
    }
    if (token->type_ == Token_Type::TOKEN_DECIMAL ||
        token->type_ == Token_Type::TOKEN_ZERO)
    {
        Literal *literal = new IntLiteral(Data_Type::DATA_TYPE_INT, token->text_);
        auto term = std::make_shared<TermExpr>(Term_Type::TERM_LITERAL);
        term->val_ = literal;
        expr = new ExprNode(token->type_, term);
        ParseEatToken();
        return expr;
    }
    else if (token->type_ == Token_Type::TOKEN_FLOAT ||
             token->type_ == Token_Type::TOKEN_EXP_FLOAT)
    {
        Literal *literal = new DoubleLiteral(Data_Type::DATA_TYPE_INT, token->text_);
        auto term = std::make_shared<TermExpr>(Term_Type::TERM_LITERAL);
        term->val_ = literal;
        expr = new ExprNode(token->type_, term);
        ParseEatToken();
        return expr;
    }
    else if (token->type_ == Token_Type::TOKEN_STRING)
    {
        Literal *literal = new Literal(Data_Type::DATA_TYPE_CHAR, token->text_);
        auto term = std::make_shared<TermExpr>(Term_Type::TERM_LITERAL);
        term->val_ = literal;
        expr = new ExprNode(Token_Type::TOKEN_STRING, term);
        ParseEatToken();
        return expr;
    }
    ParseError("Syntax error: unenabled data type: " + token->text_ + "!");
    return nullptr;
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