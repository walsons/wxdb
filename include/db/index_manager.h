#ifndef INDEXMANAGER_H_
#define INDEXMANAGER_H_

#include <memory>
#include "../defs.h"
#include "../btree/btree.hpp"
#include "../db/col_val.h"

class IndexManager
{
    // rowid(4 bytes) null_mark(1 byte) index(x bytes)
    char *buf_;
    std::shared_ptr<IndexBTree> btr_;
    int size_;
    std::shared_ptr<Pager> pg_;
    void fill_buf(const char *key, int rid);
public:
    // using comparer = int(*)(const char *, const char  *);
    using comparer = std::function<int(const char *, const char *)>;
    static comparer GetIndexComparer(Col_Type type);
    IndexManager(std::shared_ptr<Pager> pg, int root_page_id, int size, comparer compare);
    ~IndexManager();
    int root_page_id();
    void Insert(const char *key, int row_id);
    void Erase(const char *key, int row_id);
};

#endif