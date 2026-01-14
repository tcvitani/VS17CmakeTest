/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : 
 * FILE       : 
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>

#define LOC_DEF
#include <trfsvc_protocol.h>
#undef LOC_DEF

#include <memclass.h>


PROTECTED BOOL FSYNC_IS_ID_REQ(void * pvMsg, DWORD dwSize)
{
	if (dwSize < sizeof(FSYNC_MSG_ID_REQ))
		return FALSE;
	if ((*(DWORD*)pvMsg) != FSYNC_CODE_ID_REQ)
		return FALSE;
	dwSize -= sizeof(FSYNC_MSG_ID_REQ); 
		
	if (dwSize < ((FSYNC_MSG_ID_REQ*)pvMsg)->dwNameSize)
			return FALSE;

	dwSize -= ((FSYNC_MSG_ID_REQ*)pvMsg)->dwNameSize;

	if (dwSize < ((FSYNC_MSG_ID_REQ*)pvMsg)->dwKeySize)
		return FALSE;
	dwSize -= ((FSYNC_MSG_ID_REQ*)pvMsg)->dwKeySize;
	return dwSize == 0;
}

PROTECTED DWORD FSYNC_SIZEOF_ID_REQ(void * pvMsg)
{
	DWORD dwSize = sizeof(FSYNC_MSG_ID_REQ) + ((FSYNC_MSG_ID_REQ*)pvMsg)->dwNameSize + ((FSYNC_MSG_ID_REQ*)pvMsg)->dwKeySize;
	return dwSize;
}



PROTECTED BOOL FSYNC_IS_ID_RSP(void * pvMsg, DWORD dwSize)
{
	if (dwSize < sizeof(FSYNC_MSG_ID_RSP))
		return FALSE; 
	if ((*(DWORD*)pvMsg) != FSYNC_CODE_ID_RSP)
		return FALSE; 
	dwSize -= sizeof(FSYNC_MSG_ID_RSP); 
	return dwSize == 0;
}

PROTECTED DWORD FSYNC_SIZEOF_ID_RSP(void * pvMsg)
{
	DWORD dwSize = sizeof(FSYNC_MSG_ID_RSP);
	return dwSize;
}



PROTECTED BOOL FSYNC_IS_FULL_NF(void * pvMsg, DWORD dwSize)
{
	if (dwSize < sizeof(FSYNC_MSG_FULL_NF))
		return FALSE;
	if ((*(DWORD*)pvMsg) != FSYNC_CODE_FULL_NF)
		return FALSE;
	dwSize -= sizeof(FSYNC_MSG_FULL_NF);

	if (dwSize < ((FSYNC_MSG_FULL_NF*)pvMsg)->dwDataSize)
		return FALSE;

	dwSize -= ((FSYNC_MSG_FULL_NF*)pvMsg)->dwDataSize;

	return dwSize == 0;
}
PROTECTED DWORD FSYNC_SIZEOF_FULL_NF(void * pvMsg)
{
	DWORD dwSize = sizeof(FSYNC_MSG_FULL_NF) + ((FSYNC_MSG_FULL_NF*)pvMsg)->dwDataSize;
	return dwSize;
}

PROTECTED BOOL FSYNC_IS_PARTIAL_NF(void * pvMsg, DWORD dwSize)
{
	if (dwSize < sizeof(FSYNC_MSG_PARTIAL_NF))
		return FALSE;
	if ((*(DWORD*)pvMsg) != FSYNC_CODE_PARTIAL_NF)
		return FALSE;
	dwSize -= sizeof(FSYNC_MSG_PARTIAL_NF);
	if (dwSize < ((FSYNC_MSG_PARTIAL_NF*)pvMsg)->dwDataSize)
		return FALSE;
	dwSize -= ((FSYNC_MSG_PARTIAL_NF*)pvMsg)->dwDataSize;
	return dwSize == 0;
}

PROTECTED DWORD FSYNC_SIZEOF_PARTIAL_NF(void * pvMsg)
{
	DWORD dwSize = sizeof(FSYNC_MSG_PARTIAL_NF) + ((FSYNC_MSG_PARTIAL_NF*)pvMsg)->dwDataSize;
	return dwSize;
}


PROTECTED BOOL FSYNC_IS_PING_REQ(void * pvMsg, DWORD dwSize)
{
	if (dwSize < sizeof(FSYNC_MSG_PING_REQ))
		return FALSE;
	if ((*(DWORD*)pvMsg) != FSYNC_CODE_PING_REQ)
		return FALSE;
	dwSize -= sizeof(FSYNC_MSG_PING_REQ);
	return dwSize == 0;
}
PROTECTED DWORD FSYNC_SIZEOF_PING_REQ(void * pvMsg)
{
	DWORD dwSize = sizeof(FSYNC_MSG_PING_REQ);
	return dwSize;
}

PROTECTED BOOL FSYNC_IS_PING_RSP(void * pvMsg, DWORD dwSize)
{
	if (dwSize < sizeof(FSYNC_MSG_PING_RSP))
		return FALSE;
	if ((*(DWORD*)pvMsg) != FSYNC_CODE_PING_RSP)
		return FALSE;
	dwSize -= sizeof(FSYNC_MSG_PING_RSP);
	return dwSize == 0;
}

PROTECTED DWORD FSYNC_SIZEOF_PING_RSP(void * pvMsg)
{
	DWORD dwSize = sizeof(FSYNC_MSG_PING_RSP);
	return dwSize;
}

