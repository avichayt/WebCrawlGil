#include <stdio.h>
#include <stdlib.h>

#define SIZE 10

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


long long ___CANARY___;

#define CANARY_PROLOGUE 	long long canary = ___CANARY___; \
	int retAddr; \
	__asm__( \
		".intel_syntax;" \
		"mov %0, [ebp + 4];" \
		".att_syntax;" \
		: "=r"(retAddr) : : "eax"); \
	*((int*)&canary) ^= retAddr; 



int foo(int num);



int main()
{
	  // Initialization, should only be called once.
	srand(time(NULL)); 
	___CANARY___ = rand();
	*(((int*)&___CANARY___) + 1) = rand();
	*(((char*)&___CANARY___) + 4) = '\0';

	int i;
	for (i = 0; i < SIZE; i++)
	{
		printf("%d : %d", i, foo(i));
	}


	/*
	__asm__(
	".intel_syntax;"
	"push eax;"
	"push ecx;"
	"mov eax, [ebp + 4];"
	"xor [ebp + 8], eax;"
	"cmp [ebp + 8], %0;"
	"jne EXIT;"
	"cmp [ebp + 0xc], %1;"
	"jne EXIT;"
	"jmp DONE;"
	"EXIT:"
	"call %2;"
	"DONE:"
	"pop ecx;"
	"pop eax;"
	".att_syntax;"
	: : "r"(*(((int*)&___CANARY___))), "r"(*(((int*)&___CANARY___) + 1)), "r"(&exit)
	: "eax");*/
	/*
	*((int*)&canary) ^= retAddr;
	if (canary != ___CANARY___)
	{
		exit(-1);
	}*/

	return 0;
}


int foo(int num)
{
	num++;
	printf("num is %d\n", num);

	return num;
}

