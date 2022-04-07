#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2.hpp"

#include <iostream>

#include "../src/token.h"
#include "../src/tokenizer.h"
#include "../src/tokenizer.cc" // TODO: for elegant

TEST_CASE( "TOKENIZER", "[tokenizer test]" ) {
    Tokenizer tokenizer("select name age from users");
    Token *token = nullptr;
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