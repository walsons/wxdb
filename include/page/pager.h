#ifndef PAGER_H_
#define PAGER_H_

#include <string>
#include <memory>
#include "page_file_system.h"

class Pager : public std::enable_shared_from_this<Pager>
{
    int file_id_;
public:
    Pager();
    Pager(const std::string &file_name);
    ~Pager();
    bool Open(const std::string &file_name);
    void Close();
    void Flush();
    int NewPage();
    void FreePage(int page_id);
    char *Read(int page_id);
    char *ReadForWrite(int page_id);
    void FreeOverflowPage(int page_id);
};

inline
Pager::Pager() : file_id_(0)
{
}

inline
Pager::Pager(const std::string &file_name) : file_id_(0)
{
    file_id_ = Open(file_name);
}

inline
Pager::~Pager()
{
    Close();
}

inline
bool Pager::Open(const std::string &file_name)
{
    PageFileSystem &fs = PageFileSystem::GetInstance();
    if (file_id_) { fs.Close(file_id_); }
    file_id_ = fs.Open(file_name);
    // If file_id_ equal to 0, it means failed
    return file_id_;
}

inline
void Pager::Close()
{
    if (file_id_) { PageFileSystem::GetInstance().Close(file_id_); }
    file_id_ = 0;
}

inline
void Pager::Flush()
{
    PageFileSystem::GetInstance().WriteBack(file_id_);
}

inline
int Pager::NewPage()
{
    return PageFileSystem::GetInstance().Allocate(file_id_);
}

inline
void Pager::FreePage(int page_id)
{
    PageFileSystem::GetInstance().Deallocate(file_id_, page_id);
}

inline
char *Pager::Read(int page_id)
{
    return PageFileSystem::GetInstance().Read(file_id_, page_id);
}

inline
char *Pager::ReadForWrite(int page_id)
{
    return PageFileSystem::GetInstance().ReadForWrite(file_id_, page_id);
}

#endif