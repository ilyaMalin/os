#include "fileSystem.h"

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>

#define NAME_BUFFER_SIZE 256
#define TEXT_BUFFER_SIZE 4096
#define DUMP_FILE_NAME "filesystem_dump.txt"

static int readLine(const char *prompt, char *buffer, size_t bufferSize)
{
    if (buffer == NULL || bufferSize < 2 || bufferSize > INT_MAX)
    {
        return 0;
    }

    if (prompt != NULL)
    {
        fputs(prompt, stdout);
        fflush(stdout);
    }

    if (fgets(buffer, (int)bufferSize, stdin) == NULL)
    {
        return 0;
    }

    size_t length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n')
    {
        buffer[--length] = '\0';
    }
    else if (!feof(stdin))
    {
        int character;
        while ((character = getchar()) != '\n' && character != EOF)
        {
        }
        puts("Введённая строка слишком длинная.");
        return 0;
    }

    if (length > 0 && buffer[length - 1] == '\r')
    {
        buffer[length - 1] = '\0';
    }

    return 1;
}

static int readInteger(const char *prompt, long *value)
{
    char input[64];
    char *end = NULL;

    if (value == NULL || !readLine(prompt, input, sizeof(input)))
    {
        return 0;
    }

    errno = 0;
    long parsed = strtol(input, &end, 10);
    if (end == input || errno == ERANGE)
    {
        return 0;
    }

    while (*end == ' ' || *end == '\t' || *end == '\r')
    {
        ++end;
    }

    if (*end != '\0')
    {
        return 0;
    }

    *value = parsed;
    return 1;
}

static int isFileUsed(const file *targetFile)
{
    return targetFile != NULL && targetFile->_name != NULL &&
           targetFile->_file != NULL;
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
        if (isFileUsed(current) && strcmp(current->_name, fileName) == 0)
        {
            return i;
        }
    }

    return -1;
}

static int findFreeSlot(const fileSystem *targetFileSystem)
{
    if (targetFileSystem == NULL)
    {
        return -1;
    }

    for (int i = 0; i < MAXFILE; ++i)
    {
        if (targetFileSystem->_file[i]._file == NULL)
        {
            return i;
        }
    }

    return -1;
}

static size_t getFileLength(const file *targetFile)
{
    size_t totalLength = 0;

    if (!isFileUsed(targetFile))
    {
        return 0;
    }

    for (const block *current = targetFile->_file;
         current != NULL; current = current->_next)
    {
        if (current->_text == NULL)
        {
            return 0;
        }

        size_t currentLength = strlen(current->_text);
        if (currentLength > BLOCKSIZE || currentLength > SIZE_MAX - totalLength)
        {
            return 0;
        }

        totalLength += currentLength;
    }

    return totalLength;
}

static size_t getBlockCount(const file *targetFile)
{
    size_t count = 0;

    if (!isFileUsed(targetFile))
    {
        return 0;
    }

    for (const block *current = targetFile->_file; current != NULL; current = current->_next)
    {
        ++count;
    }

    return count;
}

static int readExistingFileName(const fileSystem *targetFileSystem, const char *prompt, int *fileIndex)
{
    char fileName[NAME_BUFFER_SIZE];

    if (fileIndex == NULL ||
        !readLine(prompt, fileName, sizeof(fileName)) || fileName[0] == '\0')
    {
        puts("Некорректное имя файла.");
        return 0;
    }

    *fileIndex = findFileIndex(targetFileSystem, fileName);
    if (*fileIndex == -1)
    {
        printf("Файл '%s' не найден.\n", fileName);
        return 0;
    }

    return 1;
}

static int createFileCommand(fileSystem *targetFileSystem)
{
    char fileName[NAME_BUFFER_SIZE];

    if (!readLine("Имя нового файла: ", fileName, sizeof(fileName)) ||
        fileName[0] == '\0')
    {
        puts("Имя файла не должно быть пустым.");
        return 0;
    }

    if (findFileIndex(targetFileSystem, fileName) != -1)
    {
        puts("Файл с таким именем уже существует.");
        return 0;
    }

    int freeSlot = findFreeSlot(targetFileSystem);
    if (freeSlot == -1)
    {
        puts("Нет свободных слотов!");
        return 0;
    }

    file *newFile = initFile(fileName);
    if (newFile == NULL)
    {
        puts("Не удалось создать файл: ошибка выделения памяти.");
        return 0;
    }

    targetFileSystem->_file[freeSlot] = *newFile;
    free(newFile);

    printf("Файл '%s' создан (слот %d).\n", fileName, freeSlot + 1);
    return 1;
}

