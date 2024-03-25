/*-------------------------------------------------------------------------------
    IPC using Shared Memory. 
    Protected & Synchronized by Un-Named Semaphore
Written By: 
     1- Dr. Mohamed Aboutabl
-------------------------------------------------------------------------------*/

#ifndef SHMEM_SEGMENT
#define SHMEM_SEGMENT

typedef struct {
    unsigned  counter ;
    sem_t     mutex ,  // Mutex to protect the 'counter'
    
              leftRdyToCnt  , leftDone  , // For Rendezvous
              
              rightRdyToCnt , rightDone ;
} shmData ;

#define SHMEM_SIZE sizeof( shmData )

#define LARGEINT   6000000L

#endif

