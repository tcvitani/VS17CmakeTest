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
#ifndef CSR_RFA_H
#define CSR_RFA_H

#ifdef LOC_DEF
	#include <public.h>
#else
	#include <export.h>
#endif


#ifndef INVALID_SET_FILE_POINTER
	#define INVALID_SET_FILE_POINTER 0xFFFFFFFF
#endif


#define RFA_LL_HI(ll) (((DWORD*)(&(ll)))[1])
#define RFA_LL_LO(ll) (((DWORD*)(&(ll)))[0])


typedef struct _RFA_STRUCT RFA_STRUCT, * RFA_HANDLE;


EXPORT HANDLE WINAPI RFACreateFile(
        LPCTSTR lpFileKey,
        DWORD dwDesiredAccess,
        DWORD dwShareMode,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        DWORD dwCreationDisposition,
        DWORD dwFlagsAndAttributes,
        HANDLE hTemplateFile );

EXPORT BOOL WINAPI RFACloseHandle(
        HANDLE hObject );

EXPORT BOOL WINAPI RFAEnter( HANDLE hFile );

EXPORT BOOL WINAPI RFALeave( HANDLE hFile );

EXPORT BOOL WINAPI RFADeleteAndCloseHandle(
        HANDLE hObject );

EXPORT BOOL WINAPI RFAWriteFile(
        HANDLE hFile,
        LPCVOID lpBuffer,
        DWORD nNumberOfBytesToWrite,
        LPDWORD lpNumberOfBytesWritten,
        LPOVERLAPPED lpOverlapped );

EXPORT BOOL WINAPI RFAReadFile(
        HANDLE hFile,
        LPVOID lpBuffer,
        DWORD nNumberOfBytesToRead,
        LPDWORD lpNumberOfBytesRead,
        LPOVERLAPPED lpOverlapped );

EXPORT BOOL WINAPI RFAGetFileSize(
        HANDLE hFile,
        LPDWORD lpFileSizeHigh );

EXPORT BOOL WINAPI RFASetFileTime(
        HANDLE hFile,
        CONST FILETIME * lpCreationTime,
        CONST FILETIME * lpLastAccessTime,
        CONST FILETIME * lpLastWriteTime );

EXPORT BOOL WINAPI RFAGetFileTime(
        HANDLE hFile,
        FILETIME * lpCreationTime,
        FILETIME * lpLastAccessTime,
        FILETIME * lpLastWriteTime );

EXPORT DWORD WINAPI RFASetFilePointer(
        HANDLE hFile,
        LONG lDistanceToMove,
        PLONG lpDistanceToMoveHigh,
        DWORD dwMoveMethod );

EXPORT BOOL WINAPI RFASetEndOfFile(
        HANDLE hFile );

EXPORT BOOL WINAPI RFADeleteFile(
        LPCTSTR lpFileKey );

EXPORT BOOL WINAPI RFAMoveFileEx(
        LPCTSTR lpExistingFileName,
        LPCTSTR lpFileKey,
        DWORD dwFlags );

EXPORT BOOL WINAPI RFACopyFile(
        LPCTSTR lpExistingFileName,
        LPCTSTR lpFileKey,
        BOOL bFailIfExists );

EXPORT BOOL WINAPI RFACopyFileKey(
        LPCTSTR lpExistingFileKey,
        LPCTSTR lpNewFileName,
        BOOL bFailIfExists );


#endif /* CSR_RFA_H */
