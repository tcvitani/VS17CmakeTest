/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     ACOM.H                                                          */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef ACOM_H
#define ACOM_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>

#ifdef ACOM_EXPORTS
	#include <public.h>
#else
	#include <export.h>
#endif

/*-------------------------------- RESERVED:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

// Handle de working set
typedef void * ACOM_WKS_HANDLE;

// Handle d'instance
typedef void * ACOM_INST_HANDLE;

// Handle de connexion
typedef DWORD ACOM_CNX_HANDLE;

// Handle de connexion
typedef LPARAM ACOM_EVT_HANDLE;

// Intervalle autorisé pour le nombre de connexions dans un working set
#define ACOM_MIN_WKS_CONNECTIONS            (1)
#define ACOM_MAX_WKS_CONNECTIONS            (512)

// Obtenir le numéro de slot d'après le handle
#define ACOM_GET_SLOT_NUMBER(hCnx)          (((DWORD)hCnx)%ACOM_MAX_WKS_CONNECTIONS)

// TYPES DE PROTOCOLES
#define ACOM_PROTOCOL_MIN     0
#define ACOM_PROTOCOL_DEFAULT 0
#define ACOM_PROTOCOL_LIFE    1
#define ACOM_PROTOCOL_MAX     1

#define ACOM_GET_PROTOCOL(x) (0x0000000F & (x))

#define ACOM_FLAG_GET_PEER_NAME     0
#define ACOM_FLAG_NO_GET_PEER_NAME  0x80000000
#define ACOM_GET_PROTOCOL(x) (0x0000000F & (x))

#define ACOM_FLAG_FIXED_LIFETIME      0
#define ACOM_FLAG_LIFETIME_FACTOR(x)  (((x)&0x00000007)<<24)
#define ACOM_GET_LIFETIME_FACTOR(x)  (((x)&0x07000000)>>24)

// WINDOW MESSAGES
// Nom texte du window message utilisé pour signaler les événements
#define ACOM_WM_EVENT_NAME   "WM_ACOM_EVENT"

/*-------------------------------- TYPEDEFS:  -------------------------------*/

// Type des événements signalé dans les window messages
typedef enum ACOM_WM_TYPE 
{
    ACOM_WM_SHUTDOWN        = 1,
    ACOM_WM_CONNECTION      = 2,
    ACOM_WM_DISCONNECTION   = 3,
    ACOM_WM_RECEIVED        = 4,
    ACOM_WM_SENT            = 5,
    ACOM_WM_TIME_SHIFT      = 6
}
ACOM_WM_TYPE;

/*-------------------------------- FUNCTIONS: -------------------------------*/

typedef DWORD64 CALLBACK ACOM_CALLBACK_CONNECTION(DWORD64 dwInstUsrKey,
												ACOM_CNX_HANDLE hCnxHandle);

typedef void CALLBACK ACOM_CALLBACK_RECEIVED(DWORD64 dwInstUsrKey,
											 DWORD64 dwCnxUsrKey,
											 DWORD dwDataSize,
											 void *pvData);

typedef void CALLBACK ACOM_CALLBACK_SENT(DWORD64 dwInstUsrKey,
										 DWORD64 dwCnxUsrKey,
										 DWORD64 dwMsgUsrKey,
										 DWORD dwError);

typedef void CALLBACK ACOM_CALLBACK_DISCONNECTION(DWORD64 dwInstUsrKey,
												  DWORD64 dwCnxUsrKey);

typedef void CALLBACK ACOM_CALLBACK_TIME_SHIFT(DWORD64 dwInstUsrKey,
											   DWORD64 dwCnxUsrKey);

typedef void CALLBACK ACOM_CALLBACK_SHUTDOWN(DWORD64 dwWksUsrKey,
											 DWORD dwError);

EXPORT ACOM_WKS_HANDLE WINAPI AComOpenWorkingSet(DWORD64 dwWksUsrKey,
												 DWORD dwMaxConnections,
												 DWORD dwMaxInstances,
												 DWORD dwPriority,
												 DWORD dwConnectLoopDelay,
												 DWORD dwWorkers,
												 DWORD dwProtocol,
												 DWORD dwLifeTime,
												 DWORD dwMaxLife,
												 DWORD dwAllowedTimeShift,
												 ACOM_CALLBACK_SHUTDOWN *pfShut);

