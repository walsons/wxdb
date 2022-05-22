#ifndef FID_MANAGER_H_
#define FID_MANAGER_H_

constexpr unsigned MAX_NUM_FILE_ID = 1024;

// file id manager
class FidManager
{
    int free_fid_num_;
    int free_fid_[MAX_NUM_FILE_ID];
    bool fid_used_[MAX_NUM_FILE_ID + 1];

public:
    FidManager();
    ~FidManager() = default;
    int Allocate();
    void Deallocate(const int &fid);
    bool IsUsed(int fid);
};

inline
bool FidManager::IsUsed(int fid)
{
    return fid_used_[fid];
}

#endif