static int deleteFileCommand(fileSystem *targetFileSystem)
{
    int fileIndex;
    if (!readExistingFileName(targetFileSystem, "Имя удаляемого файла: ", &fileIndex))
    {
        return 0;
    }

    char deletedName[NAME_BUFFER_SIZE];
    snprintf(deletedName, sizeof(deletedName), "%s", targetFileSystem->_file[fileIndex]._name);

    deallocateFile(&targetFileSystem->_file[fileIndex]);
    targetFileSystem->_file[fileIndex]._name = NULL;
    targetFileSystem->_file[fileIndex]._file = NULL;

    printf("Файл '%s' удалён.\n", deletedName);
    return 1;
}

static file *cloneFile(const file *source, const char *newName)
{
    if (!isFileUsed(source) || newName == NULL)
    {
        return NULL;
    }

    file *copy = initFile(newName);
    if (copy == NULL)
    {
        return NULL;
    }

    for (const block *current = source->_file;
         current != NULL; current = current->_next)
    {
        if (current->_text == NULL || !setTextToFile(copy, current->_text))
        {
            deallocateFile(copy);
            free(copy);
            return NULL;
        }
    }

    return copy;
}

static int copyFileCommand(fileSystem *targetFileSystem)
{
    int sourceIndex;
    if (!readExistingFileName(targetFileSystem, "Имя исходного файла: ", &sourceIndex))
    {
        return 0;
    }

    char destinationName[NAME_BUFFER_SIZE];
    if (!readLine("Имя копии: ", destinationName, sizeof(destinationName)) ||
        destinationName[0] == '\0')
    {
        puts("Имя копии не должно быть пустым.");
        return 0;
    }

    if (findFileIndex(targetFileSystem, destinationName) != -1)
    {
        puts("Файл с таким именем уже существует.");
        return 0;
    }

    int freeSlot = findFreeSlot(targetFileSystem);
    if (freeSlot == -1)
    {
        puts("Нет свободного места для копии.");
        return 0;
    }

    file *copy = cloneFile(&targetFileSystem->_file[sourceIndex], destinationName);
    if (copy == NULL)
    {
        puts("Не удалось скопировать файл.");
        return 0;
    }

    targetFileSystem->_file[freeSlot] = *copy;
    free(copy);

    printf("Файл скопирован: '%s' -> '%s' (слот %d).\n", targetFileSystem->_file[sourceIndex]._name, destinationName, freeSlot + 1);
    return 1;
}

static int moveFileCommand(fileSystem *targetFileSystem)
{
    int sourceIndex;
    if (!readExistingFileName(targetFileSystem, "Имя перемещаемого файла: ", &sourceIndex))
    {
        return 0;
    }

    long destinationSlot;
    if (!readInteger("Номер целевого слота (1-10): ", &destinationSlot) ||
        destinationSlot < 1 || destinationSlot > MAXFILE)
    {
        puts("Некорректный номер слота.");
        return 0;
    }

    int destinationIndex = (int)destinationSlot - 1;
    if (destinationIndex == sourceIndex)
    {
        puts("Файл уже находится в этом слоте.");
        return 1;
    }

    if (targetFileSystem->_file[destinationIndex]._file != NULL)
    {
        puts("Целевой слот занят. Выберите свободный слот.");
        return 0;
    }

    targetFileSystem->_file[destinationIndex] = targetFileSystem->_file[sourceIndex];
    targetFileSystem->_file[sourceIndex]._name = NULL;
    targetFileSystem->_file[sourceIndex]._file = NULL;

    printf("Файл перемещён в слот %ld.\n", destinationSlot);
    return 1;
}

static int writeFileCommand(fileSystem *targetFileSystem)
{
    int fileIndex;
    if (!readExistingFileName(targetFileSystem, "Имя файла для записи: ", &fileIndex))
    {
        return 0;
    }

    char text[TEXT_BUFFER_SIZE];
    if (!readLine("Текст для добавления в конец файла: ", text, sizeof(text)))
    {
        return 0;
    }

    if (!setTextToFile(&targetFileSystem->_file[fileIndex], text))
    {
        puts("Не удалось записать данные: ошибка выделения памяти.");
        return 0;
    }

    puts("Данные записаны.");
    return 1;
}

static int readFileCommand(const fileSystem *targetFileSystem)
{
    int fileIndex;
    if (!readExistingFileName(targetFileSystem, "Имя файла для чтения: ", &fileIndex))
    {
        return 0;
    }

    const file *targetFile = &targetFileSystem->_file[fileIndex];
    printf("\nСодержимое файла '%s':\n", targetFile->_name);

    for (const block *current = targetFile->_file;
         current != NULL; current = current->_next)
    {
        if (current->_text != NULL)
        {
            fputs(current->_text, stdout);
        }
    }

    if (getFileLength(targetFile) == 0)
    {
        puts("[файл пуст]");
    }
    else
    {
        putchar('\n');
    }

    return 1;
}

