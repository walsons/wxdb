#ifndef TOKEN_H_
#define TOKEN_H_

#include <string>

enum class Operator_Type
{
    POWER = 0,               // ^
    PLUS,                    // +
    MINUS,                   // -
    MULTIPLY,                // *
    DIVIDE,                  // /
    MOD,                     // %
    LT,                      // <   less than 
    GT,                      // >   greater than
    EQ,                      // =  equal to
    NOT_EQ,                  // !=  not equal to
    LE,                      // <=  less than or equal to
    GE,                      // >=  greater than or equal to
    AND,                     // and
    OR,                      // or
    // Unary operator
    UNARY_DELIMETER,         // if Operator_Type > UNARY_DELIMETER, means a unary type
    NOT,                     // not
    POSITIVE,                // + 
    NEGATIVE,                // - 
    NONE                     // not a operator
};

enum class Token_Type
{
    TOKEN_OPEN_PARENTHESIS = 0,    // (
    TOKEN_CLOSE_PARENTHESIS,       // )
    TOKEN_POWER,                   // ^
    TOKEN_PLUS,                    // +
    TOKEN_MINUS,                   // -
    TOKEN_MULTIPLY,                // *
    TOKEN_DIVIDE,                  // /
    TOKEN_MOD,                     // %
    TOKEN_LT,                      // <   less than 
    TOKEN_GT,                      // >   greater than
    TOKEN_EQ,                      // =  equal to
    TOKEN_NOT_EQ,                  // !=  not equal to
    TOKEN_LE,                      // <=  less than or equal to
    TOKEN_GE,                      // >=  greater than or equal to
    TOKEN_AND,                     // and
    TOKEN_OR,                      // or
    TOKEN_NOT,                     // not
    TOKEN_COMMA,                   // ,
    TOKEN_SEMICOLON,               // ;

    TOKEN_NULL,                    // null

    // TOKEN_IN,                    
    // TOKEN_LIKE,                    
    // TOKEN_FUN,

    TOKEN_RESERVED_WORD,
    TOKEN_WORD,
    TOKEN_UNENDED_STRING,
    TOKEN_STRING,
    TOKEN_FLOAT,
    TOKEN_EXP_FLOAT,               
    // Integer
    TOKEN_OCTAL,
    TOKEN_HEX,
    TOKEN_DECIMAL,
    TOKEN_ZERO,                    
    // Invalid token 
    TOKEN_INVALID
};

class Token
{
public:
    Token(std::string text, Token_Type type)
        : text_(text), type_(type) 
    {
        // If it's string, need to remove single quotation marks
        if (type_ == Token_Type::TOKEN_STRING)
        {
            text_ = text_.substr(1, text.size() - 2);
        }
    }
    std::string text_;
    Token_Type type_;
};

#endif