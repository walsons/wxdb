#include "../../include/sql/sra.h"

TableRef::TableRef(const std::string &table_name, const std::string &alias)
    : table_name_(table_name), alias_(alias)
{
}

TableRef::~TableRef() = default;

SRATable::SRATable(TableRef *table_ref) : table_ref_(table_ref)
{
}

SRATable::~SRATable() = default;

SRAJoin::SRAJoin(SRA *sra1, SRA *sra2, JoinCondition *join_condition)
    : sra1_(sra1), sra2_(sra2), join_condition_(join_condition)
{
}

SRAJoin::~SRAJoin() = default;

SRA::SRA(SRA_Type type) : type_(type)
{
}

SRA::~SRA()
{
    // TODO: call member destructor correctly
}