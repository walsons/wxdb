#ifndef BTREE_HPP_
#define BTREE_HPP_

#include <cstring>
#include <memory>
#include <functional>
#include "../page/pager.h"
#include "../page/fixed_page.hpp"
#include "../page/index_leaf_page.hpp"
#include "../page/data_page.hpp"
#include "comparer.hpp"
#include "search.hpp"

template <typename KeyType, typename Comparer, typename Copier>
class BTree
{
    std::shared_ptr<Pager> pg_;
    int root_page_id_;
    int field_size_;
    Comparer comparer_;
    Copier copier_;
public:
    // using key_t = KeyType;
    using interior_page = FixedPage<KeyType>;
    using leaf_page = typename std::conditional<std::is_same<KeyType, const char *>::value,
                                                IndexLeafPage<KeyType>,
                                                DataPage<KeyType>>::type;
    using search_result = std::pair<int, int>;  // (page_id, pos);

    BTree(std::shared_ptr<Pager> pg, int root_page_id, int field_size, Comparer compare, Copier copier);
    virtual ~BTree() = default;
    void Insert(KeyType key, const char *data, int data_size);
    bool Erase(KeyType key);
    int root_page_id() { return root_page_id_; }
    search_result upper_bound(int now_page_id, KeyType key);

private:
    struct insert_ret
    {
        bool split;
        int upper_page_id;
        char *lower_half, *upper_half;
    };
    struct merge_ret
    {
        bool merged_left, merged_right;
        int merged_page_id;
    };
    struct erase_ret
    {
        bool found;
        merge_ret merge;
        KeyType largest;
    };
    insert_ret insert_leaf(int now_page_id, char *now_addr, KeyType key, const char *data, int data_size);
    insert_ret insert_interior(int now_page_id, char *now_addr, KeyType key, const char *data, int data_size);
    template <typename Page>
    void insert_split_root(insert_ret ret);
    template <typename Page, typename ChildPage>
    insert_ret insert_post_process(int page_id, int child_page_id, int child_pos, insert_ret child_ret);
};

template <typename KeyType, typename Comparer, typename Copier>
BTree<KeyType, Comparer, Copier>::BTree(std::shared_ptr<Pager> pg, 
    int root_page_id, int field_size, Comparer comparer, Copier copier)
    : pg_(pg), root_page_id_(root_page_id), field_size_(field_size)
    , comparer_(comparer), copier_(copier)
{
    if (root_page_id_ == 0)
    {
        root_page_id_ = pg_->NewPage();
        leaf_page{pg_->ReadForWrite(root_page_id_), pg_}.Init(field_size_);
    }
}

// For IntBTree data_page, key is row_id(int), data is a row, data_size is row data size.
// For IndexBTree index_leaf_page, key is buf(row_id, is_null, index), data is buf too, data_size is delegate row_id.
// For both interior_page, child delegate page id.
// 
// Even though buf have row_id info in first 4 bytes, we can store row_id in child so that no need to rewrite 
// insert function(which means interior_page and index_leaf_page share the same insert function)
template <typename KeyType, typename Comparer, typename Copier>
void BTree<KeyType, Comparer, Copier>::Insert(KeyType key, const char *data, int data_size)
{
    char *addr = pg_->ReadForWrite(root_page_id_);
    Page_Type page_type = GeneralPage::GetPageType(addr);
    if (page_type == Page_Type::FIXED_PAGE)
    {
        insert_ret ret = insert_interior(root_page_id_,
            addr, key, data, data_size);
        insert_split_root<interior_page>(ret);
    }
    // INDEX_LEAF_PAGE, VARIANT_PAGE,
    else
    {
        insert_ret ret = insert_leaf(root_page_id_, addr, key, data, data_size);
        insert_split_root<leaf_page>(ret);
    }
}

template <typename KeyType, typename Comparer, typename Copier>
bool BTree<KeyType, Comparer, Copier>::Erase(KeyType key)
{
    // TODO
    return true;
}

