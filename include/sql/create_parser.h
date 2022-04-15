#ifndef STATEMENT_PARSER_H_
#define STATEMENT_PARSER_H_

#include "parser.h"
#include "sql_stmt_create.h"
#include <memory>

class CreateParser : public Parser
{
public:
    CreateParser(std::shared_ptr<Tokenizer> tokenizer);
    ~CreateParser();

    std::shared_ptr<SQLStmtCreate> ParseSQLStmtCreate();

private:
    std::shared_ptr<FieldInfo> ParseSQLStmtColumnExpr();
};

#endif