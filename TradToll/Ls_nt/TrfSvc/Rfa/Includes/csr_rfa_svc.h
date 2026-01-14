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
#ifndef CSR_RFA_SVC_H
#define CSR_RFA_SVC_H

#ifdef LOC_DEF
#include <public.h>
#else
#include <export.h>
#endif


#define RFA_SERVICE_ACCESS 0xCEFADEBA


#define RFA_CALLBACK_DATE       ((ULONGLONG)(1))
#define RFA_CALLBACK_CUT        ((ULONGLONG)(2))
#define RFA_CALLBACK_NEW        ((ULONGLONG)(3))
#define RFA_CALLBACK_DEL        ((ULONGLONG)(4))
#define RFA_CALLBACK_REPLACE    ((ULONGLONG)(5))


typedef struct _RFA_STRUCT RFA_STRUCT, * RFA_HANDLE;

typedef void CALLBACK RFA_WRITE_HOOK( RFA_STRUCT * psHandle, ULONGLONG ullPos, DWORD dwSize, BYTE * pbBuffer );

typedef struct _RFA_STRUCT
{
    DWORD dwMark;

    BOOL bImport;
    BOOL bExport;
    BOOL bWriteAccess;
    BOOL bFailMode;

    ULONGLONG ullPointer;
    ULONGLONG ullSize;
    ULONGLONG ullLastWrite;

    HANDLE hMutex;

    RFA_WRITE_HOOK * pfWriteHook;

    char szKey[MAX_PATH];
    char szFile[MAX_PATH];
    char szMutex[MAX_PATH];
}
RFA_STRUCT, * RFA_HANDLE;

EXPORT BOOL WINAPI RFASetHook(
        void * pfWriteHook );

#endif /* CSR_RFA_SVC_H */
