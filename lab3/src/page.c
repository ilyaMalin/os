#include "../include/page.h"

struct page
{
    struct frame *_frame;
};

page *initPage(unsigned int _pageSize)
{
    page *_page = malloc(sizeof(page));
    if (_page == NULL)
    {
        return NULL;
    }

    _page->_frame = initFrame(_pageSize);
    if (_page->_frame == NULL)
    {
        free(_page);
        return NULL;
    }

    return _page;
}

void freePage(page *_page)
{
    if (_page != NULL)
    {
        freeFrame(_page->_frame);
        free(_page);

        _page = NULL;
    }
}

struct frame *getFrame(page *_page)
{
    return _page->_frame;
}
