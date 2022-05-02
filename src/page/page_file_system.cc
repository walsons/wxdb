#include "../../include/page/page_file_system.h"

char *PageFileSystem::read(int file_id, int page_id, int &index)
{
    file_page_t key = {file_id, page_id};
    auto it = page2index_.find(key);
    // If the page doesn't exist in the cache
    if (it == page2index_.end())
    {
        // If cache is full
        if (index2page_[cache_manager_.LastNode()].first != 0)
        {
            free_last_cache();
        }
        // It's quick that read to last page of cache then access it, 
        index = cache_manager_.LastNode();
        cache_manager_.Access(index);
        dirty_[index] = false;
        index2page_[index] = key;
        page2index_[key] = index;
        files_[file_id].seekp(page_id * PAGE_SIZE, std::ios::beg);
        files_[file_id].write(reinterpret_cast<const char *>(buffer_ + index * PAGE_SIZE), PAGE_SIZE);
    }
    else
    {
        index = it->second;
        cache_manager_.Access(index);
    }
    return buffer_ + index * PAGE_SIZE;
}

void PageFileSystem::free_last_cache()
{
    int last = cache_manager_.LastNode();
    file_page_t key = index2page_[last];
    // The page of cache is used
    if (key.first != 0)
    {
        // If this page is modified
        if (dirty_[last])
        {
            write_page_to_file(key.first, key.second, buffer_ + last * PAGE_SIZE);
            // dirty_ will be reset to false when a new page 
            // is read to this page of cache
        }
        page2index_.erase(key);
        index2page_[last] = {0, 0};
    }
}

void PageFileSystem::write_page_to_file(int file_id, int page_id, const char *data)
{
    files_[file_id].seekp(page_id * PAGE_SIZE, std::ios::beg);
    files_[file_id].write(data, PAGE_SIZE);
}

int PageFileSystem::Open(const std::string &name)
{
    int file_id = fid_manager_.Allocate();
    // Allocate failed
    if (file_id == 0) { return 0; }
    std::ifstream ifs(name);
    bool file_exist = ifs.is_open();
    if (file_exist) { ifs.close(); }
    std::fstream fd(name, std::ios::in | std::ios::out | std::ios::binary);
    PageFileHeader header;
    if (!file_exist)
    {
        header.page_num = 0;
        fd.seekp(0, std::ios::beg);
        fd.write(reinterpret_cast<const char *>(&header), sizeof(header));
    }
    else 
    {
        fd.seekg(0, std::ios::beg);
        fd.read(reinterpret_cast<char *>(&header), sizeof(header));
    }
    file_info_[file_id] = header;
    files_[file_id] = std::move(fd);
    return file_id;
}

void PageFileSystem::Close(int file_id)
{
    WriteBack(file_id);
    fid_manager_.Deallocate(file_id);
    files_[file_id].close();
}

void PageFileSystem::WriteBack(int file_id)
{
    for (int i = 0; i < CACHE_CAPACITY; ++i)
    {
        file_page_t page = index2page_[i];
        if (page.first == file_id && dirty_[i])
        {
            files_[page.first].seekp(page.second * PAGE_SIZE, std::ios::beg);
            files_[page.first].write(buffer_ + i * PAGE_SIZE, PAGE_SIZE);
            page2index_.erase(page);
            index2page_[i] = {0, 0};
        }
    }
    // write PageFileHeader
    files_[file_id].seekp(0, std::ios::beg);
    files_[file_id].write(reinterpret_cast<const char *>(&file_info_[file_id]), sizeof(PageFileHeader));
}

int PageFileSystem::Allocate(int file_id)
{
    int page_id = ++file_info_[file_id].page_num;
    Read(file_id, page_id);
    return page_id;
}

void PageFileSystem::Deallocate(int file_id, int page_id)
{
    // This will deallocate all pages whose page_id 
    // greater or equal to page_id;
    file_info_[file_id].page_num = page_id - 1;
    // TODO: reduce file size
}

char *PageFileSystem::Read(int file_id, int page_id)
{
    int index;
    return read(file_id, page_id, index);
}

char *PageFileSystem::ReadForWrite(int file_id, int page_id)
{
    int index;
    char *buf = read(file_id, page_id, index);
    dirty_[index] = true;
    return buf;
}