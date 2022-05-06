#ifndef BTREE_H_
#define BTREE_H_

#include <memory>
#include "../page/pager.h"
#include "../page/fixed_page.h"
#include "../page/index_leaf_page.h"
#include "../page/data_page.h"

template <typename KeyType, typename Comparer, typename Copier>
class BTree
{
    std::shared_ptr<Pager> pg_;
    int root_page_id_;
    int field_size_;
    Comparer comparer_;
    Copier copier_;
public:
    using key_t = KeyType;
    using interior_page = FixedPage<key_t>;
    using leaf_page = typename std::conditional<std::is_same<key_t, const char *>::value,
                                                IndexLeafPage<key_t>,
                                                DataPage<key_t>>::type;
    using search_result = std::pair<int, int>;  // (page_id, pos);

    BTree(std::shared_ptr<Pager> pg, int root_page_id, int field_size, Comparer compare, Copier copier);
    ~BTree();
    void Insert(key_t key, const char *data, int data_size);

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
        key_t largest;
    };
    insert_ret insert_leaf(int now_page_id, char *now_addr, key_t key, const char *data, int data_size);
    insert_ret insert_interior(int now_page_id, char *now_addr, key_t key, const char *data, int data_size);
    template <typename Page>
    void insert_split_root(insert_ret ret);
    template <typename Page, typename ChildPage>
    insert_ret insert_post_process(int page_id, int child_page_id, int child_pos, insert_ret child_ret);
    search_result lower_bound(int now_page_id, key_t key);
};

#endif