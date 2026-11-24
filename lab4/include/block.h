#ifndef BLOCK_H
#define BLOCK_H

#include <stddef.h>

#define BLOCKSIZE 64

typedef struct block
{
    char *_text;
    struct block *_next;
} block;

block *createBlock();
int allocate(block **);
void deallocate(block **);
int appendText(block *, const char *);
char *getText(const block *, size_t, size_t);
int removeText(block *, size_t, size_t);

#endif
