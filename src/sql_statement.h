#ifndef SQL_STATEMENT_H_
#define SQL_STATEMENT_H_

typedef enum {
    OTHER_TYPE,
    INSERT_TYPE,
    SELECT_TYPE
} StatementType;

class SQLStatement {
public:
    SQLStatement() = default;
    ~SQLStatement() = default;
    StatementType &statement_type() {
        return statement_type_;
    }
    void parse(const std::string &user_input) {
        statement_type_ = OTHER_TYPE;
        if (user_input.substr(0, 6) == "insert") {
            statement_type_ = INSERT_TYPE;
        } else if (user_input.substr(0, 6) == "select") {
            statement_type_ = SELECT_TYPE;
        }
        switch (statement_type_) {
            case INSERT_TYPE:
                std::cout << "do insert" << std::endl;
                break;
            case SELECT_TYPE:
                std::cout << "do select" << std::endl;
                break;
            default:
                std::cout << "unknown command" << std::endl;
                break;
        }
    }
private:
    StatementType statement_type_;
};

#endif