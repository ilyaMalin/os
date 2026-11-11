#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memoryManager.h"
#include "test.h"

#define MODE_PROGRAM "--program"
#define MODE_TEST "--test"
#define INPUT_BUFFER_SIZE 512

static void printUsage(const char *_programName)
{
    printf("Использование: %s [режим]\n\n", _programName);

    printf("Режимы:\n");
    printf("  --program    Запуск менеджера памяти\n");
    printf("  --test       Запуск модульных тестов\n");
}

static void clearInputBuffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
    }
}

static int runProgram(void)
{
    memoryManager mm;
    memset(&mm, 0, sizeof(memoryManager));
    int isInitialized = 0;
    int choice = -1;

    do
    {
        printf("Меню\n");
        printf("1. Инициализировать менеджер памяти%s\n", isInitialized ? " (переинициализация)" : "");
        printf("2. Выделить блок памяти\n");
        printf("3. Записать данные в память\n");
        printf("4. Прочитать данные из памяти\n");
        printf("5. Освободить блок памяти\n");
        printf("6. Показать состояние памяти\n");
        printf("0. Выход\n");
        printf("Выберите действие: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Некорректный выбор! Введите число.\n");
            clearInputBuffer();
            choice = -1;
            continue;
        }
        clearInputBuffer();

        switch (choice)
        {
        case 1:
        {
            unsigned int pageSize = 0;
            unsigned int totalMemory = 0;

            printf("Введите размер страницы (байт): ");
            if (scanf("%u", &pageSize) != 1 || pageSize == 0)
            {
                printf("Ошибка: размер страницы должен быть больше 0!\n");
                clearInputBuffer();
                break;
            }
            clearInputBuffer();

            printf("Введите общий объём памяти (байт): ");
            if (scanf("%u", &totalMemory) != 1 || totalMemory == 0)
            {
                printf("Ошибка: объём памяти должен быть больше 0!\n");
                clearInputBuffer();
                break;
            }
            clearInputBuffer();

            if (isInitialized)
            {
                freeMemoryManager(&mm);
            }

            initMemoryManager(&mm, pageSize, totalMemory);
            isInitialized = 1;
            printf("Менеджер памяти инициализирован (страница: %u байт, всего: %u байт).\n", pageSize, totalMemory);
            break;
        }

        case 2:
        {
            if (!isInitialized)
            {
                printf("Менеджер памяти ещё не инициализирован!\n");
                break;
            }

            unsigned int size = 0;
            printf("Введите размер выделяемого блока (байт): ");
            if (scanf("%u", &size) != 1 || size == 0)
            {
                printf("Ошибка: размер блока должен быть больше 0!\n");
                clearInputBuffer();
                break;
            }
            clearInputBuffer();

            int res = allocateMemory(&mm, size);
            if (res == SUCCESS)
            {
                printf("Выделено %u байт. Свободной памяти осталось: %u байт.\n", size, mm._freeMemory);
            }
            else if (res == ERRORALLOC)
            {
                printf("Недостаточно свободной памяти для выделения %u байт!\n", size);
            }
            else
            {
                printf("Ошибка выделения! Сначала освободите текущий блок!\n");
            }
            break;
        }

        case 3:
        {
            if (!isInitialized)
            {
                printf("Менеджер памяти ещё не инициализирован!\n");
                break;
            }

            unsigned int address = 0;
            printf("Введите начальный адрес (смещение): ");
            if (scanf("%u", &address) != 1)
            {
                printf("Некорректный адрес!\n");
                clearInputBuffer();
                break;
            }
            clearInputBuffer();

            char text[INPUT_BUFFER_SIZE];
            printf("Введите текст для записи: ");
            if (fgets(text, sizeof(text), stdin) == NULL)
            {
                printf("Ошибка чтения ввода!\n");
                break;
            }
            text[strcspn(text, "\r\n")] = '\0';

            unsigned int len = (unsigned int)strlen(text);
            if (len == 0)
            {
                printf("Текст не может быть пустым!\n");
                break;
            }

            int res = write(&mm, address, text, len);
            if (res == SUCCESS)
            {
                printf("Успешно записано %u байт по адресу %u.\n", len, address);
            }
            else
            {
                printf("Ошибка: выход за границы блока или память не выделена!\n");
            }
            break;
        }

        case 4:
        {
            if (!isInitialized)
            {
                printf("Менеджер памяти ещё не инициализирован!\n");
                break;
            }

            unsigned int address = 0;
            printf("Введите начальный адрес (смещение): ");
            if (scanf("%u", &address) != 1)
            {
                printf("Некорректный адрес!\n");
                clearInputBuffer();
                break;
            }
            clearInputBuffer();

            unsigned int size = 0;
            printf("Введите количество байт для чтения: ");
            if (scanf("%u", &size) != 1 || size == 0)
            {
                printf("Ошибка: количество байт должно быть больше 0!\n");
                clearInputBuffer();
                break;
            }
            clearInputBuffer();

            unsigned char *buffer = calloc(size + 1, sizeof(unsigned char));
            if (buffer == NULL)
            {
                printf("Ошибка выделения системной памяти!\n");
                break;
            }

            int res = read(&mm, address, buffer, size);
            if (res == SUCCESS)
            {
                printf("Прочитано %u байт с адреса %u:\n", size, address);
                printf("  HEX  : ");
                for (unsigned int i = 0; i < size; i++)
                {
                    printf("%02X ", buffer[i]);
                }
                printf("\n  ТЕКСТ: \"");
                for (unsigned int i = 0; i < size; i++)
                {
                    if (buffer[i] >= 32 && buffer[i] <= 126)
                    {
                        putchar(buffer[i]);
                    }
                    else
                    {
                        putchar('.');
                    }
                }
                printf("\"\n");
            }
            else
            {
                printf("Ошибка: выход за границы блока или память не выделена!\n");
            }

            free(buffer);
            break;
        }

        case 5:
        {
            if (!isInitialized)
            {
                printf("Менеджер памяти ещё не инициализирован!\n");
                break;
            }

            int res = freeMemoryBlock(&mm);
            if (res == SUCCESS)
            {
                printf("Блок памяти освобождён. Свободно: %u байт.\n", mm._freeMemory);
            }
            else
            {
                printf("Нет выделенного блока памяти для освобождения!\n");
            }
            break;
        }

        case 6:
        {
            if (!isInitialized)
            {
                printf("Менеджер памяти не инициализирован!\n");
            }
            else
            {
                printf("\n--- Состояние менеджера памяти ---\n");
                printf("Размер страницы    : %u байт\n", mm._pageSize);
                printf("Свободная память   : %u байт\n", mm._freeMemory);
                printf("Запрошенный размер : %u байт\n", mm._requestedSize);
                printf("Статус блока       : %s\n", mm._pageTable != NULL ? "ВЫДЕЛЕН" : "СВОБОДЕН");
            }
            break;
        }

        case 0:
            if (isInitialized)
            {
                freeMemoryManager(&mm);
            }
            printf("Выход из программы.\n");
            break;

        default:
            printf("Некорректный выбор!\n");
            break;
        }

    } while (choice != 0);

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc == 2 && strcmp(argv[1], MODE_PROGRAM) == 0)
    {
        return runProgram();
    }
    else if (argc == 2 && strcmp(argv[1], MODE_TEST) == 0)
    {
        return runTests();
    }

    printUsage(argv[0]);

    return 1;
}
