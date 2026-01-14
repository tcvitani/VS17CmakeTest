#ifndef TOLANESSVC_DB_H
#define TOLANESSVC_DB_H

#include <protect.h>

PROTECTED DWORD DBInitRequests();
PROTECTED DWORD DBImportList( DB_CNX ** phCnx, char * pcType, char * pcBuffer, DWORD dwBufferSize,
                              char * szFile, char * szErrFile, HANDLE hLog );
PROTECTED DWORD DBGetExternalFileName( DB_CNX ** phCnx, char * pcType, char * pcFile, DWORD dwFileSize );
PROTECTED DWORD DBSetExternalFileName( DB_CNX ** phCnx, char * pcType, char * pcFile );
PROTECTED BOOLEAN WINAPI DBReplaceText( char * szText, DWORD dwMaxBytes, char * szReplace, char * szBy );
PROTECTED BOOL WINAPI DBFormatText( char * szFormat, char * szText, DWORD dwMaxBytes, ... );


#endif
