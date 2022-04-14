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
    SRA *sra_;
    std::vector<Expression *> expr_list_;
    std::vector<Expression *> order_by_;
    bool distinct_;
    std::vector<Expression *> group_by_;
};

struct SRASelect
{
    SRA *sra_;
    Expression *condition_;
};

enum Join_Condition_Type
{
    JOIN_CONDITION_ON,
    JOIN_CONDITION_USING   
};

struct StringList
{
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
    SRAJoin(SRA *sra1, SRA *sra2, JoinCondition *join_condition);
    ~SRAJoin();
    SRA *sra1_, *sra2_;
    JoinCondition *join_condition_;   
};

struct SRABinary
{
    SRA *sra1_, *sra2_;
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

SRA *SRAOfTable(TableRef *table_ref);
SRA *SRAOfJoin(SRA *sra1, SRA *sra2, JoinCondition *join_condition);