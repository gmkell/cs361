//---------------------------------------------------------------------
// Assignment : PA-02 Concurrent Processes & IPC
// Date       :
// Author     : Mohamed Aboutabl
//----------------------------------------------------------------------
#include <sys/types.h>

typedef enum 
{
    PRODUCTION_MSG = 1 , COMPLETION_MSG 
} msgPurpose_t;

typedef struct {
    long mtype ;               /* not used here */

    msgPurpose_t  purpose ;  /* Purpose of this message to Supervisor */

    int  facID    ,          /* sender's Factory ID */
         capacity ,          /* #of parts made in most recent iteration */
         partsMade ,         /* #of parts made in most recent iteration */
         duration ;          /* how long it took to make them */

} msgBuf ;

#define MSG_INFO_SIZE ( sizeof(msgBuf) - sizeof(long) )

void printMsg( msgBuf *m ) ;

