#include "../../include/db/buffer_manager.h"
#include "../../include/db/file_manager.h"

MemoryBuffer::MemoryBuffer(std::shared_ptr<FileManager> file_manager_)
    : contents_(std::make_shared<MemoryPage>(file_manager_))
    , disk_block_(nullptr)
    , pins_(0)
    , is_modified_(-1)
    , log_sequence_number_(-1)
{
}

int MemoryBuffer::GetInt(int offset)
{
}

int MemoryBuffer::GetDouble(int offset)
{
}

int MemoryBuffer::GetString(int offset)
{
}

int MemoryBuffer::SetInt(int offset)
{
}

int MemoryBuffer::SetDouble(int offset)
{
}

int MemoryBuffer::SetString(int offset)
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

int MemoryBuffer::IsModified(int num)
{
}

int MemoryBuffer::AssignTo(std::shared_ptr<DiskBlock> disk_block)
{
}
int MemoryBuffer::AssignToNew(const std::string &file_name, 
                              std::shared_ptr<TableInfo> table_info)
{
}