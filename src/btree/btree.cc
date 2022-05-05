#include "../../include/btree/btree.h"
#include "../../include/btree/search.h"

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

template <typename KeyType, typename Comparer, typename Copier>
BTree<KeyType, Comparer, Copier>::~BTree()
{
}

template <typename KeyType, typename Comparer, typename Copier>
void BTree<KeyType, Comparer, Copier>::Insert(key_t key, const char *data, int data_size)
{
    char *addr = pg_->ReadForWrite(root_page_id_);
    Page_Type page_type = GeneralPage::GetPageType(addr);
    if (page_type == Page_Type::FIXED_PAGE)
    {

    }
    // INDEX_LEAF_PAGE, VARIANT_PAGE,
    else
    {
        
    }
}

template <typename KeyType, typename Comparer, typename Copier>
typename BTree<KeyType, Comparer, Copier>::insert_ret 
BTree<KeyType, Comparer, Copier>::insert_leaf(int now, 
    char *addr, key_t key, const char *data, int data_size)
{
    leaf_page page{addr, pg_};
    // Find the first pos of key int page greater or equal to key
    int pos = Search::upper_bound(0, page.size(), [&](int id) {
        return comparer_(page.keys(id), key) >= 0;
    });
    insert_ret ret;
    ret.split = false;

    bool succ = page.Insert(pos, data, data_size);
}