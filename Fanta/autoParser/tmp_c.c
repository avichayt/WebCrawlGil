#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "library.h"

char* FAILURE_CANARY_MSG = "buffer overflow!\n";

/**
long long ___CANARY___;
void initCanary()
{
    srand(time(NULL));
	___CANARY___ = rand();
	*(((int*)&___CANARY___) + 1) = rand();
	*(((char*)&___CANARY___) + 4) = '\0';
}**/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

typedef unsigned long long ULLONG;



void foo(int a);
void g();



int main (int argc, char** argv)
{
    unsigned long stackEnd;
    __asm__(
    "mov %%esp, %0;"
    : "=g"(stackEnd) : :
    );

    printf("stack End: %lx\n", stackEnd);
    //randomStack();
    //protectStack();

    printf("\npress a button to run shellcode!\n");
    getchar();
    getchar();
    fflush(stdin);

    //runFromStack();

    //getchar();
    //getchar();

    printf("done\n");

    return 0;
}

void g()
{
    unsigned long ebp;
    if (fork() != 0) // parent
    {
          __asm__(
        "mov %%ebp, %0"
        : "=r"(ebp) : :
        );
        ULLONG __canary = *((ULLONG*)(ebp + 4));
        printf("parent canary = %llx\n", __canary);
    }
    else
    {
         __asm__(
        "mov %%ebp, %0"
        : "=r"(ebp) : :
        );
        ULLONG __canary = *((ULLONG*)(ebp + 4));
        printf("child canary = %llx\n", __canary);
    }
}

void foo(int a)
{
    char* buff = (char*)malloc(0x1001);

    //g();


    //getchar();
    getchar();


}

