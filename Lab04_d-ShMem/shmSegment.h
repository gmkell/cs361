/*-------------------------------------------------------------------------------
IPC using Shared Memory & Signals
Written By: 
     1- Dr. Mohamed Aboutabl
-------------------------------------------------------------------------------*/

#ifndef SHMEM_SEGMENT
#define SHMEM_SEGMENT


typedef struct {
    int    num1 , num2 ;
    double ratio ;
} shmData ;

#define SHMEM_SIZE sizeof(shmData)

#endif
