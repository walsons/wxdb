#include "../../include/page/cache_manager.h"

CacheManager::CacheManager()
{
    head = 0;
    nodes = new node[CACHE_CAPACITY];
    int stop = CACHE_CAPACITY - 1;
    for (int i = 0; i < stop; ++i)
    {
        nodes[i].next = i + 1;
        nodes[i + 1].prev = i;
    }
    // link head and tail
    nodes[CACHE_CAPACITY - 1].next = 0;
    nodes[0].prev = CACHE_CAPACITY - 1;
}

void CacheManager::Access(int id)
{
    if (id == head) { return; }
    // remove
    nodes[nodes[id].prev].next = nodes[id].next;
    nodes[nodes[id].next].prev = nodes[id].prev;
    // insert to head
    nodes[id].next = head;
    nodes[id].prev = nodes[head].prev;
    nodes[nodes[head].prev].next = id;
    nodes[head].prev = id;
    head = id;
}