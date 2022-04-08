#ifndef STATEMENT_PARSER_H_
#define STATEMENT_PARSER_H_

#include "parser.h"
#include "sql_statement.h"
#include <memory>

constexpr int INT_SIZE = 4;

class CreateParser : public Parser
{
public:
    CreateParser(Tokenizer *tokenizer);
    ~CreateParser();

    std::shared_ptr<SQLStmtCreate> ParseSQLStmtCreate();
    std::shared_ptr<FieldInfo> ParseSQLStmtColumnExpr();
};

#endif