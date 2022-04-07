#ifndef STATEMENT_PARSER_H_
#define STATEMENT_PARSER_H_

#include "parser.h"

class CreateParser : public Parser
{
public:
    CreateParser(Tokenizer *tokenizer);
    ~CreateParser();
};

#endif