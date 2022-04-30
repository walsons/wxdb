#ifndef PAGE_FILE_SYSTEM_H_
#define PAGE_FILE_SYSTEM_H_

#include <cstddef>
#include <utility>
#include <fstream>
#include <string>
#include <unordered_map>
#include "cache_manager.h"
#include "fid_manager.h"

constexpr unsigned PAGE_SIZE = 4092;

// The FileHeader is saved in first page of the file, 
// it is not counted to page_num, and if page_num = 0,
// which means no page. So page_id start from 1
struct PageFileHeader
{
    int page_num;
};


class PageFileSystem
{
private:
    struct pair_hash
    {
        template <typename T1, typename T2>
        std::size_t operator()(const std::pair<T1, T2> p) const
        {
            return std::hash<T1>{}(p.first) ^ (std::hash<T2>{}(p.second) << 1);
        }
    };

    // Cache
    bool dirty_[CACHE_CAPACITY];
    char buffer_[CACHE_CAPACITY * PAGE_SIZE];
    char tmp_page_[PAGE_SIZE];
    CacheManager cache_manager_;

    // The page.first is file id, page.second is page id
    // The index indicates the position of buffer_
    using file_page_t = std::pair<int, int>;
    std::unordered_map<file_page_t, int, pair_hash> page2index_;
    file_page_t index2page_[CACHE_CAPACITY]; 
    
    // File id manager
    FidManager fid_manager_;
    std::fstream files_[MAX_NUM_FILE_ID];
    PageFileHeader file_info_[MAX_NUM_FILE_ID];

private:
    PageFileSystem();
    char *read(int file_id, int page_id, int &index);
    void free_last_cache();
    void write_page_to_file(int file_id, int page_id, const char *data);

public:
    static PageFileSystem &GetInstance();
    ~PageFileSystem();
    int Open(const std::string &name);
    void Close(int file_id);
    void WriteBack(int file_id);
    int Allocate(int file_id);
    void Deallocate(int file_id, int page_id);
    // Return the buffer just for read
    char *Read(int file_id, int page_id);
    // Return the buffer that can be write,
    // it will set dirty to true automatically
    char *ReadForWrite(int file_id, int page_id);
};

inline
PageFileSystem::PageFileSystem()
{
    // file id and page id both start from 1, if they are 0, 
    // which means this page of cache is not used
    for (int i = 0; i < CACHE_CAPACITY; ++i)
    {
        index2page_[0] = {0, 0};
    }
}

inline 
PageFileSystem &PageFileSystem::GetInstance()
{
    static PageFileSystem page_file_system;
    return page_file_system;
}

inline
PageFileSystem::~PageFileSystem()
{
    for (int i = 1; i <= MAX_NUM_FILE_ID; ++i)
    {
        if (fid_manager_.IsUsed(i)) { Close(i); }
    }
}

#endif