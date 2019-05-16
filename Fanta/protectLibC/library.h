#ifndef PROTECTLIB_LIBRARY_H
#define PROTECTLIB_LIBRARY_H

//#define WRAP_FORK

#include <unistd.h>

typedef unsigned long long ULLONG;

void initProtector();
ULLONG* popCanary();
ULLONG* getNewCanary(ULLONG* stackCanary);

#ifdef WRAP_FORK
pid_t __real_fork(void);
pid_t __wrap_fork(void);
#endif

#endif //PROTECTLIB_LIBRARY_H