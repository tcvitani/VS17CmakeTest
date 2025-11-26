/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : TAG
 * FILE       : CSR_TAG.H
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Définitions exportées
 * --------------------------------------------------------------------
 * HISTORY    :
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef TAG_H
#define TAG_H

#ifdef TAG_EXPORTS
#include <public.h>
#else
#include <export.h>
#endif


#pragma pack( push, TAG_PACK, 1 )


//
// Constantes pour l'espionnage de la liaison
//
#define TAG_HOOK_DIRECTION_FILTER    0x30000000
#define TAG_HOOK_INPUT               0x10000000
#define TAG_HOOK_OUTPUT              0x20000000
#define TAG_HOOK_TYPE_FILTER         0xC0000000
#define TAG_HOOK_PROTOCOL            0x40000000
#define TAG_HOOK_DATA                0x80000000
#define TAG_HOOK_COMMENT             0x01000000
#define TAG_HOOK_FLUSH               0x00000001


//
// Constantes pour le contrôle des données émises
//
#define TAG_RETRY_LAST_REQUEST       ((CENL7_API*)0xFFFFFFFF)



//
// Définition du callback pour l'espionnage
//
typedef void WINAPI TAG_HOOK( void * pvHookContext, DWORD dwTypeDir, BYTE * pbData, DWORD dwBytes );
typedef void * TAG_HOOK_CONTEXT;

typedef struct _TAG_HOOK_LOG TAG_HOOK_LOG;



#define TAG_NO_SUBL7    ((void*)1)


#define TAG_TRUNCATE_RESPONSE   0x80000000


//
// Type d'interface
//
typedef enum
{
	//The tag is connected directly to the serial port using some
	// special CS adapter (for old CS tags only) 
    TAG_INTERFACE_WIRED                    = 0, 
	
	//The TRSP compatible beacon is connected to the serial port
	// any DSRC compatible tag type can be used
    TAG_INTERFACE_COMMUNICATOR             = 1,
    TAG_INTERFACE_COMMUNICATOR_FAST        = 2,

	// The specific GEA beacon is used
	// This type of beacon does not have TRSP and it for communication
	// the producer dll driver is used
    TAG_INTERFACE_ISO7816                  = 3,

	// The specific QFREE  beacon is used
	// This beacon uses QFree L2AppSrv service and named pipes 
	// to communicate with the beacon and the tag
	TAG_INTERFACE_QFREE					   = 4,

	// GEA IP beacon 
	// this beacon uses producer provided "BeaconManager.dll" 
	TAG_INTERFACE_IPGEA					   = 5,

	// The QFREE MD5850 MR with DMI DSRC Interface beacon is used
	TAG_INTERFACE_QFREE_DMI						= 6,

	// Used to get interface type from the TAG_INTERFACE_TYPES variable
    TAG_INTERFACE_MASK                     = 0x000000FF,

	// Used to get the way to get params from the TAG_INTERFACE_TYPES variable
    TAG_INTERFACE_FLAG_BUILT_PARAMS        = 0x80000000,
    TAG_INTERFACE_FLAG_REGISTRY_PARAMS     = 0,

    TAG_INTERFACE_FLAG_CONSOLE_TRACE       = 0x40000000,

    TAG_INTERFACE_FLAG_COLLISION_POSTPONE  = 0x00000000,
    TAG_INTERFACE_FLAG_COLLISION_CANCEL    = 0x20000000,
}
    TAG_INTERFACE_TYPES;


//
// Etats de session
//
typedef enum
{
    TAG_SESSION_BEGIN,
    TAG_SESSION_PROCESS,
    TAG_SESSION_END,
}
    TAG_SESSION_STATES;


typedef enum
{
    TAG_STATUS_NONE                = 0x00000000,

    TAG_STATUS_SESSION_DATA        = 0x00000001,
    TAG_STATUS_ERROR               = 0x00000002,

    TAG_STATUS_ERR_TRANSACTING     = 0x00000100,
    TAG_STATUS_ERR_MODE            = 0x00000200,

    TAG_STATUS_ERR_REFUSED         = 0x00010000,
    TAG_STATUS_ERR_HARDWARE        = 0x00020000,
    TAG_STATUS_ERR_TIMEOUT         = 0x00040000,
    TAG_STATUS_ERR_RESET           = 0x00080000,
    TAG_STATUS_ERR_PARAM           = 0x00100000,
    TAG_STATUS_ERR_CONFIG          = 0x00200000,

    TAG_STATUS_ERR_UNKNOWN         = 0x80000000,
    
    TAG_STATUS_ERR_ANY             = 0xFFFF0000,
}
    TAG_INTERFACE_STATUS;


