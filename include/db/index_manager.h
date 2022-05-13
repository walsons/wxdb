#ifndef INDEXMANAGER_H_
#define INDEXMANAGER_H_

#include <memory>
#include "../btree/btree.h"

class IndexManager
{
    char *buf_;
    std::shared_ptr<IndexBTree> btr_;
    int size_;
    std::shared_ptr<Pager> pg_;
    void fill_buf(const char *key, int rid);
public:
    using comparer = int(*)(const char *, const char  *);
    IndexManager(std::shared_ptr<Pager> pg, int size, int root_page_id, comparer compare);
    ~IndexManager();
    int root_page_id();
    void Insert(const char *key, int row_id);
    void Erase(const char *key, int row_id);
};

#endif