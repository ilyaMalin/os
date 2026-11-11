#include "../include/frame.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void testFrame(void)
{
    printf("Running testFrame...\n");

    frame *f = initFrame(16);
    assert(f != NULL);

    void *info = getInfo(f);
    assert(info != NULL);

    unsigned char *bytes = (unsigned char *)info;
    for (int i = 0; i < 16; i++)
    {
        assert(bytes[i] == 0);
    }

    const char *sample = "Hello, Frame!";
    memcpy(info, sample, strlen(sample) + 1);
    assert(strcmp((char *)getInfo(f), "Hello, Frame!") == 0);

    freeFrame(f);
    freeFrame(NULL);

    printf("testFrame PASSED!\n\n");
}
