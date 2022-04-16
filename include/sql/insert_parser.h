#ifndef INSERT_PARSER_H_
#define INSERT_PARSER_H_

#include "parser.h"
#include "sql_stmt_insert.h"
#include <memory>

class InsertParser : public Parser
{
public:
    InsertParser(std::shared_ptr<Tokenizer> tokenizer);
    ~InsertParser();

    std::shared_ptr<SQLStmtInsert> ParseSQLStmtInsert();

public:
    std::shared_ptr<std::vector<DataValue>> ParseDataValueExpr();
};

#endif