#include "../../include/db/record_manager.h"
#include <cstring>
#include "../../include/page/data_page.hpp"
#include "../../include/page/overflow_page.h"

void RecordManager::Open(int page_id, int pos, bool dirty)
{
    page_id_ = page_id;
    pos_ = pos;
    cur_page_id_ = page_id;
    dirty_ = dirty;
    offset_ = 0;
    if (page_id)
    {
        DataPage<int> page{dirty_ ? pg_->ReadForWrite(page_id_): pg_->Read(page_id_), pg_};
        auto block = page.GetBlock(pos_);
        remain_ = block.first.size - sizeof(DataPage<int>::BlockHeader);
        next_page_id_ = block.first.overflow_page;
        cur_buf_ = block.second;
    }
}

RecordManager &RecordManager::Seek(int offset)
{
    if (offset >= offset_)
    {
        Forward(offset - offset_);
    }
    else
    {
        cur_page_id_ = page_id_;
        this->offset_ = 0;
        DataPage<int> page{dirty_ ? pg_->ReadForWrite(page_id_): pg_->Read(page_id_), pg_};
        auto block = page.GetBlock(pos_);
        remain_ = block.first.size - sizeof(DataPage<int>::BlockHeader);
        next_page_id_ = block.first.overflow_page;
        cur_buf_ = block.second;
        Forward(offset);
    }
    return *this;
}

RecordManager &RecordManager::Write(const void *data, int size)
{
    const char *tmp = reinterpret_cast<const char *>(data);
    while (size)
    {
        int len = size < remain_ ? size : remain_;
        std::memcpy(cur_buf_, tmp, len);
        if (!dirty_) { pg_->MarkDirty(cur_page_id_); }
        tmp += len;
        size -= len;
        Forward(len);
    }
    return *this;
}

RecordManager &RecordManager::Read(void *buf, int size)
{
    char *tmp = reinterpret_cast<char *>(buf);
    while (size)
    {
        int len = size < remain_ ? size : remain_;
        std::memcpy(tmp, cur_buf_, len);
        tmp += len;
        size -= len;
        Forward(len);
    }
    return *this;
}

RecordManager &RecordManager::Forward(int size)
{
    remain_ -= size;
    offset_ += size;
    cur_buf_ += size;
    while (remain_ <= 0 && next_page_id_)
    {
        OverflowPage page{dirty_ ? pg_->ReadForWrite(next_page_id_) : pg_->Read(next_page_id_), pg_};       
        cur_buf_ = page.block() + (-remain_);
        remain_ += page.size();
        cur_page_id_ = next_page_id_;
        next_page_id_ = page.next();
    }
    return *this;
}