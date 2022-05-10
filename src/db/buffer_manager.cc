// #include "../../include/db/buffer_manager.h"
// #include "../../include/db/file_manager.h"

// /******************** MemoryBuffer ********************/
// MemoryBuffer::MemoryBuffer(std::shared_ptr<FileManager> file_manager_)
//     : memory_page_(std::make_shared<MemoryPage>(file_manager_))
//     , disk_block_(nullptr)
//     , pins_(0)
//     , modified_by_(-1)
//     , log_sequence_number_(-1)
// {
// }

// int MemoryBuffer::GetInt(int offset)
// {
//     return memory_page_->GetInt(offset);
// }

// void MemoryBuffer::SetInt(int offset, const int &val, int tx_num, int lsn)
// {
//     // lsn is a abbreviation of log_sequence_number_
//     modified_by_ = tx_num;
//     if (lsn >= 0) { log_sequence_number_ = lsn; }
//     memory_page_->SetInt(offset, val);
// }

// double MemoryBuffer::GetDouble(int offset)
// {
//     return memory_page_->GetDouble(offset);
// }

// void MemoryBuffer::SetDouble(int offset, const double &val, int tx_num, int lsn)
// {
//     modified_by_ = tx_num;
//     if (lsn >= 0) { log_sequence_number_ = lsn; }
//     memory_page_->SetDouble(offset, val);
// }

// std::string MemoryBuffer::GetString(int offset)
// {
//     return memory_page_->GetString(offset);
// }

// void MemoryBuffer::SetString(int offset, const std::string &val, int tx_num, int lsn)
// {
//     modified_by_ = tx_num;
//     if (lsn >= 0) { log_sequence_number_ = lsn; }
//     memory_page_->SetString(offset, val);
// }

// void MemoryBuffer::Flush()
// {
//     if (modified_by_ >= 0)
//     {
//         memory_page_->Write(disk_block_);
//         modified_by_ = -1;
//     }
// }

// void MemoryBuffer::Pin()
// {
//     ++pins_;
// }

// void MemoryBuffer::Unpin()
// {
//     --pins_;
// }

// bool MemoryBuffer::IsPinned()
// {
//     return pins_ > 0;
// }

// bool MemoryBuffer::IsModifiedBy(int tx_num)
// {
//     return tx_num == modified_by_;    
// }

// void MemoryBuffer::AssignTo(std::shared_ptr<DiskBlock> disk_block)
// {
//     Flush();
//     disk_block_ = disk_block;
//     memory_page_->Read(disk_block_);
//     pins_ = 0;
// }

// void MemoryBuffer::AssignToNew(const std::string &file_name, 
//                               std::shared_ptr<TableInfo> table_info)
// {
//     Flush();
//     memory_page_->RecordFormatter(table_info);
//     memory_page_->Append(this, file_name, table_info);
//     pins_ = 0;
// }

// /******************** BufferManager ********************/
// BufferManager::BufferManager(int buffer_size, std::shared_ptr<FileManager> file_manager)
//     : num_available_(buffer_size)
// {
//     for (int i = 0; i < buffer_size; ++i)
//     {
//         buffer_pool_[i] = std::make_shared<MemoryBuffer>(file_manager);
//     }
// }

// bool BufferManager::Pin(std::shared_ptr<DiskBlock> block)
// {
//     auto memory_buffer = FindExisting(block);   
//     if (memory_buffer == nullptr)
//     {
//         memory_buffer = FindChooseUnpinnedBuffer();
//         // If all memory buffer are pinned, return false to indicate
//         if (memory_buffer == nullptr)
//         {
//             return false;
//         }
//         memory_buffer->AssignTo(block);
//     }
//     if (!memory_buffer->IsPinned())
//     {
//         --num_available_;
//         memory_buffer->Pin();
//     }
//     return true;
// }

// bool BufferManager::PinNew(const std::string &file_name, std::shared_ptr<TableInfo> table_info)
// {
//     auto memory_buffer = FindChooseUnpinnedBuffer();
//     if (memory_buffer == nullptr) { return false; }
//     memory_buffer->AssignToNew(file_name, table_info);
//     --num_available_;
//     memory_buffer->Pin();
//     return true;

// }

// void BufferManager::Unpin(std::shared_ptr<MemoryBuffer> memory_buffer)
// {
//     if (memory_buffer->IsPinned())
//     {
//         memory_buffer->Unpin();
//         --num_available_;
//     }
// }

// void BufferManager::FlushAll(int tx_num)
// {
//     for (int i = 0; i < BUFFER_MAX_SIZE; ++i)
//     {
//         if (buffer_pool_[i]->IsModifiedBy(tx_num))
//         {
//             buffer_pool_[i]->Flush();
//         }
//     }
// }

// int BufferManager::num_available()
// {
//     return num_available_;
// }
    
// // TODO: optimize the find algorithm
// std::shared_ptr<MemoryBuffer> BufferManager::FindExisting(std::shared_ptr<DiskBlock> disk_block)
// {
//     for (int i = 0; i < BUFFER_MAX_SIZE; ++i)
//     {
//         auto memory_buffer = buffer_pool_[i];
//         auto block = memory_buffer->disk_block_;
//         if (block != nullptr && 
//             block->file_name_ == disk_block->file_name_ &&
//             block->block_number_ == disk_block->block_number_)
//         {
//             return memory_buffer;
//         }
//     }
//     return nullptr;
// }

// // TODO: maybe need to consider other replacement algorithm
// std::shared_ptr<MemoryBuffer> BufferManager::FindChooseUnpinnedBuffer()
// {
//     for (int i = 0; i < BUFFER_MAX_SIZE; ++i)
//     {
//         if (!buffer_pool_[i]->IsPinned())
//         {
//             return buffer_pool_[i];
//         }
//     }
//     return nullptr;
// }