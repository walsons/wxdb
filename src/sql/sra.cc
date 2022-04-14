#include "../../include/sql/sra.h"

// TableRef
TableRef::TableRef(const std::string &table_name, const std::string &alias)
    : table_name_(table_name), alias_(alias)
{
}

TableRef::~TableRef() = default;

// SRATable
SRATable::SRATable(TableRef *table_ref) : table_ref_(table_ref)
{
}

SRATable::~SRATable() = default;

// SRAJoin
SRAJoin::SRAJoin(SRA *sra1, SRA *sra2, JoinCondition *join_condition)
    : sra1_(sra1), sra2_(sra2), join_condition_(join_condition)
{
}

SRAJoin::~SRAJoin() = default;

// SRA
SRA::SRA(SRA_Type type) : type_(type)
{
}

SRA::~SRA()
{
    // TODO: call member destructor correctly
}


/**********************************
 ***** Super Relation Algebra *****
 **********************************/

SRA *SRAOfTable(TableRef *table_ref)
{
    SRA *sra = new SRA(SRA_Type::SRA_TABLE);
    sra->table_ = SRATable(table_ref);
    return sra;
}

SRA *SRAOfJoin(SRA *sra1, SRA *sra2, JoinCondition *join_condition)
{
    SRA *sra = new SRA(SRA_Type::SRA_JOIN);
    sra->join_.sra1_ = sra1;
    sra->join_.sra2_ = sra2;
    sra->join_.join_condition_ = join_condition;
    return sra;
}