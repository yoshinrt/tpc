#include <stdio.h>
#include <stdlib.h>
#include "Signal.h"

#ifdef _WIN32
	#include <windows.h>
	#include "OlsApi.h"
#endif

#ifdef __linux
	#include "cpuPrimitives.h"
	#include <string.h>
#endif

#include "Processor.h"
#include "Interlagos.h"
#include "Kaveri.h"
#include "PCIRegObject.h"
#include "MSRObject.h"
#include "PerformanceCounter.h"
//#include "winring.h"

#include "sysdep.h"

//Kaveri class constructor
Kaveri::Kaveri ()
{
	DWORD eax,ebx,ecx,edx;
	PCIRegObject *pciReg60;
	PCIRegObject *pciReg160;
	bool pciReg60Success;
	bool pciReg160Success;
	DWORD nodes;
	DWORD cores;
	
	//Check extended CpuID Information - CPUID Function 0000_0001 reg EAX
	if (Cpuid(0x1,&eax,&ebx,&ecx,&edx)!=TRUE)
	{
		printf ("Kaveri::Kaveri - Fatal error during querying for Cpuid(0x1) instruction.\n");
		return;
	}
	
	int familyBase = (eax & 0xf00) >> 8;
	int model = (eax & 0xf0) >> 4;
	int stepping = eax & 0xf;
	int familyExtended = ((eax & 0xff00000) >> 20) + familyBase;
	int modelExtended = ((eax & 0xf0000) >> 12) + model; /* family 15h: modelExtended is valid */

	//Check Brand ID and Package type - CPUID Function 8000_0001 reg EBX
	if (Cpuid(0x80000001,&eax,&ebx,&ecx,&edx)!=TRUE)
	{
		printf ("Kaveri::Kaveri - Fatal error during querying for Cpuid(0x80000001) instruction.\n");
		return;
	}

	int brandId=(ebx & 0xffff);
	int processorModel=(brandId >> 4) & 0x7f;
	int string1=(brandId >> 11) & 0xf;
	int string2=(brandId & 0xf);
	int pkgType=(ebx >> 28);

	//Sets processor Specs
	setSpecFamilyBase (familyBase);
	setSpecModel (model);
	setSpecStepping (stepping);
	setSpecFamilyExtended (familyExtended);
	setSpecModelExtended (modelExtended);
	setSpecBrandId (brandId);
	setSpecProcessorModel (processorModel);
	setSpecString1 (string1);
	setSpecString2 (string2);
	setSpecPkgType (pkgType);
	setMaxSlots (6);

	// determine the number of nodes, and number of processors.
	// different operating systems have different APIs for doing similiar, but below steps are OS agnostic.

	pciReg60 = new PCIRegObject();
	pciReg160 = new PCIRegObject();

	//Are the 0x60 and 0x160 registers valid for Kaveri...they should be, the same board is used for MagnyCours and Kaveri
	pciReg60Success = pciReg60->readPCIReg(PCI_DEV_NORTHBRIDGE, PCI_FUNC_HT_CONFIG, 0x60, getNodeMask(0));
	pciReg160Success = pciReg160->readPCIReg(PCI_DEV_NORTHBRIDGE, PCI_FUNC_HT_CONFIG, 0x160, getNodeMask(0));

	if (pciReg60Success && pciReg160Success)
	{
		nodes = pciReg60->getBits(0, 4, 3) + 1;
	}
	else
	{
		printf ("Warning: unable to detect multiprocessor machine\n");
		nodes = 1;
	}

	free (pciReg60);
	free (pciReg160);

	//Check how many physical cores are present - CPUID Function 8000_0008 reg ECX
	if (Cpuid(0x80000008, &eax, &ebx, &ecx, &edx) != TRUE)
	{
		printf("Kaveri::Kaveri- Fatal error during querying for Cpuid(0x80000008) instruction.\n");
		return;
	}

	cores = ((ecx & 0xff) + 1); /* cores per node */
	
	/*
	 * Normally we assume that nodes per package is always 1 (one physical processor = one package), but
	 * with Kaveri chips (modelExtended>=8) this is not true since they share a single package for two
	 * chips (16 cores distributed on a single package but on two nodes).
	 */
	int nodes_per_package = 1;
	if (modelExtended >= 8)
	{
		PCIRegObject *pci_F3xE8_NbCapReg = new PCIRegObject();
		
		if ((pci_F3xE8_NbCapReg->readPCIReg(PCI_DEV_NORTHBRIDGE, PCI_FUNC_MISC_CONTROL_3, 0xE8, getNodeMask(0))) == TRUE)
		{
			if (pci_F3xE8_NbCapReg->getBits(0, 29, 1))
			{
				nodes_per_package = 2;
			}
		}
		else
		{
			printf ("Kaveri::Kaveri - Error discovering nodes per package, results may be unreliable\n");
		}

		free(pci_F3xE8_NbCapReg);
	}
	
	setProcessorNodes(nodes);
	setProcessorCores(cores);
	setNode(0);
	setBoostStates (getNumBoostStates());
	setPowerStates(8);
	setProcessorIdentifier(PROCESSOR_15H_FAMILY);
	setProcessorStrId("Family 15h (Kaveri) Processor");
}

