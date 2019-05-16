//
// Created by navot on 4/13/19.
//



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>

void runFromStack()
{
    char shellcode[] = "\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69"
                       "\x6e\x89\xe3\x50\x53\x89\xe1\xb0\x0b\xcd\x80";
    (*(void(*)()) shellcode)();
}


void protectStack()
{
    int currpid = getpid();
    unsigned long stackBase, stackLen;


    char mapsCmd[100];
    char* line = NULL;
    size_t len = 0;
    sprintf(mapsCmd, "/proc/%d/maps", currpid);
    FILE* maps = fopen(mapsCmd, "r");

    int foundStack = 0;
    while (!foundStack)
    {
        getline(&line, &len, maps);
        char* startOfMemType = line + strlen(line) - 1;
        *(startOfMemType--) = '\0';

        while (*startOfMemType != ' ')
            startOfMemType--;
        startOfMemType++;

        if (!strcmp(startOfMemType, "[stack]"))
        {
            unsigned long stackEnd;
            foundStack = 1;
            sscanf(line, "%lx-%lx", &stackBase, &stackEnd);
            stackLen = stackEnd - stackBase;
        }

    }

    printf("found stack at %lx with length %lx,  something at top: %p, diff = %lx\n", stackBase, stackLen, &currpid,
           (unsigned long)(&currpid) - stackBase);

    if (mprotect((void*)stackBase, stackLen, PROT_WRITE) == 0)
    {
        printf("mprotect succeed!\n");
    }
    else
    {
        printf("mprotect fail!\n");
    }

}


// returns new esp
unsigned long copyStack(unsigned long oldStackBase, unsigned long oldStackTop, unsigned long newStackBase)
{
    unsigned long oldStackLen = oldStackBase - oldStackTop;

    unsigned long* pNewStackTop = (unsigned long*)(newStackBase - oldStackLen);
    unsigned long* pOldStackTop = (unsigned long*)oldStackTop;
    pNewStackTop += 13;
    pOldStackTop += 13;

    for (int i = 0; i < 2; i++)
    {
        *pNewStackTop = newStackBase - (oldStackBase - *pOldStackTop);

        unsigned long* pNewParam = (unsigned long*)(*pNewStackTop);
        unsigned long* pOldParam = (unsigned long*)(*pOldStackTop);

        while (*pNewParam != 0)
        {
            *pNewParam = newStackBase - (oldStackBase - *pOldParam);
            pNewParam++;
            pOldParam++;
        }

        pNewStackTop++;
        pOldStackTop++;
    }

    unsigned long oldEbp;
    __asm__(
    "mov (%%ebp), %0;"
    : "=g"(oldEbp) : :
    );

    __asm__(
    "mov %0, (%%ebp);"
    :  : "g"(newStackBase - (oldStackBase - oldEbp)):
    );

    return (newStackBase - oldStackLen);
}



void getStackInfo(unsigned long* _stackBase, unsigned long* _stackLen)
{
    int currpid = getpid();
    unsigned long stackBase, stackLen;

    char mapsCmd[100];
    char* line = NULL;
    size_t len = 0;
    sprintf(mapsCmd, "/proc/%d/maps", currpid);
    FILE* maps = fopen(mapsCmd, "r");

    int foundStack = 0;
    while (!foundStack)
    {
        getline(&line, &len, maps);
        char* startOfMemType = line + strlen(line) - 1;
        *(startOfMemType--) = '\0';

        while (*startOfMemType != ' ')
            startOfMemType--;
        startOfMemType++;

        if (!strcmp(startOfMemType, "[stack]"))
        {
            unsigned long stackEnd;
            foundStack = 1;
            sscanf(line, "%lx-%lx", &stackBase, &stackEnd);
            stackLen = stackEnd - stackBase;
        }

    }

    *_stackBase = stackBase;
    *_stackLen = stackLen;
}

unsigned long getRandomStackTop(unsigned long stackTop)
{
    const unsigned int RAND_MASK = 0x7ff;
    const unsigned int PAGE_LEN = 12;

    unsigned int rndVar = random() & RAND_MASK;
    return (stackTop - (rndVar << 12));
}

void randomStack()
{
    unsigned long stackBase;
    unsigned long newStackBase;
    unsigned long stackLen;

    //todo replace this
    getStackInfo(&stackBase, &stackLen);

    newStackBase = (getRandomStackTop(stackBase + stackLen) - stackLen);
    unsigned long lengthToAlloc = (newStackBase + stackLen > stackBase )?  stackBase - newStackBase : stackLen;

    unsigned long startToUnmap = (newStackBase + stackLen > stackBase )? newStackBase + stackLen : stackBase;


    unsigned long stackEnd;
    __asm__(
            "mov %%esp, %0;"
            : "=g"(stackEnd) : :
            );

    printf("stack End: %lx", stackEnd);

    void* newStack = mmap(newStackBase, lengthToAlloc, PROT_WRITE | PROT_READ, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    printf("new stack at address %p\n", newStack);
    if (newStack == MAP_FAILED)
    {
        printf("failed with error: %d\n", errno);
        return;
    }

    memcpy(newStack, stackBase, stackLen);
    stackEnd = copyStack(((unsigned long)stackBase) + stackLen, stackEnd, ((unsigned long)newStack) + stackLen);

    __asm__(
    "mov %0, %%esp;"
    :  : "g"(stackEnd):
    );

    if (munmap(startToUnmap, lengthToAlloc) != 0)
    {
        printf("\n error in unmap with errno: %d\n", errno);
    }
}


int main (int argc, char** argv)
{
    printf("pid is %d\n", getpid());
    randomStack(NULL);
    //protectStack();

    printf("press a button to run shellcode!\n");
    getchar();
    getchar();
    fflush(stdin);
    //runFromStack();

    //getchar();
    //getchar();

    printf("done\n");

    return 0;
}

/*
int main()
{
    getchar();
    char st[] = "$(echo 'hello')";
    int fd = open("/home/navot/CLionProjects/fantaTest/wow",O_WRONLY);
    printf("fd = %d, errno = %d\n", fd, errno);
    write(fd,st, strlen(st));
	
    close(fd);
    return 0;
}*/



