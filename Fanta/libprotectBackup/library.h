#ifndef PROTECTLIB_LIBRARY_H
#define PROTECTLIB_LIBRARY_H

//#define WRAP_FORK

#include <unistd.h>

typedef unsigned long long ULLONG;

extern "C" void initProtector();
extern "C" ULLONG* popCanary();
extern "C" ULLONG* getNewCanary(ULLONG* stackCanary);

#ifdef WRAP_FORK
extern "C" pid_t __real_fork(void);
extern "C" pid_t __wrap_fork(void);
#endif

#endif //PROTECTLIB_LIBRARY_H