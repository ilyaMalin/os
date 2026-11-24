#include "fileSystem.h"

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int readLine(const char *prompt, char *buffer, size_t bufferSize)
{
    if (buffer == NULL || bufferSize < 2)
    {
        return 0;
    }

    fputs(prompt, stdout);
    if (fgets(buffer, (int)bufferSize, stdin) == NULL)
    {
        return 0;
    }

    char *newline = strchr(buffer, '\n');
    if (newline != NULL)
    {
        *newline = '\0';
    }
    else if (!feof(stdin))
    {
        int nextCharacter = getchar();
        if (nextCharacter != '\n' && nextCharacter != EOF)
        {
            while (nextCharacter != '\n' && nextCharacter != EOF)
            {
                nextCharacter = getchar();
            }
            return 0;
        }
    }

    size_t length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\r')
    {
        buffer[length - 1] = '\0';
    }

    return 1;
}

static int parseSizePair(const char *line, size_t *first, size_t *second)
{
    if (line == NULL || first == NULL || second == NULL)
    {
        return 0;
    }

    const char *cursor = line;
    char *end = NULL;
    unsigned long long firstValue;
    unsigned long long secondValue;

    while (isspace((unsigned char)*cursor))
    {
        ++cursor;
    }
    if (*cursor == '-' || *cursor == '+')
    {
        return 0;
    }

    errno = 0;
    firstValue = strtoull(cursor, &end, 10);
    if (end == cursor || errno == ERANGE || !isspace((unsigned char)*end))
    {
        return 0;
    }

    cursor = end;
    while (isspace((unsigned char)*cursor))
    {
        ++cursor;
    }
    if (*cursor == '-' || *cursor == '+')
    {
        return 0;
    }

    errno = 0;
    secondValue = strtoull(cursor, &end, 10);
    if (end == cursor || errno == ERANGE)
    {
        return 0;
    }

    cursor = end;
    while (isspace((unsigned char)*cursor))
    {
        ++cursor;
    }
    if (*cursor != '\0' || firstValue > SIZE_MAX || secondValue > SIZE_MAX)
    {
        return 0;
    }

    *first = (size_t)firstValue;
    *second = (size_t)secondValue;
    return 1;
}

static int findFileIndex(const fileSystem *targetFileSystem, const char *fileName)
{
    if (targetFileSystem == NULL || fileName == NULL)
    {
        return -1;
    }

    for (int i = 0; i < MAXFILE; ++i)
    {
        const file *current = &targetFileSystem->_file[i];
        if (current->_file != NULL && current->_name != NULL &&
            strcmp(current->_name, fileName) == 0)
        {
            return i;
        }
    }

    return -1;
}

void initFileSystem(fileSystem *targetFileSystem)
{
    if (targetFileSystem == NULL)
    {
        return;
    }

    for (int i = 0; i < MAXFILE; ++i)
    {
        targetFileSystem->_file[i]._name = NULL;
        targetFileSystem->_file[i]._file = NULL;
    }
}

void init(fileSystem *targetFileSystem)
{
    if (targetFileSystem == NULL)
    {
        return;
    }

    char fileName[256];
    if (!readLine("Введите имя файла: ", fileName, sizeof(fileName)) ||
        fileName[0] == '\0')
    {
        puts("Некорректное имя файла.");
        return;
    }

    if (findFileIndex(targetFileSystem, fileName) != -1)
    {
        printf("Файл %s уже существует!\n", fileName);
        return;
    }

    int availableCell = -1;
    for (int i = 0; i < MAXFILE; ++i)
    {
        if (targetFileSystem->_file[i]._file == NULL)
        {
            availableCell = i;
            break;
        }
    }

    if (availableCell == -1)
    {
        puts("Создано максимальное число файлов!");
        return;
    }

    file *newFile = initFile(fileName);
    if (newFile == NULL)
    {
        puts("Не удалось создать файл: недостаточно памяти или некорректное имя.");
        return;
    }

    targetFileSystem->_file[availableCell] = *newFile;
    free(newFile);
}

void append(fileSystem *targetFileSystem)
{
    if (targetFileSystem == NULL)
    {
        return;
    }

    char fileName[256];
    if (!readLine("Введите имя файла: ", fileName, sizeof(fileName)))
    {
        puts("Слишком длинное имя файла.");
        return;
    }

    int fileIndex = findFileIndex(targetFileSystem, fileName);
    if (fileIndex == -1)
    {
        printf("Файла %s не существует!\n", fileName);
        return;
    }

    char text[1024];
    if (!readLine("Введите текст: ", text, sizeof(text)))
    {
        puts("Текст слишком длинный.");
        return;
    }

    if (!setTextToFile(&targetFileSystem->_file[fileIndex], text))
    {
        puts("Не удалось добавить текст: недостаточно памяти.");
    }
}

void removeFile(fileSystem *targetFileSystem)
{
    if (targetFileSystem == NULL)
    {
        return;
    }

    char fileName[256];
    if (!readLine("Выберите файл (имя): ", fileName, sizeof(fileName)))
    {
        puts("Слишком длинное имя файла.");
        return;
    }

    int fileIndex = findFileIndex(targetFileSystem, fileName);
    if (fileIndex == -1)
    {
        printf("Файла %s не существует!\n", fileName);
        return;
    }

    char input[128];
    size_t amountByte;
    size_t offset;
    if (!readLine("Введите количество удаляемых байт и смещение относительно начала файла (через пробел): ",
                  input, sizeof(input)) ||
        !parseSizePair(input, &amountByte, &offset))
    {
        puts("Некорректные значения количества байт или смещения.");
        return;
    }

    if (!removeFileText(&targetFileSystem->_file[fileIndex], amountByte, offset))
    {
        puts("Невозможно удалить указанный диапазон: проверьте смещение и количество байт.");
    }
}

void changeFileName(fileSystem *targetFileSystem)
{
    if (targetFileSystem == NULL)
    {
        return;
    }

    char fileName[256];
    char newFileName[256];

    if (!readLine("Введите имя файла: ", fileName, sizeof(fileName)) ||
        !readLine("Введите новое имя файла: ", newFileName, sizeof(newFileName)) ||
        newFileName[0] == '\0')
    {
        puts("Некорректное имя файла.");
        return;
    }

    int fileIndex = findFileIndex(targetFileSystem, fileName);
    if (fileIndex == -1)
    {
        printf("Файла %s не существует!\n", fileName);
        return;
    }

    int existingIndex = findFileIndex(targetFileSystem, newFileName);
    if (existingIndex != -1 && existingIndex != fileIndex)
    {
        printf("Файл %s уже существует!\n", newFileName);
        return;
    }

    if (!setFileName(&targetFileSystem->_file[fileIndex], newFileName))
    {
        puts("Не удалось изменить имя файла: недостаточно памяти.");
    }
}

void view(fileSystem *targetFileSystem)
{
    if (targetFileSystem == NULL)
    {
        return;
    }

    int found = 0;
    for (int i = 0; i < MAXFILE; ++i)
    {
        if (targetFileSystem->_file[i]._file != NULL &&
            targetFileSystem->_file[i]._name != NULL)
        {
            printf("%d. %s\n", i + 1, targetFileSystem->_file[i]._name);
            found = 1;
        }
    }

    if (!found)
    {
        puts("Файлов нет.");
    }
}

void deallocateFileSystem(fileSystem *targetFileSystem)
{
    if (targetFileSystem == NULL)
    {
        return;
    }

    for (int i = 0; i < MAXFILE; ++i)
    {
        deallocateFile(&targetFileSystem->_file[i]);
    }
}
