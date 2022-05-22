#ifndef TABLE_HEADER_H
#define TABLE_HEADER_H

#include "../defs.h"
#include "../sql/table_info.h"

struct TableHeader
{
    char table_name[MAX_LENGTH_NAME];
    // The name of each column
    char column_name[MAX_NUM_COLUMN][MAX_LENGTH_NAME];
    // The type of each column
    Col_Type column_type[MAX_NUM_COLUMN];
    // The length of each column
    unsigned column_length[MAX_NUM_COLUMN];
    // The offset of each column
    unsigned column_offset[MAX_NUM_COLUMN];
    // The auto increment counter;
    uint64_t auto_inc;

    // The number of column
    uint8_t num_column;
    // The column which is main index for this table, 
    uint8_t main_index;
    // Is the main index auto increment
    bool is_main_index_auto_inc;
    // The number of records
    unsigned num_record;

    // Flags: not null, unique, primary, default, index
    // The column of primary and unique will be add index automatically
    // Each bit delegate a column
    uint32_t flag_not_null, flag_unique, flag_primary, flag_foreign;
    uint32_t flag_default, flag_index;
    // reference table
    char foreign_key_ref_table[MAX_NUM_COLUMN][MAX_LENGTH_NAME];
    // reference column
    char foreign_key_ref_column[MAX_NUM_COLUMN][MAX_LENGTH_NAME];
    // store default value, which has been eval;
    char default_value[MAX_NUM_COLUMN][MAX_LENGTH_DEFAULT_VALUE];
    // The root page of index for each column, 0 if no index
    unsigned index_root_page[MAX_NUM_COLUMN];
    // The number of check constraints, foreign keys
    unsigned num_check_constraint;
    // store check expression
    char check_constraint[MAX_NUM_CHECK_CONSTRAINT][MAX_LENGTH_CHECK_CONSTRAINT];
};

bool fill_table_header(std::shared_ptr<TableHeader> header, const TableInfo &table_info);

#endif