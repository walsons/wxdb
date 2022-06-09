#ifndef BTREE_ITERATOR_H_
#define BTREE_ITERATOR_H_

#include <memory>
#include <cassert>
#include "../page/pager.h"
#include "../page/general_page.h"

template <typename PageType>
class BTreeIterator
{
    int page_id_, pos_;
    // first way to iterate via traverse
    int original_page_id_, original_pos_;
    std::shared_ptr<Pager> pg_;
    int cur_size_, prev_page_id_, next_page_id_;
    // another way to iterate via rowid
    std::vector<int> rows_;
    int rows_counter_;
    std::shared_ptr<IntBTree> btr_;
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
    BTreeIterator() = default;
    BTreeIterator(std::shared_ptr<Pager> pg, int page_id, int pos)
        : original_page_id_(page_id), original_pos_(pos) 
        , pg_(pg), page_id_(page_id), pos_(pos) { load_info(page_id_); }
    BTreeIterator(std::shared_ptr<Pager> pg, value_t value)
        : BTreeIterator(pg, value.first, value.second) {}
    BTreeIterator(std::vector<int> rows, std::shared_ptr<IntBTree> btr) 
        : rows_(rows), rows_counter_(0), btr_(btr) 
    {
        auto page_pos = btr_->upper_bound(btr_->root_page_id(), rows_[rows_counter_]);
        page_id_ = page_pos.first;
        pos_ = page_pos.second;

    }
    std::shared_ptr<Pager> GetPager() 
    { 
        // via rowid
        if (!rows_.empty()) { return btr_->pg(); }
        // via traverse
        return pg_; 
    }
    value_t Get() 
    { 
        return {page_id_, pos_}; 
    }
    value_t operator*() { return Get(); }
    value_t next()
    {
        // via rowid
        if (!rows_.empty())
        {
            auto page_pos = btr_->upper_bound(btr_->root_page_id(), rows_[rows_counter_++]);
            page_id_ = page_pos.first;
            pos_ = page_pos.second;
            return Get();
        }
        // via traverse
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
        // via rowid
        if (!rows_.empty())
        {
            auto page_pos = btr_->upper_bound(btr_->root_page_id(), rows_[rows_counter_--]);
            page_id_ = page_pos.first;
            pos_ = page_pos.second;
            return Get();
        }
        // via traverse
        assert(page_id_);
        if (pos_-- == 0)
        {
            load_info(prev_page_id_);
            pos_ = cur_size_ - 1;
        }
        return Get();
    }
    bool IsEnd() 
    {
        // via rowid
        if (!rows_.empty())
        {
            return rows_counter_ == rows_.size() || rows_counter_ == -1;
        }
        // via traverse
        return page_id_ == 0; 
    }
    void Reset() 
    { 
        // via rowid
        if (!rows_.empty())
        {
            rows_counter_ = 0;
            return;
        }
        // via traverse
        load_info(original_page_id_);
        pos_ = original_pos_;
    }
};

#endif