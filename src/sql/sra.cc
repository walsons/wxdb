#include "../../include/sql/sra.h"
#include <memory>

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
    switch (type_)
    {
    case SRA_Type::SRA_TABLE:
        table_.~SRATable();
        break;
    case SRA_Type::SRA_PROJECT:
        project_.~SRAProject();
        break;
    case SRA_Type::SRA_SELECT:
        select_.~SRASelect();
        break;
    case SRA_Type::SRA_JOIN:
        join_.~SRAJoin();
        break;
    default:
        break;
    }
}


/**********************************
 ***** Super Relation Algebra *****
 **********************************/

std::shared_ptr<SRA> SRAOfTable(TableRef *table_ref)
{
    auto new_sra = std::make_shared<SRA>(SRA_Type::SRA_TABLE);
    new (&new_sra->table_) SRATable(table_ref);
    return new_sra;
}

std::shared_ptr<SRA> SRAOfJoin(std::shared_ptr<SRA> sra1, std::shared_ptr<SRA> sra2, JoinCondition *join_condition)
{
    auto new_sra = std::make_shared<SRA>(SRA_Type::SRA_JOIN);
    new (&new_sra->join_) SRAJoin(sra1, sra2, join_condition);
    return new_sra;
}

std::shared_ptr<SRA> SRAOfProject(std::shared_ptr<SRA> sra, std::vector<std::shared_ptr<Expression>> expr_list)
{
    auto new_sra = std::make_shared<SRA>(SRA_Type::SRA_PROJECT);
    new (&new_sra->project_) SRAProject(sra, expr_list);
    return new_sra;
}