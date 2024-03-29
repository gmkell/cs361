//----------------------------------------------------------------------------
// Assignment   :   PA2 IPC
// Date         :   3/25/24
// 
// Group 6      : Gillian Kelly,   Ethan Pae          
//
// Supervisor   :   supervisor.c
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

#include "shmem.h"
#include "wrappers.h"
#include "message.h"

#define SEM_MUTEX_NAME "/sem_mutex"
#define SEM_SALES_STARTED "/sem_sales_started"
#define SEM_SUPER_STARTED "/sem_super_started"
#define SEM_SALES_FINISHED "/sem_sales_finished"
#define SEM_SUPER_FINISHED "/sem_super_finished"
#define SEM_FACTORY_STARTED "/sem_factory_started"
#define SEM_FACTORY_FINISHED "/sem_factory_finished"

#define MSG_KEY 0x5678 

void sort(int arr[], int size);

int main(int argc, char *argv[])
{
    // Step 1: set up synchronization mechanisms
    sem_t *mutex = Sem_open(SEM_MUTEX_NAME, O_CREAT, 0644, 1);
    sem_t *salesStarted = Sem_open(SEM_SALES_STARTED, O_CREAT, 0644, 0);
    sem_t *supervisorStarted = Sem_open(SEM_SUPER_STARTED, O_CREAT, 0644, 0);
    sem_t *salesFinished = Sem_open(SEM_SALES_FINISHED, O_CREAT, 0644, 0);
    sem_t *supervisorFinished = Sem_open(SEM_SUPER_FINISHED, O_CREAT, 0644, 0);

    // Step 2: create/initialize variables 
    int activeFactories = atoi(argv[1]);
    key_t msgKey = (key_t) atoi(argv[2]);
    int completedFactories = 0;
    int factories[activeFactories]; // holds completion information for each active factory


    Sem_post(supervisorStarted);


    // Step 3: open the supervisor.log file and make sure it redirects stdout
    FILE *supervisor_log = fopen("supervisor.log", "w"); // open supervisor.log file
    if (supervisor_log == NULL)
    {
        perror("failed to open supervisor.log in Supervisor");
        exit(EXIT_FAILURE);
    }
    if (freopen("supervisor.log", "w", stdout) == NULL) // redirect stdout
    {
        perror("freopen() failed");
        exit(EXIT_FAILURE);
    }

    
    printf("SUPERVISOR: Started\n");

    // Step 4: open the shmem created by Sales 
    key_t key;
    int shmID;
    shData *data;

    key = ftok("shmem.h", 'R');
    shmID = shmget(key, SHMEM_SIZE, 0);
    data = (shData *) shmat(shmID, NULL, 0);

    int order_size = data->order_size;
    int made = data->made;

    // Step 5: open the message queue
    int msgStatus;
    int supervisorKey;
    int mailboxID, factoryMailboxID;
    msgBuf incomingMsg;

    int msg_id = Msgget(msgKey, 0666 | IPC_CREAT);
    if (msg_id < 0){
        perror("msgget");
        exit(1);
    }

    // Step 5: main work
    int iterations = 0; // how many times factory process has to repeat to complete part of order

    Sem_wait(mutex);

    while ( activeFactories > 0 )
    {
        // receive message from a factory process thru msgQueue
        msgStatus = msgrcv(msg_id, &incomingMsg, MSG_INFO_SIZE, 0, 0);
        if (incomingMsg.purpose == PRODUCTION_MSG)
        {
            printf("Factory #   %3d produced    %5d parts in    %4d milliSecs\n", incomingMsg.facID, incomingMsg.partsMade, incomingMsg.duration);
            // update per-factory productions aggregates (num parts built, num-iterations)
            data->made += incomingMsg.partsMade;
            data->remain -= incomingMsg.partsMade;
            iterations++;
        } else if (incomingMsg.purpose == COMPLETION_MSG)
        {
            factories[completedFactories] = incomingMsg.facID; // collect the factoryID of all active factories 
            printf("Factory #   %3d COMPLETED its task\n", incomingMsg.facID);            
            activeFactories--;
            completedFactories++;
        } else 
        {
            fprintf(stderr, "Unkown message type received.\n");
        }
    }

    Sem_post(mutex);

    // Step 6: inform the Sales that manufacturing is done
    Sem_post(supervisorFinished);
        
    // Step 7: wait for permission from Sales to print final report
    printf("\n");
    printf("SUPERVISOR: Manufacturing is complete. Awaiting permission to print final report\n");
    printf("\n");
    Sem_wait(salesFinished);     // synchronize with Sales through NAMED semaphore
    
    // Step 8: print per-factory production aggregates sorted by factoryID.
    printf("******  SUPERVISOR: Final Report   ******\n");
    
    //sort(factories, completedFactories);

    for (int i = 0; i < completedFactories; i++)
    {
        printf("Factory #  %3d made a total of   %5d parts in       %4d iterations\n", factories[i], data->made, iterations);    

    }
    //printf("Factory #  %3d made a total of   %5d parts in       %4d iterations\n", incomingMsg.facID , incomingMsg.partsMade , iterations );    
    printf("--------------------------------------------------------------------------------\n");
    printf("--------------------------------------------------------------------------------\n");
    printf("Grand total parts made =    %5d     vs      order size of   %5d\n", data->made , data->order_size );    
    
    // close supervisor.log
    fclose(supervisor_log);

    return 0;
}


// helper method to sort factory id's in ascending order
void sort(int arr[], int size)
{
    int a;
    for (int i = 0; i < size; i++)
    {
        for (int j = i + 1; j < size; ++j)
        {
            a = arr[i];
            arr[i] = arr[j];
            arr[j] = a;
        }
    }
}