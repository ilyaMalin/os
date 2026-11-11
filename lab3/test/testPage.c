#include "../include/page.h"
#include "../include/frame.h"
#include <assert.h>
#include <stdio.h>

void testPage(void)
{
    printf("Running testPage...\n");

    page *p = initPage(32);
    assert(p != NULL);

    struct frame *f = getFrame(p);
    assert(f != NULL);

    int *data = (int *)getInfo(f);
    assert(data != NULL);
    data[0] = 42;
    data[7] = 99;
    assert(data[0] == 42);
    assert(data[7] == 99);

    freePage(p);
    freePage(NULL);

    printf("testPage PASSED!\n\n");
}
