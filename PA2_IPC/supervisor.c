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

    // open the message queue

    // synchronize with Sales through NAMED semaphore



    // CLOSE LOG!! 
    fclose(log);

}