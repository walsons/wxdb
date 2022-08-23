#include "../../include/page/variant_page.h"
#include <cstring>
#include <algorithm>
#include "../../include/page/overflow_page.h"

// The minimum size of free block
constexpr int FREE_BLOCK_MIN_SIZE = 16;
// The maximum size of block, it's a overflow block if excess
constexpr int BLOCK_MAX_SIZE = (PAGE_SIZE - VariantPage::header_size() - 4 * 2) / 4;
// The size of a overflow block in this page
constexpr int OVERFLOW_BLOCK_SIZE = 64;
/*
 * used_size1: used size of all slots and blocks, exclude free blocks in page1,
 * used_size2: used size of all slots and blocks, exclude free blocks in page2,
 * When page1 is underflow, if used_size1 + used_size2 <= PAGE_SIZE - header_size, we use Merge rather than MoveFrom,
 * if used_size1 + used_size2 <= PAGE_SIZE - header_size, we use MoveFrom, in this case, we should ensure page2 will 
 * not underflow after MoveFrom, the maximun move size is (BLOCK_MAX_SIZE + 2), so:
 * x + (BLOCK_MAX_SIZE + 2) + x = PAGE_SIZE - header_size (x: used size of all slots and blocks, exclude free blocks)
 * -----> x = (PAGE_SIZE - header_size - (BLOCK_MAX_SIZE + 2)) / 2 
 * When a page which used size <= x, meaning underflow 
 * The explaining of this formula why x can be the underflow condition:
 * when used_size1 > x, it's not underflow, we don't need to do anything, it's fine,
 * but if used_size1 <= x, it's underflow, we need to choose Merge or MoveFrom another 
 * page(page2), if used_size2 <= x + (BLOCK_MAX_SIZE + 2), we can use Merge, otherwise,
 * we use MoveFrom, after move, used_size2 > x, and it's still not underflow
 *
 * When transform to use free_size to express the conditon:
 * y = PAGE_SIZE - header_size - x (y: free_size)
 * -----> y = PAGE_SIZE - header_size - (PAGE_SIZE - header_size - (BLOCK_MAX_SIZE + 2)) / 2
 * When a page which free_size > FREE_SIZE_IN_UNDERFLOW, meaning underflow
 * ----------------------------------------------------------------------------------------------------
 //  constexpr int FREE_SIZE_IN_UNDERFLOW = PAGE_SIZE - VariantPage::header_size() - (PAGE_SIZE - VariantPage::header_size() - (BLOCK_MAX_SIZE + 2)) / 2;
 */

// We can ensure after move or merge the page has at least two items, but we don't ensure it's not underflow, 
// however, it doesn't matter(the ultimate aim is to make full use of btree space)
constexpr int FREE_SIZE_IN_UNDERFLOW = PAGE_SIZE - VariantPage::header_size() - (BLOCK_MAX_SIZE + 2);


char *VariantPage::allocate(int sz)
{
    if (free_size() < sz + 2) { return nullptr; } 

    if (free_block())
    {
        auto *header = &free_block_header(free_block());
        decltype(header) prev_header = nullptr;
        // Search a free block which has proper size
        bool has_proper_free_block = true;
        while (header->size < sz)
        {
            if (header->next == 0) 
            { 
                has_proper_free_block = false;
                break; 
            }
            prev_header = header;
            header = &free_block_header(header->next);
        }
        if (has_proper_free_block)
        {
            if (header->size - sz < FREE_BLOCK_MIN_SIZE)
            {
                if (prev_header == nullptr) { free_block() = header->next; }
                else { prev_header->next = header->next; }
            }
            // Using rest of space to create a new free block
            else
            {
                if (prev_header == nullptr)
                {
                    auto new_free_blk_header = &free_block_header(free_block() + sz);
                    new_free_blk_header->size = header->size - sz; 
                    new_free_blk_header->next = header->next;
                    free_block() += sz;
                }
                else
                {
                    auto new_free_blk_header = &free_block_header(prev_header->next + sz);
                    new_free_blk_header->size = header->size - sz; 
                    new_free_blk_header->next = header->next;
                    prev_header->next += sz;
                }
            }
            free_size() -= sz;
            return reinterpret_cast<char *>(header);
        }
    }  // No free blocks or no free blocks with proper size

    int unallocated_space = PAGE_SIZE - (header_size() + size() * 2 + bottom_used());
    if (unallocated_space > 2 + sz)
    {
        char *offset = buf_ + PAGE_SIZE - bottom_used() - sz;
        free_size() -= sz;
        bottom_used() += sz;
        return offset;
    }  // unallocated space is not enough

    defragment();
    return allocate(sz);
}

