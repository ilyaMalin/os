#include "file.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static char *copyString(const char *source)
{
    if (source == NULL)
    {
        return NULL;
    }

    size_t length = strlen(source);
    if (length == SIZE_MAX)
    {
        return NULL;
    }

    char *copy = malloc(length + 1);
    if (copy != NULL)
    {
        memcpy(copy, source, length + 1);
    }
    return copy;
}

file *initFile(const char *fileName)
{
    if (fileName == NULL || fileName[0] == '\0')
    {
        return NULL;
    }

    file *newFile = malloc(sizeof(*newFile));
    if (newFile == NULL)
    {
        return NULL;
    }

    newFile->_name = copyString(fileName);
    newFile->_file = createBlock();

    if (newFile->_name == NULL || newFile->_file == NULL)
    {
        free(newFile->_name);
        deallocate(&newFile->_file);
        free(newFile);
        return NULL;
    }

    return newFile;
}

void deallocateFile(file *targetFile)
{
    if (targetFile == NULL)
    {
        return;
    }

    deallocate(&targetFile->_file);
    free(targetFile->_name);
    targetFile->_name = NULL;
}

int setFileName(file *targetFile, const char *fileName)
{
    if (targetFile == NULL || fileName == NULL || fileName[0] == '\0')
    {
        return 0;
    }

    if (targetFile->_name != NULL && strcmp(targetFile->_name, fileName) == 0)
    {
        return 1;
    }

    char *newName = copyString(fileName);
    if (newName == NULL)
    {
        return 0;
    }

    free(targetFile->_name);
    targetFile->_name = newName;
    return 1;
}

int setTextToFile(file *targetFile, const char *text)
{
    if (targetFile == NULL)
    {
        return 0;
    }
    return appendText(targetFile->_file, text);
}

char *getFileText(const file *targetFile, size_t amountByte, size_t offset)
{
    if (targetFile == NULL)
    {
        return NULL;
    }
    return getText(targetFile->_file, amountByte, offset);
}

int removeFileText(file *targetFile, size_t amountByte, size_t offset)
{
    if (targetFile == NULL)
    {
        return 0;
    }
    return removeText(targetFile->_file, amountByte, offset);
}
