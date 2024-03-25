//---------------------------------------------------------------------
// Assignment : PA-02 Concurrent Processes & IPC
// Date       :
// Author     : Mohamed Aboutabl
//----------------------------------------------------------------------
#include <stdio.h>

#include "message.h"

/*--------------------------------------------------------------------
   Print a message buffer
----------------------------------------------------------------------*/
void printMsg( msgBuf *m )
{
    printf( "{type=%ld, (Purpose=%d, FacID %3d, Capacity %3d, Parts %3d, duration %4d) }\n"
       , m->mtype    , m->purpose   , m->facID 
       , m->capacity , m->partsMade , m->duration  ) ;
}

