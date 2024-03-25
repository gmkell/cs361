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
#include <cstdlib>


#include "shmSegment.h"
#include "shmem.h"
#include "wrappers.h"
#include "message.h"

int main(int argc, char* argv[])
{
    // set up synchronization mechanisms


    if (argv[1] > MAXFACTORIES) 
    {
        printf("%d exceeded maximum number of factory lines allowed. \n", argv[0]);
        exit(EXIT_FAILURE); 
    }
    
    int num_factories = argv[0]; //specs say argv[1], but only 2 args passed 
    int num_parts = argv[1];
    // create & initialize shmem
    int shmID;
    key_t key;
    pid_t mypid;
    shmData *data;

    // create shmkey by converting pathname to an IPC key 
    if (key = ftok("shmSegment.h", 'R') == -1)
    {
        perror("ftok key");
        exit(EXIT_FAILURE);  
    }

    // create shmem segment 
    if ( (shmID = shmget(key, SHMEM_SIZE, IPC_CREAT | IPC_EXCL)) == -1 )
    {
        perror("shmget");
        fprintf(stderr, "Error code: %d", errno);
    }

    // say what shmat does
    data = (shmData *) shmat(shmID, NULL, 0);
    if (data == (void *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    mypid = getpid(); // the sales process pid

    // set up msgQue
    msgBuf msgQue;
    int msgStatus; 
    
    // create num_factories + 1 child processes based on num_lines (+1 for the Supervisor child process)
    while(i < num_factories + 1)
    {
        fork();
        i++;
    }

    // create reandevous semaphore for sales and supervisor 



}