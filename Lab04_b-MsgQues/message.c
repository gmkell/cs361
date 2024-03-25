#include <stdio.h>

#include "message.h"

/*--------------------------------------------------------------------
   Print a message buffer
----------------------------------------------------------------------*/
void printMsg( msgBuf *m )
{
    printf( "{type=%ld , sender=\"%.*s\" , sender's mailbox ID= %d, text=\"%s\"  }\n"
       , m->msgType  , MAXNAMELEN , m->name, m->returnMailbox, m->text  ) ;
}

