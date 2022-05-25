#ifndef CACHE_MANAGER_H_
#define CACHE_MANAGER_H_

constexpr unsigned CACHE_CAPACITY = 100;

class CacheManager
{
private:
    int head;
    struct node
    {
        int prev, next;
    };
    node *nodes;
public:
    CacheManager();
    ~CacheManager();
    void Access(int id);
    int LastNode() const;
};

inline
CacheManager::~CacheManager()
{
    delete[] nodes;
}

inline
int CacheManager::LastNode() const
{
    return nodes[head].prev;
}

#endif