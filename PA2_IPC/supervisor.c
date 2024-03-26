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

#define MSG_KEY 0x5678

int main(int argc, char *argv[])
{
    // set up synchronization mechanisms
    if (argc != 2) {
        printf("Usage: %s <N factories> <order size>\n", argv[0]);
        exit(1);
    }

    sem_t *mutex = Sem_open(SEM_MUTEX_NAME, O_CREAT, 0644, 1);
    sem_t *salesStarted = Sem_open(SEM_SALES_STARTED, O_CREAT, 0644, 0);
    sem_t *supervisorStarted = Sem_open(SEM_SUPER_STARTED, O_CREAT, 0644, 0);
    sem_t *salesFinished = Sem_open(SEM_SALES_FINISHED, O_CREAT, 0644, 0);
    sem_t *supervisorFinished = Sem_open(SEM_SUPER_FINISHED, O_CREAT, 0644, 0);

    // create/initialize variables 
    int activeFactories = atoi(argv[1]);
    int completedFactories = 0;

    printf("SUPERVISOR: Started\n");
    Sem_post(supervisorStarted);

    // redirect stdout to 'supervisor.log'
    FILE *log = fopen("supervisor.log", "w"); // create supervisor.log file
    if (log == NULL)
    {
        perror("fopen() failed");
        return 1;
    }

    if (freopen("supervisor.log", "w", stdout) == NULL) // redirect stdout
    {
        perror("freopen() failed");
        return 1;
    }

    // open the shmem created by Sales 
    int msgStatus;
    int supervisorKey;
    int mailboxID, factoryMailboxID;
    msgBuf incomingMsg;

    // open the message queue
    int msg_id = msgget(MSG_KEY, 0666 | IPC_CREAT);
    if (msg_id < 0){
        perror("msgget");
        exit(1);
    }

    // main work
    Sem_wait(mutex);
    int iterations = 0;
    while ( activeFactories > completedFactories)
    {
        // receive a message from msgQueue
        msgStatus = msgrcv( msg_id , &incomingMsg , MSG_INFO_SIZE, 0, 0);
        if (msgStatus == -1)
        {
            printf("Failed to receive message when %d factories running\n", activeFactories);
        }
        // validate the message
        switch (incomingMsg.purpose){
            case PRODUCTION_MSG:
                printf("Factory %3d produced %5d parts in %4d miliSecs\n", incomingMsg.facID, incomingMsg.partsMade, incomingMsg.duration);
                // update per-factory productions aggregates (num parts built, num-iterations)
                break;
            case COMPLETION_MSG:
                printf("Factory %3d Completed its production.\n", incomingMsg.facID);
                completedFactories++;
                break;
            default:
                fprintf(stderr, "Unkown message type received.\n");
        }
    }
    Sem_post(mutex);

    printf("All factories have completed production.\n");

    // inform the Sales that manufacturing is done
    Sem_post(supervisorFinished);

    // synchronize with Sales through NAMED semaphore
    
    
    // wait for permission from Sales to print final report
    Sem_wait(salesFinished);
    // print per-factory production aggregates sorted by factoryID.
    
    //<fill in here>


    // CLOSE LOG!! 
    fclose(log);

    return 0;
}