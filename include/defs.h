#ifndef DEFS_H_
#define DEFS_H_

#include <string>
#include <ctime>
#include <iomanip>

const std::string DB_DIR = ".db/";
constexpr unsigned MAX_LENGTH_NAME = 256;  // Include end '\0'
constexpr unsigned MAX_NUM_TABLE = 64;
constexpr unsigned MAX_NUM_CHECK_CONSTRAINT = 32;
constexpr unsigned MAX_LENGTH_CHECK_CONSTRAINT = 1024;
constexpr unsigned MAX_NUM_COLUMN = 32;
constexpr unsigned MAX_LENGTH_DEFAULT_VALUE = 256;

const char *const DATE_TEMPLATE = "%Y-%m-%d";
struct Date
{
    Date(const time_t &t = -1) : timestamp(t) {}
    Date(const std::string &str)
    {
        timestamp = str2timestamp(str);
    }
    ~Date() = default;
    time_t str2timestamp(const std::string &str)
    {
        std::tm t{};
        std::string tmp{str};
        std::istringstream ss(tmp);
        ss >> std::get_time(&t, DATE_TEMPLATE);
        if (ss.fail()) { return -1; }
        else { return std::mktime(&t); }
    }
    std::string timestamp2str()
    {
        tm *t = localtime(&timestamp);
        char buf[32];
        std::strftime(buf, 32, DATE_TEMPLATE, t);
        return std::string{buf};
    }

    time_t timestamp = 0;
};

#endif