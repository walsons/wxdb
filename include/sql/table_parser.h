#ifndef TABLE_PARSER_H_
#define TABLE_PARSER_H_

#include <memory>
#include "parser.h"
#include "table_info.h"

class TableParser : public Parser
{
public:
    using Parser::Parser;
    ~TableParser() = default;
    std::shared_ptr<TableInfo> CreateTable();
    std::shared_ptr<InsertInfo> InsertTable();
private:
    std::shared_ptr<FieldInfo> parse_column_expr();
    std::shared_ptr<std::vector<DataValue>> parse_value_expr();
};

#endif