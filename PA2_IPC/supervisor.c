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
#include <cstdlib>


#include "shmSegment.h"
#include "shmem.h"
#include "wrappers.h"
#include "message.h"

int main(argc, char* argv[])
{
    // create/initialize variables 
    int activeFactories = argv[1];
    printf("SUPERVISOR: Started\n");

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

    // main work
    while ( activeFactories > 0 )
    {
        // receive a message from msgQueue
        msgStatus = msgrcv( ... , ... , MSG_INFO_SIZE, 0);
        if (msgStatus == -1)
        {
            printf("Failed to receive message when %d factories running\n", activeFactories);
        }
        // validate the message 
        if (incomingMsg.purpose == PRODUCTION_MSG)
        {
            printf("Factory %3d produced %5d parts in %4d miliSecs\n", ..., ..., ...);
            // update per-factory productions aggregates (num parts built, num-iterations)
        } elsif ( terminated )
        {
            printf("Factory %3d Terminated\n", ...);
        } else 
        {
            // discard unsupported message 
        }
        

    }

    // synchronize with Sales through NAMED semaphore
    
    // inform the Sales that manufacturing is done
    // wait for permission from Sales to print final report
    // print per-factory production aggregates sorted by factoryID.



    // CLOSE LOG!! 
    fclose(log);

}