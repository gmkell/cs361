/*-------------------------------------------------------------------------------
IPC using Shared Memory & Signals Divider Version 00
Written By: 
     1- Dr. Mohamed Aboutabl
-------------------------------------------------------------------------------*/

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include  <signal.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include "shmSegment.h"

int main()
{

    mypid      = getpid() ;

    printf("\n\nHELLO! I am the newly-born CHILD ID= %d\n" , mypid );

    // Awaken my Parent

    return 0;
}
