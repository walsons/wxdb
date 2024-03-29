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
    std::shared_ptr<SelectInfo> SelectTable();
    std::shared_ptr<DeleteInfo> DeleteTable();
    std::shared_ptr<UpdateInfo> UpdateTable();
private:
    std::shared_ptr<FieldInfo> parse_field_expr();
    std::shared_ptr<ConstraintInfo> parse_constraint_expr();
    std::shared_ptr<std::vector<ColVal>> parse_value_expr();
    std::shared_ptr<std::unordered_map<std::string, ColVal>> parse_set_expr();
};

#endif