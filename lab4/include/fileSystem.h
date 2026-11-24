#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#define MAXFILE 10
#include "file.h"

typedef struct
{
    file _file[MAXFILE];
} fileSystem;

void initFileSystem(fileSystem *);
void init(fileSystem *);
void append(fileSystem *);
void removeFile(fileSystem *);
void changeFileName(fileSystem *);
void view(fileSystem *);
void deallocateFileSystem(fileSystem *);

#endif
