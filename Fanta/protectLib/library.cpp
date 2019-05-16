#include "library.h"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <link.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <elf.h>
#include <sys/random.h>

#include <dlfcn.h>


#define ALIGN_PAGE_UP(x) ( (x) & (0xfffff000)  )
#define ALIGHT_PAGE_DOWN(x) ( ((x) & (0xfffff000)) + ( ((x) & (0x00000fff))? 0x1000 : 0 )   )
void getBaseAddresses(char* progName, void** baseText, void** endText, void** baseData, void** endData);
void performDEP(char* progName);


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
    canaryData data = {newCanary, stackCanary};
    canStack->_canaries[canStack->_length++] = data;
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

size_t NUM_RANDOMS = 0;



void initProtector(char** argv)
{
    char* progName = argv[0];
    printf("prog name: %s\n", progName);

    // performs DEP
    getchar();
    performDEP(progName);

    fflush(stdin);
    getchar();


    NUM_RANDOMS = sysconf(_SC_PAGE_SIZE) / sizeof(ULLONG);
    char* baseMem = (char*)mmap(0, sysconf(_SC_PAGE_SIZE) * 2, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    printf("new mem allocated at: %p\n", baseMem);

    // init canary stack
    canaries = (canaryStack*)baseMem;
    initCanaryStack(canaries, (canaryData*)(baseMem + sizeof(canaryStack)));


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
    return canaryPop(canaries);
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

pid_t __real_fork(void);

pid_t __wrap_fork(void)
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
static const long int DEFAULT_LIBC_OFFSET = 0x1D3000;
static const long int DEFAULT_LIBC_READONLY_SIZE = 0x2000;
static const long int DEFAULT_LIBC_WRITABLE_SIZE = 0x1000;
static const long int DEFAULT_LD_OFFSET= 0X26000;
static const long int DEFAULT_LD_READONLY_SIZE= 0x1000;
static const long int DEFAULT_LD_WRITABLE_SIZE= 0X1000;



typedef struct dynamicLibData
{
    void* libcAddr;
    void* ldAddr;
} dynamicLibData;

enum libType {NONE, LIBC, LD};


int dep_callback(struct dl_phdr_info *info, size_t size, void *data)
{
    dynamicLibData* output = (dynamicLibData*)data;
    const char *type;
    int p_type, j;
    enum libType currLib = NONE;

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
                    if (!output->libcAddr)
                        output->libcAddr = (void*)(info->dlpi_addr + info->dlpi_phdr[j].p_vaddr);
                    break;
                case LD:
                    if (!output->ldAddr)
                        output->ldAddr = (void*)(info->dlpi_addr + info->dlpi_phdr[j].p_vaddr);
                    break;
            }
        }
    }

    return 0;
}


void getDynamicLibsAddresses(dynamicLibData* libData)
{

    libData->libcAddr = NULL;
    libData->ldAddr = NULL;
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
    char* heapEnd = (char*)sbrk(0);
    return ((void*)(heapEnd - DEFAULT_HEAP_SIZE));
}

void getBaseAddresses(char* progName, void** baseText, void** endText, void** baseData, void** endData)
{
    printf("getting addresses\n");
    FILE* progFile = fopen(progName, "rb");
    if (!progFile)
    {
        printf("error opening file!\n");
        return;
    }
    Elf32_Ehdr progHeader;
    Elf32_Shdr* progSections;



    fread(&progHeader, 1, sizeof(progHeader), progFile);
    printf("machine: %x\n", progHeader.e_entry);

    Elf32_Off sectionTableStart = progHeader.e_shoff;
    fseek(progFile, sectionTableStart, SEEK_SET);

    printf("machine: %x\n", progHeader.e_entry);

    // read section table
    progSections = (Elf32_Shdr*) malloc(progHeader.e_shentsize * progHeader.e_shnum);

    printf("machine: %x\n", progHeader.e_entry);
    fread(progSections,  progHeader.e_shnum, progHeader.e_shentsize, progFile);


    Elf32_Shdr* stringSection = progSections + progHeader.e_shstrndx;
    char* secNames = (char*)malloc(stringSection->sh_size);
    fseek(progFile, stringSection->sh_offset, SEEK_SET);
    fread(secNames, 1, stringSection->sh_size, progFile);

    printf("sec: %x\n", progSections[1].sh_addr);
    printf("%d  %d   %x", progHeader.e_shstrndx, progHeader.e_shentsize, progHeader.e_shoff);

    char foundBase = 0;
    for (int i = 0; i < progHeader.e_shnum; i++)
    {
        char* secName = secNames + progSections[i].sh_name;
        if (!foundBase && progSections[i].sh_addr != 0)
        {
            *baseText = (void*)ALIGN_PAGE_UP(progSections[i].sh_addr);
            foundBase = 1;
        }
        else if (!strcmp(secName, ".fini"))
        {
            *endText = (void*)ALIGHT_PAGE_DOWN(progSections[i].sh_addr + progSections[i].sh_size);
        }
        else if (!strcmp(secName, ".data"))
        {
            *baseData = (void*)ALIGN_PAGE_UP(progSections[i].sh_addr);
        }
        else if (!strcmp(secName, ".bss"))
        {
            *endData = (void*)ALIGHT_PAGE_DOWN(progSections[i].sh_addr + progSections[i].sh_size);
        }

    }
    printf("base: %p\n", *baseText);
    printf("base ebd: %p\n", *endText);
    printf("data start: %p\n", *baseData);
    printf("data end: %p\n", *endData);
    fclose(progFile);
    free(progSections);
    free(secNames);
    //*baseText = ALIGN_PAGE_UP(progSections.);
}


#include <sys/personality.h>
#include <errno.h>


