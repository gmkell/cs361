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
#include <stdlib.h>

#include "shmem.h"
#include "wrappers.h"
#include "message.h"

// #define MSG_KEY 0x5678
#define SEM_FACTORY_FINISHED "/sem_factory_finished"
#define SEM_MUTEX_NAME "/sem_mutex"

int determine_parts_to_make(int capacity, int remain);
int smaller_of(int remain, int capacity, shData* shm_data);

int main(int argc, char* argv[])
{
    // Step 1: re-attach shared memory segment
    key_t shmKey = ftok("shmem.h", 'R');
    shData *shm_data;
    if (shmKey == -1) 
    {
        perror("Factory could not re-establish connection to shared memory");
        exit(EXIT_FAILURE);
    }

    int shmID = Shmget(shmKey, SHMEM_SIZE, 0);
    if (shmID == -1)
    {
        perror("shmget in factory");
        fprintf(stderr, " Error code: %d", errno);
    }

    shm_data = (shData *)Shmat(shmID, NULL, 0);
    if (shm_data == (shData*) -1)
    {
        perror("shmat() in factory.c");
        exit(EXIT_FAILURE);
    }
    
    sem_t *factoryFinished = Sem_open(SEM_FACTORY_FINISHED, O_CREAT, 0644, 0);
    sem_t *mutex = Sem_open(SEM_MUTEX_NAME, O_CREAT, 0644, 0);

    // Step 2: get variables from command line and create others 
    int factory_ID = atoi(argv[1]);
    int capacity = atoi(argv[2]);
    int duration = atoi(argv[3]);
    key_t msgKey = (key_t) atoi(argv[4]);
    int remain = shm_data->remain;
    int iterations = 0;
    int parts_made_by_me = 0;

    // Step 3: re-attach message queue
    int mailbox;
    msgBuf msg;
    int msgFlgs = S_IWUSR;
    int msgID = Msgget(msgKey, msgFlgs);
    if (mailbox == -1)
    {
        perror("failed to open message queue in factory.c");
        exit(EXIT_FAILURE);
    } 
    // Step 4: open factory.log in append mode so that processes don't overwrite each other
    FILE *factory_log = fopen("factory.log", "a");
    if (factory_log == NULL)
    {
        perror("failed to open factory.log in Supervisor");
        exit(EXIT_FAILURE);
    }

    if (freopen("factory.log", "a", stdout) == NULL)
    {
        perror("freopen() failed for factory.log");
        exit(EXIT_FAILURE);
    }

    // Step 5: Factory Process
    printf("Factory #   %d STARTED. My capacity is =    %d parts, in   %4d milliSeconds\n", factory_ID, capacity, duration);
    fflush(stdout);

    while (remain > 0)
    {
        // determine how many parts to make
        int parts_to_make = determine_parts_to_make(capacity, remain);
        remain = remain - parts_to_make; // update remain
        //shm_data->remain = remain - parts_to_make;
        //shm_data->remain = remain;
        if (parts_to_make > 0 && (parts_made_by_me + shm_data->made + parts_to_make <= shm_data->order_size))
        {
            printf("Factory #   %d: Going to make %5d parts in %4d milliSecs\n", factory_ID, parts_to_make, duration);
            fflush(stdout); 
            Usleep(duration * 1000); // Simulate production time in microseconds
            // Create & send production message to supervisor
            msg.facID = factory_ID;
            msg.purpose = PRODUCTION_MSG;
            msg.mtype = 1;
            msg.capacity = capacity;
            msg.partsMade = parts_to_make;
            msg.duration = duration;
            msgsnd(msgID, &msg, MSG_INFO_SIZE, 0);
            iterations++; // increment iterations 
            // update factory record of total # parts made so far
            parts_made_by_me += parts_to_make;
        } else {
            break;
        }
    }

    //shm_data->made += parts_made_by_me; 

    // Create & send completion message to Supervisor
    msg.facID = factory_ID;
    msg.purpose = COMPLETION_MSG;
    msg.capacity = capacity;
    msg.partsMade = parts_made_by_me;
    msg.duration = duration;
    msgsnd(msgID, &msg, MSG_INFO_SIZE, 0);
    
    printf("Factory #   %d: Terminating after making a total of    %d parts in      %d iterations\n",
        factory_ID, parts_made_by_me, iterations);
    fflush(stdout);

    Sem_post(mutex); 
    
    // Cleanup and detach shared memory
    Sem_post(factoryFinished);
    Shmdt(shm_data);

    fclose(factory_log);

    return EXIT_SUCCESS;
}

// helper function to determine number of parts to make based on 
// the capacity of the factory and the remaining parts to make
int determine_parts_to_make(int capacity, int remain) {
    int parts_to_make = remain < capacity ? remain : capacity;
    if (remain < parts_to_make) {
        parts_to_make = remain;
    }
    return parts_to_make;
}

// factory determine whether it needs to make more parts
// factory process must not make more parts than what is needed
// nor exceed its full capacity to the concurrent making of the parts
int smaller_of(int remain, int capacity, shData* shm_data)
{
    int parts_to_make = 0; // initialize to zero in case no parts needed
    // check how many total parts have been made, compare with order_size and remain
    if (shm_data->made == shm_data->order_size)
    {
        parts_to_make = 0;
    }

    return parts_to_make;

}


