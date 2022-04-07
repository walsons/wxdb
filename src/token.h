#ifndef TOKEN_H_
#define TOKEN_H_

#include <string>

enum class Token_Type
{
    /* Operator */
    TOKEN_OPEN_PARENTHESIS = 0,    // (
    TOKEN_CLOSE_PARENTHESIS,       // )
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
    TOKEN_NULL,
    TOKEN_ASSIGNMENT,
    TOKEN_FUN,
    TOKEN_COMMA,

    /* Other token */
    TOKEN_INVALID,                 // invalid token
    TOKEN_RESERVED_WORD,
    TOKEN_WORD,
    TOKEN_UNENDED_STRING,
    TOKEN_STRING,
    TOKEN_OCTAL,
    TOKEN_HEX,
    TOKEN_FRACTION,
    TOKEN_ZERO,
    TOKEN_DECIMAL,
    TOKEN_SEMICOLON,
    
    /* No meaning, only for end */
    Token_end                   
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
            text_ = text_.substr(1, text.size() - 1);
        }
    }
    std::string text_;
    Token_Type type_;
};

#endif