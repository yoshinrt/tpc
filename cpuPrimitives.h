#include <inttypes.h>
#include <sys/types.h>
#include "Processor.h"

#define DWORD uint32_t
//typedef uint32_t DWORD;

#ifndef TRUE
	#define TRUE true
	#define FALSE false
#endif

bool Cpuid (DWORD fn, DWORD *eax, DWORD *ebx, DWORD *ecx, DWORD *edx);
bool ReadPciConfigDwordEx (DWORD devfunc, DWORD reg, DWORD *res);

bool WritePciConfigDwordEx (DWORD devfunc, DWORD reg, DWORD res) ;

bool RdmsrPx (DWORD msr,DWORD *eax,DWORD *ebx,PROCESSORMASK processor) ;
bool Rdmsr (DWORD msr,DWORD *eax,DWORD *ebx) ;

bool WrmsrPx (DWORD msr,DWORD eax,DWORD ebx,PROCESSORMASK processor);
bool Wrmsr (DWORD msr,DWORD eax,DWORD ebx);

#ifndef _WIN32
void Sleep (DWORD ms);

int GetTickCount ();
#endif
