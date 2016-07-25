#pragma once

class Kaveri: protected Interlagos
{
public:

	Kaveri();

 	static bool isProcessorSupported();

	float convertVIDtoVcore(DWORD);
	DWORD convertVcoretoVID(float);
	
	void setVID(PState, DWORD);
	DWORD getVID(PState);
	DWORD minVID();
	DWORD maxVID();
};
