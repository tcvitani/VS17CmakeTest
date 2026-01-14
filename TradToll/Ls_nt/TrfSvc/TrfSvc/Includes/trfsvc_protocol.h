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

#ifndef TRFSVC_PROTOCOL_H
#define TRFSVC_PROTOCOL_H

#include <protect.h>


#define FSYNC_MASK_REQ          0x00000000
#define FSYNC_MASK_RSP          0x80000000
#define FSYNC_MASK_NF           0x40000000



PROTECTED BOOL FSYNC_IS_ID_REQ(void * pvMsg, DWORD dwSize);
PROTECTED DWORD FSYNC_SIZEOF_ID_REQ(void * pvMsg);

PROTECTED BOOL FSYNC_IS_ID_RSP(void * pvMsg, DWORD dwSize);
PROTECTED DWORD FSYNC_SIZEOF_ID_RSP(void * pvMsg);



PROTECTED BOOL FSYNC_IS_FULL_NF(void * pvMsg, DWORD dwSize);
PROTECTED DWORD FSYNC_SIZEOF_FULL_NF(void * pvMsg);


PROTECTED BOOL FSYNC_IS_PARTIAL_NF(void * pvMsg, DWORD dwSize);
PROTECTED DWORD FSYNC_SIZEOF_PARTIAL_NF(void * pvMsg);


PROTECTED BOOL FSYNC_IS_PING_REQ(void * pvMsg, DWORD dwSize);
PROTECTED DWORD FSYNC_SIZEOF_PING_REQ(void * pvMsg);


PROTECTED BOOL FSYNC_IS_PING_RSP(void * pvMsg, DWORD dwSize);
PROTECTED DWORD FSYNC_SIZEOF_PING_RSP(void * pvMsg);




typedef struct FSYNC_MSG_HEADER_REQ
{
        DWORD       dwMessageCode; 
        DWORD       dwRequestId;
}
FSYNC_MSG_HEADER_REQ;



typedef struct FSYNC_MSG_HEADER_RSP
{
        DWORD       dwMessageCode; 
        DWORD       dwRequestId;
        DWORD       dwResultCode;
}
FSYNC_MSG_HEADER_RSP;



typedef struct FSYNC_MSG_HEADER_NF
{
        DWORD       dwMessageCode; 
}
FSYNC_MSG_HEADER_NF;



#define FSYNC_CODE_ID_REQ 0x00000001

typedef struct FSYNC_MSG_ID_REQ
{
    DWORD       dwMessageCode; 
    DWORD       dwRequestId;
    
    DWORD       dwNameSize;
    DWORD       dwKeySize;

    BYTE        tbNameAndKey[];
}
FSYNC_MSG_ID_REQ;



#define FSYNC_CODE_ID_RSP 0x80000001

typedef struct FSYNC_MSG_ID_RSP
{
    DWORD       dwMessageCode; 
    DWORD       dwRequestId;
    DWORD       dwResultCode;
}
FSYNC_MSG_ID_RSP;


#define FSYNC_CODE_FULL_NF 0x40000002

typedef struct FSYNC_MSG_FULL_NF
{
    DWORD       dwMessageCode; 

    ULONGLONG   ullLastWrite;
    ULONGLONG   ullFileSize;
    ULONGLONG   ullOffset;
    DWORD       dwDataSize;
    BYTE        tbData[];
}
FSYNC_MSG_FULL_NF;



#define FSYNC_CODE_PARTIAL_NF 0x40000003

typedef struct FSYNC_MSG_PARTIAL_NF
{
    DWORD       dwMessageCode; 

    ULONGLONG   ullLastWrite;
    ULONGLONG   ullFileSize;
    ULONGLONG   ullOffset;
    DWORD       dwDataSize;
    BYTE        tbData[];
}
FSYNC_MSG_PARTIAL_NF;


#define FSYNC_CODE_PING_REQ 0x40000004

typedef struct FSYNC_MSG_PING_REQ
{
    DWORD       dwMessageCode; 
    DWORD       dwCnxId;
}
FSYNC_MSG_PING_REQ;


#define FSYNC_CODE_PING_RSP 0x40000005

typedef struct FSYNC_MSG_PING_RSP
{
    DWORD       dwMessageCode; 
    DWORD       dwCnxId;
}
FSYNC_MSG_PING_RSP;




#endif
    