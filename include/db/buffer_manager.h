// #ifndef BUFFER_MANAGER_H_
// #define BUFFER_MANAGER_H_

// #include <memory>
// #include "../sql/table_info.h"

// // TODO: set a proper value
// const unsigned BUFFER_MAX_SIZE = 100;

// class DiskBlock;
// class MemoryPage;
// class FileManager;

// class MemoryBuffer
// {
// public:
//     MemoryBuffer(std::shared_ptr<FileManager> file_manager_);
//     ~MemoryBuffer() = default;
//     int GetInt(int offset);
//     void SetInt(int offset, const int &val, int tx_num, int lsn);
//     double GetDouble(int offset);
//     void SetDouble(int offset, const double &val, int tx_num, int lsn);
//     std::string GetString(int offset);
//     void SetString(int offset, const std::string &val, int tx_num, int lsn);

//     void Flush();
//     void Pin();
//     void Unpin();
//     bool IsPinned();
//     bool IsModifiedBy(int tx_num);

//     void AssignTo(std::shared_ptr<DiskBlock> disk_block);
//     void AssignToNew(const std::string &file_name, 
//                     std::shared_ptr<TableInfo> table_info);

//     std::shared_ptr<MemoryPage> memory_page_;
//     std::shared_ptr<DiskBlock> disk_block_;
//     int pins_;
//     int modified_by_;  // Value -1 represent is not modified
//     int log_sequence_number_;  // Value -1 represent that it doesn't have respective log record
// };

// class BufferManager
// {
// public:
//     BufferManager(int buffer_size, std::shared_ptr<FileManager> file_manager);
//     ~BufferManager() = default;

//     // If all memory buffer are pinned, return false to indicate pin failed
//     bool Pin(std::shared_ptr<DiskBlock> block);
//     bool PinNew(const std::string &file_name, std::shared_ptr<TableInfo> table_info);

//     void Unpin(std::shared_ptr<MemoryBuffer> buffer);
//     void FlushAll(int tx_num);
//     int num_available();

//     int num_available_;
//     std::shared_ptr<MemoryBuffer> buffer_pool_[BUFFER_MAX_SIZE];

// private:
//     std::shared_ptr<MemoryBuffer> FindExisting(std::shared_ptr<DiskBlock> disk_block);
//     std::shared_ptr<MemoryBuffer> FindChooseUnpinnedBuffer();
// };

// #endif