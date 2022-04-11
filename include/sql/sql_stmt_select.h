#ifndef SQL_STMT_SELECT_H_
#define SQL_STMT_SELECT_H_

#include "parser.h"

class SelectParser : public Parser
{
public:
    SelectParser(std::shared_ptr<Tokenizer> tokenizer);
    ~SelectParser();

    
};

#endif