/*
 * Static methods to allow external Main to detect current configuration status
 * without instantiating an object. This method that detects if the system
 * has a processor supported by this module
*/
bool Kaveri::isProcessorSupported() {

	DWORD eax;
	DWORD ebx;
	DWORD ecx;
	DWORD edx;

	//Check base CpuID information
	if (Cpuid(0x0, &eax, &ebx, &ecx, &edx) != TRUE)
	  return false;
	
	//Checks if eax is 0xd. It determines the largest CPUID function available
	//Family 15h returns eax=0xd
	if (eax != 0xd)
	  return false;

	//Check "AuthenticAMD" string
	if ((ebx != 0x68747541) || (ecx != 0x444D4163) || (edx != 0x69746E65))
	  return false;

	//Check extended CpuID Information - CPUID Function 0000_0001 reg EAX
	if (Cpuid(0x1, &eax, &ebx, &ecx, &edx) != TRUE)
	  return false;

	int familyBase = (eax & 0xf00) >> 8;
	int familyExtended = ((eax & 0xff00000) >> 20) + familyBase;

	if (familyExtended != 0x15)
	  return false;
	
	int model = (eax & 0xf0) >> 4;
	int modelExtended = ((eax & 0xf0000) >> 12) + model; /* family 15h: modelExtended is valid */
	if( modelExtended < 0x30 || 0x3F < modelExtended ) return FALSE;
	
	//Detects a Family 15h processor, i.e. Bulldozer/Kaveri/Valencia
	return true;
}

//Miscellaneous function inherited by Processor abstract class and that
//needs to be reworked for family 10h
float Kaveri::convertVIDtoVcore(DWORD curVid)
{

	/*How to calculate VID from Vcore. It doesn't matter if your processor is working in
	 PVI or SVI mode, since processsor register is always 7 bit wide. Then the processor
	 takes care to convert it to Parallel or Serial implementation.

	 Serial VID Interface is simple to calculate.
	 To obtain vcore from VID you need to do:

	 vcore = 1,55 ? (VID * 0.0125)

	 The inverse formula to obtain VID from vcore is:

	 vid = (1.55-vcore)/0.0125

	 */

	return 1.55 - ( curVid * 0.00625 );
}

DWORD Kaveri::convertVcoretoVID (float vcore)
{
	DWORD vid;

	vid = round(((1.55 - vcore) / 0.00625));
	
	return vid;

}

//-----------------------setVID-----------------------------
//Overloads abstract class setVID to allow per-core personalization
void Kaveri::setVID (PState ps, DWORD vid)
{

	MSRObject *msrObject;

	if ((vid > minVID()) || (vid < maxVID()))
	{
		printf ("Kaveri.cpp: VID Allowed range %d-%d\n", minVID(), maxVID());
		return;
	}

	msrObject=new MSRObject();

	if (!msrObject->readMSR(BASE_15H_PSTATEMSR + ps.getPState(), getMask ()))
	{
		printf ("Kaveri.cpp: unable to read MSR\n");
		free (msrObject);
		return;
	}

	//To set VID, base offset is 9 bits and value is 7 bit wide.
	msrObject->setBitsLow(9, 8, vid);

	if (!msrObject->writeMSR())
	{
		printf ("Kaveri.cpp: unable to write MSR\n");
		free (msrObject);
		return;
	}

	free (msrObject);

	return;

}

//-----------------------getVID-----------------------------
DWORD Kaveri::getVID (PState ps)
{

	MSRObject *msrObject;
	DWORD vid;

	msrObject=new MSRObject ();

	if (!msrObject->readMSR(BASE_15H_PSTATEMSR + ps.getPState(), getMask()))
	{
		printf ("Kaveri.cpp::getVID - unable to read MSR\n");
		free (msrObject);
		return false;
	}

	//Returns data for the first cpu in cpuMask.
	//VID is stored after 9 bits of offset and is 7 bits wide
	vid = msrObject->getBitsLow(0, 9, 8);

	free (msrObject);

	return vid;
}

//minVID is reported per-node, so selected core is always discarded
DWORD Kaveri::minVID ()
{
	MSRObject *msrObject;
	DWORD minVid;

	msrObject = new MSRObject;

	if (!msrObject->readMSR(COFVID_STATUS_REG, getMask(0, selectedNode)))
	{
		printf ("Kaveri::minVID - Unable to read MSR\n");
		free (msrObject);
		return false;
	}
	
	minVid=msrObject->getBits(0, 42, 7);

	free (msrObject);

	//If minVid==0, then there's no minimum vid.
	//Since the register is 7-bit wide, then 127 is
	//the maximum value allowed.
	if (getPVIMode())
	{
		//Parallel VID mode, allows minimum vcore VID up to 0x5d
		if (minVid == 0)
			return 0x5d;
		else
			return minVid;
	}
	else
	{
		//Serial VID mode, allows minimum vcore VID up to 0x7b
		if (minVid==0)
			return 0x7b;
		else
			return minVid;
	}
}

//maxVID is reported per-node, so selected core is always discarded
DWORD Kaveri::maxVID()
{
	MSRObject *msrObject;
	DWORD maxVid;

	msrObject = new MSRObject;

	if (!msrObject->readMSR(COFVID_STATUS_REG, getMask(0, selectedNode)))
	{
		printf("Kaveri::maxVID - Unable to read MSR\n");
		free(msrObject);
		return false;
	}
	
	maxVid = msrObject->getBits(0, 35, 7);
	
	free(msrObject);
	
	//If maxVid==0, then there's no maximum set in hardware
	if (maxVid == 0)
		return 0;
	else
		return maxVid;
}
