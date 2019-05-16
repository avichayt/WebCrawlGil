#ifndef PROTECTLIB_LIBRARY_H
#define PROTECTLIB_LIBRARY_H

typedef unsigned long long ULLONG;

void initProtector(char** argv);
 ULLONG* popCanary();
ULLONG* getNewCanary();




#endif //PROTECTLIB_LIBRARY_H