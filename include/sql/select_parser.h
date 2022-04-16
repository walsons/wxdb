#ifndef SSELECT_PARSER_H_
#define SELECT_PARSER_H_

#include "parser.h"
#include "sra.h"
#include <memory>

class SelectParser : public Parser
{
public:
    SelectParser(std::shared_ptr<Tokenizer> tokenizer);
    ~SelectParser();

    std::shared_ptr<SRA> ParseSQLStmtSelect();

private:
    std::vector<std::shared_ptr<Expression>> ParseFieldsExpr();
    std::shared_ptr<SRA> ParseTablesExpr();
};

#endif