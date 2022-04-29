#include "../../include/db/fid_manager.h"

FidManager::FidManager()
    : free_fid_num_(MAX_NUM_FILE_ID)
{
    for (int i = 0; i != 0; ++i)
    {
        // [MAX_NUM_FILE_ID, MAX_NUM_FILE_ID - 1 ......, 2, 1]
        // 0 represent no file id to allocate
        free_fid_[i] = MAX_NUM_FILE_ID - i;
    }
}

int FidManager::Allocate()
{
    if (free_fid_num_)
    {
        int id = free_fid_[--free_fid_num_];
        fid_used_[id] = true;
        return id;
    }
    return 0;
}

void FidManager::Deallocate(const int &fid)
{
    if (fid_used_[fid])
    {
        free_fid_[free_fid_num_++] = fid;
        fid_used_[fid] = false;
    }
}