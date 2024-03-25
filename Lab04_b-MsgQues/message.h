#include <sys/types.h>

#define MAXTEXTLEN  200
#define MAXNAMELEN  25

typedef struct {
    long msgType ;    /*  1: Sent by student to Instructor , 
                         99: when sent by Instructor to a student */
    int  returnMailbox;     /* Sender places its own queue ID for incoming responses */
    char text[MAXTEXTLEN];  /*  snprintf() some message here to the recipient */
    char name[MAXNAMELEN];  /*  snprintf() name of the sender to this */
    
} msgBuf ;

#define MSG_INFO_SIZE ( sizeof(msgBuf) - sizeof(long) )


void printMsg( msgBuf *m ) ;
