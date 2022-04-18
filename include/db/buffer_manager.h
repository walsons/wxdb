#ifndef BUFFER_MANAGER_H_
#define BUFFER_MANAGER_H_

#include <memory>
#include "../sql/common.h"

class DiskBlock;
class MemoryPage;
class FileManager;

class MemoryBuffer
{
public:
    MemoryBuffer(std::shared_ptr<FileManager> file_manager_);
    ~MemoryBuffer() = default;
    int GetInt(int offset);
    int GetDouble(int offset);
    int GetString(int offset);
    int SetInt(int offset);
    int SetDouble(int offset);
    int SetString(int offset);

    int Flush();
    int Pin();
    int Unpin();
    int IsPinned();
    int IsModified(int num);

    int AssignTo(std::shared_ptr<DiskBlock> disk_block);
    int AssignToNew(const std::string &file_name, 
                    std::shared_ptr<TableInfo> table_info);

    std::shared_ptr<MemoryPage> contents_;
    std::shared_ptr<DiskBlock> disk_block_;
    int pins_;
    int is_modified_;  // Value -1 represent is not modified
    int log_sequence_number_;  // Value -1 represent that it doesn't have respective log record
};

#endif