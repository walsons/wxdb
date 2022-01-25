#ifndef DATABASE_H_
#define DATABASE_H_

#include "wxdb_define.h"

#include <vector>
#include <string>
#include <cstring>
#include <unordered_map>
#include <fstream>
#include <memory>

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

struct Pager
{
    Pager(const std::string &file_name) 
    {
        fd.open(file_name, std::ios::binary | std::ios::in | std::ios::out);
        if (fd)
        {
            fd.seekg(0, std::ios::end);
            file_length = fd.tellg();
            size_t row_num = file_length / sizeof(Row);
            size_t page_num = row_num / kRowNumberPerPage;
            fd.seekg(0, std::ios::beg);
            // Initialization
            for (size_t i = 0; i < MAX_PAGE_NUMBER; ++i)
            {
                pages[i] = nullptr;
            }
            // Allocate
            for (size_t i = 0; i < page_num; ++i)
            {
                pages[i] = new char[PAGE_SIZE];
                fd.read(pages[i], PAGE_SIZE);
            }
            // rest of buffer in file
            pages[page_num] = new char[PAGE_SIZE];
            fd.read(pages[page_num], (row_num - page_num * kRowNumberPerPage) * sizeof(Row));
        }
        else
        {
            std::ofstream out_fd(file_name, std::ios::out);
            out_fd.close();
            file_length = 0;
            fd.open(file_name, std::ios::binary | std::ios::in | std::ios::out);
            for (size_t i = 0; i < MAX_PAGE_NUMBER; ++i)
            {
                pages[i] = nullptr;
            }
        }
    }
    ~Pager()
    {
        fd.close();
        for (size_t i = 0; i < MAX_PAGE_NUMBER; ++i)
        {
            if (pages[i] != nullptr)
            {
                delete[] pages[i];
                pages[i] = nullptr;
            }
        }
    }
    std::fstream fd;
    unsigned int file_length;
    char *pages[MAX_PAGE_NUMBER];
};

struct Table
{
    Table(const std::string &file_name = "wxdb.db")
    {
        pager = new Pager(file_name);
        row_number = pager->file_length / sizeof(Row);
    }
    ~Table()
    {
        if (pager != nullptr)
        {
            delete pager;
            pager = nullptr;
        }
    }
    char table_name[TABLE_NAME_MAX_LENGTH + 1];
    wxdb_uint row_number;
    Pager *pager;
};


class Database {
public:
    Database() : table_(nullptr) 
    { 
        std::ifstream in("table_attribute.db", std::ifstream::binary | std::ifstream::in);
        if (in)
        {
            table_ = new Table();
            in.read((char*)table_, TABLE_NAME_MAX_LENGTH + 1);
            in.close();
        }
    }
    ~Database() 
    {
        if (table_ != nullptr)
        {
            FlushToDisk();
            delete table_;
            table_ = nullptr;
        }
    }

    ExecuteResult CreateTable(char tableNameForCreate[], wxdb_uint bufferSize)
    {
        if (table_ == nullptr)
        {
            table_ = new Table();
            table_->row_number = 0;
            std::memcpy(table_->table_name, tableNameForCreate, bufferSize);
            std::cout << "Create table " << "\"" << table_->table_name << "\" successfully" << std::endl;
        }
        else
        {
            std::cout << "You can only create one table" << std::endl;
        }
        return EXECUTE_SUCCESS;
    }

    ExecuteResult InsertRow(Row rowForInsert)
    {
        auto pager = table_->pager;
        wxdb_uint pageNumber = table_->row_number / kRowNumberPerPage;
        if (pageNumber >= 100)
        {
            std::cout << "Table \"" << table_->table_name << "\" is full" << std::endl;
            return EXECUTE_TABLE_FULL;
        }
        wxdb_uint rowNumber = table_->row_number % kRowNumberPerPage;
        if (pager->pages[pageNumber] == nullptr)
        {
            pager->pages[pageNumber] = new char[PAGE_SIZE];
        }
        serialize_row(&rowForInsert, pager->pages[pageNumber] + rowNumber * sizeof(Row));
        ++table_->row_number;
        std::cout << "Insert record into table \"" << table_->table_name << "\" successfully" << std::endl;
        return EXECUTE_SUCCESS;
    }

