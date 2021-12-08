#ifndef DATABASE_H_
#define DATABASE_H_

#include <vector>
#include <string>
#include <cstring>
#include <unordered_map>

#include "wxdb_define.h"

#define wxdb_uint uint32_t

#define size_of_attribure(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

#define USER_NAME_MAX_LENGTH 25
#define EMAIL_MAX_LENGTH 255

struct Row 
{
    wxdb_uint id;
    char user_name[USER_NAME_MAX_LENGTH + 1];
    char email[EMAIL_MAX_LENGTH + 1];
};

const wxdb_uint kIdSize = size_of_attribure(Row, id);
const wxdb_uint kUserNameSize = size_of_attribure(Row, user_name);
const wxdb_uint kEmailSize = size_of_attribure(Row, email);
const wxdb_uint kIdOffset = 0;
const wxdb_uint kUserNameOffset = kIdOffset + kIdSize;
const wxdb_uint kEmailOffset = kUserNameOffset + kUserNameSize;

#define TABLE_NAME_MAX_LENGTH 25
#define PAGE_SIZE 4096
#define MAX_PAGE_NUMBER 100
const wxdb_uint kRowNumberPerPage = PAGE_SIZE / sizeof(Row);
const wxdb_uint kMaxRowNumber = kRowNumberPerPage * MAX_PAGE_NUMBER;

struct Table
{
    char table_name[TABLE_NAME_MAX_LENGTH + 1];
    wxdb_uint row_number;
    char *page[MAX_PAGE_NUMBER];
};


class Database {
public:
    Database() : table_(nullptr) { }
    ~Database() 
    {
        if (table_ != nullptr)
        {
            delete table_;
            table_ = nullptr;
        }
    }

    ExecuteResult CreateTable(char *tableNameForCreate[], wxdb_uint bufferSize)
    {
        if (table_ == nullptr)
        {
            table_ = new Table();
            table_->row_number = 0;
            for (size_t cnt = 0; cnt < MAX_PAGE_NUMBER; ++cnt)
            {
                table_->page[cnt] = nullptr;
            }
        }
        std::memcpy(table_->table_name, tableNameForCreate, bufferSize);
        std::cout << "Create table " << "\"" << table_->table_name << "\" successfully" << std::endl;
        return EXECUTE_SUCCESS;
    }

    Table *AcquireTable()
    {
        return table_;
    }

    void serialize_row(Row *row, char *destination)
    {
        std::memcpy(destination + kIdOffset, &row->id, kIdSize);
        std::memcpy(destination + kUserNameOffset, &row->user_name, kUserNameSize);
        std::memcpy(destination + kEmailOffset, &row->email, kEmailSize);
    }

    void deserialize_row(Row *row, char *destination)
    {
        std::memcpy(&row->id, destination + kIdOffset, kIdSize);
        std::memcpy(&row->user_name, destination + kUserNameOffset, kUserNameSize);
        std::memcpy(&row->email, destination + kEmailOffset, kEmailSize);
    }

    void *RowSlot(Table *table, wxdb_uint rowNumber)
    {
        wxdb_uint pageNumber = rowNumber / kRowNumberPerPage;
        if (pageNumber >= 100)
        {
            std::cout << "Row number beyond max range" << std::endl;
            return nullptr;
        }
        wxdb_uint rowNumberInPage = rowNumber % kRowNumberPerPage;
        return table->page[pageNumber] + rowNumberInPage * sizeof(Row);
    }
private:
    Table *table_;
};

#endif
