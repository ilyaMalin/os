#define _POSIX_C_SOURCE 199309L
#include "../include/memoryManager.h"
#include <stdio.h>
#include <time.h>

#define RESULTS_FILE "load_results.csv"

static double getTimeUs(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e6 + ts.tv_nsec / 1e3;
}

static FILE *openCsvAppend(void)
{
    FILE *check = fopen(RESULTS_FILE, "r");
    int exists = (check != NULL);
    if (check)
    {
        fclose(check);
    }

    FILE *csv = fopen(RESULTS_FILE, "a");
    if (csv && !exists)
    {
        fprintf(csv, "test_type,param1,param2,alloc_us,free_us,rw_us,waste_metric\n");
    }
    return csv;
}

void testLoadPageSizeImpact(void)
{
    printf("Running testLoadPageSizeImpact...\n");

    FILE *csv = openCsvAppend();
    if (!csv)
    {
        return;
    }

    unsigned int totalMemory = 64 * 1024 * 1024;
    unsigned int reqSize = 1024 * 1024; /* 1 MiB */

    for (unsigned int pageSize = 64; pageSize <= 8192; pageSize *= 2)
    {
        memoryManager mm;
        initMemoryManager(&mm, pageSize, totalMemory);

        double t1 = getTimeUs();
        int res = allocateMemory(&mm, reqSize);
        double t2 = getTimeUs();

        double t3 = getTimeUs();
        freeMemoryBlock(&mm);
        double t4 = getTimeUs();

        freeMemoryManager(&mm);

        if (res == SUCCESS)
        {
            unsigned int actualAllocated = totalMemory - mm._freeMemory;
            unsigned int wasted = (actualAllocated > reqSize)
                                  ? (actualAllocated - reqSize) : 0;
            fprintf(csv, "pagesize,%u,%u,%.3f,%.3f,0.0,%u\n",
                    pageSize, reqSize, t2 - t1, t4 - t3, wasted);
        }
    }

    fclose(csv);
    printf("testLoadPageSizeImpact PASSED! (saved to %s)\n\n", RESULTS_FILE);
}