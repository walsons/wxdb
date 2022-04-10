/**************************************
 ********** lexical analysis **********
 **************************************/

#ifndef TOKENIZER_H_
#define TOKENIZER_H_

#include "token.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

class Tokenizer
{
public:
    Tokenizer(std::string statement);
    ~Tokenizer();

    std::shared_ptr<Token> MakeToken(Token_Type token_type);
    std::shared_ptr<Token> GetNextToken();

private:
    std::string input_stream_;
    std::string::size_type input_iter_;
    std::string token_buffer_;
    std::string::size_type buffer_iter_;
    Token_Type curr_token_type_;
    std::unordered_map<std::string, bool> reserved_word_;

private:
    bool NextChar();
    void ClearBuffer();
    bool IsReservedWord(std::string word);
    std::shared_ptr<Token> word();
    std::shared_ptr<Token> zero();
    std::shared_ptr<Token> octal();
    std::shared_ptr<Token> hex(bool is_x);
    std::shared_ptr<Token> fraction(bool is_dot);
    std::shared_ptr<Token> exp_fraction(bool is_e, bool is_sign);
    std::shared_ptr<Token> decimal();
    std::shared_ptr<Token> not_equal();
    std::shared_ptr<Token> double_quote();
    std::shared_ptr<Token> mod();
    std::shared_ptr<Token> single_quote();
    std::shared_ptr<Token> open_parenthesis();
    std::shared_ptr<Token> close_parenthesis();
    std::shared_ptr<Token> plus();
    std::shared_ptr<Token> minus();
    std::shared_ptr<Token> multiply();
    std::shared_ptr<Token> divide();
    std::shared_ptr<Token> comma();
    std::shared_ptr<Token> semicolon();
    std::shared_ptr<Token> lt();
    std::shared_ptr<Token> equal();
    std::shared_ptr<Token> gt();
    std::shared_ptr<Token> power();
    std::shared_ptr<Token> invalid();
};

#endif