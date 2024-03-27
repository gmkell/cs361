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

int main(int argc, char *argv[])
{
    // set up synchronization mechanisms
    // if (argc != 2) {
    //     printf("Usage: %s <N factories> <order size>\n", argv[0]);
    //     exit(1);
    // }

    sem_t *mutex = Sem_open(SEM_MUTEX_NAME, O_CREAT, 0644, 1);
    sem_t *salesStarted = Sem_open(SEM_SALES_STARTED, O_CREAT, 0644, 0);
    sem_t *supervisorStarted = Sem_open(SEM_SUPER_STARTED, O_CREAT, 0644, 0);
    sem_t *salesFinished = Sem_open(SEM_SALES_FINISHED, O_CREAT, 0644, 0);
    sem_t *supervisorFinished = Sem_open(SEM_SUPER_FINISHED, O_CREAT, 0644, 0);

    // create/initialize variables 
    int activeFactories = atoi(argv[1]);
    key_t msgKey = (key_t) atoi(argv[2]);
    int completedFactories = 0;


    Sem_post(supervisorStarted);


    // open the supervisor.log file and make sure it redirects stdout
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

    // open the shmem created by Sales 
    key_t key;
    int shmID;
    shData *data;

    key = ftok("shmem.h", 'R');
    shmID = shmget(key, SHMEM_SIZE, 0);
    data = (shData *) shmat(shmID, NULL, 0);

    int order_size = data->order_size;
    int made = data->made;

    // open the message queue
    int msgStatus;
    int supervisorKey;
    int mailboxID, factoryMailboxID;
    msgBuf incomingMsg;

    int msg_id = Msgget(msgKey, 0666 | IPC_CREAT);
    if (msg_id < 0){
        perror("msgget");
        exit(1);
    }

    // main work
    Sem_wait(mutex);
    int iterations = 0; // how many times factory process has to repeat to complete part of order

    while ( activeFactories > 0 )
    {
        // receive message from a factory process thru msgQueue
        msgStatus = msgrcv(msg_id, &incomingMsg, MSG_INFO_SIZE, 0, 0);
        if (incomingMsg.purpose == PRODUCTION_MSG)
        {
            printf("Factory #   %3d produced    %5d parts in    %4d milliSecs\n", incomingMsg.facID, incomingMsg.partsMade, incomingMsg.duration);
            // update per-factory productions aggregates (num parts built, num-iterations)
            data->made = data->made + incomingMsg.partsMade;
            data->remain = data-> remain - incomingMsg.partsMade;
            iterations++;
        } else if (incomingMsg.purpose == COMPLETION_MSG)
        {
            printf("Factory #   %3d COMPLETED its task\n", incomingMsg.facID);
            activeFactories--;
        } else 
        {
            fprintf(stderr, "Unkown message type received.\n");
        }
    }

    Sem_post(mutex);

    // inform the Sales that manufacturing is done
    Sem_post(supervisorFinished);
        
    // wait for permission from Sales to print final report
    printf("\n");
    printf("SUPERVISOR: Manufacturing is complete. Awaiting permission to print final report\n");
    printf("\n");
    Sem_wait(salesFinished);     // synchronize with Sales through NAMED semaphore
    // print per-factory production aggregates sorted by factoryID.
    printf("******  SUPERVISOR: Final Report   ******\n");
    // loop through all factory processes
    // printf("Factory #  %3d made a total of   %5d parts in       %4d iterations\n", ... , ... , ... );    
    printf("--------------------------------------------------------------------------------\n");
    printf("--------------------------------------------------------------------------------\n");
    printf("Grand total parts made =    %5d     vs      order size of   %5d\n", data->made , data->order_size );    
    
    // close supervisor.log
    fclose(supervisor_log);

    return 0;
}