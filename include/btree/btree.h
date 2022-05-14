#ifndef BTREE_H_
#define BTREE_H_

#include <cstring>
#include <memory>
#include <functional>
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
    virtual ~BTree();
    void Insert(key_t key, const char *data, int data_size);
    bool Erase(key_t key);
    int root_page_id();

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
    search_result upper_bound(int now_page_id, key_t key);
};

template <typename KeyType, typename Comparer, typename Copier> 
inline int BTree<KeyType, Comparer, Copier>::root_page_id() { return root_page_id_; }

/************************* IntBTree **************************/
class IntBTree : public BTree<int, int(*)(const int &, const int &), int(*)(int)>
{
    static int copy_int(int x);
public:
    IntBTree(std::shared_ptr<Pager> pg, int root_page_id = 0);
    ~IntBTree() = default;
};

inline int IntBTree::copy_int(int x) { return x; }

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
    using comparer = std::function<int(const char *, const char *)>;
    using base_class = BTree<const char *, comparer, IndexBTreeCopier>;
    IndexBTree(std::shared_ptr<Pager> pg, int root_page_id, int size, comparer compare);
    ~IndexBTree() = default;
    void Insert(const char *key, int row_id);
    bool Erase(const char *key);
};

#endif