static void listFiles(const fileSystem *targetFileSystem)
{
    int found = 0;

    puts("\nФайлы:");
    for (int i = 0; i < MAXFILE; ++i)
    {
        const file *current = &targetFileSystem->_file[i];
        if (isFileUsed(current))
        {
            printf("  Слот %2d: %-24s размер: %zu байт, блоков: %zu\n",
                   i + 1, current->_name,
                   getFileLength(current), getBlockCount(current));
            found = 1;
        }
    }

    if (!found)
    {
        puts("  Файлов нет.");
    }
}

static void printDump(FILE *output, const fileSystem *targetFileSystem)
{
    fprintf(output, "Количество слотов: %d\n", MAXFILE);

    for (int i = 0; i < MAXFILE; ++i)
    {
        const file *currentFile = &targetFileSystem->_file[i];
        fprintf(output, "\n[СЛОТ %d] ", i + 1);

        if (!isFileUsed(currentFile))
        {
            fprintf(output, "ПУСТО\n");
            continue;
        }

        fprintf(output, "ФАЙЛ: %s\n", currentFile->_name);
        fprintf(output, "Размер: %zu байт\n", getFileLength(currentFile));
        fprintf(output, "Количество блоков: %zu\n", getBlockCount(currentFile));
        fprintf(output, "Цепочка блоков: ");

        size_t blockIndex = 0;
        for (const block *currentBlock = currentFile->_file;
             currentBlock != NULL;
             currentBlock = currentBlock->_next, ++blockIndex)
        {
            fprintf(output, "block[%zu]%s", blockIndex, currentBlock->_next != NULL ? " -> " : " -> NULL\n");
        }

        fprintf(output, "Структура блоков:\n");
        blockIndex = 0;
        for (const block *currentBlock = currentFile->_file;
             currentBlock != NULL;
             currentBlock = currentBlock->_next, ++blockIndex)
        {
            size_t blockLength = currentBlock->_text != NULL ? strlen(currentBlock->_text) : 0;
            fprintf(output, "  block[%zu]: length=%zu, text=\"%s\"\n", blockIndex, blockLength, currentBlock->_text != NULL ? currentBlock->_text : "<null>");
        }

        fprintf(output, "Полное содержимое: ");
        for (const block *currentBlock = currentFile->_file;
             currentBlock != NULL; currentBlock = currentBlock->_next)
        {
            if (currentBlock->_text != NULL)
            {
                fputs(currentBlock->_text, output);
            }
        }
        if (getFileLength(currentFile) == 0)
        {
            fprintf(output, "[файл пуст]");
        }
        fprintf(output, "\n");
    }
}

static int dumpFileSystem(const fileSystem *targetFileSystem)
{
    FILE *dump = fopen(DUMP_FILE_NAME, "w");
    if (dump == NULL)
    {
        perror("Не удалось создать файл дампа");
        return 0;
    }

    printDump(dump, targetFileSystem);
    if (fclose(dump) != 0)
    {
        perror("Ошибка при закрытии файла дампа");
        return 0;
    }

    printf("Дамп файловой системы сохранён в '%s'.\n", DUMP_FILE_NAME);
    puts("\nСодержимое дампа:\n");
    printDump(stdout, targetFileSystem);
    return 1;
}

static void printMenu(void)
{
    puts("\n========== Меню ==========");
    puts("1. Создать файл");
    puts("2. Удалить файл");
    puts("3. Копировать файл");
    puts("4. Переместить файл в другой слот");
    puts("5. Записать данные в файл (добавить в конец)");
    puts("6. Прочитать файл");
    puts("7. Просмотреть список файлов");
    puts("8. Создать дамп файловой системы");
    puts("0. Выход");
}

int main(void)
{
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    fileSystem disk;
    initFileSystem(&disk);

    for (;;)
    {
        long choice;
        printMenu();

        if (!readInteger("Выберите команду: ", &choice))
        {
            if (feof(stdin))
            {
                putchar('\n');
                break;
            }
            puts("Введите целое число из меню.");
            continue;
        }

        switch (choice)
        {
        case 1:
            createFileCommand(&disk);
            break;
        case 2:
            deleteFileCommand(&disk);
            break;
        case 3:
            copyFileCommand(&disk);
            break;
        case 4:
            moveFileCommand(&disk);
            break;
        case 5:
            writeFileCommand(&disk);
            break;
        case 6:
            readFileCommand(&disk);
            break;
        case 7:
            listFiles(&disk);
            break;
        case 8:
            dumpFileSystem(&disk);
            break;
        case 0:
            deallocateFileSystem(&disk);
            puts("Файловая система закрыта.");
            return 0;
        default:
            puts("Неизвестная команда. Повторите ввод.");
            break;
        }
    }

    deallocateFileSystem(&disk);
    return 0;
}
