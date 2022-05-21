#ifndef DEFS_H_
#define DEFS_H_

#include <string>

const std::string DB_DIR = ".db/";
constexpr unsigned MAX_LENGTH_NAME = 256;  // Include end '\0'
constexpr unsigned MAX_NUM_TABLE = 64;
constexpr unsigned MAX_NUM_CHECK_CONSTRAINT = 32;
constexpr unsigned MAX_LENGTH_CHECK_CONSTRAINT = 1024;
constexpr unsigned MAX_NUM_COLUMN = 32;
constexpr unsigned MAX_LENGTH_DEFAULT_VALUE = 256;

struct Date
{
    int timestamp;
};

#endif