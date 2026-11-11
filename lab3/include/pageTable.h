#ifndef PAGETABLE_H

#define PAGETABLE_H
#include <stdlib.h>

struct page *initPage(unsigned int);
void freePage(struct page *);
struct frame *getFrame(struct page *);

typedef struct pageTable pageTable;

pageTable *initPageTable(unsigned int, unsigned int);
void freePageTable(pageTable *);
struct frame *getFrameFromPageTable(pageTable *, unsigned int);

#endif