    ExecuteResult Select(char tableNameForSelect[], wxdb_uint bufferSize)
    {
        Pager *pager = table_->pager;
        std::cout << "\tid\tuser name\temail\t" << std::endl;
        Row oneRow;
        for (size_t row = 0; row < table_->row_number; ++row)
        {
            wxdb_uint pageNumber = row / kRowNumberPerPage;
            wxdb_uint rowNumber = row % kRowNumberPerPage;
            deserialize_row(&oneRow, pager->pages[pageNumber] + rowNumber * sizeof(Row));
            std::cout << "\t" << oneRow.id << "\t" << oneRow.user_name << "\t" << oneRow.email << "\t" << std::endl;
        }
        return EXECUTE_SUCCESS;
    }

    Table *AcquireTable()
    {
        return table_;
    }
    // Get the position to insert a row by row number.
    void *RowSlot(Table *table, wxdb_uint rowNumber)
    {
        wxdb_uint pageNumber = rowNumber / kRowNumberPerPage;
        if (pageNumber >= 100)
        {
            std::cout << "Row number beyond max range" << std::endl;
            return nullptr;
        }
        wxdb_uint rowNumberInPage = rowNumber % kRowNumberPerPage;
        return table->pager->pages[pageNumber] + rowNumberInPage * sizeof(Row);
    }
    // Get page
    void *GetPage(Pager *pager, unsigned page_num)
    {
        if (page_num > MAX_PAGE_NUMBER)
        {
            std::cout << "excess max page number" << std::endl;
            return nullptr;
        }
        if (pager->pages[page_num] == nullptr)
        {
            pager->fd.seekg(page_num * PAGE_SIZE, std::ios::beg);
            pager->fd.read(pager->pages[page_num], PAGE_SIZE);
        }
        return pager->pages[page_num];
    }

private:
    Table *table_;

    // copy row buffer to destination.
    void serialize_row(Row *row, char *destination)
    {
        std::memcpy(destination + kIdOffset, &row->id, kIdSize);
        std::memcpy(destination + kUserNameOffset, &row->user_name, kUserNameSize);
        std::memcpy(destination + kEmailOffset, &row->email, kEmailSize);
    }

    // copy destination buffer to row.
    void deserialize_row(Row *row, char *destination)
    {
        std::memcpy(&row->id, destination + kIdOffset, kIdSize);
        std::memcpy(&row->user_name, destination + kUserNameOffset, kUserNameSize);
        std::memcpy(&row->email, destination + kEmailOffset, kEmailSize);
    }
    
    // Flush buffer to disk
    void FlushToDisk()
    {
        if (table_ != nullptr)
        {
            size_t nums_of_pages = table_->row_number / kRowNumberPerPage;
            for (size_t i = 0; i < nums_of_pages; ++i)
            {
                if (table_->pager->pages[i] != nullptr)
                {
                    table_->pager->fd.seekp(i * PAGE_SIZE, std::ios::beg);
                    table_->pager->fd.write(table_->pager->pages[i], PAGE_SIZE);
                }
            }
            // last page not full
            size_t rest_row_num = table_->row_number - kRowNumberPerPage * nums_of_pages;
            if (rest_row_num > 0)
            {
                table_->pager->fd.seekp(nums_of_pages * PAGE_SIZE, std::ios::beg);
                table_->pager->fd.write(table_->pager->pages[nums_of_pages], rest_row_num * sizeof(Row));
            }
            // Write table name and row number to file
            std::ofstream out("table_attribute.db", std::ofstream::out | std::ofstream::binary);
            out.write((char*)table_, sizeof(Table));
            out.close();
        }
    }
};

#endif
