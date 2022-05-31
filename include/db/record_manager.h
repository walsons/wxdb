#ifndef RECORD_MANAGER_H_
#define RECORD_MANAGER_H_

#include <memory>
#include "../page/pager.h"

class RecordManager
{
    std::shared_ptr<Pager> pg_;
    int page_id_, pos_, cur_page_id_;
    char *cur_buf_;
    int remain_, next_page_id_, offset_;
    bool dirty_;
public:
    RecordManager(std::shared_ptr<Pager> pg) : pg_(pg), page_id_(0) {}
    ~RecordManager() = default;
    void Open(int page_id, int pos, bool dirty);
    void Open(std::pair<int, int> p, bool dirty) { Open(p.first, p.second, dirty); }
    RecordManager &Seek(int offset);
    RecordManager &Write(const void *data, int size);
    RecordManager &Read(void *buf, int size);
    RecordManager &Forward(int size);
};

#endif