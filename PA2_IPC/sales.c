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

#include "shmSegment.h"
#include "shmem.h"
#include "wrappers.h"
#include "message.h"
#include <cstdlib>

int main(int argc, char* argv[])
{
    // set up synchronization mechanisms
    if (atoi(argv[1]) > MAXFACTORIES) 
    {
        printf("%d exceeded maximum number of factory lines allowed. \n", argv[1]);
        exit(EXIT_FAILURE); 
    }
    
    int num_factories = atoi(argv[1]);

    int num_parts = atoi(argv[2]);

    printf("SALES: Will Request an Order of Size = %d parts\n", num_parts);

    // Step 1: set up shared mem & initialize its objects
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

    data = (shmData *) shmat(shmID, NULL, 0);
    if (data == (void *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    mypid = getpid(); // the sales process pid

    // Step 2: set up message queue and semaphores
    msgBuf msgQue;
    int msgStatus; 

    // create reandevous semaphore for sales and supervisor 
    // sales waits via semaphore for supervisor to finish

    // Step 3: Fork/Execute Supervisor process    

    // Step 4: Fork/Execute all factory processes
    printf("Creating %d Factory(ies)", num_factories);

    // seed random number generator only ONCE
    srandom(time(NULL));

    // create num_factories child processes based on num_factories
    for (int i = 0; i < num_factories; i++)
    {   
        // Step 4.1 create the factory arguments using random()
        int capacity = (int) random() % (50 - 10 + 1) + 10;
        int duration = (int)random() % (1200 - 500 + 1) + 500; ; // in miliseconds 

        pid_t factory_pid = fork();
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
            sprintf(factory_id_str, "%d", factory_pid); 
            
            // Step 4.2: redirect all factory process stdout to 'factory.log'
            int factory_log = open("factory.log", O_WRONLY, O_CREAT, O_TRUNC, 0666);
            if (factory_log == -1)
            {
                perror("Failed to open factory.log\n");
                exit(EXIT_FAILURE);
            } 
            dup2(factory_log, STDOUT_FILENO);
            close(factory_log); // no need to keep open

            // Step 4.3: send the arguments to the processes via execlp()
            execlp("./factory", "factory", factory_id_str, capacity_str, duration_str, (char*)NULL);
            perror("excelp factory\n");
            exit(EXIT_FAILURE);

        }
        // Step 4.3 Parent process: continue to next iteration to fork next factory
        printf("SALES: Factory #   %d was created, with Capacity=   %d  and Duration=   %d",i , capacity, duration);
    }
    // handle critical selection made by redirection of stdout using synchronization method

    // Step 5: Wait for supervisor to indicate manufacturing is done
    // sales waits via semaphore for supervisor to finish
    
    printf("SALES: Supervisor says all Factories have completed their mission\n");

    // Step 6: Grant supervisor permission to print the Final Report
    printf("SALES: Permission granted to print final report\n");

    // Step 7: Clean up zombie processes (Supervisior + all Factories)
    printf("SALES: Cleanign up after the Supervisor Factory Process\n");
    
    // Step 8: destroy shmem

    // Step 9: destroy semaphores and message queue


}