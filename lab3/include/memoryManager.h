#ifndef MEMORYMANAGER_H

#define MEMORYMANAGER_H
#include "frame.h"
#include "page.h"
#include "pageTable.h"
#include <string.h>

#define DEFAULTFREEMEMORY 1 << 18

#define SUCCESS 0
#define ERROR -1
#define ERRORALLOC -2

typedef struct memoryManager memoryManager;
struct memoryManager
{
    pageTable *_pageTable;
    unsigned int _pageSize;
    unsigned int _requestedSize;
    unsigned int _freeMemory;
};

void initMemoryManager(memoryManager *, unsigned int, unsigned int);
void freeMemoryManager(memoryManager *);
int allocateMemory(memoryManager *, unsigned int);
int freeMemoryBlock(memoryManager *);
int read(memoryManager *, unsigned int, void *, unsigned int);
int write(memoryManager *, unsigned int, const void *, unsigned int);

#endif