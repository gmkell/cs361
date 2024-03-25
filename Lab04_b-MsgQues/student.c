/*----------------------------------------------------------------------
Demo of IPC using Message Queues:  The Student process

Written By: 
    1- Put Your Full Name Here
    
Submitted on: mm/dd/yyyy
----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/stat.h>

#include "message.h"

// Replace with your name
#define  MYNAME   "Gillian Kelly"

/* -------------------------------------------------------------------*/
int main ( int argc , char * argv[] )
{

    msgBuf  studMsg ,   // used for sending 
            instMsg ;   // used for receiving
    int     myMailboxID , mypid , instructorMailboxID;
    int     msgStatus ;

    mypid = getpid() ;
   
    printf( "\nThis is " MYNAME " (process id = %d).\t\n" , mypid );

    /*
        Create a private mailbox for Student to receive messages from the world
        You must be the ONLY one who can pick up messages from this mailbox.
        However, everyone else can send you messages at this mailbox
        MUST use ftok() to generate a unique KEY for this mailbox
    */

    key_t studentKey;
    char *studentPath = "/cs/home/kellygm/cs361/Lab04_b-MsgQues/";
    studentKey = ftok(studentPath, mypid);

    int msgflg  =  IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IWOTH  ;
	myMailboxID =  msgget(studentKey, msgflg | IPC_CREAT); /*  Create the message queue */ ; 
    if ( myMailboxID == -1 ) 
    {
        printf( MYNAME " Failed to create a private mailbox. Error code=%d\n" , errno ) ;
        perror("Reason");
        exit( EXIT_FAILURE ) ;  
    }
    else
        printf( MYNAME " Created a private mailbox with qid=%d\n" , myMailboxID ) ;
    
    /* Find the key of the message queue already created by the Instructor process */
    key_t   instructorKey ;
    char *instructorPath = "/cs/home/stu-f/aboutams/cs361/mailbox" ;

    instructorKey  = ftok( instructorPath  , 1 ) ;
    if ( instructorKey == -1 ) 
    {
        printf( MYNAME " Failed to find Instructor's path of the mailbox '%s'. Error code=%d\n" 
               , instructorPath , errno ) ;
        perror( "Reason" );
        exit( EXIT_FAILURE ) ;  
    }
	
    /* Find ructor's msgQue ID. Request write-only access  
	   Must fail if it did not exist. */    
    int instructorflgs = S_IWUSR;
    instructorMailboxID   = msgget(instructorKey, instructorflgs); /*  Fill in the blank to get the Insructor's msg queue ID */  
    if ( instructorMailboxID == -1 ) 
    {
        printf( MYNAME " Failed to gain access to Instructor's mailbox '%s'. Error code=%d\n" 
               , instructorPath , errno ) ;
        perror("Reason");
        exit( EXIT_FAILURE ) ;  
    }
    else
        printf( MYNAME " Found Instructor's mailbox '%s' with qid=%d\n" 
               , instructorPath , instructorMailboxID ) ;

    /* prepare a message to send to the Instructor process */
    /* The msgType  must be 1 */
    studMsg.msgType = 1;
    strncpy(studMsg.name, MYNAME, sizeof(studMsg.name));    
    studMsg.returnMailbox  = myMailboxID ; /*  Put the ID of your personal msgQueue */
    printf("\nWhat do you want to say to the Instructor? ");
    char somestring[40] ;
    fgets( somestring , 40 , stdin );
    somestring[ strlen(somestring)-1 ] = '\0' ; // get rid of the new line character
    snprintf( studMsg.text , MAXTEXTLEN , MYNAME " (pid=%d) says %s" , 
             mypid , somestring );

    /* Send one message to the Instructor process */
    /* the msg flag is set to 0 */
    msgStatus =  msgsnd(instructorMailboxID, &studMsg, MSG_INFO_SIZE, 0) /*  Fill in the blank to send the message to Instructor */ ;  
    if ( msgStatus == -1 ) 
    {
        printf( MYNAME " Failed to send on queuID %d. Error code=%d\n" 
               , instructorMailboxID , errno ) ;
        perror("Reason") ;
        exit( EXIT_FAILURE ) ;  
    }
    else 
    {
        printf( "\n"  MYNAME " sent this message to Instructor\n" );
        printMsg( & studMsg );
    }

    /* Now, wait for a response message to arrive from the Instructor process */
    printf ( "\n"  MYNAME " is now waiting to receive the response ...\n" );
    msgStatus = msgrcv(myMailboxID, &instMsg, MSG_INFO_SIZE, 0 , 0 ); /*  Fill in the blank to receive  */     
    if ( msgStatus < 0 ) 
    {
        printf( MYNAME " Failed to receive message on queuID %d. Error code=%d\n" 
               , myMailboxID , errno ) ;
        perror("Reason");
        exit( EXIT_FAILURE ) ;  
    }
    else 
    {
        printf( MYNAME " received this message:\n" );
        printMsg( & instMsg );
    }

    // MUST remove your personal message queue. Use msgctl()
    msgctl(myMailboxID, IPC_RMID, NULL);
    return 0 ;
}