EXPORT ACOM_WKS_HANDLE WINAPI AComOpenWorkingSetWM(DWORD64 dwWksUsrKey,
												   DWORD dwMaxConnections,
												   DWORD dwMaxInstances,
												   DWORD dwPriority,
												   DWORD dwConnectLoopDelay,
												   DWORD dwWorkers,
												   DWORD dwProtocol,
												   DWORD dwLifeTime,
												   DWORD dwMaxLife,
												   DWORD dwAllowedTimeShift,
												   HWND hWndEvents);

EXPORT ACOM_WKS_HANDLE WINAPI AComOpenWorkingSetTQ(DWORD64 dwWksUsrKey,
												   DWORD dwMaxConnections,
												   DWORD dwMaxInstances,
												   DWORD dwPriority,
												   DWORD dwConnectLoopDelay,
												   DWORD dwWorkers,
												   DWORD dwProtocol,
												   DWORD dwLifeTime,
												   DWORD dwMaxLife,
												   DWORD dwAllowedTimeShift,
												   DWORD dwThreadId);

EXPORT ACOM_INST_HANDLE WINAPI AComOpenPipeServerInstance(ACOM_WKS_HANDLE hWks,
														  char *pcPipeName,
														  DWORD64 dwInstUsrKey,
														  DWORD dwTimeToReconnect,
														  DWORD dwMaxConnections,
														  DWORD dwOutBufferSize,
														  DWORD dwInBufferSize,
														  DWORD dwQueueSize,
														  DWORD dwMaxMessageSize,
														  ACOM_CALLBACK_CONNECTION *pfCnx,
														  ACOM_CALLBACK_DISCONNECTION *pfDcnx,
														  ACOM_CALLBACK_RECEIVED *pfRecv,
														  ACOM_CALLBACK_SENT *pfSent,
														  ACOM_CALLBACK_TIME_SHIFT *pfTime);

EXPORT ACOM_INST_HANDLE WINAPI AComOpenPipeClientInstance(ACOM_WKS_HANDLE hWks,
														  char *pcPipeName,
														  DWORD64 dwInstUsrKey,
														  DWORD dwTimeToReconnect,
														  DWORD dwQueueSize,
														  DWORD dwMaxMessageSize,
														  ACOM_CALLBACK_CONNECTION *pfCnx,
														  ACOM_CALLBACK_DISCONNECTION *pfDcnx,
														  ACOM_CALLBACK_RECEIVED *pfRecv,
														  ACOM_CALLBACK_SENT *pfSent,
														  ACOM_CALLBACK_TIME_SHIFT *pfTime);

EXPORT ACOM_INST_HANDLE WINAPI AComOpenMailslotServerInstance(ACOM_WKS_HANDLE hWks,
															  char *pcMailslotName,
															  DWORD64 dwInstUsrKey,
															  DWORD dwTimeToReconnect,
															  DWORD dwMaxMessageSize,
															  ACOM_CALLBACK_CONNECTION *pfCnx,
															  ACOM_CALLBACK_DISCONNECTION *pfDcnx,
															  ACOM_CALLBACK_RECEIVED *pfRecv);

EXPORT ACOM_INST_HANDLE WINAPI AComOpenMailslotClientInstance(ACOM_WKS_HANDLE hWks,
															  char *pcMailslotName,
															  DWORD64 dwInstUsrKey,
														 	  DWORD dwTimeToReconnect,
															  DWORD dwQueueSize,
															  DWORD dwMaxMessageSize,
															  ACOM_CALLBACK_CONNECTION *pfCnx,
															  ACOM_CALLBACK_DISCONNECTION *pfDcnx,
															  ACOM_CALLBACK_SENT *pfSent);

EXPORT ACOM_INST_HANDLE WINAPI AComOpenTcpServerInstance(ACOM_WKS_HANDLE hWks,
														 char *pcAddress,
														 DWORD dwPort,
														 DWORD64 dwInstUsrKey,
														 DWORD dwTimeToReconnect,
														 DWORD dwMaxConnections,
														 DWORD dwOutBufferSize,
														 DWORD dwInBufferSize,
														 DWORD dwQueueSize,
														 DWORD dwMaxMessageSize,
														 ACOM_CALLBACK_CONNECTION *pfCnx,
														 ACOM_CALLBACK_DISCONNECTION *pfDcnx,
														 ACOM_CALLBACK_RECEIVED *pfRecv,
														 ACOM_CALLBACK_SENT *pfSent,
														 ACOM_CALLBACK_TIME_SHIFT *pfTime);

