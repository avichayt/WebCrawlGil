//
// Created by navot on 4/13/19.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>

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

    printf("found stack at %lx with length %lx\n", stackBase, stackLen);

    if (mprotect((void*)stackBase, stackLen, PROT_WRITE) == 0)
    {
        printf("mprotect succeed!\n");
    }
    else
    {
        printf("mprotect fail!\n");
    }

}

int main ()
{
    printf("pid is %d\n", getpid());
    getchar();

    protectStack();

    printf("tries run shellcode!\n");
    runFromStack();

    getchar();
    getchar();

    printf("done\n");

    return 0;
}