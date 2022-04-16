/****************************
 ** super relation algebra **
 ****************************/

/****************************
SQL:
select f.a as col1, g.a as col2 from ft f, gt g where col1 != col2;

SRA:
Pi([(f,a,col1), (g,a,col2)],
    Sigma(col1 != col2,
        Join([(ft,f), (gt,g)])
    )
}
*****************************/

#include "expression.h"
#include <string>
#include <vector>

enum class SRA_Type
{
    SRA_TABLE,
    SRA_PROJECT,
    SRA_SELECT,
    SRA_NATURAL_JOIN,
    SRA_JOIN,
    SRA_FULL_OUTER_JOIN,
    SRA_LEFT_OUTER_JOIN,
    SRA_RIGHT_OUTER_JOIN,
    SRA_UNION,
    SRA_EXCEPT,
    SRA_INTERSECT
};

struct SRA;

struct TableRef
{
    TableRef(const std::string &table_name, const std::string &alias = "");
    ~TableRef();
    std::string table_name_;
    std::string alias_;
};

struct SRATable
{
    SRATable(TableRef *table_ref);
    ~SRATable();
    TableRef *table_ref_;
};

struct SRAProject
{
    SRAProject(std::shared_ptr<SRA> sra, std::vector<std::shared_ptr<Expression>> expr_list, bool distinct = false);
    ~SRAProject();
    std::shared_ptr<SRA> sra_;
    std::vector<std::shared_ptr<Expression>> expr_list_;
    bool distinct_;
    std::vector<std::shared_ptr<Expression>> order_by_;
    std::vector<std::shared_ptr<Expression>> group_by_;
};

struct SRASelect
{
    SRASelect(std::shared_ptr<SRA> sra, Expression *condition);
    ~SRASelect();
    std::shared_ptr<SRA> sra_;
    Expression *condition_;
};

enum Join_Condition_Type
{
    JOIN_CONDITION_ON,
    JOIN_CONDITION_USING   
};

struct StringList
{
    StringList(const std::string &str, StringList *next);
    ~StringList();
    std::string str_;
    StringList *next_;
};

struct JoinCondition
{
    Join_Condition_Type type_;   
    // TODO: add destruction function to destruct StringList correctly
    union 
    {
        Expression *on_;
        StringList *col_list_;  
    };
};

struct SRAJoin
{
    SRAJoin(std::shared_ptr<SRA> sra1, std::shared_ptr<SRA> sra2, JoinCondition *join_condition);
    ~SRAJoin();
    std::shared_ptr<SRA> sra1_, sra2_;
    JoinCondition *join_condition_;   
};

struct SRABinary
{
    std::shared_ptr<SRA> sra1_, sra2_;
};

struct SRA
{
    SRA(SRA_Type type);
    ~SRA();
    SRA_Type type_;
    union
    {
        SRATable table_;
        SRAProject project_;
        SRASelect select_;
        SRAJoin join_;
        SRABinary binary;
    };
};


/**********************************
 ***** Super Relation Algebra *****
 **********************************/

std::shared_ptr<SRA> SRAOfTable(TableRef *table_ref);
std::shared_ptr<SRA> SRAOfJoin(std::shared_ptr<SRA> sra1, std::shared_ptr<SRA> sra2, JoinCondition *join_condition);
std::shared_ptr<SRA> SRAOfProject(std::shared_ptr<SRA> sra, std::vector<std::shared_ptr<Expression>> expr_list);