template <typename KeyType, typename Comparer, typename Copier>
typename BTree<KeyType, Comparer, Copier>::insert_ret 
BTree<KeyType, Comparer, Copier>::insert_leaf(int now_page_id, 
    char *now_addr, KeyType key, const char *data, int data_size)
{
    leaf_page page{now_addr, pg_};
    // Find the first pos of key int page greater or equal to key
    int child_pos = Search::upper_bound(0, page.size(), [&](int id) {
        return comparer_(page.get_key(id), key) >= 0;
    });
    insert_ret ret;
    ret.split = false;

    bool succ = page.Insert(child_pos, data, data_size);
    if (!succ)
    {
        auto upper = page.Split(now_page_id);
        leaf_page upper_page = upper.second;
        leaf_page lower_page = page;
        // TODO: check underflow
        if (child_pos < upper_page.size())
        {
            upper_page.Insert(child_pos, data, data_size);
        }
        else
        {
            lower_page.Insert(child_pos - upper_page.size(), data, data_size);
        }
        ret.split = true;
        ret.lower_half = lower_page.buf_;
        ret.upper_half = upper_page.buf_;
        ret.upper_page_id = upper.first;
    }
    return ret;
}

template <typename KeyType, typename Comparer, typename Copier>
typename BTree<KeyType, Comparer, Copier>::insert_ret 
BTree<KeyType, Comparer, Copier>::insert_interior(int now_page_id, 
    char *now_addr, KeyType key, const char *data, int data_size)
{
    interior_page page{now_addr, pg_};
    int child_pos = Search::upper_bound(0, page.size(), [&](int id) {
        return comparer_(page.get_key(id), key) >= 0;
    });
    child_pos = std::min(page.size() - 1, child_pos);
    int child_page_id = page.children(child_pos);
    char *child_addr = pg_->ReadForWrite(child_page_id);
    Page_Type child_page_type = GeneralPage::GetPageType(child_addr);
    if (child_page_type == Page_Type::FIXED_PAGE)
    {
        auto child_ret = insert_interior(child_page_id, child_addr, key, data, data_size);
        return insert_post_process<interior_page, interior_page>(now_page_id,
            child_page_id, child_pos, child_ret);
    }
    // else: INDEX_LEAF_PAGE, VARIANT_PAGE,
    auto child_ret = insert_leaf(child_page_id, child_addr, key, data, data_size);
    return insert_post_process<interior_page, leaf_page>(now_page_id, 
        child_page_id, child_pos, child_ret);
}

template <typename KeyType, typename Comparer, typename Copier>
template <typename Page>
void BTree<KeyType, Comparer, Copier>::insert_split_root(insert_ret ret)
{
    if (ret.split)
    {
        int new_page_id = pg_->NewPage();
        interior_page page{pg_->ReadForWrite(new_page_id), pg_};
        page.Init(field_size_);
        Page lower{ret.lower_half, pg_};
        Page upper{ret.upper_half, pg_};
        page.Insert(0, upper.get_key(upper.size() - 1), ret.upper_page_id);
        page.Insert(1, lower.get_key(lower.size() - 1), root_page_id_);
        root_page_id_ = new_page_id;
    }
}

