#include "../include/pageTable.h"
#include "../include/frame.h"
#include <assert.h>
#include <stdio.h>

void testPageTable(void)
{
    printf("Running testPageTable...\n");

    assert(initPageTable(0, 5) == NULL);
    assert(initPageTable(16, 0) == NULL);

    pageTable *pt = initPageTable(64, 4);
    assert(pt != NULL);

    for (unsigned int i = 0; i < 4; i++)
    {
        struct frame *f = getFrameFromPageTable(pt, i);
        assert(f != NULL);
        assert(getInfo(f) != NULL);
    }

    assert(getFrameFromPageTable(pt, 4) == NULL);
    assert(getFrameFromPageTable(pt, 100) == NULL);
    assert(getFrameFromPageTable(NULL, 0) == NULL);

    freePageTable(pt);
    freePageTable(NULL);

    printf("testPageTable PASSED!\n\n");
}
