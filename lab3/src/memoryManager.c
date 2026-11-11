#include "../include/memoryManager.h"

void initMemoryManager(memoryManager *_memoryManager, unsigned int _pageSize, unsigned int _freeMemory)
{
    if (_memoryManager == NULL || _pageSize == 0)
    {
        return;
    }

    _memoryManager->_pageTable = NULL;
    _memoryManager->_pageSize = _pageSize;
    _memoryManager->_freeMemory = _freeMemory;
}

void freeMemoryManager(memoryManager *_memoryManager)
{
    if (_memoryManager == NULL)
    {
        return;
    }

    freePageTable(_memoryManager->_pageTable);

    _memoryManager->_pageTable = NULL;
    _memoryManager->_requestedSize = 0;
    _memoryManager->_pageSize = 0;
}

int allocateMemory(memoryManager *_memoryManager, unsigned int _allocatingMemory)
{
    if (_memoryManager == NULL || _memoryManager->_pageTable != NULL || _memoryManager->_pageSize == 0 || _allocatingMemory == 0)
    {
        return ERROR;
    }

    unsigned int _amountPage = _allocatingMemory / _memoryManager->_pageSize;
    if (_allocatingMemory % _memoryManager->_pageSize != 0)
    {
        _amountPage++;
    }

    if (_amountPage > _memoryManager->_freeMemory / _memoryManager->_pageSize)
    {
        return ERRORALLOC;
    }

    unsigned int _allocatedSize = _amountPage * _memoryManager->_pageSize;

    pageTable *_pageTable = initPageTable(_memoryManager->_pageSize, _amountPage);

    if (_pageTable == NULL)
    {
        return ERRORALLOC;
    }

    _memoryManager->_pageTable = _pageTable;
    _memoryManager->_requestedSize = _allocatingMemory;
    _memoryManager->_freeMemory -= _allocatedSize;

    return SUCCESS;
}

int freeMemoryBlock(memoryManager *_memoryManager)
{
    if (_memoryManager == NULL || _memoryManager->_pageTable == NULL)
    {
        return ERROR;
    }

    unsigned int _allocatedSize = _memoryManager->_requestedSize / _memoryManager->_pageSize;
    if (_memoryManager->_requestedSize % _memoryManager->_pageSize != 0)
    {
        _allocatedSize++;
    }

    _allocatedSize *= _memoryManager->_pageSize;

    freePageTable(_memoryManager->_pageTable);

    _memoryManager->_pageTable = NULL;
    _memoryManager->_requestedSize = 0;
    _memoryManager->_freeMemory += _allocatedSize;

    return SUCCESS;
}

static int transferMemory(memoryManager *_memoryManager, unsigned int _address, const void *_source, void *_destination, unsigned int _size)
{
    if (_memoryManager == NULL ||
        _memoryManager->_pageTable == NULL ||
        _memoryManager->_pageSize == 0)
    {
        return ERROR;
    }

    if (_address > _memoryManager->_requestedSize ||
        _size > _memoryManager->_requestedSize - _address)
    {
        return ERROR;
    }

    if (_size == 0)
    {
        return SUCCESS;
    }

    if ((_source == NULL) == (_destination == NULL))
    {
        return ERROR;
    }

    unsigned int _transferred = 0;

    while (_transferred < _size)
    {
        unsigned int _currentAddress = _address + _transferred;
        unsigned int _pageIndex = _currentAddress / _memoryManager->_pageSize;
        unsigned int _pageOffset = _currentAddress % _memoryManager->_pageSize;
        unsigned int _available = _memoryManager->_pageSize - _pageOffset;
        unsigned int _remaining = _size - _transferred;
        unsigned int _copySize = _remaining < _available ? _remaining : _available;

        struct frame *_frame = getFrameFromPageTable(_memoryManager->_pageTable, _pageIndex);

        if (_frame == NULL)
        {
            return ERROR;
        }

        void *_frameInfo = getInfo(_frame);

        if (_frameInfo == NULL)
        {
            return ERROR;
        }

        if (_source != NULL)
        {
            memcpy((unsigned char *)_frameInfo + _pageOffset, (const unsigned char *)_source + _transferred, _copySize);
        }
        else
        {
            memcpy((unsigned char *)_destination + _transferred, (unsigned char *)_frameInfo + _pageOffset, _copySize);
        }

        _transferred += _copySize;
    }

    return SUCCESS;
}

int write(memoryManager *_memoryManager, unsigned int _address, const void *_data, unsigned int _size)
{
    return transferMemory(_memoryManager, _address, _data, NULL, _size);
}

int read(memoryManager *_memoryManager, unsigned int _address, void *_data, unsigned int _size)
{
    return transferMemory(_memoryManager, _address, NULL, _data, _size);
}