template <typename KeyType, typename Comparer, typename Copier>
template <typename Page, typename ChildPage>
typename BTree<KeyType, Comparer, Copier>::insert_ret 
BTree<KeyType, Comparer, Copier>::insert_post_process(int page_id, 
    int child_page_id, int child_pos, insert_ret child_ret)
{
    insert_ret ret;
    ret.split = false;
    Page page{pg_->ReadForWrite(page_id), pg_};
    if (child_ret.split)
    {
        ChildPage lower_child{child_ret.lower_half, pg_};
        ChildPage upper_child{child_ret.upper_half, pg_};
        // Update the key of page
        page.set_key(child_pos, lower_child.get_key(lower_child.size() - 1));
        // Insert the new page to page
        KeyType child_largest = copier_(upper_child.get_key(upper_child.size() - 1));
        bool succ = page.Insert(child_pos, child_largest, child_ret.upper_page_id);
        if (!succ)
        {
            auto upper = page.Split(page_id);
            Page upper_page = upper.second;
            Page lower_page = page;
            // TODO: check underflow
            if (child_pos + 1 < lower_page.size())
            {
                lower_page.Insert(child_pos + 1, child_largest, child_ret.upper_page_id);
            }
            else
            {
                upper_page.Insert(child_pos + 1 - lower_page.size(), 
                                  child_largest, child_ret.upper_page_id);
            }
            ret.split = true;
            ret.lower_half = lower_page.buf_;
            ret.upper_half = upper_page.buf_;
            ret.upper_page_id = upper.first;
        }
    }
    else
    {
        ChildPage child_page{pg_->ReadForWrite(child_page_id), pg_};
        // Update the key of page
        page.set_key(child_pos, child_page.get_key(child_page.size() - 1));
    }
    return ret;
}

template <typename KeyType, typename Comparer, typename Copier>
typename BTree<KeyType, Comparer, Copier>::search_result 
BTree<KeyType, Comparer, Copier>::upper_bound(int now_page_id, KeyType key)
{
    char *now_addr = pg_->ReadForWrite(now_page_id);
    Page_Type now_page_type = GeneralPage::GetPageType(now_addr);
    if (now_page_type == Page_Type::FIXED_PAGE)
    {
        interior_page page{now_addr, pg_};
        int child_pos = Search::upper_bound(0, page.size(), [&](int id) {
            return comparer_(page.get_key(id), key) >= 0;
        });
        child_pos = std::min(page.size() - 1, child_pos);
        return upper_bound(page.children(child_pos), key);
    }
    // else: INDEX_LEAF_PAGE, VARIANT_PAGE,
    leaf_page page{now_addr, pg_};
    int pos = Search::upper_bound(0, page.size(), [&](int id) {
        return comparer_(page.get_key(id), key) >= 0;
    });
    if (pos == page.size()) { return {0, 0}; }
    return {now_page_id, pos};
}


/************************* IntBTree **************************/
class IntBTree : public BTree<int, int(*)(const int &, const int &), int(*)(int)>
{
    static int copy_int(int x) { return x; }
public:
    IntBTree(std::shared_ptr<Pager> pg, int root_page_id = 0)
        : BTree(pg, root_page_id, sizeof(int), integer_comparer, copy_int)  
    {
    }
    ~IntBTree() = default;
};


/************************* IndexBTree **************************/
template <typename T>
struct ArrayDeleter
{
    void operator()(const T *p) { delete[] p; }
};

struct IndexBTreeCopier
{
    int size;
    std::shared_ptr<char> buf;
public: 
    IndexBTreeCopier(int size)
        : size(size), buf(new char[size], ArrayDeleter<char>()) {}
    ~IndexBTreeCopier() = default;
    char *operator()(const char *src)
    {
        std::memcpy(buf.get(), src, size);
        return buf.get();
    }
};

class IndexBTree : public BTree<const char *, 
                                std::function<int(const char *, const char *)>, 
                                IndexBTreeCopier>
{
public:
    using Comparer = std::function<int(const char *, const char *)>;
    using BaseClass = BTree<const char *, Comparer, IndexBTreeCopier>;
    IndexBTree(std::shared_ptr<Pager> pg, int root_page_id, int size, Comparer comparer)
        : BTree(pg, root_page_id, size, comparer, IndexBTreeCopier(size)) {}
    ~IndexBTree() = default;
    void Insert(const char *key, int row_id)
    {
        BTree::Insert(key, key, row_id);
    }
    bool Erase(const char *key)
    {
        return BTree::Erase(key);
    }
};

#endif