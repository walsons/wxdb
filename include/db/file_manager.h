#ifndef FILE_MANAGER_H_
#define FILE_MANAGER_H_

#include <string>
#include <fstream>
#include <unordered_map>
#include <memory>
#include "../sql/common.h"

const unsigned DISK_BLOCK_SIZE = 4096;

class FileManager;
class MemoryBuffer;

/******************************************************
 * DiskBlock represents the blocks saving data
 ******************************************************/
class DiskBlock
{
public:
    DiskBlock(const std::string &file_name, int block_number, 
              std::shared_ptr<TableInfo> table_info);
    ~DiskBlock() = default;

    std::string file_name_;
    int block_number_;
    std::shared_ptr<TableInfo> table_info_;
};

/******************************************************
 * MemoryPage represents a memory page whose size is 
 * equal to a disk block
 ******************************************************/
class MemoryPage
{
public:
    MemoryPage(std::shared_ptr<FileManager> file_manager);
    int Read(std::shared_ptr<DiskBlock> block);
    int Write(std::shared_ptr<DiskBlock> block);
    int Append(std::shared_ptr<MemoryBuffer> memory_buffer,
               const std::string &file_name,
               std::shared_ptr<TableInfo> table_info);
    int RecordFormatter(std::shared_ptr<TableInfo> table_info);
    int SetInt(int offset, int val);

    char contents_[DISK_BLOCK_SIZE];
    std::shared_ptr<FileManager> file_manager_;
};

/******************************************************
 * FileManager is responsible for a directory(database),
 * table that is created will save in it as file.
 ******************************************************/
class FileManager
{
public:
    FileManager(const std::string &db_dir_name, const std::string &db_name);
    ~FileManager();
    int Read(MemoryPage *memory_page, std::shared_ptr<DiskBlock> disk_block);
    int Write(MemoryPage *memory_page, std::shared_ptr<DiskBlock> disk_block);
    int Append(std::shared_ptr<MemoryBuffer> memory_buffer,
               const std::string &file_name,
               std::shared_ptr<TableInfo> table_info);

private:
    std::string db_dir_name_;
    std::fstream fd_;
    bool is_new_;
    std::unordered_map<std::string, std::fstream &> open_files_;

private:
    std::fstream &GetFile(const std::string &file_name);
    int Size(const std::string &file_name);
};

#endif