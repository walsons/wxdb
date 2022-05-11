#ifndef LITERAL_H_
#define LITERAL_H_

#include "common.h"

class Literal
{
public:
    Literal(std::string original_value);
    ~Literal();

    std::string original_value_;
};

#endif