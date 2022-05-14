#include "../../include/btree/btree.h"
#include "../../include/btree/search.h"
#include "../../include/btree/comparer.h"

// BTree()
template <typename KeyType, typename Comparer, typename Copier>
BTree<KeyType, Comparer, Copier>::BTree(std::shared_ptr<Pager> pg, 
    int root_page_id, int field_size, Comparer compare, Copier copier)
    : pg_(pg), root_page_id_(root_page_id), field_size_(field_size)
    , comparer_(compare), copier_(copier)
{
    if (root_page_id == 0)
    {
        root_page_id = pg_->NewPage();
        leaf_page{pg_->ReadForWrite(root_page_id), pg_}.Init(field_size_);
    }
}

// ~BTree()
template <typename KeyType, typename Comparer, typename Copier>
BTree<KeyType, Comparer, Copier>::~BTree()
{
}

// Insert()
template <typename KeyType, typename Comparer, typename Copier>
void BTree<KeyType, Comparer, Copier>::Insert(key_t key, const char *data, int data_size)
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

// Erase
template <typename KeyType, typename Comparer, typename Copier>
bool BTree<KeyType, Comparer, Copier>::Erase(key_t key)
{
    // TODO
    return true;
}

// insert_leaf()
template <typename KeyType, typename Comparer, typename Copier>
typename BTree<KeyType, Comparer, Copier>::insert_ret 
BTree<KeyType, Comparer, Copier>::insert_leaf(int now_page_id, 
    char *now_addr, key_t key, const char *data, int data_size)
{
    leaf_page page{now_addr, pg_};
    // Find the first pos of key int page greater or equal to key
    int child_pos = Search::upper_bound(0, page.size(), [&](int id) {
        return comparer_(page.keys(id), key) >= 0;
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
        if (child_pos < lower_page.size())
        {
            lower_page.Insert(child_pos, data, data_size);
        }
        else
        {
            upper_page.Insert(child_pos - lower_page.size(), data, data_size);
        }
        ret.split = true;
        ret.lower_half = lower_page.buf_;
        ret.upper_half = upper_page.buf_;
        ret.upper_page_id = upper.first;
    }
    return ret;
}

// insert_interior()
template <typename KeyType, typename Comparer, typename Copier>
typename BTree<KeyType, Comparer, Copier>::insert_ret 
BTree<KeyType, Comparer, Copier>::insert_interior(int now_page_id, 
    char *now_addr, key_t key, const char *data, int data_size)
{
    interior_page page{now_addr, pg_};
    int child_pos = Search::upper_bound(0, page.size(), [&](int id) {
        return comparer_(page.keys(id), key) >= 0;
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

// insert_split_root()
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
        page.Insert(0, lower.keys(lower.size() - 1), root_page_id_);
        page.Insert(1, upper.keys(upper.size() - 1), ret.upper_page_id);
        root_page_id_ = new_page_id;
    }
}

// insert_post_process()
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
        page.keys(child_pos) = lower_child.keys(lower_child.size() - 1);
        // Insert the new page to page
        key_t child_largest = copier_(upper_child.keys(upper_child.size() - 1));
        bool succ = page.Insert(child_pos + 1, child_largest, child_ret.upper_page_id);
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
        page.keys(child_pos) = child_page.keys(child_page.size() - 1);
    }
    return ret;
}

template <typename KeyType, typename Comparer, typename Copier>

typename BTree<KeyType, Comparer, Copier>::search_result 
BTree<KeyType, Comparer, Copier>::upper_bound(int now_page_id, key_t key)
{
    char *now_addr = pg_->ReadForWrite(now_page_id);
    Page_Type now_page_type = GeneralPage::GetPageType(now_addr);
    if (now_page_type == Page_Type::FIXED_PAGE)
    {
        interior_page page{now_addr, pg_};
        int child_pos = Search::upper_bound(0, page.size(), [&](int id) {
            return comparer_(page.keys(id), key) >= 0;
        });
        child_pos = std::min(page.size() - 1, child_pos);
        return upper_bound(page.children(child_pos), key);
    }
    // else: INDEX_LEAF_PAGE, VARIANT_PAGE,
    leaf_page page{now_addr, pg_};
    int pos = Search::upper_bound(0, page.size(), [&](int id) {
        return comparer_(page.keys(id), key) >= 0;
    });
    if (pos == page.size()) { return {0, 0}; }
    return {now_page_id, pos};
}

/*
 * IntBTree
 */
IntBTree::IntBTree(std::shared_ptr<Pager> pg, int root_page_id) 
    : BTree(pg, root_page_id, sizeof(int), integer_comparer, copy_int)
{
}

/*
 * IndexBTree
 */
IndexBTree::IndexBTree(std::shared_ptr<Pager> pg, int root_page_id, int size, comparer compare)
    : BTree(pg, root_page_id, size, compare, IndexBTreeCopier(size))
{
}

void IndexBTree::Insert(const char *key, int row_id)
{
    // base_class::Insert(key, reinterpret_cast<const char *>(&row_id), sizeof(row_id));
}

bool IndexBTree::Erase(const char *key)
{
    // return base_class::Erase(key);
    return true;
}