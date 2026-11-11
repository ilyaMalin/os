#include <stdlib.h>

typedef struct frame frame;

frame* initFrame(unsigned int);
void* getInfo(frame*);
void freeFrame(frame *);
 