#include "../../include/db/file_manager.h"
#include "../../include/db/buffer_manager.h"
#include "../../include/db/record_file.h"
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
    std::memset(contents_, 0, sizeof(contents_));
}

int MemoryPage::Read(std::shared_ptr<DiskBlock> block)
{
    return file_manager_->Read(this, block);
}

int MemoryPage::Write(std::shared_ptr<DiskBlock> block)
{
    return file_manager_->Write(this, block);
}

int MemoryPage::Append(std::shared_ptr<MemoryBuffer> memory_buffer,
            const std::string &file_name,
            std::shared_ptr<TableInfo> table_info)
{
    // TODO: Think of implement like above
    return memory_buffer->contents_->file_manager_->Append(memory_buffer, file_name, table_info);
}

int MemoryPage::RecordFormatter(std::shared_ptr<TableInfo> table_info)
{
    // Reserve a size of int to save slot usage state, 
    // which default is RECORD_PAGE_EMPTY
    int record_size = table_info->record_length_ + sizeof(int);
    for (int pos = 0; pos + record_size <= DISK_BLOCK_SIZE; pos += record_size)
    {
        // First, write slot state
        SetInt(pos, Record_Page_Status::RECORD_PAGE_EMPTY);
        // TODO
    }
}

int MemoryPage::SetInt(int offset, int val)
{
    contents_[offset] = val >> 24;
    contents_[offset + 1] = val >> 16;
    contents_[offset + 2] = val >> 8;
    contents_[offset + 3] = val;
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

int FileManager::Read(MemoryPage *memory_page, std::shared_ptr<DiskBlock> disk_block)
{
    std::fstream &fp = GetFile(disk_block->file_name_);
    fp.seekg(disk_block->block_number_ * DISK_BLOCK_SIZE, std::fstream::beg);
    fp.read(memory_page->contents_, sizeof(memory_page->contents_));
}

int FileManager::Write(MemoryPage *memory_page, std::shared_ptr<DiskBlock> disk_block)
{
    std::fstream &fp = GetFile(disk_block->file_name_);
    fp.seekp(disk_block->block_number_ * DISK_BLOCK_SIZE, std::fstream::beg);
    fp.write(memory_page->contents_, sizeof(memory_page->contents_));
}

int FileManager::Append(std::shared_ptr<MemoryBuffer> memory_buffer,
                        const std::string &file_name,
                        std::shared_ptr<TableInfo> table_info)
{
    int new_block_number = Size(file_name);
    auto disk_block = std::make_shared<DiskBlock>(file_name, new_block_number, table_info);
    memory_buffer->disk_block_ = disk_block;
    Write(memory_buffer->contents_.get(), disk_block);
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

int FileManager::Size(const std::string &file_name)
{
    std::fstream &fd = GetFile(file_name);
    fd.seekg(0, std::fstream::end);
    auto file_length = fd.tellg();
    return file_length / DISK_BLOCK_SIZE;
}