#include "../../include/db/file_manager.h"
#include <direct.h>
#include <cstring>
#include <iostream>

/******************** DiskBlock ********************/
DiskBlock::DiskBlock(const std::string &file_name, int block_number, 
                     std::shared_ptr<TableInfo> table_info)
    : file_name_(file_name), block_number_(block_number), table_info_(table_info)
{
}

/******************** MemoryPage ********************/
MemoryPage::MemoryPage(std::shared_ptr<FileManager> file_manager)
    : file_manager_(file_manager)
{
    std::memset(content_, 0, sizeof(content_));
}
int Read(std::shared_ptr<DiskBlock> block)
{
    // TODO
}
int Write(std::shared_ptr<DiskBlock> block)
{
    // TODO
}

/******************** FileManager ********************/
FileManager::FileManager(const std::string &db_dir_name, const std::string &db_name)
    : db_dir_name_(db_dir_name), is_new_(false)
{
    // Check directory is exist, create one if not
    if (db_dir_name_.back() != '/') { db_dir_name_ += "/"; }
    int exists = access(db_dir_name.c_str(), F_OK);
    if (exists != 0)
    {
        is_new_ = true;
        int res = mkdir(db_dir_name.c_str());
        if (res != 0)
        {
            std::cerr << "Create directory \"" << db_dir_name << "\" failed!" << std::endl;
        }
    }
    // Create a database file
    std::string db_file_path = db_dir_name_ + db_name; 
    fd_.open(db_file_path, std::ios::in | std::ios::out | std::ios::binary);
}

FileManager::~FileManager()
{
    // TODO: check all files are opened and close them
}

int FileManager::Read(std::shared_ptr<MemoryPage> memory_page, std::shared_ptr<DiskBlock> disk_block)
{
    // TODO: std::fstream fp = 
}

std::fstream &FileManager::GetFile(const std::string &file_name)
{
    auto it = open_files_.find(file_name);
    if (it == open_files_.end())
    {
        std::string file_path = db_dir_name_ + file_name;
        std::fstream fd(file_path, std::ios::out);
        if (fd.is_open())
        {
            std::cerr << "Open file error!" << std::endl;
            return fd;
        }
        open_files_.insert(std::pair<std::string, std::fstream &>(file_name, fd));
        return fd;
    }
    return it->second;
}
