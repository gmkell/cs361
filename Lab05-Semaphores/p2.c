/*-------------------------------------------------------------------------------
Demo: IPC using Shared Memory
Written By:
     1- Dr. Mohamed Aboutabl

Group 4:
     2 - Gillian Kelly
     3 - Ethan Pae 

     The P2 Process:    p2.c

-------------------------------------------------------------------------------*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>

#include "shmSegment.h"
#include "wrappers.h"

#define SEM_MUTEX_NAME "/sem_mutex"
#define SEM_P1_STARTED "/sem_p1_started"
#define SEM_P2_STARTED "/sem_p2_started"
#define SEM_P1_FINISHED "/sem_p1_finished"
#define SEM_P2_FINISHED "/sem_p2_finished"

int main(int argc, char *argv[])
{
     int shmid;
     key_t shmkey;
     int shmflg;
     shmData *p;

     shmkey = ftok("shmSegment.h", 5);
     shmflg = S_IRUSR | S_IWUSR; // No IPC_CREAT, so P2 must run after P1

     shmid = shmget(shmkey, SHMEM_SIZE, shmflg);
     if (shmid == -1)
     {
          printf("\nP2 Failed to get shared memory id=%d\n", shmid);
          perror("Reason: ");
          exit(-1);
     }

     p = (shmData *)shmat(shmid, NULL, 0); // Attach for R/W
     if (p == (shmData *)-1)
     {
          printf("\nP2 Failed to attach shared memory id=%d\n", shmid);
          perror("Reason: ");
          exit(-1);
     }

     sem_t *mutex = Sem_open(SEM_MUTEX_NAME, 0, 0644, 0);
     sem_t *p1Started = Sem_open(SEM_P1_STARTED, 0, 0644, 0);
     sem_t *p2Started = Sem_open(SEM_P2_STARTED, O_CREAT, 0644, 0);
     sem_t *p1Finished = Sem_open(SEM_P1_FINISHED, 0, 0644, 0);
     sem_t *p2Finished = Sem_open(SEM_P2_FINISHED, O_CREAT, 0644, 0);

     Sem_post(p2Started);

     printf("P2 started. MANY = %10d\n", LARGEINT);
     printf("Waiting for P1  to start, too.\n");

     Sem_wait(p1Started);

     printf("P2 now will increment the counter\n");

     Sem_wait(mutex);
     for (unsigned i = 1; i <= LARGEINT; i++)
          p->counter++;
     Sem_post(mutex);

     printf("P2 is done. Waiting for P1 to finish, too.\n");
     Sem_post(p2Finished);

     Sem_wait(p1Finished);

     unsigned long expected = LARGEINT << 1; // 2*MANY
     printf("P2 reports final counter value = %10u  Expecting: %10lu", p->counter, expected);

     if (p->counter == expected)
          printf("    CORRECT\n");
     else
          printf("    NOT CORRECT\n");

         // Cleanup
    shmdt(p);

    Sem_close(mutex);
    Sem_close(p1Started);
    Sem_close(p2Started);
    Sem_close(p1Finished);
    Sem_close(p2Finished);

    return 0;
}