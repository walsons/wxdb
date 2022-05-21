#include "../../include/sql/tokenizer.h"
#include <cctype>
#include <cassert>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>

Tokenizer::Tokenizer(std::string statement)
    : input_stream_(statement), 
      input_iter_(0),
      token_buffer_(""),
      buffer_iter_(0),
      curr_token_type_(Token_Type::TOKEN_INVALID)
{
    std::vector<std::string> reserved_word{
        "create", "use", "select", "from",
        "database", "table",
        "where", "order", "by", "group", "index", "and", "not", "or", "null", "references",
        "like", "in", "grant", "values", "insert", "into", "update", 
        "delete", "set", "on", "user", "view", "rule", "default", "check", "between", "trigger", "unique", "primary", 
        "key", "foreign", "check",
        "int", "integer", "double", "char", "varchar", "date", "bool"
    };
    for (const auto &word : reserved_word)
    {
        reserved_word_[word] = true;
    }
}

Tokenizer::~Tokenizer() = default;

inline
std::shared_ptr<Token> Tokenizer::MakeToken(Token_Type token_type)
{
    if (token_type == Token_Type::TOKEN_RESERVED_WORD || 
        token_type == Token_Type::TOKEN_AND ||
        token_type == Token_Type::TOKEN_OR ||
        token_type == Token_Type::TOKEN_NOT ||
        token_type == Token_Type::TOKEN_NULL)
    {
        std::for_each(token_buffer_.begin(), token_buffer_.end(), [&](char &c){c = tolower(c); });
    }
    return std::make_shared<Token>(token_buffer_, token_type);
}

std::shared_ptr<Token> Tokenizer::GetNextToken()
{
    // If reach end of statement, return nullptr
    if (input_iter_ == input_stream_.size()) { return nullptr; }

    // Skip space character
    char c = input_stream_[input_iter_];
    while (c == ' ' || c == '\n')
    {
        if (!NextChar()) { return nullptr; }
        c = input_stream_[input_iter_];
    }
    ClearBuffer();

    if (std::isalpha(c) || c == '_') { return word(); }
    else if (c == '0') { return zero(); }
    else if (std::isdigit(c)) { return decimal(); }
    else if (c == '!') { return not_equal(); }
    else if (c == '"') { return double_quote(); }
    else if (c == '%') { return mod(); } 
    else if (c == '\'') { return single_quote(); }
    else if (c == '(') { return open_parenthesis(); }
    else if (c == ')') { return close_parenthesis(); }
    else if (c == '+') { return plus(); }
    else if (c == '-') { return minus(); }
    else if (c == '*') { return multiply(); }
    else if (c == '/') { return divide(); }
    else if (c == ',') { return comma(); }
    else if (c == ';') { return semicolon(); }
    else if (c == '<') { return lt(); }
    else if (c == '=') { return equal(); }
    else if (c == '>') { return gt(); }
    else if (c == '^') { return power(); }
    return invalid();
}

bool Tokenizer::NextChar()
{
    token_buffer_.insert(buffer_iter_, 1, input_stream_[input_iter_]);
    ++input_iter_;
    ++buffer_iter_;
    bool next_not_null = true;
    if (input_iter_ == input_stream_.size())
    {
        next_not_null = false;
    }
    return next_not_null;
}

inline
void Tokenizer::ClearBuffer()
{
    buffer_iter_ = 0;
    token_buffer_.clear();
}

bool Tokenizer::IsReservedWord(std::string word)
{
    std::for_each(word.begin(), word.end(), [](auto &c){
        c = std::tolower(c);
    });
    if (reserved_word_.find(word) != reserved_word_.end()) 
    { 
        return true; 
    }
    return false;
}

/**********************************
 ********** Token states **********
 **********************************/

