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
 * expr: plus | mnus
 * term: mutiply | divide
 * power
 * unary: not | plus | minus (positive negative)
 * paren: open_paren | close_paren ------
 * ---> or
 * ---> read_builtin: word | func | identifier | mutiply (wildcard)
 *      literal: float | decimal | exp_float | string | char 
 ******************************************************/ 

Expression *Parser::ParseExpressionRD()
{
    Expression *expr = ParseReadBooleanOr();
    return expr;
}

Expression *Parser::ParseReadBooleanOr()
{
    Expression *expr0 = nullptr, *expr1 = nullptr, *expr2 = nullptr;
    auto token = ParseNextToken();
    expr0 = ParseReadBooleanAnd();
    return expr0;
}

Expression *Parser::ParseReadBooleanAnd()
{
    Expression *expr0 = nullptr, *expr1 = nullptr, *expr2 = nullptr;
    auto token = ParseNextToken();
    expr0 = ParseReadBooleanEquality();
    return expr0;
}

Expression *Parser::ParseReadBooleanEquality()
{
    Expression *expr0 = nullptr, *expr1 = nullptr, *expr2 = nullptr;
    auto token = ParseNextToken();
    expr0 = ParseReadBooleanComparison();
    return expr0;
}

Expression *Parser::ParseReadBooleanComparison()
{
    Expression *expr0 = nullptr, *expr1 = nullptr, *expr2 = nullptr;
    auto token = ParseNextToken();
    expr0 = ParseReadExpr();
    return expr0;
}

Expression *Parser::ParseReadExpr()
{
    Expression *expr0 = nullptr, *expr1 = nullptr, *expr2 = nullptr;
    auto token = ParseNextToken();
    if (token == nullptr)
    {
        ParseError("Syntax error!");
        return nullptr;
    }
    // Processing + -
    // TODO: probably this form: '+'->'expression'->'expression'
    if (token->type_ == Token_Type::TOKEN_PLUS || 
        token->type_ == Token_Type::TOKEN_MINUS)
    {
        ParseEatToken();
        expr0 = ParseReadTerm();
    }
    return expr0;
}

Expression *Parser::ParseReadTerm()
{
    Expression *expr0 = nullptr, *expr1 = nullptr, *expr2 = nullptr;
    auto token = ParseNextToken();
    expr0 = ParseReadPower();
    return expr0;
}

Expression *Parser::ParseReadPower()
{
    Expression *expr0 = nullptr, *expr1 = nullptr;
    auto token = ParseNextToken();
    expr0 = ParseReadUnary();
    return expr0;
}

Expression *Parser::ParseReadUnary()
{
    Expression *expr0 = nullptr, *expr1 = nullptr;
    auto token = ParseNextToken();
    if (token == nullptr)
    {
        ParseError("Syntax error!");
        return nullptr;
    }
    // Not, positive sign and minus sign
    if (token != nullptr && 
        (token->type_ == Token_Type::TOKEN_NOT ||
         token->type_ == Token_Type::TOKEN_MINUS ||
         token->type_ == Token_Type::TOKEN_PLUS))
    {
        ParseEatAndNextToken();
        expr1 = ParseReadParen();
    }
    return expr1;
}

Expression *Parser::ParseReadParen()
{
    Expression *expr0 = nullptr;
    auto token = ParseNextToken();
    if (token == nullptr)
    {
        ParseError("Syntax error!");
        return nullptr;
    }
    if (token->type_ == Token_Type::TOKEN_OPEN_PARENTHESIS)
    {
        ParseEatAndNextToken();
        expr0 = ParseReadBooleanOr();
        token = ParseNextToken();
        if (token == nullptr || token->type_ != Token_Type::TOKEN_CLOSE_PARENTHESIS)
        {
            ParseError("Syntax error: missing ')'!");
            return nullptr;
        }
        else 
        { 
            ParseEatToken(); 
        }
    }
    else 
    {
        expr0 = ParseReadBuiltin();
    }
    return expr0;
}

Expression *Parser::ParseReadBuiltin()
{
    Expression *expr0 = nullptr, *expr1 = nullptr;
    auto token = ParseNextToken();
    if (token == nullptr)
    {
        ParseError("Syntax error: missing id or number!");
        return nullptr;
    }
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
            // Identifier: there are two forms of column_ref:
            // student.*, student.sno, sno
            ColumnRef *column_ref = new ColumnRef(text);
            TermExpr *term = new TermExpr(Term_Type::TERM_COL_REF);
            term->ref = column_ref;
            expr0 = new Expression(Token_Type::TOKEN_WORD, term, nullptr);
        }
    }
    else if (token->type_ == Token_Type::TOKEN_MULTIPLY)
    {
        // Select statement wildcard *
        std::string text = token->text_;
        ColumnRef *column_ref = new ColumnRef(text);
        TermExpr *term = new TermExpr(Term_Type::TERM_COL_REF);
        term->ref = column_ref;
        expr0 = new Expression(Token_Type::TOKEN_WORD, term, nullptr);
        ParseEatAndNextToken();
    }
    else
    {
        // TODO: expr0 = ParseReadLiteral()
    }
    return expr0;
}

Expression *Parser::ParseReadLiteral()
{
    Expression *expr0 = nullptr;
    auto token = ParseNextToken();
    if (token == nullptr)
    {
        ParseError("Syntax error: missing number!");
        return nullptr;
    }
    if (token->type_ == Token_Type::TOKEN_FLOAT ||
        token->type_ == Token_Type::TOKEN_DECIMAL ||
        token->type_ == Token_Type::TOKEN_EXP_FLOAT)
    {
        // TODO: support other numerical type
        Literal *literal = new IntLiteral(Data_Type::DATA_TYPE_INT, token->text_);
        TermExpr *term = new TermExpr(Term_Type::TERM_LITERAL);
        term->val = literal;
        expr0 = new Expression(token->type_, term, nullptr);
        ParseEatToken();
        return expr0;
    }
    else if (token->type_ == Token_Type::TOKEN_STRING ||
             token->type_ == Token_Type::TOKEN_CHAR)
    {
        // Both string and char use DATA_TYPE_CHAR to process
        // If it's string, remove double quotation
        if (token->text_[0] == '"')
        {
            token->text_ = token->text_.substr(1, token->text_.size() - 2);
        }
        Literal *literal = new Literal(Data_Type::DATA_TYPE_CHAR, token->text_);
        TermExpr *term = new TermExpr(Term_Type::TERM_LITERAL);
        term->val = literal;
        expr0 = new Expression(Token_Type::TOKEN_STRING, term, nullptr);
        ParseEatToken();
        return expr0;
    }
    // TODO: support other type
    ParseError("Syntax error: unenabled data type: " + token->text_ + "!");
    return nullptr;
}