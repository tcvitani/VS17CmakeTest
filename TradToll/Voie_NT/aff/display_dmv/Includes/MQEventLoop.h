/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 MQEventLoop													*/
/* FILE:		MQEventLoop.h															 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MQEVENTLOOP_H
#define MQEVENTLOOP_H


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*-------------------------------- INCLUDES:  -------------------------------*/
#ifdef __cplusplus


extern "C"
{
#endif

#include <noyau.h>


#ifdef __cplusplus
}
#endif


/*-------------------------------- RESERVED:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#define MAX_BUFFER_LENGTH					4096
#define PROJECT_MAX_NB_OCR_SIMILARTY		40

#ifdef __cplusplus
#define EXPORT_C extern "C"
#else
#define EXPORT_C
#endif

/*-------------------------------- TYPEDEFS:  -------------------------------*/

//qt worker init function type
typedef int InitWorker(LPVOID pTWorkerStructure);
//qt worker de-init function
typedef void DeinitWorker(LPVOID pTWorkerStructure);


#ifdef __cplusplus
class QObject;
#else
typedef struct QObject QObject;
#endif


typedef struct _TWorkerStructure{

	InitWorker *pfInitWorker;
	DeinitWorker *pfDeinitWorker;
	QObject * pWorkerInstance;
	int m_iLoopInstance;
	short int siInstId;
	LPVOID pGlobalStructure;
	
	DWORD						dwLocalIPPort;
	DWORD						dwLocalEchoPort;
	DWORD						dwDeviceIPPort;
	DWORD						dwDeviceEchoPort;
	UCHAR						szLocalIPAddress[MAX_PATH];
	UCHAR						szDeviceIPAddress[MAX_PATH];

	DWORD						dwWaitForResponseTimeout;
	DWORD						dwWaitForStatusTimeout;

	DWORD						dwFullTrace;

	DWORD						dwDeviceType;
	DWORD						dwBrightness; // 0-100

	// AFF
	DWORD						dwFont;
	DWORD						dwFontColor;
	DWORD						dwAlignment;
	DWORD						dwMaxSpacing; //Max 3
	DWORD						dwTextYCoordinate;
	DWORD						dwTextXRCoordinate;
	DWORD						dwTextXLCoordinate;

	// TLM
	DWORD						dwIsTlmServiceIntegrated;
	DWORD						dwImageXCoordinate;
	DWORD						dwImageYCoordinate;

	unsigned char				szFormatFile[MAX_PATH];
	int							iPosition;	

	DWORD						MaxLinkError;
} TWorkerStructure;

/*-------------------------------- FUNCTIONS: -------------------------------*/

EXPORT_C void MQEventLoop_Quit(int iLoopHandle);
EXPORT_C DWORD WINAPI MQEventLoop_LoopThread(PVOID pvParam);

/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/