std::shared_ptr<Token> Tokenizer::word()
{
    if (NextChar())
    {
        char c = input_stream_[input_iter_];
        if (std::isalnum(c) || c == '_' || c == '.' || c == '*') { return word(); }
    }
    if (IsReservedWord(token_buffer_)) 
    {
        std::string word = token_buffer_;
        std::for_each(word.begin(), word.end(), [](auto &c){
            c = std::tolower(c);
        });
        if (word == "and") { return MakeToken(Token_Type::TOKEN_AND); }
        else if (word == "or") { return MakeToken(Token_Type::TOKEN_OR); }
        else if (word == "not") { return MakeToken(Token_Type::TOKEN_NOT); }
        else if (word == "null") { return MakeToken(Token_Type::TOKEN_NULL); }
        // else if (word == "like") { return MakeToken(Token_Type::TOKEN_LIKE); }
        // else if (word == "in") { return MakeToken(Token_Type::TOKEN_IN); }
        // Other reserved word 
        return MakeToken(Token_Type::TOKEN_RESERVED_WORD);
    }
    return MakeToken(Token_Type::TOKEN_WORD);
}

std::shared_ptr<Token> Tokenizer::zero()
{
    if (NextChar())
    {
        char c = input_stream_[input_iter_];
        if (c >= '0' && c <= '7') { return octal(); }
        else if (c == 'x' || c == 'X') { return hex(true); }
        else if (c == '.') { return fraction(true); }
    }
    return MakeToken(Token_Type::TOKEN_ZERO);
}

std::shared_ptr<Token> Tokenizer::octal()
{
    if (NextChar())
    {
        char c = input_stream_[input_iter_];
        if (c >= '0' && c <= '7') { return octal(); }
    }
    return MakeToken(Token_Type::TOKEN_OCTAL);
}

std::shared_ptr<Token> Tokenizer::hex(bool is_x)
{
    if (NextChar())
    {
        char c = input_stream_[input_iter_];
        if ((c >= '0' && c <= '9') || 
            (c >= 'a' && c <= 'f') || 
            (c >= 'A' && c <= 'F'))
        {
            return hex(false);
        }
    }
    if (is_x) { return MakeToken(Token_Type::TOKEN_INVALID); }
    return MakeToken(Token_Type::TOKEN_HEX);
}

std::shared_ptr<Token> Tokenizer::fraction(bool is_dot)
{
    if (NextChar())
    {
        char c = input_stream_[input_iter_];
        if (std::isdigit(c)) { return fraction(false); }
        // Support 0.e3
        if (c == 'e' || c == 'E') { return exp_fraction(true, false); }
    }
    if (is_dot) { return MakeToken(Token_Type::TOKEN_INVALID); }
    return MakeToken(Token_Type::TOKEN_FLOAT);
}

std::shared_ptr<Token> Tokenizer::exp_fraction(bool is_e, bool is_sign)
{
    if (NextChar())
    {
        char c = input_stream_[input_iter_];
        if (std::isdigit(c)) { return exp_fraction(false, false); }
        // If both is_e and is_sign are false, indicate that end is digit.
        if (!is_e && !is_sign) { return MakeToken(Token_Type::TOKEN_EXP_FLOAT); }
        if (c == '+' || c == '-') 
        { 
            if (is_sign) { return MakeToken(Token_Type::TOKEN_INVALID); }
            return exp_fraction(false, true);
        }
    }
    if (is_e) { return MakeToken(Token_Type::TOKEN_INVALID); }
    if (is_sign) { return MakeToken(Token_Type::TOKEN_INVALID); }
    return MakeToken(Token_Type::TOKEN_EXP_FLOAT);
}

std::shared_ptr<Token> Tokenizer::decimal()
{
    if (NextChar())
    {
        char c = input_stream_[input_iter_];
        if (std::isdigit(c)) { return decimal(); }
        if (c == '.') { return fraction(true); }
        if (c == 'e' || c == 'E') { return exp_fraction(true, false); }
    }
    return MakeToken(Token_Type::TOKEN_DECIMAL);
}

