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
#include <sys/msg.h>

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

pid_t Fork(void)
{
    pid_t n;
    n = fork() ;
    if ( n < 0 )
    {        
        perror("Fork failed");
        exit( -1 ) ;
    }
    return n ;
}

/************************************************
 * Wrapper for sigaction 
  ************************************************/

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

/************************************************
 * Wrappers for Pthreads thread control functions
 ************************************************/
void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp , 
		            void * (*routine)(void *), void *argp) 
{
    int rc;

    if ((rc = pthread_create(tidp, attrp, routine, argp)) != 0)
        posix_error(rc, "Pthread_create error");
}

void Pthread_cancel(pthread_t tid) 
{
    int rc;

    if ((rc = pthread_cancel(tid)) != 0)
        posix_error(rc, "Pthread_cancel error");
}

void Pthread_join(pthread_t tid, void **thread_return) 
{
    int rc;

    if ((rc = pthread_join(tid, thread_return)) != 0)
        posix_error(rc, "Pthread_join error");
}

void Pthread_detach(pthread_t tid) 
{
    int rc;

    if ((rc = pthread_detach(tid)) != 0)
        posix_error(rc, "Pthread_detach error");
}


pthread_t Pthread_self(void) 
{
    return pthread_self();
}

void Pthread_exit(void *retval) 
{
    pthread_exit(retval);
}

/*******************************
 * Wrappers for Posix semaphores
 *******************************/

void Sem_init(sem_t *sem, int pshared, unsigned int value) 
{
    if (sem_init(sem, pshared, value) < 0)
        unix_error("Sem_init error");
}

int  Sem_wait(sem_t *sem) 
{
    int code ;

    code = sem_wait(sem) ;
    if ( code != 0 )
        unix_error("Sem_wait error");
    return code ;
}

int  Sem_post(sem_t *sem) 
{
    int code ;

    code = sem_post(sem)  ;
    if ( code != 0 )
        unix_error("Sem_post error");

    return code ;
}

int  Sem_destroy(sem_t *sem) 
{
    int code ;

    code = sem_destroy( sem ) ;
    if ( code < 0)
        unix_error("Sem_destroy error");
    return code ;
}

sem_t  *Sem_open(const char *name, int oflag, mode_t mode, unsigned int value)
{
    sem_t  *s;
    char  buf[100] ;

    s = sem_open( name , oflag , mode , value );
    if ( s == SEM_FAILED ) 
    { 
        snprintf ( buf , 100 , "Sem_open failed to open semaphore '%s'" , name );
        unix_error( buf ) ;
    }
    return s;
}

sem_t  *Sem_open2( const char *name, int oflag )
{
    sem_t  *s;
    char  buf[100] ;

    s = sem_open( name , oflag );
    if ( s == SEM_FAILED ) 
    { 
        snprintf ( buf , 100 , "Sem_open2 failed to open semaphore '%s'" , name );
        unix_error( buf ) ;
 
    }
    return s;
}

int  Sem_close(sem_t *sem)
{
    int code ;

    code = sem_close( sem ) ;
    if ( code != 0 )
        unix_error( "Sem_close failed" );
    return code ;    
}

int  Sem_unlink( const char *name )
{
    int code ;
    char  buf[100] ;

    code = sem_unlink( name ) ;
    if ( code != 0 )
    {
        snprintf ( buf , 100 , "Failed to unlink semaphore '%s'" , name );
        unix_error( buf );
    } 
    return code ;    
}

/******************************************
 * Wrappers for System V Message Queues
 ******************************************/

int   Msgget( key_t key, int msgflg )
{
    int code ;
    char  buf[100] ;

    code = msgget( key , msgflg ) ;
    if ( code == -1 )
    {
        snprintf ( buf , 100 , "Failed to get Msg queue with key=%d" , key );
        perror( buf ) ; 
        exit( -1 ) ;
    } 
    return code ;    
    
}

/******************************************
 * Wrappers for System V Shared Memory
 ******************************************/

int  Shmget(key_t key, size_t size, int shmflg)
{
    int   shmid ;
    char  buf[100] ;

    shmid = shmget( key , size , shmflg ) ;
    if ( shmid == -1 ) 
    {
        snprintf ( buf, 100 , "Failed to get shared memory key=%d" , key );
        perror( buf ) ;
        exit( -1 ) ;
    }
    return shmid ;
}

void *Shmat(int shmid, const void *shmaddr, int shmflg)
{
    void *p ;
    char  buf[100] ;

    p = shmat( shmid , shmaddr , shmflg );
    if ( p == (void *) -1 ) 
    {
        snprintf ( buf , 100 , "Failed to attach shared memory id=%d" , shmid );
        perror( buf ) ;
        exit( -1 ) ;
    }
    return p;    
}

int  Shmdt( const void *shmaddr )
{
    int code ;

    code = shmdt( shmaddr );
    if ( code != 0  ) 
    {
        perror( "shmdt Failed" ) ;
        exit( -1 ) ;
    }
    return code ;    
}

