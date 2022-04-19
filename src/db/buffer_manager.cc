#include "../../include/db/buffer_manager.h"
#include "../../include/db/file_manager.h"

MemoryBuffer::MemoryBuffer(std::shared_ptr<FileManager> file_manager_)
    : memory_page_(std::make_shared<MemoryPage>(file_manager_))
    , disk_block_(nullptr)
    , pins_(0)
    , modified_by_(-1)
    , log_sequence_number_(-1)
{
}

int MemoryBuffer::GetInt(int offset)
{
    return memory_page_->GetInt(offset);
}

void MemoryBuffer::SetInt(int offset, int val, int tx_num, int lsn)
{
    // lsn is a abbreviation of log_sequence_number_
    modified_by_ = tx_num;
    if (lsn >= 0) { log_sequence_number_ = lsn; }
    memory_page_->SetInt(offset, val);
}

double MemoryBuffer::GetDouble(int offset)
{
    return memory_page_->GetDouble(offset);
}

void MemoryBuffer::SetDouble(int offset)
{
}

std::string MemoryBuffer::GetString(int offset)
{
    return memory_page_->GetString(offset);
}

void MemoryBuffer::SetString(int offset)
{
}

int MemoryBuffer::Flush()
{
}

int MemoryBuffer::Pin()
{
}

int MemoryBuffer::Unpin()
{
}

int MemoryBuffer::IsPinned()
{
}

int MemoryBuffer::IsModifiedBy(int tx_num)
{
}

int MemoryBuffer::AssignTo(std::shared_ptr<DiskBlock> disk_block)
{
}
int MemoryBuffer::AssignToNew(const std::string &file_name, 
                              std::shared_ptr<TableInfo> table_info)
{
}