EXPORT ACOM_INST_HANDLE WINAPI AComOpenTcpClientInstance(ACOM_WKS_HANDLE hWks,
														 char *pcAddress,
														 DWORD dwPort,
														 DWORD64 dwInstUsrKey,
														 DWORD dwTimeToReconnect,
														 DWORD dwOutBufferSize,
														 DWORD dwInBufferSize,
														 DWORD dwQueueSize,
														 DWORD dwMaxMessageSize,
														 ACOM_CALLBACK_CONNECTION *pfCnx,
														 ACOM_CALLBACK_DISCONNECTION *pfDcnx,
														 ACOM_CALLBACK_RECEIVED *pfRecv,
														 ACOM_CALLBACK_SENT *pfSent,
														 ACOM_CALLBACK_TIME_SHIFT *pfTime);

EXPORT ACOM_INST_HANDLE WINAPI AComOpenSerialPortInstance(ACOM_WKS_HANDLE hWks,
														  DWORD dwPort,
														  char *pcParameters,
														  DWORD64 dwInstUsrKey,
														  DWORD dwTimeToReconnect,
														  DWORD dwOutBufferSize,
														  DWORD dwInBufferSize,
														  DWORD dwQueueSize,
														  DWORD dwMaxMessageSize,
														  DWORD dwReadInterval,
														  ACOM_CALLBACK_CONNECTION *pfCnx,
														  ACOM_CALLBACK_DISCONNECTION *pfDcnx,
														  ACOM_CALLBACK_RECEIVED *pfRecv,
														  ACOM_CALLBACK_SENT *pfSent,
														  ACOM_CALLBACK_TIME_SHIFT *pfTime );

EXPORT ACOM_INST_HANDLE WINAPI AComOpenPipeServerInstanceWM(ACOM_WKS_HANDLE hWks,
															char *pcPipeName,
															DWORD64 dwInstUsrKey,
															DWORD dwTimeToReconnect,
															DWORD dwMaxConnections,
															DWORD dwOutBufferSize,
															DWORD dwInBufferSize,
															DWORD dwQueueSize,
															DWORD dwMaxMessageSize,
															HWND  hWndEvents );

EXPORT ACOM_INST_HANDLE WINAPI AComOpenPipeClientInstanceWM(ACOM_WKS_HANDLE hWks,
															char *pcPipeName,
															DWORD64 dwInstUsrKey,
															DWORD dwTimeToReconnect,
															DWORD dwQueueSize,
															DWORD dwMaxMessageSize,
															HWND  hWndEvents );

EXPORT ACOM_INST_HANDLE WINAPI AComOpenMailslotServerInstanceWM(ACOM_WKS_HANDLE hWks,
																char *pcMailslotName,
																DWORD64 dwInstUsrKey,
																DWORD dwTimeToReconnect,
																DWORD dwMaxMessageSize,
																HWND  hWndEvents );

EXPORT ACOM_INST_HANDLE WINAPI AComOpenMailslotClientInstanceWM(ACOM_WKS_HANDLE hWks,
																char *pcMailslotName,
																DWORD64 dwInstUsrKey,
																DWORD dwTimeToReconnect,
																DWORD dwQueueSize,
																DWORD dwMaxMessageSize,
																HWND hWndEvents);

EXPORT ACOM_INST_HANDLE WINAPI AComOpenTcpServerInstanceWM(ACOM_WKS_HANDLE hWks,
														   char *pcAddress,
													       DWORD dwPort,
														   DWORD64 dwInstUsrKey,
														   DWORD dwTimeToReconnect,
														   DWORD dwMaxConnections,
														   DWORD dwOutBufferSize,
														   DWORD dwInBufferSize,
														   DWORD dwQueueSize,
														   DWORD dwMaxMessageSize,
														   HWND  hWndEvents);

EXPORT ACOM_INST_HANDLE WINAPI AComOpenTcpClientInstanceWM(ACOM_WKS_HANDLE hWks,
														   char *pcAddress,
														   DWORD dwPort,
														   DWORD64 dwInstUsrKey,
														   DWORD dwTimeToReconnect,
														   DWORD dwOutBufferSize,
														   DWORD dwInBufferSize,
														   DWORD dwQueueSize,
														   DWORD dwMaxMessageSize,
														   HWND  hWndEvents);

EXPORT ACOM_INST_HANDLE WINAPI AComOpenSerialPortInstanceWM(ACOM_WKS_HANDLE hWks,
															DWORD dwPort,
															char *pcParameters,
															DWORD64 dwInstUsrKey,
															DWORD dwTimeToReconnect,
															DWORD dwOutBufferSize,
															DWORD dwInBufferSize,
															DWORD dwQueueSize,
															DWORD dwMaxMessageSize,
															DWORD dwReadInterval,
															HWND  hWndEvents);

