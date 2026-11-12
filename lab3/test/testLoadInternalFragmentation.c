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