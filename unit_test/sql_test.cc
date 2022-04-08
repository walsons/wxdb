#include "catch.hpp"

#include <iostream>
#include <memory>

#include "../include/token.h"
#include "../include/tokenizer.h"

TEST_CASE( "TOKENIZER", "[tokenizer test]" ) {
    Tokenizer tokenizer("select name age from users");
    std::shared_ptr<Token> token = nullptr;
    token = tokenizer.GetNextToken();
    CHECK(token->text_ == "select");
    CHECK(token->type_ == Token_Type::TOKEN_RESERVED_WORD);
    token = tokenizer.GetNextToken();
    CHECK(token->text_ == "name");
    CHECK(token->type_ == Token_Type::TOKEN_WORD);
    token = tokenizer.GetNextToken();
    CHECK(token->text_ == "age");
    CHECK(token->type_ == Token_Type::TOKEN_WORD);
    token = tokenizer.GetNextToken();
    CHECK(token->text_ == "from");
    CHECK(token->type_ == Token_Type::TOKEN_RESERVED_WORD);
    token = tokenizer.GetNextToken();
    CHECK(token->text_ == "users");
    CHECK(token->type_ == Token_Type::TOKEN_WORD);
    token = tokenizer.GetNextToken();
    CHECK(token == nullptr);
};