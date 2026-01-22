/****************** (v) 2017 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 CmdSvc_tool												 */
/* FILE:     MCmdACOM.h														 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef CMDSVC_TOOL_SIMU_H
#define CMDSVC_TOOL_SIMU_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <QObject>
#include <QList>
#include <QMap>
#include <QString>
#include <QFont>
#include <QRect>
#include <QThread>
#include <QByteArray>

#include "MCmdSvcToolConfig.h"
#include "MDefines.h"
#include "MGlobalStructs.h"

extern "C"
{
	#include "acom.h"
};

#include <protect.h>

/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/

/*-------------------------------- TYPEDEFS:  -------------------------------*/


class MCmdACOM : public QObject
{
	Q_OBJECT

	public:
		explicit MCmdACOM(QObject *parent);
		virtual ~MCmdACOM();

		void ConnectACOM(struct _ServiceCmdData * pData);
		void DisconnectACOM();
		void SendData(QByteArray baData);

		static void CALLBACK MainCallbackShutdown(DWORD64 dwWksUsrKey, DWORD dwError);
		static DWORD64 CALLBACK CmdCallbackConnection(DWORD64 dwInstUsrKey, ACOM_CNX_HANDLE hCnxHandle);
		static void CALLBACK CmdCallbackDisconnection(DWORD64 dwInstUsrKey, DWORD64 dwCnxUsrKey);
		static void CALLBACK CmdCallbackReceived(DWORD64 dwInstUsrKey, DWORD64 dwCnxUsrKey, DWORD dwDataSize, void *pvData);
		static void CALLBACK CmdCallbackSent(DWORD64 dwInstUsrKey, DWORD64 dwCnxUsrKey, DWORD64 dwMsgUsrKey, DWORD dwError);

		void setConnectionHandle(ACOM_CNX_HANDLE hCnxHandle)
		{ 
			m_hCnxHandle = hCnxHandle; 
			emit connected();
		};
		
		void setDisconnected()
		{ 
			m_hCnxHandle = NULL;
			emit disconnected();
		};

		void onMessageReceived(QByteArray baData)
		{
			emit messageReceived(baData);
		};

		void onMessageSent(QByteArray baData)
		{
			emit messageSent(baData);
		};

	signals:
		void disconnected();
		void connected();
		void messageReceived(QByteArray);
		void messageSent(QByteArray);

	private:
		void openWorkingSet();
		
		ACOM_WKS_HANDLE m_hWks;               // Handle du working set gerant la com
		ACOM_INST_HANDLE m_hInst;
		ACOM_CNX_HANDLE m_hCnxHandle;

		QMap <DWORD64, QByteArray> m_lstSentMsg;
		DWORD64 getNextKey();
		DWORD64 m_dwMsgCounter;

		
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

#include <undef.h>
#endif

/*-------------------------------- END OF FILE ------------------------------*/