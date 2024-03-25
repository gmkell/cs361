//----------------------------------------------------------------------------
// Assignment   :   PA2 IPC
// Date         :   3/25/24
// 
// Group 6      : Gillian Kelly,   Ethan Pae          
//
// Supervisor   :   factory.c
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

int main(int argc, char* argv[])
{
    // get variables from command line and create others 
    int capacity;
    int duration;
    int factory_ID = getpid();
    
    // open message queue to be able to send messages to Supervisor 
    int myMailboxID, supervisorMailboxID;
    int msgStatus;
    msgBuf factoryMsg;

    key_t factoryKey, supervisorKey;
    char *supervisorPath;
    



    // factory production 
    int iterations, parts_made = 0; // initialize variables 
    int remain = capacity; // number of parts still needing to be made 
    while ( remain > 0 ) 
    {
        how_many = Math.min(remain, capacity);
        // update remain 
        remain = remain - how_many;
        // create and send message to factory.log
        printf("Factory #%3d: Going to make %5d parts in %4d milliSecs", factory_ID, how_many, duration); // same duration even if I make less than my capacity
        // create & send production message to Supervisior
        char somestring = "Factory # %d produced %d parts in %3d milliSecs", factory_ID, how_many, duration;
        factoryMsg.purpose = PRODUCTION_MSG;
        msgStatus = msgsnd(supervisorMailboxID, &factoryMsg, MSG_INFO_SIZE, 0);
        // catch error with sending message to supervisor 
        if (msgStatus == -1)
        {
            printf("Factory # %3d failed to send production info to supervisor. Errorcode=%d\n", factory_ID, errno);
            perror("Reason");
            exit (EXIT_FAILURE);
        }
        // increment iterations
        iterations++;
        // update my record of total # parts made
        parts_made += how_many;
    }

    // send a COMPLETION_MSG to the Supervisior once there are no more parts to make
    factoryMsg.purpose = COMPLETION_MSG;
    
}