#include "../../include/page/pager.h"
#include <memory>
#include "../../include/page/overflow_page.h"

void Pager::FreeOverflowPage(int page_id)
{
    auto overflow_page = OverflowPage(ReadForWrite(page_id), shared_from_this());
    int next_page_id = overflow_page.next();
    FreePage(page_id);
    FreeOverflowPage(next_page_id);
}