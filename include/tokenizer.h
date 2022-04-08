/**************************************
 ********** lexical analysis **********
 **************************************/

#ifndef TOKENIZER_H_
#define TOKENIZER_H_

#include "token.h"
#include <string>
#include <vector>
#include <unordered_map>

class Tokenizer
{
public:
    Tokenizer(std::string statement);
    ~Tokenizer();

    bool NextChar();
    void ClearBuffer();
    Token *MakeToken(Token_Type token_type);
    Token *GetNextToken();

private:
    std::string input_stream_;
    std::string::size_type input_iter_;
    std::string token_buffer_;
    std::string::size_type buffer_iter_;
    Token_Type curr_token_type_;
    std::unordered_map<std::string, bool> reserved_word_;

private:
    bool IsReservedWord(std::string word);
    Token *word();
    Token *zero();
    Token *octal();
    Token *hex(bool is_x);
    Token *fraction(bool is_dot);
    Token *decimal();
    Token *not_equal();
    Token *double_quote();

    Token *open_parenthesis();
    Token *close_parenthesis();

    Token *semicolon();

    Token *invalid();
};

#endif