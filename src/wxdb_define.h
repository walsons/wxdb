#ifndef WXDB_DEFINE_H_
#define WXDB_DEFINE_H_

enum class META_COMMAND
{
    URECOGNIZED,
    EXIT,
    HELP
};

enum class PREPARE_RESULT
{
    SUCCESS,
    FAILED,
    UNRECOGNIZED,
    SYNTAX_ERROR,
    TABLE_NAME_TOO_LONG,
    ATTRIBUTE_SIZE_EXCESS,
    ATTRIBUTE_ERROR,
    TABLE_NOT_EXIST
};

typedef enum
{
    EXECUTE_SUCCESS,
    EXECUTE_FAILED,
    EXECUTE_TABLE_FULL
} ExecuteResult;

typedef enum 
{
    STATEMENT_CREATE,
    STATEMENT_INSERT,
    STATEMENT_SELECT
} StatementType;

/*********
 * Pager *
 *********/
const unsigned PAGE_SIZE = 4096;
const unsigned MAX_PAGE_NUMBER = 100;

#endif