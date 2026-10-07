#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define CHAIRS 4

sem_t mutex;
sem_t customers;
sem_t barber;

int waiting = 0;
int stop = 0;

void *barber_thread(void *arg) 
{
    while (1) 
    {
        sem_wait(&customers);
        
        sem_wait(&mutex);
        if (stop && waiting == 0) {
            sem_post(&mutex);
            break;
        }
        
        waiting--;
        sem_post(&mutex);

        sem_post(&barber);
        printf("Парикмахер стрижёт клиента...\n");
        sleep(5); 
        printf("Парикмахер закончил стрижку\n");
    }
    return NULL;
}

void *customer_thread(void *arg) 
{
    long id = (long)arg; 

    sem_wait(&mutex);

    if (waiting < CHAIRS) {
        waiting++;
        printf("Клиент %ld ждёт своей очереди\n", id);

        sem_post(&customers);
        sem_post(&mutex);

        sem_wait(&barber);
        printf("Клиент %ld постригся\n", id);
    } else {
        sem_post(&mutex);
        printf("Клиенту %ld не хватило места — он ушел\n", id);
    }

    return NULL;
}

int main(void) {
    sem_init(&mutex, 0, 1);
    sem_init(&customers, 0, 0);
    sem_init(&barber, 0, 0);

    pthread_t barber_tid;
    pthread_create(&barber_tid, NULL, barber_thread, NULL);

    long client_id = 1;
    int choice;

    do 
    {
        printf("\nДобавить клиента? (1 - да, 0 - выход): ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: 
            {
                pthread_t tid;
                pthread_create(&tid, NULL, customer_thread, (void *)client_id);
                pthread_detach(tid);
                client_id++;
                break;
            }
            case 0:
                break;
            default:
                printf("Некорректный выбор!\n");
                break;
        }

    } while (choice != 0);

    sem_wait(&mutex);
    stop = 1;
    sem_post(&mutex);
    sem_post(&customers);

    pthread_join(barber_tid, NULL);

    sem_destroy(&mutex);
    sem_destroy(&customers);
    sem_destroy(&barber);
}