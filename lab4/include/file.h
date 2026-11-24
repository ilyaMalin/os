#ifndef FILE_H
#define FILE_H

#include "block.h"
#include <stddef.h>

typedef struct
{
    char *_name;
    block *_file;
} file;

file *initFile(const char *);
void deallocateFile(file *);
int setFileName(file *, const char *);
int setTextToFile(file *, const char *);
char *getFileText(const file *, size_t, size_t);
int removeFileText(file *, size_t, size_t);

#endif
