#ifndef SQL_STATEMENT_H_
#define SQL_STATEMENT_H_

#include <string>
#include <sstream>
#include <iostream>
#include "database.h"

typedef enum 
{
    STATEMENT_CREATE,
    STATEMENT_INSERT,
    STATEMENT_SELECT
} StatementType;

typedef enum
{
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

class SQLStatement 
{
public:
    SQLStatement() = default;
    ~SQLStatement() = default;

    void Parse(Database &database, const std::string &userInput)
    {
        switch (PrepareStatement(userInput))
        {
            case PREPARE_UNRECOGNIZED_STATEMENT:
                std::cout << "Unknown command " << "\"" << userInput << "\"" << std::endl;
                break;
            case PREPARE_SUCCESS:
                ExecuteStatement(database, userInput);       
                break;
        }
    }
private:
    StatementType type_;
    std::unordered_map<std::string, int> types{
        {"int", 0},
        {"double", 1}
    };

private:
    PrepareResult PrepareStatement(const std::string &userInput)
    {
        if (userInput.substr(0, 6) == "create")
        {
            type_ = STATEMENT_CREATE;
        }
        else if (userInput.substr(0, 6) == "insert") 
        {
            type_ = STATEMENT_INSERT;
        } 
        else if (userInput.substr(0, 6) == "select") 
        {
            type_ = STATEMENT_SELECT;
        }
        else
        {
            return PREPARE_UNRECOGNIZED_STATEMENT;
        }
        return PREPARE_SUCCESS;
    }

    void ExecuteStatement(Database &database, const std::string &userInput)
    {
        switch (type_) {
            case STATEMENT_CREATE:
                do_create(database, userInput);
                break;
            case STATEMENT_INSERT:
                do_insert(database, userInput);
                break;
            case STATEMENT_SELECT:
                do_select(database, userInput);
                break;
        }
    }

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
            if (str[0] == ')') {
                break;
            }
            table->column_names_.emplace_back(str);
            step_reader >> str;
            if (str.back() == ',') {
                str.pop_back();
            }
            table->column_types_.emplace_back(str);
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
        for (const auto &c : table->column_types_) {
                    std::cout << c << std::endl;
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
        std::cout << "insert successfully" << std::endl;
    }

    void do_select(Database &database, std::string user_input) {
        std::istringstream step_reader(user_input);
        std::string str;
        // Currently only support select * from ...
        step_reader >> str;
        step_reader >> str;
        step_reader >> str;
        std::string table_name;
        step_reader >> table_name;
        Table *table = database.tables_[table_name];
        for (const auto &c : table->column_names_) {
            std::cout << c << "\t";
        }
        std::cout << std::endl;
        for (const auto &c: table->rows_) {
            for (size_t k = 0; k < c->items_.size(); ++k) {
                switch (types[table->column_types_[k]]) {
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
