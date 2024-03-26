//----------------------------------------------------------------------------
// Assignment   :   PA2 IPC
// Date         :   3/25/24
// 
// Group 6      : Gillian Kelly,   Ethan Pae          
//
// Sales        :   sales.c
//----------------------------------------------------------------------------

#include <stdio.h> 
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <fcntl.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>

#include "shmem.h"
#include "wrappers.h"
#include "message.h"

#define SEM_MUTEX_NAME "/sem_mutex"
#define SEM_SALES_STARTED "/sem_sales_started"
#define SEM_SUPER_STARTED "/sem_super_started"
#define SEM_SALES_FINISHED "/sem_sales_finished"
#define SEM_SUPER_FINISHED "/sem_super_finished"


//------------------------------------------------------------
// Handles cleanup when TERM or INT is caught by to sales
//------------------------------------------------------------
// void sigHandler_kill(int sig)
// {
//     printf("Sales gracefully shutdown via %d \n", sig);
//     fflush(stdout);
//     cleanup_resources();
//     exit(EXIT_SUCCESS);
// }

// // helper function to do some of the work for sigHandler
// void cleanup_resources()
// {
//     // kill ALL child processes
//     if (supervisor_pid > 0)
//     {
//         kill(supervisor_pid, SIGKILL);
//     }

//     for (int i = 0; i < num_factories; i++)
//     {
//         kill(factory_pid[i], SIGKILL);
//     }
//     // clean up semaphores

//     // remove shared memory
//     if (shmID != -1)
//     {
//         shmctl(shmID, IPC_RMID, NULL);
//     }

//     // destroy message queue
//     msgctl(mailboxID, IPC_REMID, NULL);
// }


int main(int argc, char* argv[])
{
    // set up synchronization mechanisms
    if (atoi(argv[1]) > MAXFACTORIES) 
    {
        printf("%d exceeded maximum number of factory lines allowed. \n", atoi(argv[1]));
        exit(EXIT_FAILURE); 
    }
    
    int num_factories = atoi(argv[1]);

    int num_parts = atoi(argv[2]);

    printf("SALES: Will Request an Order of Size = %d parts\n", num_parts);

    // Step 1: set up shared mem & initialize its objects
    int shmID;
    key_t shmKey;
    shData *data;

    // generate unique key for shared memory
    shmKey = ftok("shmem.h", 'R');
    if (shmKey == -1)
    {
        perror("ftok on shmkey\n");
        exit(EXIT_FAILURE);
    }
    // create shared memory segment 
    shmID = shmget(shmKey, SHMEM_SIZE, IPC_CREAT | 0666);
    if (shmID == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    // attack shmem to Sales;s address space 
    shData *ptr;
    ptr = (shData*) shmat(shmID, NULL, 0);
    if (ptr == (void*)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    
    int mypid = getpid();

    // initialize shmem content
    ptr->order_size = num_parts;
    ptr->made = 0;
    ptr->remain = num_parts;

    // Step 2: set up message queue and semaphores
    key_t msgKey;
    int msgStatus;
    char* msgPath = ".";
    msgKey = ftok(msgPath, mypid);
    int msgflg  =  IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IWOTH  ;
    int mailboxID = msgget(msgKey, msgflg | IPC_CREAT);

    // Step 2.1 semaphore setup
    // create reandevous semaphore for sales and supervisor 
    sem_t *mutex = Sem_open(SEM_MUTEX_NAME, O_CREAT, 0644, 1);
    sem_t *salesStarted = Sem_open(SEM_SALES_STARTED, O_CREAT, 0644, 0);
    sem_t *supervisorStarted = Sem_open(SEM_SUPER_STARTED, O_CREAT, 0644, 0);
    sem_t *salesFinished = Sem_open(SEM_SALES_FINISHED, O_CREAT, 0644, 0);
    sem_t *supervisorFinished = Sem_open(SEM_SUPER_FINISHED, O_CREAT, 0644, 0);     

    
    // Step 3: Fork/Execute Supervisor process 
    int supervisor_pid = Fork();
    if (supervisor_pid == 0)
    {
        // Step 3.1 redirect stdout for supervisor child to 'supevisor.log'
        char num_factories_str[10];
        sprintf(num_factories_str, "%d", num_factories);
        // Step 3.2 send arguments and execute supervisor process
        execlp("./supervisor", "supervisor", num_factories_str, (char*)NULL);
        perror("excelp supervisor");
        exit(EXIT_FAILURE);
    }   




    Sem_wait(mutex); 
    

    // Step 4: Fork/Execute all factory processes
    printf("Creating %d Factory(ies)\n", num_factories);

    srandom(time(NULL));     // seed random number generator only ONCE



    // create num_factories child processes based on num_factories
    for (int i = 0; i < num_factories; i++)
    {   
        // Step 4.1 create the factory arguments using random()
        int capacity = (int) random() % (50 - 10 + 1) + 10;
        int duration = (int)random() % (1200 - 500 + 1) + 500; ; // in miliseconds 

        int factory_pid = fork();
        if (factory_pid == -1) 
        {
            perror("fork child process");
            exit(EXIT_FAILURE);
        }

        // child process
        if (factory_pid == 0)
        {
            // convert arguments to string so they can used in excelp
            char capacity_str[10];
            char duration_str[10];
            char factory_id_str[10];
            sprintf(capacity_str, "%d", capacity);
            sprintf(duration_str, "%d", duration);
            sprintf(factory_id_str, "%d", i + 1); 
            
            // Step 4.2: redirect all factory process stdout to 'factory.log'
            int factory_log = open("factory.log", O_WRONLY, O_CREAT, O_TRUNC, 0666);
            dup2(factory_log, STDOUT_FILENO);
            close(factory_log); // no need to keep open

            // Step 4.3: send the arguments to the processes via execlp()
            execlp("./factory", "factory", factory_id_str, capacity_str, duration_str, (char*)NULL);
            perror("excelp factory\n");
            exit(EXIT_FAILURE);

        }
        // Step 4.3 Parent process: continue to next iteration to fork next factory
        printf("SALES: Factory #   %d was created, with Capacity=   %d  and Duration=   %d\n", i , capacity, duration);
    }

    // handle critical selection made by redirection of stdout using a synchronization method
    Sem_post(mutex);

    // Handle signals
    // sigactionWrapper(SIGINT, sigHandler_kill);
    // sigactionWrapper(SIGTERM, sigHandler_kill);

    // Step 5: Wait for supervisor to indicate manufacturing is done
    // sales waits via semaphore for supervisor to finish
    Sem_wait(supervisorFinished);
    printf("SALES: Supervisor says all Factories have completed their mission\n");

    // Step 6: Grant supervisor permission to print the Final Report
    printf("SALES: Permission granted to print final report\n");

    // Step 7: Clean up zombie processes (Supervisior + all Factories)
    printf("SALES: Cleaning up after the Supervisor Factory Process\n");
    for (int i = 0; i < num_factories; i++)
    {
        if (fork() > 0)
        {
            kill(fork(), SIGKILL);
        }
    }

    if (supervisor_pid > 0)
    {
        kill(supervisor_pid, SIGKILL);
    }
    
    // Step 8: destroy shmem
    shmdt(ptr);
    shmctl(shmID, IPC_RMID, NULL);

    // Step 9: destroy semaphores and message queue
    msgctl(mailboxID, IPC_RMID, NULL);

    return 0;
}