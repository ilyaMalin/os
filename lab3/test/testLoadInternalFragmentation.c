#define _POSIX_C_SOURCE 199309L
#include "../include/memoryManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define RESULTS_FILE "load_results.csv"

// Вспомогательная функция замера времени (решает ошибку implicit declaration of function 'getTimeUs')
static double getTimeUs(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e6 + ts.tv_nsec / 1e3;
}

// Вспомогательная функция работы с CSV (решает ошибку openCsvAppend)
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

void testLoadInternalFragmentation(void)
{
    printf("Running testLoadInternalFragmentation...\n");

    FILE *csv = openCsvAppend();
    if (!csv)
    {
        return;
    }

    unsigned int pageSize = 4096; /* 4 KiB */
    unsigned int totalMemory = 64 * 1024 * 1024; /* 64 MiB */

    /* Проверяем разные размеры запросов: от 1 байта до 16 КБ с шагом 127 байт */
    for (unsigned int reqSize = 1; reqSize <= 16384; reqSize += 127)
    {
        memoryManager mm;
        initMemoryManager(&mm, pageSize, totalMemory);

        double t1 = getTimeUs();
        int res = allocateMemory(&mm, reqSize);
        double t2 = getTimeUs();

        if (res == SUCCESS)
        {
            /* Вычисляем реальный выделенный объем и потери (внутреннюю фрагментацию) */
            unsigned int actualAllocated = totalMemory - mm._freeMemory;
            unsigned int wasted = (actualAllocated > reqSize)
                                  ? (actualAllocated - reqSize) : 0;

            double t3 = getTimeUs();
            freeMemoryBlock(&mm);
            double t4 = getTimeUs();

            fprintf(csv, "fragmentation,%u,%u,%.3f,%.3f,0.0,%u\n",
                    reqSize, pageSize, t2 - t1, t4 - t3, wasted);
        }
        else
        {
            freeMemoryBlock(&mm);
        }

        freeMemoryManager(&mm);
    }

    fclose(csv);
    printf("testLoadInternalFragmentation PASSED! (saved to %s)\n\n", RESULTS_FILE);
}
