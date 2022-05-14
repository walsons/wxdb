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
    }
    // Constraint
    header->flag_not_null = table_info.flag_not_null;
    header->flag_primary = table_info.flag_primary;
    header->flag_unique = table_info.flag_unique;
    header->flag_default = table_info.flag_default;
    for (auto item : table_info.default_value)
    {
        Expression expression{item.second};
        std::memcpy(header->default_value[item.first], reinterpret_cast<char *>(&expression), sizeof(expression));
    }
    // Foreign key
    for (auto item : table_info.foreign_key_ref)
    {
        header->foreign_key[header->num_foreign_key] = item.first;
        std::strncpy(header->foreign_key_ref_table[header->num_foreign_key], 
                     item.second.table_name.c_str(), MAX_LENGTH_NAME);
        std::strncpy(header->foreign_key_ref_column[header->num_foreign_key], 
                     item.second.column_name.c_str(), MAX_LENGTH_NAME);
        ++header->num_foreign_key;
    }
    // Check constraint
    for (auto item : table_info.check_constraint)
    {
        std::ostringstream os;
        Expression::DumpExprNode(os, item.check_condition);
        std::strncpy(header->check_constraint[header->num_check_constraint++], os.str().c_str(), MAX_LENGTH_CHECK_CONSTRAINT);
    }

    // Add "__rowid__" column (with highest index)
    int col_num = header->num_column++;
    std::strncpy(header->column_name[col_num], "__rowid__", MAX_LENGTH_NAME);
    header->column_type[col_num] = Data_Type::DATA_TYPE_INT;
    header->column_length[col_num] = sizeof(int);
    header->column_offset[col_num] = 0;
    header->main_index = col_num;
    header->is_main_index_auto_inc = true;
    // Add index for __rowid__
    header->flag_index |= (1 << col_num);
    // If table doesn't have primary key, set __rowid__ to primary key
    if (!header->flag_primary) { header->flag_primary |= (1 << col_num); }
    // Add index for unique column
    header->flag_index |= header->flag_unique;
    // Add not null for primary key column
    header->flag_not_null |= header->flag_primary;
    // Add index for the first primary key column
    int first_primary = 0;
    while (!(header->flag_primary & (1 << first_primary)))
    {
        ++first_primary;
    }
    header->flag_index |= (1 << first_primary);
    header->auto_inc = 1;
    header->num_primary_key = 0;
    for (size_t i = 0; i != header->num_column; ++i)
    {
        if (header->flag_primary & (1 << i)) { ++header->num_primary_key; }
    }
    
    return true;
}