EXPORT ACOM_INST_HANDLE WINAPI AComOpenPipeServerInstanceTQ(ACOM_WKS_HANDLE hWks,
															char *pcPipeName,
															DWORD64 dwInstUsrKey,
															DWORD dwTimeToReconnect,
															DWORD dwMaxConnections,
															DWORD dwOutBufferSize,
															DWORD dwInBufferSize,
															DWORD dwQueueSize,
															DWORD dwMaxMessageSize,
															DWORD dwThreadId );

EXPORT ACOM_INST_HANDLE WINAPI AComOpenPipeClientInstanceTQ(ACOM_WKS_HANDLE hWks,
															char *pcPipeName,
															DWORD64 dwInstUsrKey,
															DWORD dwTimeToReconnect,
															DWORD dwQueueSize,
															DWORD dwMaxMessageSize,
															DWORD dwThreadId );

EXPORT ACOM_INST_HANDLE WINAPI AComOpenMailslotServerInstanceTQ(ACOM_WKS_HANDLE hWks,
																char *pcMailslotName,
																DWORD64 dwInstUsrKey,
																DWORD dwTimeToReconnect,
																DWORD dwMaxMessageSize,
																DWORD dwThreadId);

EXPORT ACOM_INST_HANDLE WINAPI AComOpenMailslotClientInstanceTQ(ACOM_WKS_HANDLE hWks,
																char *pcMailslotName,
																DWORD64 dwInstUsrKey,
																DWORD dwTimeToReconnect,
																DWORD dwQueueSize,
																DWORD dwMaxMessageSize,
																DWORD dwThreadId);
        
EXPORT ACOM_INST_HANDLE WINAPI AComOpenTcpServerInstanceTQ(ACOM_WKS_HANDLE hWks,
														   char *pcAddress,
														   DWORD dwPort,
														   DWORD64 dwInstUsrKey,
														   DWORD dwTimeToReconnect,
														   DWORD dwMaxConnections,
														   DWORD dwOutBufferSize,
														   DWORD dwInBufferSize,
														   DWORD dwQueueSize,
														   DWORD dwMaxMessageSize,
														   DWORD dwThreadId);

EXPORT ACOM_INST_HANDLE WINAPI AComOpenTcpClientInstanceTQ(ACOM_WKS_HANDLE hWks,
														   char *pcAddress,
														   DWORD dwPort,
														   DWORD64 dwInstUsrKey,
														   DWORD dwTimeToReconnect,
														   DWORD dwOutBufferSize,
														   DWORD dwInBufferSize,
														   DWORD dwQueueSize,
														   DWORD dwMaxMessageSize,
														   DWORD dwThreadId);

EXPORT ACOM_INST_HANDLE WINAPI AComOpenSerialPortInstanceTQ(ACOM_WKS_HANDLE hWks,
															DWORD dwPort,
															char *pcParameters,
															DWORD64 dwInstUsrKey,
															DWORD dwTimeToReconnect,
															DWORD dwOutBufferSize,
															DWORD dwInBufferSize,
															DWORD dwQueueSize,
															DWORD dwMaxMessageSize,
															DWORD dwReadInterval,
															DWORD dwThreadId);

EXPORT void WINAPI AComCloseInstance(ACOM_INST_HANDLE hInst);

EXPORT void WINAPI AComCloseWorkingSet(ACOM_WKS_HANDLE hWks, DWORD dwTimeout);

EXPORT DWORD WINAPI AComSendMessage(ACOM_WKS_HANDLE hWks,
									ACOM_CNX_HANDLE hCnxHandle,
									DWORD64 dwMsgUsrKey,
									DWORD dwDataSize,
									void *pvData);

EXPORT DWORD WINAPI AComDisconnectPeer(ACOM_WKS_HANDLE hWks,
									   ACOM_CNX_HANDLE hCnxHandle,
									   BOOL bDisable);

EXPORT DWORD WINAPI AComGetPeer(ACOM_WKS_HANDLE hWks, 
								ACOM_CNX_HANDLE hCnxHandle, 
								char *pcPeer,
								DWORD dwPeerBytes);

EXPORT DWORD WINAPI AComGetConnectionStatistics(ACOM_WKS_HANDLE hWks,
												ACOM_CNX_HANDLE hCnxHandle,
												ULONGLONG *pullNow,
												ULONGLONG *pullCnxDate,
												ULONGLONG *pullInBytes,
												ULONGLONG *pullOutBytes,
												ULONGLONG *pullInMsg,
												ULONGLONG *pullOutMsg);

