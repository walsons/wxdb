#ifndef RECORD_FILE_H_
#define RECORD_FILE_H_

// Can't define as enum class, because it will be used as a integer in function
enum Record_Page_Status : int
{
    RECORD_PAGE_EMPTY,
    RECORD_PAGE_IN_USE,
};

#endif