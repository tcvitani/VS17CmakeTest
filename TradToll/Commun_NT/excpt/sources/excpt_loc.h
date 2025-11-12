#ifndef EXCPT_LOC_H
#define EXCPT_LOC_H

#include <protect.h>


typedef struct _EXCPT_THREAD
{
    LPTHREAD_START_ROUTINE      pfStartAddress;
    void                      * pvParameter;
    void                      * pvExcptHook;
    char                        szName[MAX_PATH];
}
EXCPT_THREAD;



PROTECTED char * WINAPI ExcptDumpTime( 
        char * pcString, 
        DWORD * pdwStringSize );

PROTECTED char * WINAPI ExcptDumpException( 
        EXCEPTION_RECORD * psExcpt,
        char * pcString,
        DWORD * pdwStringSize );

PROTECTED char * WINAPI ExcptDumpModules( 
        HANDLE hProcess,
        char * pcString, 
        DWORD * pdwStringSize );

PROTECTED char * WINAPI ExcptDumpRegisters( 
        CONTEXT * psCtx,
        char * pcString,
        DWORD * pdwStringSize );

PROTECTED char * WINAPI ExcptDumpStack( 
        HANDLE hProcess,
        HANDLE hThread,
        CONTEXT * psCtx, 
        char * pcString, 
        DWORD * pdwStringSize );

PROTECTED char * WINAPI ExcptRawDumpStack( 
        HANDLE hProcess,
        HANDLE hThread,
        CONTEXT * psCtx,
        char * pcString,
        DWORD * pdwStringSize );

PROTECTED char * WINAPI ExcptDetailedDumpStack( 
        HANDLE hProcess,
        HANDLE hThread,
        CONTEXT * psCtx, 
        char * pcString, 
        DWORD * pdwStringSize );

PROTECTED void WINAPI ExcptGetLogicalAddress( 
		PVOID pvAddr,
        char * szModule, 
        long sizeModule, 
        DWORD * pdwSection, 
		PVOID * pdwOffset);

PROTECTED char * WINAPI ExcptGetExceptionString( 
        DWORD dwCode );

PROTECTED char * WINAPI ExcptGetPriorityString( 
        DWORD dwCode );

PROTECTED char * WINAPI ExcptGetPriorityClassString( 
        DWORD dwCode );

PROTECTED char * WINAPI ExcptPrint( 
        char * pcString, 
        DWORD * pdwSize, 
        char * pcFormat, 
        ... );

PROTECTED DWORD WINAPI ExcptThreadWrapper( 
        void * pvThread );

PROTECTED char * WINAPI ExcptBuildSearchPath( 
        char * szList, 
        DWORD dwListSize );



#endif