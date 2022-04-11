#ifndef LITERAL_H_
#define LITERAL_H_

#include "common.h"

class Literal
{
public:
    Literal(Data_Type data_type, std::string original_value);
    ~Literal();

    Data_Type data_type_;
    std::string original_value_;
};

class IntLiteral : public Literal {
public:
    IntLiteral(Data_Type data_type, std::string original_value);
    ~IntLiteral();

    int value;
};

class DoubleLiteral : public Literal {
public:
    DoubleLiteral(Data_Type data_type, std::string original_value);
    ~DoubleLiteral();

    double value;
};

#endif