//
// Niveaux de reset
//
typedef enum
{
    TAG_REQUEST_MODE,
    TAG_REQUEST_ABORT,
    TAG_REQUEST_STATUS,

    TAG_REQUEST_CUSTOM = 0xFFFFFFFF,
}
    TAG_INTERFACE_REQUEST;


//
// Définition de la structure d'instance (contenu non exporté)
//
typedef struct _TAG_INSTANCE TAG_INSTANCE;



EXPORT BOOL WINAPI TagIsVersionCompatible(
                IN   DWORD  dwMajor, 
                IN   DWORD  dwMinor, 
                IN   DWORD  dwBuild );

EXPORT DWORD WINAPI TagOpen(
                IN   TAG_INTERFACE_TYPES     eInterface,
                IN   char                  * pcRegPath,
                IN   TAG_HOOK              * pfHook,
                IN   TAG_HOOK_CONTEXT        pvHookContext,
                OUT  TAG_INSTANCE         ** ppsInst );

EXPORT void * WINAPI TagBuildCommunicatorParams( 
                IN   DWORD        dwMaxEnqRetry,
                IN   DWORD        dwMaxMsgRetry,
                IN   DWORD        dwTimer1,
                IN   DWORD        dwTimer2,
                IN   DWORD        dwMaxPendingMsg,
                IN   DWORD        dwPort,
                IN   DWORD        dwPriority,
                IN   DWORD        dwAutoClearBuffer,
                IN   const char * szPortSettings );

EXPORT void * WINAPI TagBuildWiredParams( 
                IN   DWORD        dwTimer,
                IN   DWORD        dwMaxPendingMsg,
                IN   DWORD        dwPort,
                IN   DWORD        dwPriority,
                IN   DWORD        dwAutoClearBuffer,
                IN   const char * szPortSettings );

EXPORT void * WINAPI TagBuildCommunicatorFastParams( 
                IN   DWORD        dwTimer,
                IN   DWORD        dwMaxPendingMsg,
                IN   DWORD        dwPort,
                IN   DWORD        dwPriority,
                IN   DWORD        dwAutoClearBuffer,
                IN   const char * szPortSettings );

EXPORT void * WINAPI TagBuildIso7816Params( 
                IN   DWORD        dwMaxPendingMsg,
                IN   DWORD        dwPort,
                IN   DWORD        dwDebit,
                IN   DWORD        dwPriority,
                IN   DWORD        dwAutoClearBuffer,
                IN   const char * szDownloadFile,
                IN   const char * szTransportDll,
                IN   const char * szLowLevelDll );

EXPORT void * WINAPI TagBuildQFREEParams( 
				IN   DWORD        dwMaxPendingMsg,
				IN   DWORD        dwPriority,
				IN   DWORD        dwAutoClearBuffer,
				IN   const char * szPortSettings,
				IN   const char * szL7DataPipeName,
				IN   const char * szSecurityDataPipeName );

EXPORT void * WINAPI TagBuildIpGEAParams( 
				IN   DWORD        dwMaxPendingMsg,
				IN   DWORD        dwPriority,
				IN   DWORD        dwAutoClearBuffer,
				IN   const char * szIpAddres,
				IN   DWORD		  dwPort,
				IN   DWORD		  dwCheckPoolingPeriod,
				IN   DWORD		  dwConnectionTimeout,
				IN	 DWORD		  dwChangeBeaconID,
				IN	DWORD		  dwUseFrequency);

EXPORT void * WINAPI TagBuildQFDMIParams(IN   DWORD        dwMaxPendingMsg,
											IN   DWORD        dwPriority,
											IN   DWORD        dwAutoClearBuffer,
											IN   DWORD		  dwListenPort,
											IN   const char * szHTTPServerIP,
											IN   DWORD		  dwHTTPServer_Port,
											IN   DWORD		  dwDMITimerDelayMs,
											IN   DWORD		  dwTrsTimerDelayMs,
											IN   DWORD		  dwHighBeaconId,
											IN   DWORD		  dwLowBeaconId,
											IN   DWORD		  dwUseMDREnable,
											IN   const char * szMdrEnable_ParamUrl,
											IN   const char * szChangeBeaconID_ParamUrl);

