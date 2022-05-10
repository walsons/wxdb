#include "../../include/db/table_header.h"
#include <cstring>
#include <memory>
#include "../../include/sql/table_info.h"

bool fill_table_header(std::shared_ptr<TableHeader> header, const TableInfo &table_info)
{
    std::memset(header.get(), 0, sizeof(TableHeader));
    std::strncpy(header->table_name, table_info.table_name.c_str(), MAX_LENGTH_NAME);
    int offset = 8;  // 4 bytes for __rowid__, and 4 bytes for not null
    auto field_info = table_info.field_info;
    for (auto field = field_info.begin(); field != field_info.end(); ++field)
    {
        // Field info
        int col_num = header->num_column++;
        std::strncpy(header->column_name[col_num], field->field_name.c_str(), MAX_LENGTH_NAME);
        header->column_type[col_num] = field->type;
        header->column_length[col_num] = field->length;
        header->column_offset[col_num] =  offset;
        offset += header->column_length[col_num];
        // constraint
        header->flag_not_null = table_info.flag_not_null;
        header->flag_primary = table_info.flag_primary;
        header->flag_unique = table_info.flag_unique;
        header->flag_default = table_info.flag_default;
        for (auto item : table_info.default_value)
        {
            Expression *expr = Expression::Eval();
            // header->default_value[item.first] = 
        }
    }
    return true;
}