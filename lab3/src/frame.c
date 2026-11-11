#include "../include/frame.h"

struct frame
{
    void *_info;
};

frame *initFrame(unsigned int _frameSize)
{
    frame *_frame = malloc(sizeof(frame));
    if (_frame == NULL)
    {
        return NULL;
    }

    _frame->_info = calloc(_frameSize, sizeof(unsigned char));
    if (_frame->_info == NULL)
    {
        free(_frame);
        return NULL;
    }

    return _frame;
}

void freeFrame(frame *_frame)
{
    if (_frame != NULL)
    {
        free(_frame->_info);
        free(_frame);

        _frame = NULL;
    }
}

void *getInfo(frame *_frame)
{
    return _frame->_info;
}