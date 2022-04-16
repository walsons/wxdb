#ifndef TOKEN_H_
#define TOKEN_H_

#include <string>

enum class Token_Type
{
    // Operator
    TOKEN_OPEN_PARENTHESIS = 0,    // (
    TOKEN_CLOSE_PARENTHESIS,       // )
    TOKEN_POWER,                   // ^
    TOKEN_PLUS,                    // +
    TOKEN_MINUS,                   // -
    TOKEN_MULTIPLY,                // *
    TOKEN_DIVIDE,                  // /
    TOKEN_LT,                      // <   less than 
    TOKEN_GT,                      // >   greater than
    TOKEN_EQ,                      // ==  equal to
    TOKEN_NOT_EQ,                  // !=  not equal to
    TOKEN_LE,                      // <=  less than or equal to
    TOKEN_GE,                      // >=  greater than or equal to
    TOKEN_IN,                      
    TOKEN_LIKE,                    
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,
    TOKEN_ASSIGNMENT,
    TOKEN_FUN,
    TOKEN_COMMA,


    // Invalid token 
    TOKEN_INVALID,

    TOKEN_RESERVED_WORD,
    TOKEN_WORD,
    TOKEN_UNENDED_STRING,
    TOKEN_STRING,
    TOKEN_MOD,
    TOKEN_INCOMPLETE_CHAR,
    TOKEN_CHAR,
    TOKEN_INVALID_CHAR,
    TOKEN_SEMICOLON,
    TOKEN_EXP_FLOAT,
    TOKEN_FLOAT,

    // Integer
    TOKEN_OCTAL,
    TOKEN_HEX,
    TOKEN_DECIMAL,
    TOKEN_ZERO,                    

    TOKEN_NULL,
    
    /* No meaning, only for end */
    TOKEN_END                   
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