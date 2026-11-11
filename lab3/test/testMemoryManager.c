#include "../include/memoryManager.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void testMemoryManager(void)
{
    printf("Running testMemoryManager...\n");

    memoryManager mm;
    initMemoryManager(&mm, 16, 64);
    assert(mm._pageSize == 16);
    assert(mm._freeMemory == 64);
    assert(mm._pageTable == NULL);

    assert(allocateMemory(NULL, 10) == ERROR);
    assert(allocateMemory(&mm, 0) == ERROR);
    assert(allocateMemory(&mm, 65) == ERRORALLOC);

    assert(allocateMemory(&mm, 20) == SUCCESS);
    assert(mm._requestedSize == 20);
    assert(mm._freeMemory == 32);

    assert(allocateMemory(&mm, 10) == ERROR);

    const char *writeBuf = "0123456789";
    char readBuf[11] = {0};

    assert(write(&mm, 10, writeBuf, 10) == SUCCESS);
    assert(read(&mm, 10, readBuf, 10) == SUCCESS);
    assert(strcmp(readBuf, "0123456789") == 0);

    assert(write(&mm, 0, writeBuf, 0) == SUCCESS);
    assert(read(&mm, 0, readBuf, 0) == SUCCESS);

    assert(write(&mm, 15, writeBuf, 6) == ERROR);
    assert(read(&mm, 21, readBuf, 1) == ERROR);
    assert(write(&mm, 0, NULL, 5) == ERROR);
    assert(read(&mm, 0, NULL, 5) == ERROR);

    assert(freeMemoryBlock(&mm) == SUCCESS);
    assert(mm._pageTable == NULL);
    assert(mm._requestedSize == 0);
    assert(mm._freeMemory == 64);

    assert(freeMemoryBlock(&mm) == ERROR);
    assert(read(&mm, 0, readBuf, 1) == ERROR);

    freeMemoryManager(&mm);
    assert(mm._pageSize == 0);

    printf("testMemoryManager PASSED!\n\n");
}
