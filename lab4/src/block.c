#include "block.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static size_t getTextLength(const block *head, int *valid)
{
    size_t total = 0;

    if (valid)
    {
        *valid = 0;
    }

    for (const block *current = head; current != NULL; current = current->_next)
    {
        if (current->_text == NULL)
        {
            return 0;
        }

        size_t length = strlen(current->_text);
        if (length > BLOCKSIZE || length > SIZE_MAX - total)
        {
            return 0;
        }

        total += length;
    }

    if (valid)
    {
        *valid = 1;
    }
    return total;
}

block *createBlock(void)
{
    block *newBlock = malloc(sizeof(*newBlock));
    if (newBlock == NULL)
    {
        return NULL;
    }

    newBlock->_text = calloc(BLOCKSIZE + 1, sizeof(*newBlock->_text));
    if (newBlock->_text == NULL)
    {
        free(newBlock);
        return NULL;
    }

    newBlock->_next = NULL;
    return newBlock;
}

int allocate(block **head)
{
    if (head == NULL)
    {
        return 0;
    }

    block *newBlock = createBlock();
    if (newBlock == NULL)
    {
        return 0;
    }

    if (*head == NULL)
    {
        *head = newBlock;
        return 1;
    }

    block *current = *head;
    while (current->_next != NULL)
    {
        current = current->_next;
    }

    current->_next = newBlock;
    return 1;
}

void deallocate(block **head)
{
    if (head == NULL)
    {
        return;
    }

    block *current = *head;
    while (current != NULL)
    {
        block *next = current->_next;
        free(current->_text);
        free(current);
        current = next;
    }

    *head = NULL;
}

int appendText(block *head, const char *text)
{
    if (head == NULL || text == NULL)
    {
        return 0;
    }

    size_t textLength = strlen(text);
    if (textLength == 0)
    {
        return 1;
    }

    block *tail = head;
    while (tail->_next != NULL)
    {
        tail = tail->_next;
    }

    if (tail->_text == NULL)
    {
        return 0;
    }

    size_t tailLength = strlen(tail->_text);
    if (tailLength > BLOCKSIZE)
    {
        return 0;
    }

    size_t availableInTail = BLOCKSIZE - tailLength;
    size_t additionalLength = textLength > availableInTail
                                  ? textLength - availableInTail
                                  : 0;
    size_t blocksNeeded = additionalLength / BLOCKSIZE;
    if (additionalLength % BLOCKSIZE != 0)
    {
        ++blocksNeeded;
    }

    block *newHead = NULL;
    block *newTail = NULL;
    for (size_t i = 0; i < blocksNeeded; ++i)
    {
        block *newBlock = createBlock();
        if (newBlock == NULL)
        {
            deallocate(&newHead);
            return 0;
        }

        if (newHead == NULL)
        {
            newHead = newBlock;
        }
        else
        {
            newTail->_next = newBlock;
        }
        newTail = newBlock;
    }

    tail->_next = newHead;

    const char *source = text;
    size_t remaining = textLength;
    size_t position = tailLength;
    block *current = tail;

    while (remaining > 0)
    {
        if (position == BLOCKSIZE)
        {
            current = current->_next;
            position = 0;
        }

        size_t available = BLOCKSIZE - position;
        size_t chunk = remaining < available ? remaining : available;

        memcpy(current->_text + position, source, chunk);
        position += chunk;
        source += chunk;
        remaining -= chunk;
        current->_text[position] = '\0';
    }

    return 1;
}

char *getText(const block *head, size_t amountByte, size_t offset)
{
    if (head == NULL || amountByte == 0 || amountByte == SIZE_MAX)
    {
        return NULL;
    }

    int valid = 0;
    size_t totalLength = getTextLength(head, &valid);
    if (!valid || offset > totalLength || amountByte > totalLength - offset)
    {
        return NULL;
    }

    char *result = malloc(amountByte + 1);
    if (result == NULL)
    {
        return NULL;
    }

    const block *current = head;
    size_t skip = offset;
    size_t copied = 0;

    while (current != NULL && copied < amountByte)
    {
        size_t currentLength = strlen(current->_text);
        if (skip >= currentLength)
        {
            skip -= currentLength;
            current = current->_next;
            continue;
        }

        size_t available = currentLength - skip;
        size_t chunk = amountByte - copied;
        if (chunk > available)
        {
            chunk = available;
        }

        memcpy(result + copied, current->_text + skip, chunk);
        copied += chunk;
        skip = 0;
        current = current->_next;
    }

    if (copied != amountByte)
    {
        free(result);
        return NULL;
    }

    result[amountByte] = '\0';
    return result;
}

int removeText(block *head, size_t amountByte, size_t offset)
{
    if (head == NULL)
    {
        return 0;
    }

    int valid = 0;
    size_t totalLength = getTextLength(head, &valid);
    if (!valid || offset > totalLength || amountByte > totalLength - offset)
    {
        return 0;
    }

    if (amountByte == 0)
    {
        return 1;
    }

    if (totalLength == SIZE_MAX)
    {
        return 0;
    }

    char *text = malloc(totalLength + 1);
    if (text == NULL)
    {
        return 0;
    }

    size_t position = 0;
    for (const block *current = head; current != NULL; current = current->_next)
    {
        size_t length = strlen(current->_text);
        memcpy(text + position, current->_text, length);
        position += length;
    }
    text[totalLength] = '\0';

    size_t newLength = totalLength - amountByte;
    memmove(text + offset, text + offset + amountByte,
            totalLength - offset - amountByte);
    text[newLength] = '\0';

    block *current = head;
    block *lastUsed = NULL;
    position = 0;

    if (newLength == 0)
    {
        head->_text[0] = '\0';
        lastUsed = head;
        current = head->_next;
    }
    else
    {
        while (current != NULL && position < newLength)
        {
            size_t chunk = newLength - position;
            if (chunk > BLOCKSIZE)
            {
                chunk = BLOCKSIZE;
            }

            memcpy(current->_text, text + position, chunk);
            current->_text[chunk] = '\0';
            position += chunk;
            lastUsed = current;
            current = current->_next;
        }
    }

    if (lastUsed != NULL)
    {
        lastUsed->_next = NULL;
    }

    while (current != NULL)
    {
        block *next = current->_next;
        free(current->_text);
        free(current);
        current = next;
    }

    free(text);
    return 1;
}