std::shared_ptr<Token> Tokenizer::not_equal()
{
    if (NextChar())
    {
        char c = input_stream_[input_iter_];
        if (c == '=')
        {
            NextChar();
            return MakeToken(Token_Type::TOKEN_NOT_EQ);
        }
    }
    return MakeToken(Token_Type::TOKEN_INVALID);
}

std::shared_ptr<Token> Tokenizer::double_quote()
{
    if (NextChar())
    {
        bool end_of_string = false;
        char c;
        while ((c = input_stream_[input_iter_]) != '"')
        {
            if (!NextChar()) 
            { 
                end_of_string = true;
                break; 
            }
        }
        if (end_of_string) { return MakeToken(Token_Type::TOKEN_UNENDED_STRING); }
        // If match second quote("), make token, such as std::cout << "Yes";
        NextChar();
        return MakeToken(Token_Type::TOKEN_STRING);
    }
    return MakeToken(Token_Type::TOKEN_UNENDED_STRING);
}

inline
std::shared_ptr<Token> Tokenizer::mod()
{
    NextChar();
    return MakeToken(Token_Type::TOKEN_MOD);
}

std::shared_ptr<Token> Tokenizer::single_quote()
{
    if (NextChar())
    {
        bool end_of_string = false;
        char c;
        while ((c = input_stream_[input_iter_]) != '\'')
        {
            if (!NextChar()) 
            { 
                end_of_string = true;
                break; 
            }
        }
        if (end_of_string) { return MakeToken(Token_Type::TOKEN_UNENDED_STRING); }
        NextChar();
        return MakeToken(Token_Type::TOKEN_STRING);
    }
    return MakeToken(Token_Type::TOKEN_UNENDED_STRING);
}

inline
std::shared_ptr<Token> Tokenizer::open_parenthesis()
{
    NextChar();
    return MakeToken(Token_Type::TOKEN_OPEN_PARENTHESIS);
}

inline
std::shared_ptr<Token> Tokenizer::close_parenthesis()
{
    NextChar();
    return MakeToken(Token_Type::TOKEN_CLOSE_PARENTHESIS);
}

inline
std::shared_ptr<Token> Tokenizer::plus()
{
    NextChar();
    return MakeToken(Token_Type::TOKEN_PLUS);
}

inline
std::shared_ptr<Token> Tokenizer::minus()
{
    NextChar();
    return MakeToken(Token_Type::TOKEN_MINUS);
}

inline
std::shared_ptr<Token> Tokenizer::multiply()
{
    NextChar();
    return MakeToken(Token_Type::TOKEN_MULTIPLY);
}

inline
std::shared_ptr<Token> Tokenizer::divide()
{
    NextChar();
    return MakeToken(Token_Type::TOKEN_DIVIDE);
}

inline
std::shared_ptr<Token> Tokenizer::comma()
{
    NextChar();
    return MakeToken(Token_Type::TOKEN_COMMA);
}

inline
std::shared_ptr<Token> Tokenizer::semicolon()
{
    NextChar();
    return MakeToken(Token_Type::TOKEN_SEMICOLON);
}

std::shared_ptr<Token> Tokenizer::lt()
{
    if (NextChar())
    {
        if (input_stream_[input_iter_] == '=')
        {
            NextChar();
            return MakeToken(Token_Type::TOKEN_LE);
        }
    }
    return MakeToken(Token_Type::TOKEN_LT);
}

std::shared_ptr<Token> Tokenizer::equal()
{
    return MakeToken(Token_Type::TOKEN_EQ);
}

std::shared_ptr<Token> Tokenizer::gt()
{
    if (NextChar())
    {
        if (input_stream_[input_iter_] == '=')
        {
            NextChar();
            return MakeToken(Token_Type::TOKEN_GE);
        }
    }
    return MakeToken(Token_Type::TOKEN_GT);
}

inline
std::shared_ptr<Token> Tokenizer::power()
{
    NextChar();
    return MakeToken(Token_Type::TOKEN_POWER);
}

inline
std::shared_ptr<Token> Tokenizer::invalid()
{
    NextChar();
    return MakeToken(Token_Type::TOKEN_INVALID);
}