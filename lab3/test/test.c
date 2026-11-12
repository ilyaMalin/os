#include "test.h"

#include <stdio.h>

void testFrame(void);
void testPage(void);
void testPageTable(void);
void testMemoryManager(void);

void testLoadAllocationScaling(void);
void testLoadPageSizeImpact(void);
void testLoadInternalFragmentation(void);

int runTests(void)
{
    printf("\n========== UNIT TESTS ==========\n\n");

    testFrame();
    testPage();
    testPageTable();
    testMemoryManager();

    printf("\n========== LOAD TESTS ==========\n\n");

    testLoadAllocationScaling();
    testLoadPageSizeImpact();
    testLoadInternalFragmentation();

    printf("\n======== TESTS FINISHED ========\n");

    return 0;
}