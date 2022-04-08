#ifndef STATEMENT_PARSER_H_
#define STATEMENT_PARSER_H_

#include "parser.h"
#include "sql_statement.h"

constexpr int INT_SIZE = 4;

class CreateParser : public Parser
{
public:
    CreateParser(Tokenizer *tokenizer);
    ~CreateParser();

    SQLStmtCreate *ParseSQLStmtCreate();
    FieldInfo *ParseSQLStmtColumnExpr();
};

#endif