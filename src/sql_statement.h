#ifndef SQL_STATEMENT_H_
#define SQL_STATEMENT_H_

#include <string>
#include <sstream>
#include <iostream>
#include "database.h"

typedef enum {
    UNKNOWN_TYPE,
    CREATE_TYPE,
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
        statement_type_ = UNKNOWN_TYPE;
        if (user_input.substr(0, 6) == "create") {
	    statement_type_ = CREATE_TYPE;
        } else if (user_input.substr(0, 6) == "insert") {
            statement_type_ = INSERT_TYPE;
        } else if (user_input.substr(0, 6) == "select") {
            statement_type_ = SELECT_TYPE;
        }
        switch (statement_type_) {
	    case CREATE_TYPE:
		do_create(user_input);
		break;
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
private:
    void do_create(std::string user_input) {
	std::istringstream step_reader(user_input);
	std::string str;
	step_reader >> str;
	step_reader >> str;
        Table *table = new Table;
	step_reader >> str;
	table->table_name_ = str;
	step_reader >> str;
	while (step_reader >> str) {
	    if (str == ")") {
	        break;
            }
	    table->columns_name_.emplace_back(str);
	    step_reader >> str;
	    str.pop_back();
	    table->columns_type_.emplace_back(str);
	}
	std::cout << "create table \"" << table->table_name_ << "\" successfully" << std::endl;
    }
    
    void do_insert(std::string user_input) {
	std::istringstream step_reader(user_input);
	std::string str;
	step_reader >> str;
	step_reader >> str;
    }
};

#endif
