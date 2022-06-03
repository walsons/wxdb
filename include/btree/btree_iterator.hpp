#ifndef BTREE_ITERATOR_H_
#define BTREE_ITERATOR_H_

#include <memory>
#include <cassert>
#include "../page/pager.h"
#include "../page/general_page.h"

template <typename PageType>
class BTreeIterator
{
    std::shared_ptr<Pager> pg_;
    int page_id_, pos_;
    int cur_size_, prev_page_id_, next_page_id_;
    void load_info(int page_id)
    {
        page_id_ = page_id;
        if (page_id_)
        {
            PageType page{pg_->Read(page_id_), pg_};
            assert(page.page_type() == Page_Type::VARIANT_PAGE ||
                   page.page_type() == Page_Type::INDEX_LEAF_PAGE);
            cur_size_ = page.size();
            next_page_id_ = page.next_page();
            prev_page_id_ = page.prev_page();
        }
    }
public:
    using value_t = std::pair<int, int>;
    BTreeIterator(std::shared_ptr<Pager> pg, int page_id, int pos)
        : pg_(pg), page_id_(page_id), pos_(pos) { load_info(page_id_); }
    BTreeIterator(std::shared_ptr<Pager> pg, value_t value)
        : BTreeIterator(pg, value.first, value.second) {}
    std::shared_ptr<Pager> GetPager() { return pg_; }
    value_t Get() { return {page_id_, pos_}; }
    value_t operator*() { return Get(); }
    value_t next()
    {
        assert(page_id_);
        if (++pos_ == cur_size_)
        {
            load_info(next_page_id_);
            pos_ = 0;
        }
        return Get();
    }
    value_t prev()
    {
        assert(page_id_);
        if (pos_-- == 0)
        {
            load_info(prev_page_id_);
            pos_ = cur_size_ - 1;
        }
        return Get();
    }
    bool IsEnd() { return page_id_ == 0; }
};

#endif