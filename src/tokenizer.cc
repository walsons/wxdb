#include "../include/tokenizer.h"
#include <cctype>
#include <cassert>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

Tokenizer::Tokenizer(std::string statement)
    : input_stream_(statement), 
      input_iter_(0),
      token_buffer_(""),
      buffer_iter_(0),
      curr_token_type_(Token_Type::TOKEN_INVALID)
{
    std::vector<std::string> reserved_word{
        "select", "from", "where", "order", "by", "group", "create", "table", "index", "and", "not", "or", "null",
        "like", "in", "grant", "int", "char", "values", "insert", "into", "update", "delete", "set", "on",
        "user", "view", "rule", "default", "check", "between", "trigger", "primary", "key", "foreign"
    };
    for (const auto &word : reserved_word)
    {
        reserved_word_[word] = true;
    }
}

Tokenizer::~Tokenizer() 
{
}

bool Tokenizer::NextChar()
{
    token_buffer_.insert(token_buffer_.size(), 1, input_stream_[input_iter_]);
    ++input_iter_;
    ++buffer_iter_;
    bool next_not_null = true;
    if (input_iter_ == input_stream_.size())
    {
        next_not_null = false;
    }
    return next_not_null;
}

void Tokenizer::ClearBuffer()
{
    buffer_iter_ = 0;
    token_buffer_.clear();
}

Token *Tokenizer::MakeToken(Token_Type token_type)
{
    return new Token(token_buffer_, token_type);
}

Token *Tokenizer::GetNextToken()
{
    // If reach end of statement, return nullptr
    if (input_iter_ == input_stream_.size()) { return nullptr; }

    // Skip space character
    char c = input_stream_[input_iter_];
    while (c == ' ')
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
    else if (c == '%') {} // TODO
    else if (c == '\'') {} // TODO
    else if (c == '(') { return open_parenthesis(); }
    else if (c == ')') { return close_parenthesis(); }
    else if (c == '*') {} // TODO
    else if (c == '+') {} // TODO
    else if (c == ',') {} // TODO
    else if (c == '-') {} // TODO
    else if (c == '/') {} // TODO
    else if (c == ';') { return semicolon(); }
    // ......
    return invalid();
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

/********** Token states **********/

Token *Tokenizer::word()
{
    if (NextChar())
    {
        char c = input_stream_[input_iter_];
        if (std::isalnum(c) || c == '_') { return word(); }
        else if (c != ' ') { return MakeToken(Token_Type::TOKEN_INVALID); }
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
        else if (word == "like") { return MakeToken(Token_Type::TOKEN_LIKE); }
        else if (word == "in") { return MakeToken(Token_Type::TOKEN_IN); }
        // Other reserved word 
        return MakeToken(Token_Type::TOKEN_RESERVED_WORD);
    }
    return MakeToken(Token_Type::TOKEN_WORD);
}

Token *Tokenizer::zero()
{
    if (NextChar())
    {
        char c = input_stream_[input_iter_];
        if (c >= '0' && c <= '7') { return octal(); }
        else if (c == 'x' || c == 'X') { return hex(true); }
        else if (c == '.') { return fraction(true); }
        else if (c != ' ') { return MakeToken(Token_Type::TOKEN_INVALID); }
    }
    return MakeToken(Token_Type::TOKEN_ZERO);
}

Token *Tokenizer::octal()
{
    if (NextChar())
    {
        char c = input_stream_[input_iter_];
        if (c >= '0' && c <= '7') { return octal(); }
        else if (c != ' ') { return MakeToken(Token_Type::TOKEN_INVALID); }
    }
    return MakeToken(Token_Type::TOKEN_OCTAL);
}

Token *Tokenizer::hex(bool is_x)
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
        else if (c != ' ') { return MakeToken(Token_Type::TOKEN_INVALID); }
    }
    if (is_x) { return MakeToken(Token_Type::TOKEN_INVALID); }
    return MakeToken(Token_Type::TOKEN_HEX);
}

Token *Tokenizer::fraction(bool is_dot)
{
    if (NextChar())
    {
        char c = input_stream_[input_iter_];
        if (c >= '0' && c <= '9') { return fraction(false); }
        // TODO: support exponent
        else if (c != ' ') { return MakeToken(Token_Type::TOKEN_INVALID); }
    }
    if (is_dot) { return MakeToken(Token_Type::TOKEN_INVALID); }
    return MakeToken(Token_Type::TOKEN_FRACTION);
}

Token *Tokenizer::decimal()
{
    if (NextChar())
    {
        char c = input_stream_[input_iter_];
        if (std::isdigit(c)) { return decimal(); }
        else if (c == '.') { return fraction(true); }
        // TODO: support exponent
        else if (c != ' ') { return MakeToken(Token_Type::TOKEN_INVALID); }
    }
    return MakeToken(Token_Type::TOKEN_DECIMAL);
}

Token *Tokenizer::not_equal()
{
    if (NextChar())
    {
        char c = input_stream_[input_iter_];
        if (c == '=')
        {
            // In this case, we can ensure to get "!=", which make expression "a !=b" make sense
            NextChar();
            return MakeToken(Token_Type::TOKEN_NOT_EQ);
        }
    }
    return MakeToken(Token_Type::TOKEN_INVALID);
}

Token *Tokenizer::double_quote()
{
    bool end_of_string = false;
    if (NextChar())
    {
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
    return MakeToken(Token_Type::TOKEN_INVALID);
}

Token *Tokenizer::open_parenthesis()
{
    NextChar();
    return MakeToken(Token_Type::TOKEN_OPEN_PARENTHESIS);
}

Token *Tokenizer::close_parenthesis()
{
    NextChar();
    return MakeToken(Token_Type::TOKEN_CLOSE_PARENTHESIS);
}

Token *Tokenizer::semicolon()
{
    NextChar();
    return MakeToken(Token_Type::TOKEN_SEMICOLON);
}

Token *Tokenizer::invalid()
{
    NextChar();
    return MakeToken(Token_Type::TOKEN_INVALID);
}