void VariantPage::defragment()
{
    int sz = size();
    char *ptr = buf_ + PAGE_SIZE;
    for (int i = sz - 1; i >= 0; ++i)
    {
        int offset = slots(i);
        auto header = block_header(offset);
        ptr -= header.size;
        slots(i) = ptr - buf_;
        std::memmove(ptr, buf_ + offset, header.size);
    }
    free_block() = 0;
    bottom_used() = PAGE_SIZE - (ptr - buf_);
}

void VariantPage::set_free_block(int offset)
{
    BlockHeader *header = &block_header(offset);
    FreeBlockHeader *free_header = &free_block_header(offset);
    free_header->size = header->size;
    // free blocks can stored without order in page
    free_header->next = free_block();
    free_block() = offset;
}

void VariantPage::Init(int)
{
    page_type() = Page_Type::VARIANT_PAGE;
    free_block() = 0;
    free_size() = PAGE_SIZE - header_size();
    size() = 0;
    bottom_used() = 0;
    next_page() = prev_page() = 0;
}

bool VariantPage::Underflow()
{
    return free_size() > FREE_SIZE_IN_UNDERFLOW;
}

bool VariantPage::UnderflowIfRemove()
{
    // int free_size_if_remove = free_size() + GetBlock(pos).first.size + 2;
    // return free_size_if_remove > FREE_SIZE_IN_UNDERFLOW;
    return free_size() > FREE_BLOCK_MIN_SIZE;
}

bool VariantPage::Insert(int pos, const char *data, int data_size)
{
    int real_size = data_size + sizeof(BlockHeader);
    bool is_overflow = (real_size > BLOCK_MAX_SIZE);
    int required_size = is_overflow ? OVERFLOW_BLOCK_SIZE : real_size;
    char *addr = allocate(required_size);
    if (addr == nullptr) { return false; }
    // Set slot
    for (int i = size(); i > pos; --i)
    {
        slots(i) = slots(i - 1);
    }
    slots(pos) = addr - buf_;
    // Set data
    BlockHeader *header = &block_header(slots(pos));
    header->size = required_size;
    header->overflow_page = 0;
    int copy_data_size = required_size - sizeof(BlockHeader);
    std::memcpy(addr + sizeof(BlockHeader), data, copy_data_size);
    free_size() -= 2;  // allocate funcion subtract data size internally
    ++size();
    if (is_overflow)
    {
        auto store_data_to_overflow_page = [&](const char *src, int sz) {
            int page_id = pg_->NewPage();
            OverflowPage page{pg_->ReadForWrite(page_id), pg_};
            page.Init();
            page.size() = sz;
            std::memcpy(page.block(), src, sz);
            return std::make_pair(page_id, page);
        };
        data += copy_data_size;
        int remain = data_size - copy_data_size;
        OverflowPage *overflow_page = nullptr;
        while (remain > 0)
        {
            int each_step_size = (remain < OverflowPage::block_size() ? remain : OverflowPage::block_size());
            auto res = store_data_to_overflow_page(data, each_step_size);
            data += each_step_size;
            remain -= each_step_size;
            if (overflow_page == nullptr) 
            { 
                header->overflow_page = res.first; 
                overflow_page = &res.second;
            }
            else
            {
                overflow_page->next() = res.first;
                overflow_page = &res.second;
            }
        }
    }
    return true;
}

