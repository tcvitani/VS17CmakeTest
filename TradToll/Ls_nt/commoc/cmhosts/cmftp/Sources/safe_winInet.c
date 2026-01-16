/* --------------------------------------------------------------------
 * (C) 2005 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : CMFTP
 * FILE       : CMFTP.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : Commoc, ftp
 * --------------------------------------------------------------------
 * SUMMARY    : The ComMOCSvc extension for the FTP remote server.
 * --------------------------------------------------------------------
 * DESCRIPTION: This extension library supports the FTP file transfer
 *              between the local host and remote host. It contains 
 *              exported functions used by the ComMOCSvc service.
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <stdio.h>
#include <wininet.h>

#include <trc.h>
#include <reg.h>
#include <csr_excpt.h>
#include <csr_timer.h>
#include <cmftp.h>
#include <safe_winInet.h>

//#include "commoc.h"

#include <memclass.h>

#define MAX_REQ 200

typedef struct _WHATCH_REQ
{
	BOOL bUsed;
	DWORD dwReqID;
	HINTERNET hSession;

	DWORD dwStartTime;
}WHATCH_REQ;

typedef struct _WATCH_DOG_THREAD_INST
{
	TIMER_INSTANCE* hTmrCr;	
	HANDLE			hEndEvent;
	DWORD dwTimeout;
	DWORD dwSessionInactivityTimeout;

	HANDLE hThrd;
	
	DWORD dwReqIDSequence;
	DWORD dwActiveRequests;
	CRITICAL_SECTION * pCS;
	
	WHATCH_REQ aReq[MAX_REQ];
	HOST_INST * psHostInst;

}WATCH_DOG_THREAD_INST;




PRIVATE void clearAllWhatchReq(WATCH_DOG_THREAD_INST * psWdInst);

PROTECTED void SetInternetSession( HOST_INST * psInst,  HINTERNET hSession);
PROTECTED HINTERNET GetInternetSession( HOST_INST * psInst);
PROTECTED void HostTrace( HOST_INST * psInst, char * szFormat, ... );
PROTECTED void HostTraceDirect( HOST_INST * psInst, char * szFormat, ... );

PROTECTED	BOOL WaitThreadToExit(HANDLE hThrd, int iTimeout);
PROTECTED BOOL SafeCloseHandle(HINTERNET hHandle, int iTimeout);

//---------------------------------------------------------------------------------------------------------------------------
BOOL isTooOld(DWORD dwStartTime,DWORD dwNow, DWORD dwSessionInactivityTimeout)
{
	DWORD dwDelta;

	if(dwNow>=dwStartTime)
	{
		dwDelta = dwNow - dwStartTime;
	}
	else
	{
		dwDelta = (0xFFFF - dwStartTime) + dwNow;  
	}

	return dwDelta>dwSessionInactivityTimeout;
}


DWORD WatchdogThreadFunc(WATCH_DOG_THREAD_INST * psWdInst) 
{ 
    HANDLE                  thHandles[2];
	DWORD dwResult;
	TIMER_INSTANCE* hTmrCr;
	DWORD i;

    thHandles[0] = psWdInst->hEndEvent;
    thHandles[1] = TimerGetWaitableHandle( psWdInst->hTmrCr );
	
	hTmrCr = psWdInst->hTmrCr;

	while (TRUE) 
	{ 
        dwResult = WaitForMultipleObjects( 2, thHandles, FALSE, INFINITE );
		
        // SI L'OBJET SIGNALÉ EST L'ÉVÉNEMENT D'ARRET
        // On sort de la boucle While.
        if ( dwResult == ( WAIT_OBJECT_0 + 0 ) )
            break;
		
		// Else it is connection timeout and handles should be reset
        else if ( dwResult == ( WAIT_OBJECT_0 + 1 ) )
        {
			DWORD dwNow = timeGetTime();
			BOOL bHandleClosedOK = FALSE;
			HANDLE hCurrentInetSession = NULL;

			//HostTrace( psWdInst->psHostInst, "WatchdogThreadFunc ==>Timer elapsed checking...");

			TimerAcknowledge( psWdInst->hTmrCr );

			__try{
				EnterCriticalSection(psWdInst->pCS);
				
				//Enum the requests in the list and find the one that is too old
				//   when old request is found, close internet session and remove all the requests from the list
				//   assure to set global session handle to NULL
				for(i = 0; i< (int)psWdInst->dwActiveRequests; i++)
				{
					if(psWdInst->aReq[i].bUsed == TRUE)
					{
						//if any is too old close session and clear all
						if(isTooOld(psWdInst->aReq[i].dwStartTime, dwNow, psWdInst->dwSessionInactivityTimeout))
						{
							
							hCurrentInetSession = GetInternetSession(psWdInst->psHostInst);

							if(hCurrentInetSession!=NULL)
							{
								HostTrace( psWdInst->psHostInst, "WatchdogThreadFunc ==> dwRequest:%u is too old (blocked). StartTime:%X, Now:%X.Start SafeCloseHandle!", 
									psWdInst->aReq[i].dwReqID,psWdInst->aReq[i].dwStartTime, dwNow );

								bHandleClosedOK = SafeCloseHandle(hCurrentInetSession, SAFE_WININET_RESONABLE_TIMEOUT);
								
								HostTrace( psWdInst->psHostInst, "WatchdogThreadFunc ==> SafeCloseHandle - %s!", bHandleClosedOK?"OK":"NOK");
								SetInternetSession(psWdInst->psHostInst, NULL);
								
								if(bHandleClosedOK)
								{
									clearAllWhatchReq(psWdInst);
									
									HostTrace( psWdInst->psHostInst, "WatchdogThreadFunc ==>Internet session reset!********");
								}
								else
								{
									HostTraceDirect( psWdInst->psHostInst, "FATAL ERROR! WatchdogThreadFunc ==> InternetSession could not be closed! Closing process...");
									Sleep(1000);
									
									ExitProcess(0xBAD);
								}

								break;
							}
							else
							{
								HostTrace( psWdInst->psHostInst, "WatchdogThreadFunc ==> dwRequest:%u is too old, but session already closed!");
							}
						}

					}	
				}

			}
			__finally
			{
				LeaveCriticalSection(psWdInst->pCS);
			}

			TimerActivate( psWdInst->hTmrCr, psWdInst->dwTimeout ); 

		}
	} 
	
	return 0; 
} 

PRIVATE void clearAllWhatchReq(WATCH_DOG_THREAD_INST * psWdInst)
{   
	int i;
	

	for(i = 0; i< MAX_REQ-1; i++)
	{
		psWdInst->aReq[i].bUsed = FALSE;
	}

	psWdInst->dwActiveRequests = 0;

}




PRIVATE WHATCH_REQ * getFirstFreeReq(WATCH_DOG_THREAD_INST * psWdInst)
{   
	int i;

	
	for(i = 0; i< MAX_REQ-1; i++)
	{
		if(psWdInst->aReq[i].bUsed == FALSE)
			return &psWdInst->aReq[i];
	}


	return NULL;
}

PRIVATE void removeReqWithID(WATCH_DOG_THREAD_INST * psWdInst, DWORD dwReqID)
{   
	int i;
	
	for(i = 0; i< MAX_REQ-1; i++)
	{
		if(psWdInst->aReq[i].dwReqID == dwReqID)
		{
			psWdInst->aReq[i].bUsed = FALSE;
			
			if(psWdInst->dwActiveRequests > 0)
				psWdInst->dwActiveRequests = psWdInst->dwActiveRequests-1;
			
			break;
		}	
	}
	
	return;
}


PROTECTED DWORD AppendCheckIfConnectionBlocked(WATCH_DOG_THREAD_INST * psWdInst, HINTERNET hSession)
{
	WHATCH_REQ *  pReq = NULL;
	DWORD dwNow = timeGetTime();
	DWORD  dwNewRequest = 0;
	DWORD  dwActiveRequests;

	if(hSession==NULL)
		return 0;

	__try{

		EnterCriticalSection(psWdInst->pCS);
		
			pReq = getFirstFreeReq(psWdInst);
			
			if(pReq!=NULL)
			{	
				dwNewRequest = ++psWdInst->dwReqIDSequence;
				
				if(dwNewRequest>0xF000)
				{
					psWdInst->dwReqIDSequence = dwNewRequest = 1;
				}	

				psWdInst->dwActiveRequests = psWdInst->dwActiveRequests + 1;
				dwActiveRequests = psWdInst->dwActiveRequests;
				pReq->bUsed = TRUE;
				pReq->dwReqID = dwNewRequest;
				pReq->hSession = hSession;
				pReq->dwStartTime = dwNow;
			}		
	}
	__finally
	{
		LeaveCriticalSection(psWdInst->pCS);
	}

	//HostTrace( psWdInst->psHostInst, "AppendCheckIfConnectionBlocked ==> dwNewRequestID:%u, dwActiveRequests:%u", dwNewRequest,dwActiveRequests);

	return dwNewRequest;
}

PROTECTED VOID RemoveCheckIfConnectionBlocked(WATCH_DOG_THREAD_INST * psWdInst, DWORD dwReqID)
{
	DWORD  dwActiveRequests;

	if(dwReqID==0)
		return;

	__try{
		EnterCriticalSection(psWdInst->pCS);
			
			removeReqWithID(psWdInst, dwReqID);
			dwActiveRequests = psWdInst->dwActiveRequests;
	}
	__finally
	{
		LeaveCriticalSection(psWdInst->pCS);
	}

	//HostTrace( psWdInst->psHostInst, "RemoveCheckIfConnectionBlocked ==> dwReqID:%u, dwActiveRequests:%u", dwReqID, dwActiveRequests);
}


PROTECTED BOOL CreateWatchdogThread(HOST_INST *psInst, WATCH_DOG_THREAD_INST **ppWATCH_DOG_THREAD_INST, DWORD dwTimeout, CRITICAL_SECTION * pCS, DWORD dwSessionInactivityTimeout)
{
	DWORD IDThread; 
	WATCH_DOG_THREAD_INST *pWATCH_DOG_THREAD_INST;

	*ppWATCH_DOG_THREAD_INST = (WATCH_DOG_THREAD_INST*)malloc(sizeof(WATCH_DOG_THREAD_INST));

	pWATCH_DOG_THREAD_INST = *ppWATCH_DOG_THREAD_INST;
	memset(pWATCH_DOG_THREAD_INST,0, sizeof(WATCH_DOG_THREAD_INST));

	pWATCH_DOG_THREAD_INST->pCS = pCS;

	pWATCH_DOG_THREAD_INST->hEndEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	if ( pWATCH_DOG_THREAD_INST->hEndEvent == NULL )
		return FALSE; 

	if ( TimerOpen( &(pWATCH_DOG_THREAD_INST->hTmrCr) ) != NO_ERROR )
	{
		return FALSE;
	}
	
	pWATCH_DOG_THREAD_INST->dwTimeout = dwTimeout+100;
	pWATCH_DOG_THREAD_INST->dwSessionInactivityTimeout = dwSessionInactivityTimeout;
	pWATCH_DOG_THREAD_INST->psHostInst = psInst;

	pWATCH_DOG_THREAD_INST->hThrd = CreateThread(NULL,  // no security attributes 
							0,                // use default stack size 
							(LPTHREAD_START_ROUTINE) WatchdogThreadFunc, 
							(LPVOID)pWATCH_DOG_THREAD_INST, // param to thread func 
							CREATE_SUSPENDED, // creation flag 
							&IDThread);       // thread identifier 
	
	if (pWATCH_DOG_THREAD_INST->hThrd == NULL) 
	{
		TimerClose(pWATCH_DOG_THREAD_INST->hTmrCr); 
		CloseHandle( pWATCH_DOG_THREAD_INST->hEndEvent);
		free(pWATCH_DOG_THREAD_INST);
		return FALSE;
	}
	else
	{
		ResumeThread(pWATCH_DOG_THREAD_INST->hThrd);
		TimerActivate( pWATCH_DOG_THREAD_INST->hTmrCr, pWATCH_DOG_THREAD_INST->dwTimeout ); 
		return TRUE;
	}	
}

PROTECTED BOOL StopWatchdogThread(WATCH_DOG_THREAD_INST *pWATCH_DOG_THREAD_INST)
{
	BOOL bThreadExitOK = FALSE;

	TimerCancel(pWATCH_DOG_THREAD_INST->hTmrCr); 
	
	SetEvent(pWATCH_DOG_THREAD_INST->hEndEvent);
	bThreadExitOK = WaitThreadToExit(pWATCH_DOG_THREAD_INST->hThrd, SAFE_WININET_RESONABLE_TIMEOUT);

	TimerClose(pWATCH_DOG_THREAD_INST->hTmrCr); 
	CloseHandle(pWATCH_DOG_THREAD_INST->hEndEvent);
	CloseHandle(pWATCH_DOG_THREAD_INST->hThrd);

	free(pWATCH_DOG_THREAD_INST);

	return bThreadExitOK;
}



