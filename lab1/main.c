#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

void printCreatID() 
{
    printf("PID %u был порождён PPID %u\n", getpid(), getppid());
}

void printCloseID()
{
    printf("PID %u && PPID %u был завершён\n", getpid(), getppid());
}

void creatProcess() 
{
    pid_t pid;

    pid = fork();
    if (pid == -1) 
    {
        printf("Процесс не создался\n");
        exit(-1);    
    }
    else if (pid == 0)
    {
        printCreatID();

        pid = fork();
        if (pid == -1)
        {  
            printf("Процесс не создался\n");
            exit(-1);        
        }
        else if (pid == 0)
        {
            printCreatID();

            pid = fork();
            if (pid == -1)
            {
                printf("Процесс не создался\n");
                exit(-1);    
            }
            else if (pid == 0)
            {
                printCreatID();
                return; 
            }

            wait(NULL); 
            return; 
        }

        pid = fork();
        if (pid == -1)
        {
            printf("Процесс не создался\n");
            exit(-1);    
        }
        else if (pid == 0)
        {
            printCreatID();

            pid = fork();
            if (pid == -1)
            {
                printf("Процесс не создался\n");
                exit(-1);    
            }
            else if (pid == 0)
            {
                printCreatID();

                pid = fork();
                if (pid == -1)
                {
                    printf("Процесс не создался\n");
                    exit(-1);    
                }
                else if (pid == 0)
                {   
                    printCreatID();
                    pid_t t_pid = getpid(), t_ppid = getppid();
                    printf("Замена процесса PID %u на whoami\n\n", t_pid);
                    execl("/usr/bin/whoami", "whoami", "--version", NULL); 
                }

                wait(NULL); 
                return; 
            }

            wait(NULL); 
            return; 
        }

        wait(NULL); 
        wait(NULL);
        return; 
    }  
}

int main()
{
    printCreatID();
    creatProcess();
    wait(NULL);
    printCloseID();
}