void VariantPage::Erase(int pos, bool erase_overflow_page)
{
    uint16_t offset = slots(pos);
    for (int i = pos; i < size() - 1; ++i)
    {
        slots(i) = slots(i + 1);
    }
    BlockHeader *header = &block_header(offset);
    if (erase_overflow_page && header->overflow_page) 
        pg_->FreeOverflowPage(header->overflow_page);
    free_size() += (2 + header->size);
    --size();
    set_free_block(offset);
}

void VariantPage::MoveFrom(VariantPage src_page, int src_pos, int dest_pos)
{
    auto src_block = src_page.GetBlock(src_pos);
    // Only need to move data in block(no need for overflow page data)
    Insert(dest_pos, src_block.second, src_block.first.size - sizeof(BlockHeader));
    src_page.Erase(src_pos, false);
}

// Split to a new page as upper page(the page store previous data) 
std::pair<int, VariantPage> VariantPage::Split(int current_id)
{
    int page_id = pg_->NewPage();
    if (page_id == 0) { return { 0, VariantPage{nullptr, nullptr} }; }
    VariantPage upper_page{pg_->ReadForWrite(page_id), pg_};
    upper_page.Init();
    if (prev_page())
    {
        VariantPage page{pg_->ReadForWrite(prev_page()), pg_};
        page.next_page() = page_id;
    }
    upper_page.prev_page() = prev_page();
    upper_page.next_page() = current_id;
    prev_page() = page_id;

    // Copy half data or remain 2 items then stop,
    // We should ensure items >= 4 and no free blocks when split
    char *dest_addr = upper_page.buf_ + PAGE_SIZE;
    int i = 0;
    int lower_page_size = size();
    for (; i < lower_page_size - 2; ++i)
    {
        if (UnderflowIfRemove()) { break; }
        char *src_addr = buf_ + slots(i);
        BlockHeader *src_header = reinterpret_cast<BlockHeader *>(src_addr);
        dest_addr -= src_header->size;
        std::memcpy(dest_addr, src_addr, src_header->size);

        upper_page.slots(i) = dest_addr - upper_page.buf_;
        --size();
        ++upper_page.size();
        int required_size = src_header->size + 2;
        free_size() += required_size;
        upper_page.free_size() -= required_size;
        set_free_block(slots(i));
    }
    // Move lower page slots
    for (int j = 0; j < i; ++j)
    {
        slots(j) = slots(i + j);
    }
    upper_page.bottom_used() = upper_page.buf_ + PAGE_SIZE - dest_addr;
    return {page_id, upper_page };
}

// Merge lower(next) page
bool VariantPage::Merge(VariantPage page, int current_id)
{
    int required_size = PAGE_SIZE - page.free_size() - header_size();
    if (free_size() < required_size) { return false; }
    if (page.next_page())
    {
        VariantPage lower_page{pg_->ReadForWrite(page.next_page()), pg_};
        lower_page.prev_page() = current_id;
        next_page() = page.next_page();
    }
    defragment();
    char *dest = buf_ + PAGE_SIZE - bottom_used();
    for (int i = 0; i < page.size(); ++i)
    {
        auto *header = &block_header(page.slots(i));
        dest -= header->size;
        std::memcpy(dest, header, header->size);
        bottom_used() += header->size;
        slots(i + size()) = PAGE_SIZE - bottom_used();
    }
    size() += page.size();
    free_size() -= required_size;
    return true;
}

std::pair<VariantPage::BlockHeader, char *> VariantPage::GetBlock(int index)
{
    char *addr = buf_ + slots(index);
    return { block_header(slots(index)), addr + sizeof(BlockHeader) };
}