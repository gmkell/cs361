/************************************************
 * Wrappers for system call functions
 ************************************************/
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <signal.h>

void    unix_error(char *msg) ;
void    posix_error(int code, char *msg) ;

pid_t   Fork(void);

int     Msgget( key_t key, int msgflg );

int     Shmget(key_t key, size_t size, int shmflg);
void   *Shmat(int shmid, const void *shmaddr, int shmflg);
int     Shmdt( const void *shmaddr ) ;

void    Pthread_create(pthread_t *tidp, pthread_attr_t *attrp , void * (*routine)(void *), void *argp) ;
void    Pthread_cancel(pthread_t tid) ;
void    Pthread_join(pthread_t tid, void **thread_return) ;
void    Pthread_detach(pthread_t tid) ;
void    Pthread_exit(void *retval);
pthread_t   Pthread_self(void) ;

void    Sem_init(sem_t *sem, int pshared, unsigned int value) ;
int     Sem_wait(sem_t *sem);
int     Sem_post(sem_t *sem) ;
int     Sem_destroy(sem_t *sem) ;
sem_t  *Sem_open(const char *name, int oflag, mode_t mode, unsigned int value);
sem_t  *Sem_open2( const char *name, int oflag );
int     Sem_close(sem_t *sem);
int     Sem_unlink( const char *name );

typedef void Sigfunc( int ) ;

Sigfunc * sigactionWrapper( int signo, Sigfunc *func ) ;