EXPORT DWORD WINAPI AComGetWorkersStatistics(ACOM_WKS_HANDLE hWks,
											 DWORD *pdwMaxConcurrentThreads,
											 ULONGLONG *pullIdleTime,
											 ULONGLONG *pullUserTime,
											 ULONGLONG *pullKernelTime);

EXPORT void WINAPI AComGetConnectionEvtParameters(ACOM_EVT_HANDLE hEvent, 
												  DWORD64 *pdwInstUsrKey,
												  ACOM_CNX_HANDLE *phCnxHandle);

EXPORT void WINAPI AComGetDisconnectionEvtParameters(ACOM_EVT_HANDLE hEvent, 
													 DWORD64 *pdwInstUsrKey,
													 DWORD64 *pdwCnxUsrKey);

EXPORT void WINAPI AComGetReceivedEvtParameters(ACOM_EVT_HANDLE hEvent, 
												DWORD64 *pdwInstUsrKey,
												DWORD64 *pdwCnxUsrKey,
												DWORD *pdwDataSize,
												void **ppvDataBuffer);

EXPORT void WINAPI AComCopyReceivedEvtParameters(ACOM_EVT_HANDLE hEvent, 
												 DWORD64 *pdwInstUsrKey,
												 DWORD64 *pdwCnxUsrKey,
												 DWORD *pdwDataSize,
												 void *pvDataBuffer);

EXPORT void WINAPI AComGetSentEvtParameters(ACOM_EVT_HANDLE hEvent, 
											DWORD64 *pdwInstUsrKey,
											DWORD64 *pdwCnxUsrKey,
											DWORD64 *pdwMsgUsrKey,
											DWORD *pdwError);

EXPORT void WINAPI AComGetTimeShiftEvtParameters(ACOM_EVT_HANDLE hEvent, 
												 DWORD64 *pdwInstUsrKey,
												 DWORD64 *pdwCnxUsrKey);

EXPORT void WINAPI AComGetShutdownEvtParameters(ACOM_EVT_HANDLE hEvent, 
												DWORD64 *pdwWksUsrKey,
												DWORD *pdwError);

EXPORT void WINAPI AComReleaseEvt(ACOM_EVT_HANDLE hEvent);

EXPORT DWORD WINAPI AComGetDefaultWmEvtId();

EXPORT DWORD WINAPI AComSetDefaultWmEvtId(DWORD dwWmId);

EXPORT DWORD WINAPI AComGetWmEvtId(ACOM_WKS_HANDLE hWks);

EXPORT DWORD WINAPI AComSetWmEvtId(ACOM_WKS_HANDLE hWks, 
								   DWORD dwWmId);

EXPORT DWORD WINAPI AComStartExclusiveAccess(ACOM_WKS_HANDLE hWks,
											 ACOM_CNX_HANDLE hCnxHandle);

EXPORT DWORD WINAPI AComEndExclusiveAccess(ACOM_WKS_HANDLE hWks,
										   ACOM_CNX_HANDLE hCnxHandle );

EXPORT DWORD WINAPI AComOpenTrace(BOOL bInfo,
								  BOOL bError,
								  char *pcFileName );

EXPORT void WINAPI AComCloseTrace();

EXPORT DWORD WINAPI AComGetGMTPeerTimeAsFileTime(ACOM_WKS_HANDLE hWks,
												 ACOM_CNX_HANDLE hCnxHandle,
												 LONGLONG *pllDate );

EXPORT DWORD WINAPI AComGetGMTPeerTime(ACOM_WKS_HANDLE hWks,
									   ACOM_CNX_HANDLE hCnxHandle,
									   DWORD  *pdwYear,
									   DWORD  *pdwMonth,
									   DWORD  *pdwDay,
									   DWORD  *pdwHour,
									   DWORD  *pdwMinute,
									   DWORD  *pdwSecond,
									   DWORD  *pdwMilli );

EXPORT void WINAPI AComGetGMTTime(DWORD *pdwYear,
								 DWORD  *pdwMonth,
								 DWORD  *pdwDay,
								 DWORD  *pdwHour,
								 DWORD  *pdwMinute,
								 DWORD  *pdwSecond,
								 DWORD  *pdwMilli );
                        
EXPORT DWORD WINAPI AComSynchronize(ACOM_WKS_HANDLE hWks, ACOM_CNX_HANDLE hCnxHandle);

EXPORT DWORD WINAPI AComSetNullSessionPipe(char *pcPipeName, BOOL bActivate);

#endif

/*-------------------------------- END OF FILE ------------------------------*/
