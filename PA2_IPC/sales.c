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
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>

#include "shmem.h"
#include "wrappers.h"
#include "message.h"

#define SEM_MUTEX_NAME "/sem_mutex"
#define SEM_SALES_STARTED "/sem_sales_started"
#define SEM_SUPER_STARTED "/sem_super_started"
#define SEM_SALES_FINISHED "/sem_sales_finished"
#define SEM_SUPER_FINISHED "/sem_super_finished"
#define SEM_FACTORY_FINISHED "/sem_factory_finished"

// declare variables in global scope for signal catching
sem_t *mutex, *salesStarted, *supervisorStarted, *salesFinished, *supervisorFinished, *factoryFinished;
int factories[MAXFACTORIES];
int num_factories = 0;
int supervisor_pid = -1;
int mailboxID = -1;
int shmID = -1;

void sigHandler_KILL(int sig);
void cleanup_resources();
void destroy_semaphores();

int main(int argc, char* argv[])
{
    // set up synchronization mechanisms
    if (atoi(argv[1]) > MAXFACTORIES) 
    {
        printf("%d exceeded maximum number of factory lines allowed. \n", atoi(argv[1]));
        exit(EXIT_FAILURE); 
    }
    
    num_factories = atoi(argv[1]);

    int num_parts = atoi(argv[2]);

    printf("SALES: Will Request an Order of Size = %d parts\n", num_parts);

    // Step 1: set up shared mem & initialize its objects
    key_t key;
    pid_t mypid;
    shData *data;

    // create shmkey by converting pathname to an IPC key 
    key = ftok("shmem.h", 'R');
    if (key == -1)
    {
        perror("ftok() on key");
        exit(EXIT_FAILURE);  
    }

    // create shared memory segment 
    if ( (shmID = shmget(key, SHMEM_SIZE, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR)) == -1 )
    {
        perror("shmget() failed to create shared memory in Sales");
        fprintf(stderr, " Error code: %d", errno);
    }

    data = (shData *) shmat(shmID, NULL, 0);
    if (data == (shData *) -1) {
        perror("shmat in sales.c");
        exit(EXIT_FAILURE);
    }
    mypid = getpid(); // the sales process pid

    // Seed random number generator only ONCE
    srandom(time(NULL));

    // Step 2: set up message queue and semaphores
    msgBuf msgQue;
    int msgStatus;
    key_t msgKey;
    // msgKey = ftok(".", mypid);
    msgKey = ftok("message.h", 'B');
    if (msgKey == -1)
    {
        perror("ftok for msg queue in sales");
        exit(EXIT_FAILURE);
    }
    int msgflg = IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IWOTH;
    mailboxID = msgget(msgKey, IPC_CREAT | 0666);
    if (mailboxID == -1)
    {
        perror("mssget failed for msg queue in sales");
        exit(EXIT_FAILURE);
    }

    // Step 2.1: semaphore setup
    // create reandevous semaphore for sales and supervisor 
    mutex = Sem_open(SEM_MUTEX_NAME, O_CREAT, 0644, 1);
    salesStarted = Sem_open(SEM_SALES_STARTED, O_CREAT, 0644, 0);
    supervisorStarted = Sem_open(SEM_SUPER_STARTED, O_CREAT, 0644, 0);
    salesFinished = Sem_open(SEM_SALES_FINISHED, O_CREAT, 0644, 0);
    supervisorFinished = Sem_open(SEM_SUPER_FINISHED, O_CREAT, 0644, 0);
    factoryFinished = Sem_open(SEM_FACTORY_FINISHED, O_CREAT, 0644, 0);

    // printf("SALES: Created semaphores and set up message queues!\n");
    data->made = 0;
    data->remain = num_parts;
    data->order_size = num_parts;

    // Step 3: Fork/Execute Supervisor process
    supervisor_pid = Fork();
    if (supervisor_pid == 0)
    {
        // Step 3.1 redirect stdout for supervisor child to 'supevisor.log'
        char num_factories_str[10];
        char msgkey_str[10];
        sprintf(num_factories_str, "%d", num_factories);
        sprintf(msgkey_str, "%d", msgKey);
        int supervisor_log_fd = open("supervisor.log", O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (supervisor_log_fd == -1)
        {
            perror("Failed to open/create supervisor.log\n");
            exit(EXIT_FAILURE);
        }
        close(supervisor_log_fd);

        // Step 3.2 send arguments and execute supervisor process
        execlp("./supervisor", "supervisor", num_factories_str, msgkey_str, (char *)NULL);
        perror("excelp supervisor\n");
        exit(EXIT_FAILURE);
    }   

    // Step 4: Fork/Execute all factory processes
    printf("Creating %d Factory(ies)\n", num_factories);

    // seed random number generator only ONCE
    srandom(time(NULL));

    // Create factory.log before forking factory processes
    int factory_log_fd = open("factory.log", O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (factory_log_fd == -1) {
        perror("Failed to open/create factory.log");
        exit(EXIT_FAILURE);
    }
    close(factory_log_fd); // Close the file descriptor as factories will open it themselves

    for (int i = 0; i < num_factories; i++) {
        // Create the factory arguments using random()
        int capacity = (int)random() % (50 - 10 + 1) + 10;
        int duration = (int)random() % (1200 - 500 + 1) + 500; // in milliseconds 

        pid_t factory_pid = fork();
        if (factory_pid == -1) {
            perror("fork child process");
            exit(EXIT_FAILURE);
        }

        // Child process: factory
        if (factory_pid == 0) {
            // Convert arguments to string to be used in execlp
            char capacity_str[10];
            char duration_str[10];
            char factory_id_str[10];
            char msgkey_str[10];
            sprintf(capacity_str, "%d", capacity);
            sprintf(duration_str, "%d", duration);
            sprintf(factory_id_str, "%d", i + 1);
            sprintf(msgkey_str, "%d", msgKey);
            factories[i] = factory_pid;
            // Execute factory program with arguments
            execlp("./factory", "factory", factory_id_str, capacity_str, duration_str, msgkey_str, (char *)NULL);
            perror("execlp factory"); // Only reached if execlp fails
            exit(EXIT_FAILURE);
        }

        // Parent process: Print factory creation details
        printf("SALES: Factory #%d was created, with Capacity = %d and Duration = %d ms\n", i + 1, capacity, duration);
    }

    Sem_wait(factoryFinished);

    // printf("SALES: Successfully ran factories!\n");
    // printf("SALES: Waiting for supervisor to finish!\n");

    // Step 5: Wait for supervisor to indicate manufacturing is done
    // sales waits via semaphore for supervisor to finish
    Sem_wait(supervisorFinished);
    printf("SALES: Supervisor says all Factories have completed their mission\n");

    // Step 6: Grant supervisor permission to print the Final Report
    Sem_post(salesFinished);
    printf("SALES: Permission granted to print final report\n");

    // Step 7: Clean up zombie processes (Supervisior + all Factories)
    printf("SALES: Cleanign up after the Supervisor Factory Process\n");
    
    // Step 8: destroy shmem
    Shmdt(data);
    shmctl(shmID, IPC_RMID, 0);

    // Step 9: destroy semaphores and message queue
    destroy_semaphores();


    // Step 10: setup signal catching 
    sigactionWrapper(SIGINT, sigHandler_KILL);
    sigactionWrapper(SIGTERM, sigHandler_KILL);
    return 0;
}

void sigHandler_KILL(int sig)
{
    fflush(stdout);
    cleanup_resources();
    exit(EXIT_SUCCESS);
}

void cleanup_resources() 
{
    // kill supervisor process
    if (supervisor_pid > 0)
    {
        kill(supervisor_pid, SIGKILL);
    }
    // kill all factory processes
    for (int i = 0; i < num_factories; i++)
    {
        kill(factories[i], SIGKILL);
    }

    // destroy shared memory
    if (shmID != -1)
    {
        shmctl(shmID, IPC_RMID, NULL);
    }

    // destroy message queue
    if (mailboxID != -1)
    {
        msgctl(mailboxID, IPC_RMID, NULL);
    }

    destroy_semaphores();
}

void destroy_semaphores()
{
    Sem_close(mutex);
    Sem_close(supervisorFinished);
    Sem_close(salesFinished);
    Sem_close(supervisorStarted);
    Sem_close(salesStarted);
    Sem_close(factoryFinished);
    Sem_unlink(SEM_MUTEX_NAME);
    Sem_unlink(SEM_SALES_FINISHED);
    Sem_unlink(SEM_SALES_STARTED);
    Sem_unlink(SEM_FACTORY_FINISHED);
    Sem_unlink(SEM_SUPER_STARTED);
    Sem_unlink(SEM_SUPER_FINISHED);
}