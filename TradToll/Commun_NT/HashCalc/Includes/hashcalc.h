#ifndef HASHCALC_H
#define HASHCALC_H

///////////////////////////// INCLUDES /////////////////////////////
#include <windows.h>

///////////////////////////// RESERVED /////////////////////////////
///////////////////////////// DEFINES //////////////////////////////
#ifdef HASHCALC_EXPORTS
	#define EXPORT __declspec(dllexport)
#else
	#define EXPORT extern __declspec(dllimport)
#endif

#define SHA256_LEN_BYTES		32
#define SHA256_LEN_HEX_STRING	64
///////////////////////////// TYPEDEFS /////////////////////////////

typedef enum
{
	ERR_OK = 0,
	ERR_BUFFER_TOO_SMALL,
	ERR_MEMORY_ALLOCATION,
}
sha256_err;

///////////////////////////// FUNCTIONS ////////////////////////////
// input and output in HEX format
EXPORT sha256_err __stdcall CalculateSha256(IN BYTE *pInputData, IN int iInputDataLen, IN int iOutputBufferSize, OUT BYTE *pCalculatedHash);
EXPORT sha256_err __stdcall CalculateSha256_w(IN WCHAR *pInputData, IN int iInputDataLen, IN int iOutputBufferSize, OUT WCHAR *pCalculatedHash);

EXPORT void __stdcall ConvertBytesToHexString(IN BYTE *bytes, IN DWORD dwLen, OUT UCHAR *szHex);
EXPORT void __stdcall ConvertHexStringToBytes(IN UCHAR *szHex, OUT BYTE *pBytes, OUT DWORD *dwLen);


#endif
