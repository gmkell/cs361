// factory.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <time.h>
#include "shmem.h" // Include the shared memory header
#include <semaphore.h>
#include <fcntl.h> // For O_CREAT, O_EXEC, etc.

#define MSG_KEY 0x5678
#define SHM_KEY 0x9ABC

// Message structure for message queue
struct msgbuf {
    long mtype;     // Message type
    int factory_id; // Factory ID
    int capacity;   // Factory capacity
    int parts_made; // Number of parts made in the iteration
    int duration;   // Duration it took to make the parts
};

int determine_parts_to_make(int capacity, shData *shm_data);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <factory_ID> <capacity> <duration>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int factory_ID = atoi(argv[1]);
    int capacity = atoi(argv[2]);
    int duration = atoi(argv[3]);

    // Attach to shared memory
    int shm_id = shmget(SHM_KEY, SHMEM_SIZE, 0666);
    if (shm_id < 0) {
        perror("shmget");
        exit(1);
    }
    shData *shm_data = (shData *)shmat(shm_id, NULL, 0);
    if (shm_data == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    // Message queue identifier
    int msgid;
    struct msgbuf msg;

    msg.mtype = 1; // This should correspond to the message type for a production update
    msg.factory_id = factory_ID;
    msg.capacity = capacity;
    msg.parts_made = shm_data->made;
    msg.duration = duration;


    // Set up the message queue
    msgid = msgget(MSG_KEY, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget failed");
        exit(EXIT_FAILURE);
    }

    // Retrieve order size from shared memory
    int order_size = shm_data->order_size;
    int parts_made_so_far = shm_data->remain;  // This factory's count of parts made

    // Main loop to simulate part production
    int count = 0;
    while (1) {
        // Determine how many parts to make in this iteration
        // This part of logic should come from the shared memory or some IPC mechanism
        // For the sake of the example, let's assume we have a function that determines it
        int parts_to_make = determine_parts_to_make(capacity, shm_data); 

        if (parts_to_make > 0) {
            printf("Factory #%d: Going to make %d parts in %d milliseconds\n",
                   factory_ID, parts_to_make, duration);
            sleep(duration / 1000); // Simulating production time

            // Populate the message with the production details
            msg.mtype = 1; // Message type should be set according to your design
            msg.factory_id = factory_ID;
            msg.capacity = capacity;
            msg.parts_made = parts_to_make;
            msg.duration = duration;

            // Send a production message to the supervisor
            if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }

            parts_made_so_far += parts_to_make;

            // Check if all parts have been made
            if (parts_made_so_far >= order_size) {
                // Send a completion message to the Supervisor
                break;
            }
        } else {
            // No more parts to make, send a completion message to the supervisor
            msg.mtype = 2; // Adjust message type for completion
            msg.factory_id = factory_ID;
            msg.capacity = 0;
            msg.parts_made = 0;
            msg.duration = 0;

            if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
            break; // Break out of the loop to terminate the process
        }
        count++;
    }

    // Cleanup and exit
    printf("Factory #%d: Terminating after making a total of %d parts in %d iterations\n",
           factory_ID, parts_made_so_far, count);
    shmdt(shm_data);
    return EXIT_SUCCESS;
}

int determine_parts_to_make(int capacity, shData *shm_data) {
    // Determine the number of parts to make in this iteration
    // It should be the minimum of the parts needed and the factory's capacity
    int parts_to_make = shm_data->remain < capacity ? shm_data->remain : capacity;

    // Ensure we do not produce more than what is required
    if (shm_data->remain < parts_to_make) {
        parts_to_make = shm_data->remain;
    }

    // Update the remaining parts, needs to be done atomically or with proper locking
    shm_data->remain -= parts_to_make;

    return parts_to_make;
}

