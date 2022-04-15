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

    SRA *ParseSQLStmtSelect();
    std::vector<Expression *> ParseFieldsExpr();
    SRA *ParseTablesExpr();
};

#endif