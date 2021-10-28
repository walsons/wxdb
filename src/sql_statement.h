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
    void parse(Database &database, const std::string &user_input) {
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
                do_create(database, user_input);
                break;
            case INSERT_TYPE:
                do_insert(database, user_input);
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
    std::unordered_map<std::string, int> types{
        {"int", 0},
        {"double", 1}
    };
private:
    void do_create(Database &database, std::string user_input) {
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
            if (str.back() == ',') {
                str.pop_back();
            }
            table->columns_type_.emplace_back(str);
        }
        if (database.tables_.find(table->table_name_) != database.tables_.end()) {
            std::cout << "there is a table named \"" << table->table_name_ << "\" in database" << std::endl;
            return;
        }
        database.tables_[table->table_name_] = table;
        std::cout << "create table \"" << table->table_name_ << "\" successfully" << std::endl;
    }
    
    void do_insert(Database &database, std::string user_input) {
        std::istringstream step_reader(user_input);
        std::string str;
        step_reader >> str;
        step_reader >> str;
        step_reader >> str;
        Table *table = database.tables_[str];
        step_reader >> str;
        step_reader >> str;
        Row *row = new Row; 
        for (const auto &c : table->columns_type_) {
            step_reader >> str;
            if (str.back() == ',') {
                str.pop_back();
            }
            void *p = nullptr;
            switch (types[c]) {
                case 0: 
                    p = new int(stoi(str));
                    break;
                case 1:
                    p = new double(stod(str));
                    break;
                default:
                    break;
            }
            row->items_.push_back(p);
        }
        table->rows_.push_back(row);
        // print
        for (const auto &c : table->columns_name_) {
            std::cout << c << "\t";
        }
        std::cout << std::endl;
        for (const auto &c: table->rows_) {
            for (size_t k = 0; k < c->items_.size(); ++k) {
                switch (types[table->columns_type_[k]]) {
                    case 0:
                        std::cout << *reinterpret_cast<int *>(c->items_[k]) << "\t";
                        break;
                    case 1:
                        std::cout << *reinterpret_cast<double *>(c->items_[k]) << "\t";
                        break;
                    default:
                        break;
                }
            }
            std::cout << std::endl;
        }
    }
};

#endif