void performDEP(char* progName)
{
    // program pages
    char *textBase, *textEnd, *dataBase, *dataEnd;
    getBaseAddresses(progName, (void**)&textBase, (void**)&textEnd, (void**)&dataBase, (void**)&dataEnd);

    unsigned int prsn = personality(0xffffffff);
    int res = personality(prsn & (~READ_IMPLIES_EXEC));
    printf("personality res : %x\n", res);
    mprotect(textEnd, dataBase - textEnd, PROT_READ);
    mprotect(dataBase, dataEnd - dataBase, PROT_WRITE | PROT_READ);

    // stack
    void* stackBase = getStackBase();
    mprotect(stackBase, DEFAULT_STACK_SIZE, PROT_WRITE |PROT_READ);

    // heap
    void* heapBase = getHeapBase();
    printf("heap base: %p\n", heapBase);
    mprotect(heapBase, DEFAULT_HEAP_SIZE, PROT_WRITE | PROT_READ);

    // libc and ld
    dynamicLibData libs;
    getDynamicLibsAddresses(&libs);
    void* libcBase = (char*)libs.libcAddr + DEFAULT_LIBC_OFFSET;
    printf("libc base: %p\n", libcBase);
    mprotect(libcBase, DEFAULT_LIBC_READONLY_SIZE, PROT_READ);
    mprotect((void*)((char*)libcBase + DEFAULT_LIBC_READONLY_SIZE), DEFAULT_LIBC_WRITABLE_SIZE, PROT_WRITE | PROT_READ);

    void* ldBase = (char*)libs.ldAddr + DEFAULT_LD_OFFSET;
    mprotect(ldBase, DEFAULT_LD_READONLY_SIZE, PROT_READ);
    mprotect((void*)((char*)ldBase + DEFAULT_LD_READONLY_SIZE), DEFAULT_LD_WRITABLE_SIZE, PROT_WRITE | PROT_READ);

}




///// --------------------  ASLR --------------------



// returns new esp
unsigned long copyStack(unsigned long oldStackBase, unsigned long oldStackTop, unsigned long newStackBase)
{
    unsigned long oldStackLen = oldStackBase - oldStackTop;

    unsigned long* pNewStackTop = (unsigned long*)(newStackBase - oldStackLen);
    unsigned long* pOldStackTop = (unsigned long*)oldStackTop;
    pNewStackTop += 14;
    pOldStackTop += 14;

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

    pNewStackTop--;
    pOldStackTop--;

    long rndVar;
    getrandom(&rndVar, sizeof(long), 0);

    printf("\n\nrandVar = %lx\n", rndVar & 0x0ffc);

    unsigned long newStackTop = (newStackBase - oldStackLen);
    unsigned long randomizedTop = newStackTop - (rndVar & 0x0fff);
    unsigned long copyLength = ((unsigned long)pNewStackTop) - newStackTop;
    memcpy((void*)randomizedTop, (void*)newStackTop, copyLength);



    unsigned long oldEbp;
    __asm__(
    "mov (%%ebp), %0;"
    : "=g"(oldEbp) : :
    );

    __asm__(
    "mov %0, (%%ebp);"
    :  : "g"(newStackBase - (oldStackBase - oldEbp) - (rndVar & 0x0fff)):
    );

    return (randomizedTop);
}



void getStackInfo(unsigned long* _stackBase, unsigned long* _stackLen)
{

    unsigned long stackBase, stackLen;

    stackBase = (unsigned long)getStackBase();
    stackLen = DEFAULT_STACK_SIZE;

    printf("stack base: %lx,  stack end: %lx\n", stackBase, stackBase + stackLen);

    *_stackBase = stackBase;
    *_stackLen = stackLen;
}

unsigned long getRandomStackTop(unsigned long stackTop)
{
    const unsigned int RAND_MASK = 0x7ff;
    const unsigned int PAGE_LEN = 12;

    unsigned int rndVar;
    getrandom(&rndVar, sizeof(rndVar), 0);
    rndVar = rndVar & RAND_MASK;
    printf("rnd value for stack end: %lx\n", stackTop - (rndVar << 12));
    return (stackTop - (rndVar << 12));
}

void randomizeStack()
{
    unsigned long stackBase;
    unsigned long newStackBase;
    unsigned long stackLen;

    //todo replace this
    getStackInfo(&stackBase, &stackLen);
    newStackBase = (getRandomStackTop(stackBase + stackLen) - stackLen);
    printf("expected new stack base: %lx\n", newStackBase);
    unsigned long lengthToAlloc = (newStackBase + stackLen > stackBase )?  stackBase - newStackBase : stackLen;

    unsigned long startToUnmap = (newStackBase + stackLen > stackBase )? newStackBase + stackLen : stackBase;


    unsigned long stackEnd;
    __asm__(
    "mov %%esp, %0;"
    : "=g"(stackEnd) : :
    );


    printf("I'm just allocating from %lx to %lx\n", newStackBase, newStackBase + lengthToAlloc);
    void* newStack = mmap((void*)newStackBase, lengthToAlloc, PROT_WRITE | PROT_READ, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    printf("new stack at address %p\n", newStack);
    if (newStack == MAP_FAILED)
    {
        printf("failed with error: %d\n", errno);
        return;
    }

    memcpy(newStack, (void*)stackBase, stackLen);
    stackEnd = copyStack(((unsigned long)stackBase) + stackLen, stackEnd, ((unsigned long)newStack) + stackLen);


    printf("hey, stack end is now: %lx\n", stackEnd);

    __asm__(
    "mov %0, %%esp;"
    :  : "g"(stackEnd):
    );



    if (munmap((void*)startToUnmap, lengthToAlloc) != 0)
    {
        printf("\n error in unmap with errno: %d\n", errno);
    }


}
