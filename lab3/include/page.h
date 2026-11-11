#ifndef PAGE_H

#define PAGE_H
#include <stdlib.h>

struct frame *initFrame(unsigned int);
void freeFrame(struct frame *);

typedef struct page page;

page *initPage(unsigned int);
void freePage(page *);
struct frame *getFrame(page *);

#endif