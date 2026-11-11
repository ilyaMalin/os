#include "../include/pageTable.h"

struct pageTable
{
    struct page **_page;
    unsigned int _amountPage;
};

pageTable *initPageTable(unsigned int _pageSize, unsigned int _amountPage)
{
    if (_pageSize == 0 || _amountPage == 0)
    {
        return NULL;
    }

    pageTable *_pageTable = malloc(sizeof(pageTable));
    if (_pageTable == NULL)
    {
        return NULL;
    }

    _pageTable->_page = calloc(_amountPage, sizeof(*_pageTable->_page));
    if (_pageTable->_page == NULL)
    {
        free(_pageTable);
        return NULL;
    }

    _pageTable->_amountPage = 0;

    for (unsigned int i = 0; i < _amountPage; i++)
    {
        _pageTable->_page[i] = initPage(_pageSize);

        if (_pageTable->_page[i] == NULL)
        {
            freePageTable(_pageTable);
            return NULL;
        }

        _pageTable->_amountPage++;
    }

    return _pageTable;
}

void freePageTable(pageTable *_pageTable)
{
    if (_pageTable == NULL)
    {
        return;
    }

    for (unsigned int i = 0; i < _pageTable->_amountPage; i++)
    {
        freePage(_pageTable->_page[i]);
    }

    free(_pageTable->_page);
    free(_pageTable);

    _pageTable = NULL;
}

struct frame *getFrameFromPageTable(pageTable *_pageTable, unsigned int _index)
{
    if (_pageTable == NULL || _index >= _pageTable->_amountPage)
    {
        return NULL;
    }

    return getFrame(_pageTable->_page[_index]);
}