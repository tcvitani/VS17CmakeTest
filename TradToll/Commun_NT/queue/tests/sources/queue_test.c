/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : QUEUE
 * FILE       : QUEUE_TEST.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Unitary test program for QUEUE
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
#include <stdio.h>

#include <CSR_QUEUE.h>

#define LOC_DEF
#include <QUEUE_test.h>
#undef LOC_DEF

#include <memclass.h>

typedef struct _BAC_PACKET
{
	void* pvContext;
	DWORD* pdwErrCode;
	DWORD               dwBufferBytes;
	BYTE                tbBuffer[];
}
BAC_PACKET;


PROTECTED int __cdecl main(
    IN int iArgc,
    IN char ** ppcArgc )
{
    int iErr = NO_ERROR;
	QUEUE_INSTANCE* psReadQueue = NULL;
	BAC_PACKET* psPacket = NULL;

    __try
    {
		DWORD dwErr = QueueOpen(&psReadQueue, 10);
		if (dwErr != NO_ERROR)
			__leave;

		// Préparer le packet de lecture
		psPacket = HeapAlloc(GetProcessHeap(), 0, sizeof(*psPacket)+ 100);
		if (psPacket != NULL)
		{
			// Mettre à zéro l'entête du packet
			ZeroMemory(psPacket, sizeof(*psPacket));
			CopyMemory(psPacket->tbBuffer, "1234567890", 10);
			psPacket->tbBuffer[10] = 0x00;
		}

		dwErr = QueueWriteItem(psReadQueue, psPacket, INFINITE);
		if (dwErr != NO_ERROR)
		{
			// Echec dans la file d'attente : On libère le buffer alloué
			__leave;

		}

		BAC_PACKET* psReadPacket = NULL;

		dwErr = QueueReadItem(psReadQueue, &psReadPacket, 100);
		if (dwErr != NO_ERROR)
		{
			if (dwErr == WAIT_TIMEOUT)
				dwErr = NO_ERROR;
			__leave;
		}


		printf("Test: %s - Read from queue: %s",(psPacket== psReadPacket)?"OK":"NOK", psReadPacket->tbBuffer);

    }
    __finally
    {
		if (psPacket != NULL)
			HeapFree(GetProcessHeap(), 0, psPacket);

		if (psReadQueue != NULL)
			QueueClose(psReadQueue);
	}

	return iErr;
}
