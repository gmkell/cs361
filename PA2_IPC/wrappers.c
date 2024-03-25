/************************************************
 * Wrappers for system call functions
 ************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "wrappers.h"

/************************************************
 * Unix vs Posix Error Handling Functions
 ************************************************/
void unix_error(char *msg) /* unix-style error */
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(-1);
}

void posix_error(int code, char *msg) /* posix-style error */
{
    fprintf(stderr, "%s: %s\n", msg, strerror(code));
    exit(-1);
}

//------------------------------------------------------------
// To be called after a function that sets errno 

void err_sys( const char* x ) 
{ 
    fflush( stderr ) ;
    perror( x ); 
    exit( -1 ); 
}

//------------------------------------------------------------
// To be called after a function that DOES NOT set errno 

void err_quit( const char* x ) 
{ 
    fflush( stderr ) ;
    fputs( x , stderr ) ; 
    exit( -1 ); 
}

//----------------------------------------------
/* A wrapper for the fork() slow system call. */

pid_t Fork(void)
{
    pid_t n;
    n = fork() ;
    if ( n < 0 )
        err_sys( "Fork failed" );

    return n ;
}

//------------------------------------------------------------
/* A wrapper for the usleep() slow system call. 
   If interrupted by a signal then retry, otherwise error   */

int Usleep( useconds_t usec )
{
	int		n;
	while ( ( n = usleep( usec ) ) < 0 ) 
	{
		if ( errno == EINTR )
            continue ; 
		else
            err_sys( "usleep() error" ); 

	}
    
	return( n );
}

//------------------------------------------------------------

int  Shmget( key_t key, size_t size, int shmflg )
{
    int   shmid ;
    char  buf[100] ;

    shmid = shmget( key , size , shmflg ) ;
    if ( shmid == -1 ) 
    {
        snprintf ( buf, 100 , "Failed to get shared memory key=%d" , key );
        err_sys( buf ) ;
    }
    
    return shmid ;
}

//------------------------------------------------------------

void *Shmat( int shmid, const void *shmaddr, int shmflg )
{
    void *p ;
    char  buf[100] ;

    p = shmat( shmid , shmaddr , shmflg );
    if ( p == (void *) -1 ) 
    {
        snprintf ( buf , 100 , "Failed to attach shared memory id=%d" , shmid );
        err_sys( buf ) ;
    }

    return p;    
}

//------------------------------------------------------------

int  Shmdt( const void *shmaddr )
{
    int code ;

    code = shmdt( shmaddr );
    if ( code != 0  ) 
        err_sys( "shmdt Failed" ) ;

    return code ;    
}

//------------------------------------------------------------

sem_t  *Sem_open( const char *name, int oflag, mode_t mode, unsigned int value )
{
    sem_t  *s;
    char  buf[100] ;

    s = sem_open( name , oflag , mode , value );
    if ( s == SEM_FAILED ) 
    { 
        snprintf ( buf , 100 , "Failed to open semaphore '%s'" , name );
        err_sys( buf ) ; 
    }

    return s;
}

//------------------------------------------------------------

sem_t  *Sem_open2( const char *name, int oflag )
{
    sem_t  *s;
    char  buf[100] ;

    s = sem_open( name , oflag );
    if ( s == SEM_FAILED ) 
    { 
        snprintf ( buf , 100 , "Failed to open semaphore '%s'" , name );
        err_sys( buf ) ; 
    }

    return s;
}

//------------------------------------------------------------

int  Sem_close( sem_t *sem )
{
    int code ;

    code = sem_close( sem ) ;
    if ( code != 0 )
        err_sys( "sem_close failed" );

    return code ;    
}

//------------------------------------------------------------

int  Sem_unlink( const char *name )
{
    int code ;
    char  buf[100] ;

    code = sem_unlink( name ) ;
    if ( code != 0 )
    {
        snprintf ( buf , 100 , "Failed to unlink semaphore '%s'" , name );
        err_sys( buf ) ; 
    } 

    return code ;    
}

//------------------------------------------------------------

int  Sem_wait( sem_t *sem ) 
{
    int code ;

    code = sem_wait(sem) ;
    if ( code != 0 )
        err_sys( "sem_wait failed" );

    return code ;
}

//------------------------------------------------------------

int  Sem_post( sem_t *sem ) 
{
    int code ;

    code = sem_post(sem)  ;
    if ( code != 0 )
        err_sys( "sem_post failed" );

    return code ;
}

//------------------------------------------------------------

int  Sem_init( sem_t *sem, int pshared, unsigned int value ) 
{
    int code ;
    
    code = sem_init( sem, pshared, value ) ;
    if ( code != 0 )
        err_sys( "sem_init failed" );

    return code ;
}

//------------------------------------------------------------

int  Sem_destroy( sem_t *sem ) 
{
    int code ;

    code = sem_destroy( sem ) ;
    if ( code < 0)
        err_sys( "sem_destroy error" );

    return code ;
}

//------------------------------------------------------------

int   Msgget( key_t key, int msgflg )
{
    int code ;
    char  buf[100] ;

    code = msgget( key , msgflg ) ;
    if ( code == -1 )
    {
        snprintf ( buf , 100 , "Failed to get Msg queue with key=%d" , key );
        err_sys( buf ) ; 
    } 

    return code ;       
}

//------------------------------------------------------------
//  Wrapper for sigaction 

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

void Pthread_create( pthread_t *tidp, pthread_attr_t *attrp, 
		             void * (*routine)(void *), void *argp ) 
{
    int rc;

    if ( ( rc = pthread_create( tidp, attrp, routine, argp) ) != 0 )
	    posix_error( rc, "Pthread_create error" );
} 

//------------------------------------------------------------

void Pthread_join( pthread_t tid, void **thread_return ) 
{
    int rc;
 
    if ( ( rc = pthread_join( tid, thread_return ) ) != 0 )
        posix_error( rc, "Pthread_join error" );
}


//------------------------------------------------------------

void Pthread_detach( pthread_t tid ) 
{
    int rc;

    if ( ( rc = pthread_detach( tid ) ) != 0 )
        posix_error( rc, "Pthread_detach error" );
}
