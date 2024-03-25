//---------------------------------------------------------------------
// Assignment : PA-02 Concurrent Processes & IPC
// Date       :
// Author     : Mohamed Aboutabl
//---------------------------------------------------------------------

#include <semaphore.h>

typedef struct 
{
    int   order_size ;
    int   made ;        // #parts made so far
    int   remain ;      // #parts remaining to be manufactured
    // When a factory is in the middle of making 'x' parts, made+remain+x = order_size
    // So, it is not always true that made + remain = order_size
} shData ;

#define SHMEM_SIZE      sizeof(shData)
#define MAXFACTORIES    20
