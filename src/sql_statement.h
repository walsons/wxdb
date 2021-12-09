#ifndef SQL_STATEMENT_H_
#define SQL_STATEMENT_H_

#include <string>
#include <sstream>
#include <iostream>
#include "wxdb_define.h"
#include "database.h"


class SQLStatement 
{
public:
    SQLStatement() = default;

    ~SQLStatement() = default;

    PrepareResult PrepareStatement(Database &database, const std::string &userInput)
    {
        std::istringstream reader(userInput);
        std::string action;
        reader >> action;

        if (action == "create")
        {
            type_ = STATEMENT_CREATE;
            std::string tableName, redundantItem;
            if (!(reader >> tableName) || (reader >> redundantItem))
            {
                std::cout << "Syntax Error" << std::endl;
                return PREPARE_SYNTAX_ERROR;
            }
            if (tableName.size() > TABLE_NAME_MAX_LENGTH)
            {
                std::cout << "Table name is larger than 25 bytes" << std::endl;
                return PREPARE_TABLE_NAME_TOO_LONG;
            }
            std::memcpy(table_name_for_create_, tableName.c_str(), tableName.size() + 1);
        }

        else if (action == "insert") 
        {
            type_ = STATEMENT_INSERT;
            std::string tableName, id, userName, email, redundantItem;
            if (!(reader >> tableName >> id >> userName >> email) || (reader >> redundantItem))
            {
                std::cout << "Syntax Error" << std::endl;
                return PREPARE_SYNTAX_ERROR;
            }
            if (database.AcquireTable() == nullptr || tableName != std::string(database.AcquireTable()->table_name))
            {
                std::cout << "Table \"" << tableName << "\" doesn't exist" << std::endl;
                return PREPARE_TABLE_NOT_EXIST;
            }
            PrepareResult res = PREPARE_FAILED;
            if ((res = CheckIntAttributeSize(id)) == PREPARE_SUCCESS &&
                (res = CheckStringAttributeSize(userName, kUserNameSize - 1)) == PREPARE_SUCCESS &&
                (res = CheckStringAttributeSize(email, kEmailSize - 1)) == PREPARE_SUCCESS) 
            {
                wxdb_uint idNum = std::stoi(id);
                std::memcpy(&row_for_insert_.id, &idNum, sizeof(idNum));
                std::memcpy(&row_for_insert_.user_name, userName.c_str(), userName.size() + 1);
                std::memcpy(&row_for_insert_.email, email.c_str(), email.size() + 1);
            }
            return res;
        } 

        else if (action == "select") 
        {
            type_ = STATEMENT_SELECT;
            std::string tableName, redundantItem;
            if (!(reader >> tableName) || (reader >> redundantItem))
            {
                std::cout << "Syntax Error" << std::endl;
                return PREPARE_SYNTAX_ERROR;
            }
            if (database.AcquireTable() == nullptr || tableName != std::string(database.AcquireTable()->table_name))
            {
                std::cout << "Table \"" << tableName << "\" doesn't exist" << std::endl;
                return PREPARE_TABLE_NOT_EXIST;
            }
            std::memcpy(table_name_for_select_, tableName.c_str(), tableName.size() + 1);
            return PREPARE_SUCCESS;
        }

        else
        {
            std::cout << "Unrecognized statement" << std::endl;
            return PREPARE_UNRECOGNIZED_STATEMENT;
        }
        return PREPARE_SUCCESS;
    }

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
    PrepareResult CheckStringAttributeSize(const std::string &attribute, wxdb_uint size)
    {
        if (attribute.size() > size)
        {
            std::cout << "The size of attribute \"" << attribute << "\" excess range" << std::endl;
            return PREPARE_ATTRIBUTE_SIZE_EXCESS;
        }
        return PREPARE_SUCCESS;
    }

    PrepareResult CheckIntAttributeSize(const std::string &attribute)
    {
        try
        {
            stoi(attribute);
        }
        catch(const std::exception& e)
        {
            std::cerr << "Error in the attribute of Integer type" << std::endl;;
            return PREPARE_ATTRIBUTE_ERROR;
        }
        return PREPARE_SUCCESS;
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
