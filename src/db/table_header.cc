#include "../../include/db/table_header.h"
#include <cstring>
#include <memory>
#include "../../include/sql/table_info.h"

void fill_table_header(std::shared_ptr<TableHeader> header, const TableInfo &table_info)
{
    std::memset(header.get(), 0, sizeof(TableHeader));
    std::strncpy(header->table_name, table_info.table_name.c_str(), MAX_LENGTH_NAME);
    // Field info
    int offset = 8;  // 4 bytes for __rowid__, and 4 bytes for not null
    auto field_info = table_info.field_info;
    for (auto field = field_info.begin(); field != field_info.end(); ++field)
    {
        int col_num = header->num_column++;
        std::strncpy(header->column_name[col_num], field->field_name.c_str(), MAX_LENGTH_NAME);
        header->column_type[col_num] = field->type;
        header->column_length[col_num] = field->length;
        header->column_offset[col_num] =  offset;
        offset += header->column_length[col_num];
        if (field->is_not_null) { header->flag_not_null |= (1 << col_num); }
        if (field->has_default)
        {
            header->flag_default |= (1 << col_num);
            // Expression exprssion{field->expr};
            std::ostringstream os;
            Expression::DumpExprNode(os, field->expr);
            std::strncpy(header->default_value[col_num], os.str().c_str(), MAX_LENGTH_CHECK_CONSTRAINT);
        }
    }
    // Constraint info
    auto constraint_info = table_info.constraint_info;
    auto set_flag = [&](std::vector<ConstraintInfo>::iterator cons, uint32_t &flag) -> int {
        int i = 0;
        for (; i < header->num_column; ++i)
        {
            if (cons->col_ref.column_name == header->column_name[i])
            {
                flag |= (1 << i);
            }
        }
        return i;
    };
    for (auto cons = constraint_info.begin(); cons != constraint_info.end(); ++cons)
    {
        switch (cons->type)
        {
        case Constraint_Type::CONS_UNIQUE:
            set_flag(cons, header->flag_unique);
            break;
        case Constraint_Type::CONS_PRIMARY_KEY:
            set_flag(cons, header->flag_primary);
            break;
        case Constraint_Type::CONS_FOREIGN_KEY:
        {
            int index = set_flag(cons, header->flag_foreign);
            strncpy(header->foreign_key_ref_table[index], cons->fk_ref.table_name.c_str(), MAX_LENGTH_NAME);
            strncpy(header->foreign_key_ref_column[index], cons->fk_ref.column_name.c_str(), MAX_LENGTH_NAME);
            break;
        }
        case Constraint_Type::CONS_CHECK:
        {
            std::ostringstream os;
            Expression::DumpExprNode(os, cons->expr);
            std::strncpy(header->check_constraint[header->num_check_constraint++], os.str().c_str(), MAX_LENGTH_CHECK_CONSTRAINT);
        }
        }

    }
    // Add "__rowid__" column (with highest index)
    int col_num = header->num_column++;
    std::strncpy(header->column_name[col_num], "__rowid__", MAX_LENGTH_NAME);
    header->column_type[col_num] = Col_Type::COL_TYPE_INT;
    header->column_length[col_num] = sizeof(int);
    header->column_offset[col_num] = 0;
    header->main_index = col_num;
    header->is_main_index_auto_inc = true;
    
    // Add not null for, unique, index __rowid__
    header->flag_not_null |= (1 << col_num);
    header->flag_unique |= (1 << col_num);
    header->flag_index |= (1 << col_num);
    // If table doesn't have primary key, set __rowid__ to primary key
    if (!header->flag_primary) { header->flag_primary |= (1 << col_num); }

    // Add index for unique column
    header->flag_index |= header->flag_unique;

    // Add not null, unique, index for primary key column
    header->flag_not_null |= header->flag_primary;
    header->flag_unique |= header->flag_primary;
    header->flag_index |= header->flag_primary;

    // __rowid__ starts from 1
    header->auto_inc = 1;
}