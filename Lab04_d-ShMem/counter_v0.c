/*-------------------------------------------------------------------------------
IPC using Shared Memory & Signals -- Counter Version 00
Written By: 
     1- Dr. Mohamed Aboutabl
-------------------------------------------------------------------------------*/

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include  <signal.h>
#include <stdbool.h>

#include "shmSegment.h"

bool FOREVER = true ;

//------------------------------------------------------------
/* Wrapper for sigaction */

typedef void Sigfunc( int ) ;

Sigfunc * sigactionWrapper( int signo, Sigfunc *func )
// 'signo' specifies the signal and can be any valid signal 
//  except SIGKILL and SIGSTOP.

{
	struct sigaction	act, oact;

	act.sa_handler = func;
	sigemptyset( &act.sa_mask );
	act.sa_flags   = 0;

	if( sigaction( signo, &act, &oact ) < 0 )
		return( SIG_ERR );

	return( oact.sa_handler );
}

//------------------------------------------------------------
void sigHandler_A( int sig ) 
{
    fflush( stdout ) ;
    printf("\n\n### I (%d) received Signal #%3d.\n\n"
           , getpid() , sig );  
}

//------------------------------------------------------------
void sigHandler_B( int sig ) 
{
    fflush( stdout ) ;
    printf("\n\n### I (%d) received Signal #%3d.\n\n"
           , getpid() , sig );  
}

//------------------------------------------------------------
void sigHandler_CONT( int sig ) 
{
    fflush( stdout ) ;
    printf("\n\n### I (%d) have been asked to RESUME by Signal #%3d.\n\n"
           , getpid() , sig );  
}

//------------------------------------------------------------

int main()
{
    int shmID;
    key_t key;
    pid_t  mypid ;

    if ( (key = ftok("shmSegment.h", 'R')) == -1 )
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    if ( (shmID = shmget(key, SHMEM_SIZE, IPC_CREAT | 0666)) == -1 )
    {
        perror("shmget");
        fprintf(stderr, "Error code: %d\n", errno);
    }

    data = (shmData *) shmat(shmID, NULL, 0);
    if (data == (void*) -1)
    {
        data -> num2 = atoi(argv[2]);
        data -> ratio = 0;
    }

    mypid = getpid() ;
    printf("\nHELLO! I AM THE COUNTER PROCESS WITH ID= %d\n" , mypid );
      
    // Set up Signal Catching here
    sigactionWrapper(SIGCONT, sigHandler_CONT);
    sigactionWrapper(SIGTSTP, sigHandler_A);

    unsigned i=0;
    while(  FOREVER )
        printf("%10X\r" , i++ ) ;

    printf("\nCOUNTER: Stopped Counting.\n\n");
    printf("\nCOUNTER: Goodbye\n\n");

    if (shmdt(data) == -1)
    {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
    
	return 0 ;

}
