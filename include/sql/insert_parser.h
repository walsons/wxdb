#ifndef INSERT_PARSER_H_
#define INSERT_PARSER_H_

#include "parser.h"
#include "sql_stmt_insert.h"
#include <memory>

class InsertParser : public Parser
{
    InsertParser(std::shared_ptr<Tokenizer> tokenizer);
    ~InsertParser();

    std::shared_ptr<SQLStmtInsert> ParseSQLStmtInsert();
    std::shared_ptr<std::vector<DataValue>> ParseDataValueExpr();
};

#endif