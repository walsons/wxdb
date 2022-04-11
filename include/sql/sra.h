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

struct SRATable
{

};