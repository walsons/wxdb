#ifndef TABLE_HEADER_H
#define TABLE_HEADER_H

#include "../defs.h"
#include "../sql/table_info.h"

struct TableHeader
{
    // The number of column
    uint8_t num_column;
    // The column which is main index for this table, 
    uint8_t main_index;
    // Is the main index auto increment
    bool is_main_index_auto_inc;
    // The number of records, primary keys, check constraints, foreign keys
    unsigned num_record, num_primary_key, num_check_constraint, num_foreign_key_num;
    // Flags: not null, primary, index, unique, default
    // The column of primary and unique will be add index automatically
    // Each bit delegate a column
    uint32_t flag_not_null, flag_primary, flag_index, flag_unique, flag_default;

    // The type of each column
    Data_Type column_type[MAX_NUM_COLUMN];
    // The length of each column
    unsigned column_length[MAX_NUM_COLUMN];
    // The offset of each column
    unsigned column_offset[MAX_NUM_COLUMN];
    // The root page of index for each column, 0 if no index
    unsigned index_root_page[MAX_NUM_COLUMN];
    // The auto increment counter;
    uint64_t auto_inc;

    char check_constraint[MAX_NUM_CHECK_CONSTRAINT][MAX_LENGTH_CHECK_CONSTRAINT];
    char default_value[MAX_NUM_COLUMN][MAX_LENGTH_DEFAULT_VALUE];
    // Which column is foreign key
    unsigned foreign_key[MAX_NUM_COLUMN];
    char foreign_key_ref_table[MAX_NUM_COLUMN][MAX_LENGTH_NAME];
    char foreign_key_ref_column[MAX_NUM_COLUMN][MAX_LENGTH_NAME];
    char column_name[MAX_NUM_COLUMN][MAX_LENGTH_NAME];
    char table_name[MAX_LENGTH_NAME];
};

#endif