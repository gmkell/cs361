/*-------------------------------------------------------------------------------
Demo: IPC using Shared Memory
Written By:
     1- Dr. Mohamed Aboutabl

     The P1 Process:    p1.c

-------------------------------------------------------------------------------*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <semaphore.h>

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
     shmflg = IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR; // So, P1 must run before P2

     shmid = shmget(shmkey, SHMEM_SIZE, shmflg);
     if (shmid == -1)
     {
          printf("\nP1 Failed to get shared memory id=%d\n", shmid);
          perror("Reason: ");
          exit(-1);
     }

     p = (shmData *)shmat(shmid, NULL, 0); // Attach for R/W
     if (p == (shmData *)-1)
     {
          printf("\nP1 Failed to attach shared memory id=%d\n", shmid);
          perror("Reason: ");
          exit(-1);
     }

     sem_t *mutex = Sem_open(SEM_MUTEX_NAME, O_CREAT, 0644, 1);
     sem_t *p1Started = Sem_open(SEM_P1_STARTED, O_CREAT, 0644, 0);
     sem_t *p2Started = Sem_open(SEM_P2_STARTED, O_CREAT, 0644, 0);
     sem_t *p1Finished = Sem_open(SEM_P1_FINISHED, O_CREAT, 0644, 0);
     sem_t *p2Finished = Sem_open(SEM_P2_FINISHED, O_CREAT, 0644, 0);

     Sem_post(p1Started);

     /* Initialize data in the shared memory */
     p->counter = 0;

     printf("P1 started. MANY = %10d\n", LARGEINT);

     Sem_wait(mutex);
     printf("P1 now will increment the counter\n");
     for (unsigned i = 1; i <= LARGEINT; i++)
          p->counter++;
     Sem_post(mutex);

     Sem_post(p1Finished);

     printf("P1 is done. Waiting for P2 to finish, too.\n");
     Sem_wait(p2Finished);

     unsigned long expected = LARGEINT << 1; // 2*MANY

     printf("P1 reports final counter value = %10u  Expecting: %10lu", p->counter, expected);

     if (p->counter == expected)
          printf("    CORRECT\n");
     else
          printf("    NOT CORRECT\n");

     shmdt(p);
     shmctl(shmid, IPC_RMID, NULL);

     Sem_close(mutex);
     Sem_close(p1Started);
     Sem_close(p2Started);
     Sem_close(p1Finished);
     Sem_close(p2Finished);
     Sem_unlink(SEM_MUTEX_NAME);
     Sem_unlink(SEM_P1_STARTED);
     Sem_unlink(SEM_P2_STARTED);
     Sem_unlink(SEM_P1_FINISHED);
     Sem_unlink(SEM_P2_FINISHED);

     return 0;
}