EXPORT void WINAPI TagDestroyParams( 
                IN   void * pvParams );

EXPORT void * WINAPI TagGetL1(
                IN  TAG_INSTANCE  * psInst );

EXPORT void * WINAPI TagGetL2(
                IN  TAG_INSTANCE  * psInst );


EXPORT void * WINAPI TagGetL7(
                IN  TAG_INSTANCE  * psInst );


EXPORT void * WINAPI TagGetSL7(
                IN  TAG_INSTANCE  * psInst );


EXPORT void WINAPI TagClose(
                IN  TAG_INSTANCE  * psInst,
                IN  DWORD           dwTimeout );

EXPORT HANDLE WINAPI TagGetWaitableHandle(
                IN  TAG_INSTANCE  * psInst );


EXPORT DWORD WINAPI TagInterfaceRequestEx( 
                IN              TAG_INSTANCE          * psInst,
                IN              TAG_INTERFACE_REQUEST   eReq,
                OUT             BOOL                  * pfResponse,
                OPTIONAL IN     HANDLE                  hEvent, 
                OPTIONAL OUT    DWORD                 * pdwErr,
                IN              DWORD                   dwDataBytes,
                OPTIONAL IN     BYTE                  * pbDataBytes );

EXPORT DWORD WINAPI TagInterfaceRequest( 
                IN              TAG_INSTANCE          * psInst,
                IN              TAG_INTERFACE_REQUEST   eReq,
                OUT             BOOL                  * pfResponse,
                OPTIONAL IN     HANDLE                  hEvent, 
                OPTIONAL OUT    DWORD                 * pdwErr );

EXPORT DWORD WINAPI TagSessionRequest(
                IN              TAG_INSTANCE      * psInst,
                IN              TAG_SESSION_STATES  eSession,
                IN              CENL7_API         * psReqTable,
                IN              DWORD               dwReqCount,
                OPTIONAL IN     HANDLE              hEvent, 
                OPTIONAL OUT    DWORD             * pdwErr );


EXPORT DWORD WINAPI TagResponseEx(
                IN              TAG_INSTANCE          * psInst,
                IN              CENL7_API             * psRspTable,
                IN OUT          DWORD                 * pdwReqCount,
                OUT             TAG_INTERFACE_STATUS  * peStatus,
                IN              DWORD                   dwTimeout,
                OPTIONAL IN OUT DWORD                 * pdwDataBytes,
                OPTIONAL OUT    BYTE                  * pbDataBytes);

EXPORT DWORD WINAPI TagResponse(
                IN              TAG_INSTANCE          * psInst,
                IN              CENL7_API             * psRspTable,
                IN OUT          DWORD                 * pdwReqCount,
                OUT             TAG_INTERFACE_STATUS  * peStatus,
                IN              DWORD                   dwTimeout );


EXPORT void * WINAPI TagHookOpenDefaultContext( IN  TAG_INSTANCE          * psInst, 
                                                OPTIONAL IN HWND            hWnd,
                                                OPTIONAL IN DWORD           dwThreadId,
                                                OPTIONAL IN TAG_HOOK_LOG  * psLog );


EXPORT void WINAPI TagHookCloseDefaultContext( void * pvHookContext );


EXPORT UINT WINAPI TagHookGetDefaultWM();

EXPORT TAG_HOOK * WINAPI TagHookGetDefaultHook();

EXPORT TAG_HOOK_LOG * WINAPI TagHookLogOpen();

EXPORT BOOL WINAPI TagHookLogPullData( IN     TAG_HOOK_LOG * psLog,
                                        IN OUT DWORD        * pdwBytes,
                                        OUT    BYTE         * pbData );

EXPORT void WINAPI TagHookLogClear( IN TAG_HOOK_LOG * psLog );

EXPORT void WINAPI TagHookLogSuspend( IN TAG_HOOK_LOG * psLog,
                                      IN BOOL           fSuspended );

EXPORT void WINAPI TagHookLogClose( IN TAG_HOOK_LOG * psLog );





#pragma pack( pop, TAG_PACK )

//
// In reference to TAG_H
//
#endif
