#include <stdint.h>
#include "Processor.h"

#ifndef TRUE
	#define TRUE true
	#define FALSE false
#endif

bool Cpuid (uint32_t fn, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx);
bool ReadPciConfigDwordEx (uint32_t devfunc, uint32_t reg, uint32_t *res);

bool WritePciConfigDwordEx (uint32_t devfunc, uint32_t reg, uint32_t res) ;

bool RdmsrPx (uint32_t msr,uint32_t *eax,uint32_t *ebx,PROCESSORMASK processor) ;
bool Rdmsr (uint32_t msr,uint32_t *eax,uint32_t *ebx) ;

bool WrmsrPx (uint32_t msr,uint32_t eax,uint32_t ebx,PROCESSORMASK processor);
bool Wrmsr (uint32_t msr,uint32_t eax,uint32_t ebx);

#ifndef _WIN32
void Sleep (uint32_t ms);

int GetTickCount ();
#endif
