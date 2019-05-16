#include "library.h"

#include <iostream>
#include <stack>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <elf.h>
#include <link.h>
#include <dlfcn.h>

/* ------------- CANARIES STACK ------------------------- */

typedef struct canaryData
{
    ULLONG refCanary;
    ULLONG* canaryInStack;
} canaryData;

typedef struct canaryStack
{
    canaryData* _canaries;
    unsigned int _length;
} canaryStack;

void initCanaryStack(canaryStack* canStack, canaryData* start)
{
    canStack->_canaries = start;
    canStack->_length = 0;
}


void canaryPush(ULLONG newCanary, ULLONG* stackCanary, canaryStack* canStack)
{
    canStack->[canStack->_length++] = {newCanary, stackCanary};
}

ULLONG* canaryTop( canaryStack* canStack) {
    return &canStack->_canaries[canStack->_length - 1].refCanary ;
}

ULLONG* canaryPop( canaryStack* canStack)
{
    return &canStack->_canaries[(canStack->_length--) - 1].refCanary;
}






// ------------------ main lib -------------

canaryStack* canaries;
ULLONG* randoms;
int randomCounter = 0;

const size_t NUM_RANDOMS = sysconf(_SC_PAGE_SIZE) / sizeof(ULLONG);



void initProtector()
{
    char* baseMem = (char*)mmap(0, sysconf(_SC_PAGE_SIZE) * 2, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    printf("new mem allocated at: %p\n", baseMem);

    // init canary stack
    canaries = (canaryStack*)baseMem;
    initCanaryStack(canaries, (canaryData*)(baseMem + sizeof(canaryStack)))


    printf("new mem allocated at: %p\n", baseMem);


    // init random chunck
    randoms = (ULLONG*)(baseMem + sysconf(_SC_PAGE_SIZE));
    FILE* randomFile = fopen("/dev/urandom", "rb");

    int bytes_read = 0;
    while (bytes_read < sysconf(_SC_PAGE_SIZE))
    {
        int currRead = fread(randoms+ bytes_read, 1, sysconf(_SC_PAGE_SIZE) - bytes_read , randomFile);
        bytes_read += currRead;
        printf("bytes read: %d\n", bytes_read);

    }

    printf("random bytes read: %d\n", bytes_read);



}

ULLONG* getNewCanary(ULLONG* stackCanary)
{

    ULLONG* newCanary = randoms + randomCounter;
    randomCounter = (randomCounter + 1) % NUM_RANDOMS;
    canaryPush(*newCanary, stackCanary, canaries);
    return newCanary;

}

ULLONG* popCanary()
{
    //printf("canary from stack: %llx\n", *canaries->_canaries[canaries->_length -1].canaryInStack);
    return popCanary(canaries);
}


// -------------------------------------- lp wrap fork -----------

void renewCanaries()
{
    canaryData* stackCanaries = canaries->_canaries;
    for (int i = 0; i < canaries->_length; i++)
    {
        ULLONG* newCanary = randoms + randomCounter;
        randomCounter = (randomCounter + 1) % NUM_RANDOMS;

        ULLONG* retAddr = stackCanaries[i].canaryInStack + 1;
        *stackCanaries[i].canaryInStack = (*newCanary) ^ (  ((ULLONG)(*retAddr)) << 32 );
        stackCanaries[i].refCanary = *newCanary;
    }
}

#ifdef WRAP_FORK

extern "C" pid_t __real_fork(void);

extern "C" pid_t __wrap_fork(void)
{
    pid_t retval = __real_fork();
    if (retval == 0) // I'm in the child process
    {
        printf("caiing renew!\n");
        renewCanaries();
    }
    return retval;

}

#endif


// ---------------------------------  DEP ---------------------------


static const long int DEFAULT_STACK_SIZE = 0x21000;
static const long int DEFAULT_HEAP_SIZE = 0x22000;

typedef struct dynamicLibData
{
    void* libcAddr;
    void* ldAddr;
} dynamicLibData;

enum libType {NONE, LIBC, LD};


static int dep_callback(struct dl_phdr_info *info, size_t size, void *data)
{
    dynamicLibData* output = (dynamicLibData*)data;
    const char *type;
    int p_type, j;
    libType currLib = NONE;

    if (!strcmp(info->dlpi_name, "/lib32/libc.so.6"))
        currLib = LIBC;
    else if (!strcmp(info->dlpi_name, "/lib/ld-linux.so.2"))
        currLib = LD;


    if (currLib == NONE)
        return 0;

    for (j = 0; j < info->dlpi_phnum; j++) {
        p_type = info->dlpi_phdr[j].p_type;
        if (p_type == PT_LOAD)
        {
            switch (currLib)
            {
                case LIBC:
                    output->libcAddr = (void*)info->dlpi_phdr[j].p_vaddr;
                    break;
                case LD:
                    output->ldAddr = (void*)info->dlpi_phdr[j].p_vaddr;
                    break;
            }
        }
    }

    return 0;
}

static void getDynamicLibsAddresses(dynamicLibData* libData)
{
    dl_iterate_phdr(dep_callback, libData);
}

static void* getStackBase()
{
    unsigned long esp;

    __asm__(
            "mov %%esp, %0"
            : "=r"(esp) : :
            );

    unsigned long offset, base;
    base = esp & ( 0xffffffff << 12 );
    offset = esp - base;
    if (offset != 0)
        base += 0x1000;
    return ((void*)(base - DEFAULT_STACK_SIZE));
}

static void* getHeapBase()
{
    return sbrk(0);
}


