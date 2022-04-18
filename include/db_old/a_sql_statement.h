#ifndef A_SQL_STATEMENT_H_
#define A_SQL_STATEMENT_H_

#include "wxdb_define.h"

#include <string>
#include <sstream>
#include <iostream>

#include "database.h"

class ASQLStatement 
{
public:
    ASQLStatement() = default;

    ~ASQLStatement() = default;

    // Processing statement error
    PREPARE_RESULT PrepareStatement(Database &database, const std::string &userInput)
    {
        std::istringstream reader(userInput);
        std::string action;
        reader >> action;
        // Create statement
        if (action == "create")
        {
            type_ = STATEMENT_CREATE;
            std::string tableName, redundantItem;
            if (!(reader >> tableName) || (reader >> redundantItem))
            {
                std::cout << "Syntax Error" << std::endl;
                return PREPARE_RESULT::SYNTAX_ERROR;
            }
            if (tableName.size() > TABLE_NAME_MAX_LENGTH)
            {
                std::cout << "Table name is larger than 25 bytes" << std::endl;
                return PREPARE_RESULT::TABLE_NAME_TOO_LONG;
            }
            std::memcpy(table_name_for_create_, tableName.c_str(), tableName.size() + 1);
        }
        // Insert statement
        else if (action == "insert") 
        {
            type_ = STATEMENT_INSERT;
            std::string tableName, id, userName, email, redundantItem;
            if (!(reader >> tableName >> id >> userName >> email) || (reader >> redundantItem))
            {
                std::cout << "Syntax Error" << std::endl;
                return PREPARE_RESULT::SYNTAX_ERROR;
            }
            if (database.table() == nullptr || tableName != std::string(database.table()->table_name))
            {
                std::cout << "Table \"" << tableName << "\" doesn't exist" << std::endl;
                return PREPARE_RESULT::TABLE_NOT_EXIST;
            }
            PREPARE_RESULT res = PREPARE_RESULT::FAILED;
            if ((res = CheckIntAttributeSize(id)) == PREPARE_RESULT::SUCCESS &&
                (res = CheckStringAttributeSize(userName, kUserNameSize - 1)) == PREPARE_RESULT::SUCCESS &&
                (res = CheckStringAttributeSize(email, kEmailSize - 1)) == PREPARE_RESULT::SUCCESS) 
            {
                unsigned idNum = std::stoi(id);
                std::memcpy(&row_for_insert_.id, &idNum, sizeof(idNum));
                std::memcpy(&row_for_insert_.user_name, userName.c_str(), userName.size() + 1);
                std::memcpy(&row_for_insert_.email, email.c_str(), email.size() + 1);
            }
            return res;
        } 
        // Select statement
        else if (action == "select") 
        {
            type_ = STATEMENT_SELECT;
            std::string tableName, redundantItem;
            if (!(reader >> tableName) || (reader >> redundantItem))
            {
                std::cout << "Syntax Error" << std::endl;
                return PREPARE_RESULT::SYNTAX_ERROR;
            }
            if (database.table() == nullptr || tableName != std::string(database.table()->table_name))
            {
                std::cout << "Table \"" << tableName << "\" doesn't exist" << std::endl;
                return PREPARE_RESULT::TABLE_NOT_EXIST;
            }
            std::memcpy(table_name_for_select_, tableName.c_str(), tableName.size() + 1);
            return PREPARE_RESULT::SUCCESS;
        }
        // Other statement
        else
        {
            std::cout << "Unrecognized statement" << std::endl;
            return PREPARE_RESULT::UNRECOGNIZED;
        }
        return PREPARE_RESULT::SUCCESS;
    }

    // Execute processed statement 
    ExecuteResult ExecuteStatement(Database &database, const std::string &userInput)
    {
        ExecuteResult res = EXECUTE_FAILED;
        switch (type_) 
        {
            case STATEMENT_CREATE:
                res = DoCreate(database);
                break;
            case STATEMENT_INSERT:
                res = DoInsert(database);
                break;
            case STATEMENT_SELECT:
                res = DoSelect(database, userInput);
                break;
        }
        return res;
    }

private:
    StatementType type_;
    Row row_for_insert_;
    char table_name_for_create_[TABLE_NAME_MAX_LENGTH + 1];
    char table_name_for_select_[TABLE_NAME_MAX_LENGTH + 1];

private:
    PREPARE_RESULT CheckStringAttributeSize(const std::string &attribute, unsigned size)
    {
        if (attribute.size() > size)
        {
            std::cout << "The size of attribute \"" << attribute << "\" excess range" << std::endl;
            return PREPARE_RESULT::ATTRIBUTE_SIZE_EXCESS;
        }
        return PREPARE_RESULT::SUCCESS;
    }

    PREPARE_RESULT CheckIntAttributeSize(const std::string &attribute)
    {
        try
        {
            stoi(attribute);
        }
        catch(const std::exception& e)
        {
            std::cerr << "Error in the attribute of Integer type" << std::endl;;
            return PREPARE_RESULT::ATTRIBUTE_ERROR;
        }
        return PREPARE_RESULT::SUCCESS;
    }

    /*
     * create tableName
     * example: create user
     */
    ExecuteResult DoCreate(Database &database) 
    {
        ExecuteResult res = database.CreateTable(table_name_for_create_, sizeof(table_name_for_create_));
        return res;
    }
    
    /*
     * insert tableName id userName email 
     * example: insert user 1 walsons walsons@163.com
     */
    ExecuteResult DoInsert(Database &database) 
    {
        ExecuteResult res = database.InsertRow(row_for_insert_);
        return res;
    }

    /*
     * select tableName
     * example: select user
     */
    ExecuteResult DoSelect(Database &database, std::string userInput) {
        ExecuteResult res = database.Select(table_name_for_select_, sizeof(table_name_for_select_));
        return res;
    }
};

#endif
