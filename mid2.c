#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    printf("Hello\n");
    if (fork() > 0)
        if (fork() > 0)
            fork();

    printf("Goodbye\n");
}