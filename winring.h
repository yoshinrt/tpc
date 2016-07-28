#include "OlsApi.h"
#include "OlsDef.h"

static bool ReadPCI( DWORD uDev, DWORD uFunc, DWORD uAddr, DWORD& dwResult ){
	return ReadPciConfigDwordEx(( uDev << 3 ) | uFunc, uAddr, &dwResult );
}

static bool WritePCI( DWORD uDev, DWORD uFunc, DWORD uAddr, DWORD dwData ){
	return WritePciConfigDwordEx(( uDev << 3 ) | uFunc, uAddr, dwData );
}

static uint32_t GetBits( uint32_t uVal, uint32_t uPos, uint32_t uWidth ){
	return ( uVal >> uPos ) & (( 1 << uWidth ) - 1 );
}

static uint32_t SetBits( uint32_t uVal, uint32_t uPos, uint32_t uWidth, uint32_t uNewVal ){
	uint32_t uMask = (( 1 << uWidth ) - 1 ) << uPos;
	return uVal & ~uMask | ( uNewVal << uPos ) & uMask;
}
