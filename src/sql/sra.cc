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

// SRAProject
SRAProject::SRAProject(std::shared_ptr<SRA> sra, std::vector<std::shared_ptr<Expression>> expr_list, bool distinct)
    : sra_(sra), expr_list_(expr_list), distinct_(distinct)
{
}

SRAProject::~SRAProject() = default;

// SRASelect
SRASelect::SRASelect(std::shared_ptr<SRA> sra, Expression *condition)
    : sra_(sra), condition_(condition)
{
}

SRASelect::~SRASelect() = default;

// StringList
StringList::StringList(const std::string &str, StringList *next)
    : str_(str), next_(next)
{
}

StringList::~StringList() = default;

// SRAJoin
SRAJoin::SRAJoin(std::shared_ptr<SRA> sra1, std::shared_ptr<SRA> sra2, JoinCondition *join_condition)
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

std::shared_ptr<SRA> SRAOfTable(TableRef *table_ref)
{
    auto sra = std::make_shared<SRA>(SRA_Type::SRA_TABLE);
    sra->table_ = SRATable(table_ref);
    return sra;
}

std::shared_ptr<SRA> SRAOfJoin(std::shared_ptr<SRA> sra1, std::shared_ptr<SRA> sra2, JoinCondition *join_condition)
{
    auto sra = std::make_shared<SRA>(SRA_Type::SRA_JOIN);
    sra->join_ = SRAJoin(sra1, sra2, join_condition);
    return sra;
}

std::shared_ptr<SRA> SRAOfProject(std::shared_ptr<SRA> sra, std::vector<std::shared_ptr<Expression>> expr_list)
{
    auto new_sra = std::make_shared<SRA>(SRA_Type::SRA_PROJECT);
    new_sra->project_ = SRAProject(sra, expr_list);
    return new_sra;
}