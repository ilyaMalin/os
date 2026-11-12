#define _POSIX_C_SOURCE 199309L
#include "../include/memoryManager.h"
#include <stdio.h>
#include <stdlib.h>
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

void testLoadAllocationScaling(void)
{
    printf("Running testLoadAllocationScaling...\n");

    FILE *initCsv = fopen(RESULTS_FILE, "w");
    if (!initCsv)
    {
        perror("Failed to create " RESULTS_FILE);
        return;
    }
    fprintf(initCsv, "test_type,param1,param2,alloc_us,free_us,rw_us,waste_metric\n");
    fclose(initCsv);

    FILE *csv = openCsvAppend();
    if (!csv)
    {
        return;
    }

    unsigned int pageSize = 64;
    unsigned int totalMemory = 64 * 1024 * 1024;
    memoryManager mm;

    for (unsigned int numPages = 100; numPages <= 25000; numPages += 250)
    {
        unsigned int reqSize = numPages * pageSize;
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
            fprintf(csv, "scaling,%u,%u,%.3f,%.3f,0.0,0.0\n",
                    numPages, pageSize, t2 - t1, t4 - t3);
        }
    }

    fclose(csv);
    printf("testLoadAllocationScaling PASSED!\n\n");
}