#include "../../include/db/index_manager.h"
#include <cassert>
#include "../../include/btree/comparer.hpp"

void IndexManager::fill_buf(const char *key, int rid)
{
    *reinterpret_cast<int *>(buf_) = rid;
    if (key != nullptr)
    {
        buf_[4] = 0;
        std::memcpy(buf_ + sizeof(int) + 1, key, size_);
    }
    else
    {
        buf_[4] = 1;
        std::memset(buf_ + sizeof(int) + 1, 0, size_);
    }
}

IndexManager::comparer IndexManager::GetIndexComparer(Col_Type type)
{
    switch (type)
    {
    case Col_Type::COL_TYPE_INT:
        return integer_cast_comparer;
        break;
    case Col_Type::COL_TYPE_DOUBLE:
        return double_cast_comparer;
        break;
    case Col_Type::COL_TYPE_CHAR:
    case Col_Type::COL_TYPE_VARCHAR:
        return string_comparer;
        break;
    default:
        assert(false);
        break;
    }
    return string_comparer;
}

 IndexManager::IndexManager(int size, std::shared_ptr<Pager> pg, int root_page_id, comparer compare)
    : buf_(new char[sizeof(int) + 1 + size]), size_(size), pg_(pg)
    , btr_(std::make_shared<IndexBTree>(pg_, root_page_id, sizeof(int) + 1 + size, 
        [compare](const char *a, const char *b) -> int {
            // One of a and b is null, null < any value
            if (a[4] != b[4])
            {
                return a[4] ? -1 : 1;
            }
            // A and B are not null, compare data
            else if (!a[4])
            {
                int res = compare(a + sizeof(int) + 1, b + sizeof(int) + 1);
                if (res != 0) { return res; }
            }
            // if data equal or both are null, compare row id
            return integer_comparer(*reinterpret_cast<const int *>(a), *reinterpret_cast<const int *>(b));
        })) {}

IndexManager::~IndexManager()
{
    delete[]buf_;
    buf_ = nullptr;
}

int IndexManager::root_page_id()
{
    return btr_->root_page_id();
}

void IndexManager::Insert(const char *key, int row_id)
{
    fill_buf(key, row_id);
    btr_->Insert(buf_, row_id);
}

void IndexManager::Erase(const char *key, int row_id)
{
    fill_buf(key, row_id);
    bool res = btr_->Erase(buf_);
}

void IndexManager::Update(const char *key, int row_id)
{
    fill_buf(key, row_id);
    btr_->Update(buf_, row_id);
}