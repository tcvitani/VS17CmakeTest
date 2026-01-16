/* --------------------------------------------------------------------
 * (C) 2005 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : CMFTP
 * FILE       : CMFTP.H
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : Commoc, ras
 * --------------------------------------------------------------------
 * SUMMARY    : Extension FTP pour commoc.
 * --------------------------------------------------------------------
 * DESCRIPTION: Extension de communication pour commoc. Cette librairie
 *              fournie l'ensemble des fonctions requises par Commoc
 *              pour mettre en oeuvre des échanges de fichiers basés
 *              sur FTP.
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef SAFE_WININET_H
#define SAFE_WININET_H

#include <protect.h>

#define SAFE_WININET_RESONABLE_TIMEOUT 60000

typedef struct _WATCH_DOG_THREAD_INST WATCH_DOG_THREAD_INST;
typedef struct _HOST_INST HOST_INST;

PROTECTED BOOL CreateWatchdogThread(HOST_INST *psInst, WATCH_DOG_THREAD_INST **ppWATCH_DOG_THREAD_INST, DWORD dwTimeout, CRITICAL_SECTION * pCS, DWORD dwSessionInactivityTimeout);
PROTECTED BOOL StopWatchdogThread(WATCH_DOG_THREAD_INST *pWATCH_DOG_THREAD_INST);
PROTECTED DWORD AppendCheckIfConnectionBlocked(WATCH_DOG_THREAD_INST *pWATCH_DOG_THREAD_INST, HINTERNET hSession); 
PROTECTED VOID RemoveCheckIfConnectionBlocked(WATCH_DOG_THREAD_INST *pWATCH_DOG_THREAD_INST, DWORD dwReqID);



#endif
