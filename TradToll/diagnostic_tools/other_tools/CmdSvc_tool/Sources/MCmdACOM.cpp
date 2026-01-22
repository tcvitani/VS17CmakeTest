/****************** (v) 2017 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 CmdSvc_tool													 */
/* FILE:	 MCmdACOM.cpp														 */
/* LANGUAGE: C++	                                                         */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>
#include <MTracer.h>
#include <MCmdACOM.h>
#include <QCoreApplication>

// Module interface
extern "C"
{

};

#include "MGlobalStructs.h"
/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/

MCmdACOM::MCmdACOM(QObject *parent) : QObject(parent)
{
	m_hWks = NULL;
	m_dwMsgCounter = 0;
	m_hInst = NULL;
	m_hCnxHandle = NULL;
}

MCmdACOM::~MCmdACOM()
{
	if (m_hWks != NULL)
	{
		AComCloseWorkingSet(m_hWks, 5000); //WORKINGSET_TIMEMOUT
		m_hWks = NULL;
	}
}

void MCmdACOM::openWorkingSet()
{
	DWORD dwMaxCnx = 16;
	DWORD dwMaxInst = 16;
	DWORD dwWorkers = 2;
	DWORD dwLifeTime = 20000;
	DWORD dwMaxLife = 1;

	
	if (m_hWks == NULL)
		m_hWks = AComOpenWorkingSet(
			(DWORD64)this,                  // dwWksUsrKey
			dwMaxCnx,                       // dwMaxConnections
			dwMaxInst,                      // dwMaxInstances
			THREAD_PRIORITY_NORMAL,			// dwPriority
			1000,                           // dwConnectLoopDelay
			dwWorkers,						// dwWorkers
			ACOM_PROTOCOL_LIFE,             // dwProtocol
			dwLifeTime,						// dwLifeTime
			dwMaxLife,						// dwMaxLife
			0,                              // dwAllowedTimeShift
			(ACOM_CALLBACK_SHUTDOWN *)MCmdACOM::MainCallbackShutdown);             // pfShut


}

void MCmdACOM::ConnectACOM(struct _ServiceCmdData * pData)
{
	DWORD dwTimeToReconnect = 1000;
	DWORD dwMaxQueuedMessages = 64;
	DWORD dwMaxMsgSize = 16384;
	DWORD dwPipeBufferSize = 16384;

	if (m_hInst == NULL)
	{
		openWorkingSet();


		if (pData->eInstanceType == enuACOM_PIPE_CLIENT)
			m_hInst = AComOpenPipeClientInstance(
			m_hWks,											// m_hWks
			pData->sInstanceFileName.toLocal8Bit().data(),	// pcPipeName
			(DWORD64)this,									// dwInstUsrKey
			dwTimeToReconnect,								// dwTimeToReconnect
			dwMaxQueuedMessages,							// dwQueueSize
			dwMaxMsgSize,									// dwMaxMessageSize
			(ACOM_CALLBACK_CONNECTION *)MCmdACOM::CmdCallbackConnection,
			(ACOM_CALLBACK_DISCONNECTION *)MCmdACOM::CmdCallbackDisconnection,
			(ACOM_CALLBACK_RECEIVED *)MCmdACOM::CmdCallbackReceived,
			(ACOM_CALLBACK_SENT *)MCmdACOM::CmdCallbackSent,
			NULL);
		else
			m_hInst = AComOpenTcpClientInstance(
			m_hWks,										// m_hWks
			pData->sMachineName.toLocal8Bit().data(),	// pcAddress
			pData->dwPort,								// dwPort
			(DWORD64)this,								// dwInstUsrKey
			dwTimeToReconnect,							// dwTimeToReconnect
			dwPipeBufferSize,							// dwOutBufferSize
			dwPipeBufferSize,							// dwInBufferSize
			dwMaxQueuedMessages,						// dwQueueSize
			dwMaxMsgSize,								// dwMaxMessageSize
			(ACOM_CALLBACK_CONNECTION *)MCmdACOM::CmdCallbackConnection,
			(ACOM_CALLBACK_DISCONNECTION *)MCmdACOM::CmdCallbackDisconnection,
			(ACOM_CALLBACK_RECEIVED *)MCmdACOM::CmdCallbackReceived,
			(ACOM_CALLBACK_SENT *)MCmdACOM::CmdCallbackSent,
			NULL);

		if (m_hInst==NULL)
			emit disconnected();
	}

}

void MCmdACOM::DisconnectACOM()
{
	if (m_hWks != NULL)
	{
		if (m_hCnxHandle != NULL)
		{
			AComDisconnectPeer(m_hWks, m_hCnxHandle, FALSE);
			m_hCnxHandle = NULL;
		}

		if (m_hInst != NULL)
		{
			AComCloseInstance(m_hInst);
			m_hInst = NULL;

			m_lstSentMsg.clear();
			emit disconnected();
		}
	}
}


DWORD64 MCmdACOM::getNextKey()
{
	m_dwMsgCounter++;
	return m_dwMsgCounter;
}

void MCmdACOM::SendData(QByteArray baData)
{
	if (m_hCnxHandle != NULL)
	{
		DWORD64 dwMsgUsrKey = getNextKey();

		m_lstSentMsg[dwMsgUsrKey] = baData;

		DWORD dwErr = AComSendMessage(m_hWks,
			m_hCnxHandle,
			dwMsgUsrKey,
			baData.size(),
			baData.data());
	}


}


void CALLBACK MCmdACOM::MainCallbackShutdown(DWORD64 dwWksUsrKey,DWORD dwError)
{
	MCmdACOM *pMCmdACOM = (MCmdACOM *)dwWksUsrKey;
	
	pMCmdACOM->setDisconnected();

	//RaiseException(0xE0000001, EXCEPTION_NONCONTINUABLE, 0, NULL);
}


DWORD64 CALLBACK MCmdACOM::CmdCallbackConnection(DWORD64 dwInstUsrKey, ACOM_CNX_HANDLE hCnxHandle)
{
	MCmdACOM *pMCmdACOM = (MCmdACOM *)dwInstUsrKey;

	pMCmdACOM->setConnectionHandle(hCnxHandle);

	return hCnxHandle;
}


void CALLBACK MCmdACOM::CmdCallbackDisconnection(DWORD64 dwInstUsrKey, DWORD64 dwCnxUsrKey)
{
	MCmdACOM *pMCmdACOM = (MCmdACOM *)dwInstUsrKey;
	pMCmdACOM->setDisconnected();

	return;
}


void CALLBACK MCmdACOM::CmdCallbackReceived(DWORD64 dwInstUsrKey, DWORD64 dwCnxUsrKey, DWORD dwDataSize, void *pvData)
{
	MCmdACOM *pMCmdACOM = (MCmdACOM *)dwInstUsrKey;

	QByteArray baData = QByteArray((const char*)pvData, dwDataSize);
	pMCmdACOM->onMessageReceived(baData);

	return;
}


void CALLBACK MCmdACOM::CmdCallbackSent(DWORD64 dwInstUsrKey, DWORD64 dwCnxUsrKey, DWORD64 dwMsgUsrKey, DWORD dwError)
{
	MCmdACOM *pMCmdACOM = (MCmdACOM *)dwInstUsrKey;
	QByteArray baData; 
	baData = pMCmdACOM->m_lstSentMsg.take(dwMsgUsrKey);
	pMCmdACOM->onMessageSent(baData);

	return;
}


/*-------------------------------- END OF FILE ------------------------------*/