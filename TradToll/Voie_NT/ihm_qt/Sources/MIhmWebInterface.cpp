
#include <MIhmWebInterface.h>
#include <MIhmHttpServer.h>
#include <MIhmHttpRequest.h>
#include <MIhmHttpResponse.h>
#include <MClientSocket.h>
#include <MIhmVirtualObject.h>
#include "MIhmWebSession.h"
#include "MIhmWebVisibleObject.h"
#include <QStringList>
#include <QDateTime>
#include <QUrl>

#include "MTracer.h"
#include "MIhmConfigGeneral.h"
#include "MIhmMsg.h"
#include "MIhmMsgVideo.h"
#include "MIhmTCLInterfaces.h"
#include "MHelpFuncs.h"
#include "MIhmVirtObjectsModel.h"
#include "MIhmWebVisibleObject.h"
#include "MIhmWebCommonObjLogic.h"
#include "MIhmMiniWebSession.h"


MIhmWebInterface::MIhmWebInterface(QObject * parent)
	:QThread(parent), m_eStatus(enuNotInitialized), m_bThreadStarted(false)
{
	MTracer::InitWeb();
	TRACE_WEB_D("MIhmWebInterface::MIhmWebInterface:Trace init..");

	m_pHttpServer = NULL;
	m_pSessionInContol = NULL;
	m_bRefreshAllObjects = TRUE;
	m_pTCLInterface = NULL;
	m_iPrivateUpdateEmmited = 0;

	m_pWEBVirtObjectsModel = new MIhmVirtObjectsModel();

	m_pCommonVisibleObjLogic = new MIhmWebCommonObjLogic(this);
}

MIhmWebInterface::~MIhmWebInterface()
{
	TRACE_WEB_D("MIhmWebInterface::~MIhmWebInterface:Closing web ...");

	m_tmDeadSessionsCheck.stop();

	foreach( MClientSocket * client, m_lstClientSockets)
	{
		client->disconnectFromHost();
		if(client->state() != QAbstractSocket::UnconnectedState)
			client->waitForDisconnected();
	}

	qDeleteAll(m_lstClientSockets);
	qDeleteAll(m_lstSessions);

	delete m_pWEBVirtObjectsModel;

	delete m_pCommonVisibleObjLogic;
	MTracer::DeinitWeb();
}


bool MIhmWebInterface::initialize(MIhmTCLInterfaces *pTCLIntf, QString sModuleConfigKey)
{
    TRACE_WEB_D(QString("MIhmWebInterface::initialize: %1").arg(sModuleConfigKey));
		
	m_sModuleConfigKey = sModuleConfigKey;

	if(!m_oWebConfig.loadConfigFromRegistry(sModuleConfigKey))
	{
        TRACE_WEB_W( "MIhmWebInterface::initialize: ERREUR loadConfigFromRegistry retourne false");
        return false;
	}
	
	//start periodical check to remove the dead sessions
	connect(&m_tmDeadSessionsCheck, SIGNAL(timeout()), this, SLOT(onDeadSessionsCheckTimer()));
	m_tmDeadSessionsCheck.start(1000); //check every 1 seconds
	
	//Check for date change to update date visible object in all existing sessions 
	onDateChanged();
	connect(&m_dateCheck, SIGNAL(dateChange()), this, SLOT(onDateChanged()));
	m_dateCheck.start();

	m_eStatus = enuInitializing;

	m_pTCLInterface = pTCLIntf;

	return true;
}

void MIhmWebInterface::onDateChanged()
{
	//Update all the CLK_REG_VAL_WEB_DATE objects in all sessions
	m_pCommonVisibleObjLogic->updateTheDateObject();

	refreshAllVirtualObjects();
}


void MIhmWebInterface::run()	
{
	m_pHttpServer = new MIhmHttpServer(m_oWebConfig.getWebPort());
	connect(m_pHttpServer, SIGNAL(newConnection(int)), this, SLOT(onIncommingConnection(int)));
	
	connect(this, SIGNAL(privateUpdateObjectsNeeded()), this, SLOT(onPrivateUpdateObjects()), Qt::QueuedConnection);

	if(m_pHttpServer->isListening())
	{
		TRACE_WEB_D(QString("MIhmWebInterface::run: MIhmHttpServer is listening on port:%1")
							.arg(m_pHttpServer->serverPort()));

		m_bThreadStarted = true;
		//emit this signal to disable the start timer exitProgram call 
		emit initOK();
		
		//schedule refresh all because it could happened that the the virtual objects were already processed
		// and marked not updated
		emit privateUpdateObjectsNeeded();

		//This line is running the web interface thread
		this->exec();
	}
	else
	{
		QString sErr = m_pHttpServer->errorString();
		TRACE_WEB_W(QString("MIhmWebInterface::run: MIhmHttpServer start up problem for port: %1 ! %2")
							.arg(m_oWebConfig.getWebPort())
							.arg(sErr.toLatin1().data()));
	}

	delete m_pHttpServer;
}


int MIhmWebInterface::rowOfClient(MClientSocket *client) const
{
    // Return the row that displays this client's status, or -1 if the
    // client is not known.
    int row = 0;
    foreach (MClientSocket *pCurrentClient, m_lstClientSockets) {
        if (pCurrentClient == client)
            return row;
        ++row;
    }
    return -1;
}


void MIhmWebInterface::onSocketError(QAbstractSocket::SocketError eError)
{
    // Delete the client.
    MClientSocket *client = qobject_cast<MClientSocket *>(sender());
    int row = rowOfClient(client);
    m_lstClientSockets.removeAt(row);

	QString sErrorString = client->errorString();
    TRACE_WEB_D(QString("MIhmWebInterface::onSocketError: row:%1 error:%2")
							.arg(row)
							.arg(sErrorString.toLatin1().data()));

    //remove session object with this client (if any)
	//MIhmWebSession::removeSessionObj(&m_lstSessions, client);

	client->deleteLater();
}

void MIhmWebInterface::onIncommingConnection(int iSocketDescriptor)
{
    
	MClientSocket * pClient;
 
	TRACE_WEB_D(QString("MIhmWebInterface::onIncommingConnection: iSocketDescriptor: %1")
					.arg(iSocketDescriptor));

	pClient = new MClientSocket(this);

	if(pClient->setSocketDescriptor(iSocketDescriptor))
	{
		connect(pClient, SIGNAL(receivedData(const QByteArray &)), this, SLOT(onReceivedData(const QByteArray &)));
		connect(pClient, SIGNAL(socketError(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
		connect(pClient, SIGNAL(disconnected()), this, SLOT(onClientDisconnected()));

		m_lstClientSockets.append(pClient);	
	}
	else
	{
		QString sErrorString = pClient->errorString();
		TRACE_WEB_W(QString("MIhmWebInterface::onIncommingConnection: ERROR:%1")
												.arg(sErrorString.toLatin1().data()));

		delete pClient;
	}
}

bool MIhmWebInterface::isClientValid(MClientSocket *pClient) const
{
	if(rowOfClient(pClient)==-1)
		return false;
	else
		return true;

}


void MIhmWebInterface::onClientDisconnected()
{
   // Delete the client.
    MClientSocket *client = qobject_cast<MClientSocket *>(sender());
	int row = rowOfClient(client);

    TRACE_WEB_D(QString("MIhmWebInterface::onClientDisconnected: row:%1")
						.arg(row));

    m_lstClientSockets.removeAt(row);

    client->deleteLater();

}




void MIhmWebInterface::onReceivedData(const QByteArray &baRequestData)
{
    MClientSocket *pClient = qobject_cast<MClientSocket *>(sender());

	//prepare the response and send it ...
	if(pClient!=NULL)
	{
        MIhmHttpRequest oRequest;

		if(oRequest.parse(baRequestData))
		{
			switch(oRequest.type())
			{
				case MIhmHttpRequest::enuReqLogin:
						processLoginReq(&oRequest,pClient);
					break;
				case MIhmHttpRequest::enuReqRefreshAll:
						processRefreshReq(&oRequest,pClient, false);
					break;
				case MIhmHttpRequest::enuReqRefreshPart:
						processRefreshReq(&oRequest,pClient, true);
					break;
				case MIhmHttpRequest::enuReqRefreshAck:
						processRefreshAckReq(&oRequest,pClient);
					break;
				case MIhmHttpRequest::enuReqKeyDown:
						processKeyDownReq(&oRequest,pClient);
					break;
				case MIhmHttpRequest::enuReqDialogResponse:
						processDialogResponseReq(&oRequest,pClient);
					break;
				case MIhmHttpRequest::enuReqClickCommand:
						processClickCommandReq(&oRequest,pClient);
					break;
				case MIhmHttpRequest::enuReqCommentsUpdate:
						processCommentsUpdate(&oRequest,pClient);
					break;
				case MIhmHttpRequest::enuReqGetDynImagePath:
						processGetDynImagePathReq(&oRequest,pClient);
					break;
				case MIhmHttpRequest::enuReqMiniWebRefreshAll:
						processReqMiniWebRefresh(&oRequest,pClient, false);
					break;
				case MIhmHttpRequest::enuReqMiniWebRefreshPart:
						processReqMiniWebRefresh(&oRequest,pClient, true);
					break;
					
				default:
					TRACE_WEB_D(QString("MIhmWebInterface::onReceivedData: Unknown request type %1")
														.arg(oRequest.type()));

					break;
			}
		}
		else
		{
			TRACE_WEB_W("MIhmWebInterface::onReceivedData: Error parsing http request!");
			MIhmHttpRspError *pRsp = new MIhmHttpRspError();
								
			if(isClientValid(pClient))
				sendResponse(pClient,pRsp);

			if(pRsp!=NULL)
				delete pRsp;

		}
		
	}	

}

void MIhmWebInterface::processLoginReq(MIhmHttpRequest *pHttpRequest,
									   MClientSocket *pClient)
{
	TRACE_WEB_D( "MIhmWebInterface::processLoginReq: ...");

	//create a new session object
	MIhmWebSession * pSession = new MIhmWebSession(this, m_pCommonVisibleObjLogic);
	
	m_lstSessions.append(pSession);

	//keep login data and keep the pointer to the client in the session object
	pSession->setLoginData(pHttpRequest->getLogin(), pHttpRequest->isTakeControl(),pClient, pHttpRequest->isProxyUsed(), pHttpRequest->getClientSessionID());	
	pSession->setAppRootUrl(pHttpRequest->getAppRootUrl());

	QString sSessionID = pSession->getSessionID();
	//since authorization from the TCL interface is required to proceed
	emit authorisationReq(pHttpRequest->getLogin(), 
							pHttpRequest->getPwd(), 
							pHttpRequest->isTakeControl(), 
							sSessionID);

}



QString MIhmWebInterface::getMainPage()
{
	QString sMainPage;

	if (m_eStatus == enuInitializing)
	{
		sMainPage =	m_oWebConfig.getInitWebPage();
	}
	else
	{
		//Get main page from virtual objects (if any)
		MIhmVirtDynConfig * pVirtLaneType = (MIhmVirtDynConfig *)m_pWEBVirtObjectsModel->getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtConfLaneType);


		if(pVirtLaneType!=NULL)
		{
			QString sLaneType = pVirtLaneType->m_sDynConfigSetting;

			if(sLaneType!="") 
			{
				MIhmLaneTypeSettings *pLaneTypeSettings = 
						m_oWebConfig.getLaneTypeSetting(MIhmLaneTypeSettings::enuWEB, sLaneType);
				
				if(pLaneTypeSettings!=NULL)
				{	
					sMainPage = pLaneTypeSettings->getParam(CFG_LANE_TYPE_PARAM_WEB_MAIN_PAGE);
				}
				else
				{
					TRACE_WEB_W(QString("MIhmWebInterface::getMainPage:ERROR: Unable to find the main page for LaneType %1")
									.arg( sLaneType.toLatin1().data()));
					sMainPage = m_oWebConfig.getInitWebPage();
				}
			}
		}
		else
		{
			TRACE_WEB_W("MIhmWebInterface::getMainPage:ERROR: Invalid Web interface status!");
			sMainPage = m_oWebConfig.getInitWebPage();
		}
	}

	return sMainPage;
}


QString MIhmWebInterface::getMiniWebPage()
{
	QString sMainPage;

	//Get main page from virtual objects (if any)
	MIhmVirtDynConfig * pVirtLaneType = (MIhmVirtDynConfig *)m_pWEBVirtObjectsModel->getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtConfLaneType);


	if(pVirtLaneType!=NULL)
	{
		QString sLaneType = pVirtLaneType->m_sDynConfigSetting;

		if(sLaneType!="") 
		{
			MIhmLaneTypeSettings *pLaneTypeSettings = 
					m_oWebConfig.getLaneTypeSetting(MIhmLaneTypeSettings::enuWEB, sLaneType);
			
			if(pLaneTypeSettings!=NULL)
			{	
				sMainPage = pLaneTypeSettings->getParam(CFG_LANE_TYPE_PARAM_WEB_MINI_WEB_PAGE);
			}
			else
			{
				TRACE_WEB_W(QString("MIhmWebInterface::getMiniWebPage:ERROR: Unable to find the MINI_WEB_PAGE for LaneType %1")
								.arg( sLaneType.toLatin1().data()));
				sMainPage = m_oWebConfig.getInitWebPage();
			}
		}
	}
	else
	{
		TRACE_WEB_W("MIhmWebInterface::getMiniWebPage:ERROR: Invalid Web interface status!");
		sMainPage = m_oWebConfig.getInitWebPage();
	}

	return sMainPage;
}




void MIhmWebInterface::onForcedUnIdentification(QString sWebSessionID)
{
	TRACE_WEB_D(QString("MIhmWebInterface::onForcedUnIdentification: web session:%1")
											.arg(sWebSessionID));

	MIhmWebSession * pSession = MIhmWebSession::findSessionObj(&m_lstSessions, sWebSessionID);
	
	if(pSession!=NULL)
	{
		deleteSession(pSession, enuRETURN_CTRL_FORCED, false);
	}

}


void MIhmWebInterface::onAuthorisationResult(QString sWebSessionID, bool bOK, QString sError)
{
	TRACE_WEB_D(QString("MIhmWebInterface::onAuthorisationResult: web session:%1, result:%2, ERR:%3")
											.arg(sWebSessionID)
											.arg(bOK?"TRUE":"FALSE")
											.arg(sError));

	MIhmWebSession * pSession = MIhmWebSession::findSessionObj(&m_lstSessions, sWebSessionID);
	
	if(pSession!=NULL)
	{

		MClientSocket * pClient = pSession->getLoginClient();
		MIhmHttpResponse *pRsp = NULL;

		if(pSession->getStatus()==MIhmWebSession::enuWebSessWaitingAuth && bOK)
		{
			pSession->setInControl(false);
			
				TRACE_WEB_D(QString("MIhmWebInterface::onAuthorisationResult: Login OK;PROXY USED:%1").
									arg(pSession->isProxyUsed()?"TRUE":"FALSE"));

				int iMaxWebClients = MIhmConfigWeb::getCfg()->getCfgDynamic()->getMaxWebClients();

				if(getNumInitializedSessions()<iMaxWebClients)
				{
					QString sLoginRedirectPage = pSession->getAppRootUrl() + getMainPage();;
					
					
					pRsp = new MIhmHttpRspLoginOK(pSession->getSessionID(), sLoginRedirectPage, pSession->isProxyUsed());
					
					connect(pSession, SIGNAL(inputDlgsMsg(MIhmMsg *)), 
								this, SLOT(onInputDlgsMsg(MIhmMsg *))), 
					
					initSessionVisibleObjects(pSession);

				}
				else
				{
					QString sLoginRedirectPage = pSession->getAppRootUrl() + m_oWebConfig.getLoginFailPage();
					QString sTranslateError = MIhmConfigWeb::getCfg()->getLanguages()->getLabelErrMsgTooManyUsersConnected(MIhmLanguages::enuTranslTargetNoTransform);
					
					pRsp = new MIhmHttpRspLoginNOK(sTranslateError, sLoginRedirectPage, pSession->isProxyUsed());

					TRACE_WEB_D(QString("MIhmWebInterface::onAuthorisationResult: Login Rejected cause too many users connected! WebSession:%1;PROXY USED:%2")
																				.arg(sWebSessionID)
																				.arg(pSession->isProxyUsed()?"TRUE":"FALSE"));
					deleteSession(pSession, enuRETURN_CTRL_FORCED);
				}
		}
		else
		{

			QString sLoginRedirectPage = pSession->getAppRootUrl() + m_oWebConfig.getLoginFailPage();
			
			if(sError.isEmpty())
				sError = IHM_LABEL_MSG_NO_AUTHORIZATION_SERVICE;

			QString sTranslateError;
			MIhmConfigWeb::getCfg()->getLanguages()->getLabelTranslation(sError, MIhmLanguages::enuTranslTargetNoTransform, sTranslateError);
			
			pRsp = new MIhmHttpRspLoginNOK(sTranslateError, sLoginRedirectPage, pSession->isProxyUsed());
			
			TRACE_WEB_D(QString("MIhmWebInterface::onAuthorisationResult: Login unsecsessfull:WebSession:%1;PROXY USED:%2;Session status:%3;")
																		.arg(sWebSessionID)
																		.arg(pSession->isProxyUsed()?"TRUE":"FALSE")
																		.arg(pSession->getStatus()));
			deleteSession(pSession, enuRETURN_CTRL_FORCED, false);
			
		}


			if(isClientValid(pClient))
			{
				sendResponse(pClient,pRsp);
			}
			else
			{
				TRACE_WEB_W("MIhmWebInterface::onAuthorisationResult: invalid TCP socket client!");
			}

			if(pRsp!=NULL)
				delete pRsp;

	}
	else
	{
		TRACE_WEB_W(QString("MIhmWebInterface::onAuthorisationResult: Unable to find session object %1!")
					.arg(sWebSessionID));
	}
}


void MIhmWebInterface::onCancelTakeOver(QString sWebSessionId)	
{
	TRACE_WEB_D(QString( "MIhmWebInterface::onCancelTakeOver: WebSession:%1!")
														.arg(sWebSessionId));

	MIhmWebSession * pSession = MIhmWebSession::findSessionObj(&m_lstSessions, sWebSessionId);
	
	if(pSession!=NULL)
	{
		pSession->closeTakeCtrlClientDlg();

		//Recheck the dialogs because it is possible that some new are pending in buffer
		if(!pSession->isAnyInputDlgInProgress())
			refreshSessionObjects(pSession);
	}
}

void MIhmWebInterface::onShowClientInfoMsg(QString sWebSessionId, QString sMsg, QString sMsgTitle)
{
	TRACE_WEB_D(QString( "MIhmWebInterface::onShowClientInfoMsg: WebSession:%1!")
														.arg(sWebSessionId));

	QString sTranslWebMsg, sTranslWebMsgTitle;
	MIhmConfigWeb::getCfg()->getLanguages()->getLabelTranslation(sMsg, MIhmLanguages::enuTranslTargetWeb, sTranslWebMsg);
	MIhmConfigWeb::getCfg()->getLanguages()->getLabelTranslation(sMsgTitle, MIhmLanguages::enuTranslTargetWeb, sTranslWebMsgTitle);

	MIhmWebSession * pSession = MIhmWebSession::findSessionObj(&m_lstSessions, sWebSessionId);
	
	if(pSession!=NULL)
		pSession->initClientInfoDlg(sTranslWebMsg, sTranslWebMsgTitle);
}



void MIhmWebInterface::onHideClientInfoMsg(QString sWebSessionId)
{
	TRACE_WEB_D(QString( "MIhmWebInterface::onHideClientInfoMsg: WebSession:%1!")
														.arg(sWebSessionId));

	MIhmWebSession * pSession = MIhmWebSession::findSessionObj(&m_lstSessions, sWebSessionId);
	
	if(pSession!=NULL)
		pSession->closeHighPrioClientDlg();
}



void MIhmWebInterface::onShowConnectingMsg(QString sWebSessionId, QString sMsg, QString sMsgTitle)
{
	TRACE_WEB_D(QString( "MIhmWebInterface::onShowConnectingMsg: WebSession:%1!")
														.arg(sWebSessionId));

	QString sTranslWebMsg, sTranslWebMsgTitle;
	MIhmConfigWeb::getCfg()->getLanguages()->getLabelTranslation(sMsg, MIhmLanguages::enuTranslTargetWeb, sTranslWebMsg);
	MIhmConfigWeb::getCfg()->getLanguages()->getLabelTranslation(sMsgTitle, MIhmLanguages::enuTranslTargetWeb, sTranslWebMsgTitle);

	MIhmWebSession * pSession = MIhmWebSession::findSessionObj(&m_lstSessions, sWebSessionId);
	
	if(pSession!=NULL)
		pSession->initConnectingInfoDlg(sTranslWebMsg, sTranslWebMsgTitle);
}

void MIhmWebInterface::onHideConnectingMsg(QString sWebSessionId)
{
	TRACE_WEB_D(QString( "MIhmWebInterface::onHideConnectingMsg: WebSession:%1!")
														.arg(sWebSessionId));

	MIhmWebSession * pSession = MIhmWebSession::findSessionObj(&m_lstSessions, sWebSessionId);
	
	if(pSession!=NULL)
		pSession->closeHighPrioClientDlg();
}



void MIhmWebInterface::onInitTakeOver(bool bQuest, QString sWebSessionId, QString sConnectingUserName)
{
	//first do the verification
	if(m_pSessionInContol!=NULL)
	{
		if(bQuest&& m_pSessionInContol->getSessionID()!=sWebSessionId)	
		{
			TRACE_WEB_W(QString( "MIhmWebInterface::initTakeOver: Invalid control state! sWebSessionId:%1 is not in control!")
					.arg(sWebSessionId));
			return;
		}
	}
	else if(bQuest)
	{
		TRACE_WEB_W(QString( "MIhmWebInterface::initTakeOver: m_pSessionInContol = NULL: WEB - TCL sinkronisation error!")
				.arg(sWebSessionId));
		return;
	}

	if(bQuest)
	{
		m_pSessionInContol->initTakeOver(true, sConnectingUserName);
	}
	else
	{
		MIhmWebSession * pSession = MIhmWebSession::findSessionObj(&m_lstSessions, sWebSessionId);
		
		if(pSession!=NULL)
		{
			pSession->initTakeOver(false);
		}
		else
		{
			emit takeOverCancel(sWebSessionId);
		}
	}


}





void MIhmWebInterface::onWebUserInControl(QString sWebSessionID)
{
	MIhmWebSession * pSession = MIhmWebSession::findSessionObj(&m_lstSessions, sWebSessionID);
	
	if(pSession!=NULL)
	{
		TRACE_WEB_D(QString("MIhmWebInterface::onWebUserInControl: Session object %1!")
					.arg(sWebSessionID));
		setSessionInControl(pSession);
	}
	else
	{
		if(sWebSessionID=="")
		{
			TRACE_WEB_D(QString("MIhmWebInterface::onWebUserInControl: Setting web %1 out of control!")
						.arg(sWebSessionID));
		}
		else
		{
			TRACE_WEB_D(QString("MIhmWebInterface::onWebUserInControl: Session object %1 not found!")
						.arg(sWebSessionID));
		}

		setSessionInControl(NULL);
	}
}


void MIhmWebInterface::onInputDlgsMsg(MIhmMsg *pMsg)
{
	MIhmWebSession *pSession = qobject_cast<MIhmWebSession *>(sender());

	if(pSession == m_pSessionInContol)
	{
		emit webInputDlgsRsp(pSession->getSessionID(), pMsg);
	}

}



void MIhmWebInterface::processDialogResponseReq(MIhmHttpRequest *pHttpRequest,
												MClientSocket *pClient)
{
	QString sWebSessionID = pHttpRequest->getSessionID();
	TRACE_WEB_D(QString("MIhmWebInterface::processDialogResponseReq: Session:%1!")
								.arg(sWebSessionID.toLatin1().data()));

	MIhmWebSession * pSession = MIhmWebSession::findSessionObj(&m_lstSessions, sWebSessionID);
	
	Q_ASSERT(pClient!=NULL);

	if(pSession!=NULL && pSession->getClientSessionID() == pHttpRequest->getClientSessionID())
	{
		if(pHttpRequest->getWebDialogId() == DLG_REG_VAL_WEB_INPUT_DIALOG)
		{
			if(pSession == m_pSessionInContol)
			{
				pSession->resetSessionIdleTimer();
				MInputDialogReq oReq;
				
				if(pSession->getCurrentInputDialog(&oReq))
				{

					//if the dialog result is valid emit 
					MIhmMsgInputDialogRes *pMsg = new MIhmMsgInputDialogRes();
					pMsg->oInputReq = oReq;
					pMsg->bCanceled = pHttpRequest->isDialogCanceled();
					pMsg->sInputResult = pHttpRequest->getDialogResponse();
					emit webInputDlgsRsp(sWebSessionID, pMsg);
				}
				else
				{
					TRACE_WEB_W(QString("MIhmWebInterface::processDialogResponseReq:Error:Input dialog should exist in session %1!")
								.arg(sWebSessionID));

				}
				pSession->setDialogNotInProgress();
			}
			else
			{
				TRACE_WEB_W(QString("MIhmWebInterface::processDialogResponseReq:Error:Dialog should be read only (session not in control)!"));
				pSession->setInControl(false);

			}
			
		}
		if(pHttpRequest->getWebDialogId() == DLG_REG_VAL_WEB_INPUT_DIALOG_EX)
		{
			if(pSession == m_pSessionInContol)
			{
				pSession->resetSessionIdleTimer();
				MInputDialogExReq oReq;
				
				if(pSession->getCurrentInputDialogEx(&oReq))
				{

					//if the dialog result is valid emit the 
					MIhmMsgInputDlgExRes *pMsg = new MIhmMsgInputDlgExRes();
					pMsg->oInputReq = oReq;
					pMsg->bCanceled = pHttpRequest->isDialogCanceled();
					pMsg->sInputResult = pHttpRequest->getDialogResponse();
					emit webInputDlgsRsp(sWebSessionID, pMsg);
				}
				else
				{
					TRACE_WEB_W(QString("MIhmWebInterface::processDialogResponseReq:Error:Input dialog should exist in session %1!")
								.arg(sWebSessionID));

				}

				pSession->setDialogExNotInProgress();
			}
			else
			{
				TRACE_WEB_W(QString("MIhmWebInterface::processDialogResponseReq:Error:Dialog should be read only (session not in control)!"));
				pSession->setInControl(false);

			}
			
		}
		else if(pHttpRequest->getWebDialogId() == DLG_VAL_WEB_HPRIO_INPUT_DIALOG)
		{
			pSession->closeHighPrioClientDlg();
			
			QString sResponse = pHttpRequest->getDialogResponse();
			bool bIsCanceled = pHttpRequest->isDialogCanceled();

			if(pSession->getHPrioDlgType()==MIhmWebVisibleHPrioDialog::enuClientLoginDlg)
			{
				if(!bIsCanceled)
				{
					QString sUserId = MIhmHttpRequest::getDialogRspParamValue(HTML_HPRIO_LOGIN_DLG_USER_NAME,sResponse);
					QString sPwd = MIhmHttpRequest::getDialogRspParamValue(HTML_HPRIO_LOGIN_DLG_PASSWORD,sResponse);

					emit authorisationReq(sUserId,sPwd,true,sWebSessionID);
				}

			}
			else if(pSession->getHPrioDlgType()==MIhmWebVisibleHPrioDialog::enuTakeOverQuestion)
			{
				if(!bIsCanceled)
					emit takeOverAccept(sWebSessionID);
				else
					emit takeOverReject(sWebSessionID);
				
			}
			else if(pSession->getHPrioDlgType()==MIhmWebVisibleHPrioDialog::enuTakeOverMsg ||
					pSession->getHPrioDlgType()==MIhmWebVisibleHPrioDialog::enuClientConnectingMsg)
			{
				emit takeOverCancel(sWebSessionID);
			}


		}
		else
		{
			TRACE_WEB_W(QString("MIhmWebInterface::processDialogResponseReq: Invalid request dialog_id parameter [%1]!")
						.arg(pHttpRequest->getWebDialogId()));

			pSession->setDialogNotInProgress();
			pSession->setDialogExNotInProgress();
			refreshSessionObjects(pSession);
		}

		MIhmHttpRspRefreshAckOK *pRsp = new MIhmHttpRspRefreshAckOK(sWebSessionID);
		sendResponse(pClient,pRsp);
		delete pRsp;

	}
	else
	{
		TRACE_WEB_W(QString("MIhmWebInterface::processDialogResponseReq: Unable to find session object %1!")
					.arg(sWebSessionID.toLatin1().data()));

		MIhmHttpRspError *pRsp = new MIhmHttpRspError();
							
		if(isClientValid(pClient))
			sendResponse(pClient,pRsp);

		if(pRsp!=NULL)
			delete pRsp;
	}
}


void MIhmWebInterface::processRefreshReq(MIhmHttpRequest *pHttpRequest,
												MClientSocket *pClient,
												bool bOnlyUpdated)
{
	QString sSessionID = pHttpRequest->getSessionID();
	TRACE_WEB_D(QString("MIhmWebInterface::processRefreshReq: Session:%1!")
								.arg(sSessionID.toLatin1().data()));
	
	MIhmWebSession * pSession = MIhmWebSession::findSessionObj(&m_lstSessions, sSessionID);
	
	Q_ASSERT(pClient!=NULL);

	if(pSession!=NULL && pSession->getClientSessionID() == pHttpRequest->getClientSessionID())
	{
		if(pHttpRequest->getSourcePage() != getMainPage())
		{
			MIhmHttpRspRedirect * pRsp = new MIhmHttpRspRedirect(getMainPage());
			sendResponse(pClient,pRsp);	
			delete pRsp;
		}
		else if(pSession->isIdleTimeout())
		{
			QString sLoginRedirectPage = pHttpRequest->getAppRootUrl() + m_oWebConfig.getLoginFailPage();
			MIhmHttpRspRedirect *pRsp = new MIhmHttpRspRedirect(sLoginRedirectPage);
			
			MIhmLanguages *pLang = MIhmConfigWeb::getCfg()->getLanguages();
			
			QString sMsg = pLang->getLabelErrSessionIdleTimeout(MIhmLanguages::enuTranslTargetNoTransform);
			sMsg = QUrl::toPercentEncoding(sMsg);

			pRsp->setMessage(sMsg);
			sendResponse(pClient,pRsp);
			delete pRsp;
			
			deleteSession(pSession, enuRETURN_CTRL_IDLE_TIMEOUT);
		}
		else if(pSession->isLoggedOff())
		{
			QString sLoginRedirectPage = pHttpRequest->getAppRootUrl() + m_oWebConfig.getLogoutPage();
			MIhmHttpRspRedirect *pRsp = new MIhmHttpRspRedirect(sLoginRedirectPage);

			sendResponse(pClient,pRsp);
			delete pRsp;

			deleteSession(pSession, enuRETURN_CTRL_VOLUNTARY);
		}
		else
		{
			
			if(bOnlyUpdated) //for optimization do not do in case of RefreshAll because it resets timestamps anyway
			{
				//If in IHM part is configured to use ack request as part of refresh requests 
				// it would send the content in IHM_WEB_REQUEST_PARAM_CTRL_ACK_LIST parameter
				// that is extracted to list
				QList <SHttpParameter *> * pAckLst = pHttpRequest->getAckObjectsList();
				SHttpParameter * pCurrentAck;

				for(int i=0;i<pAckLst->count();i++)
				{
					pCurrentAck = pAckLst->at(i);

					if(pCurrentAck!=NULL)
					{
						pSession->updateVisibleObjectTimestamp(pCurrentAck->sName, pCurrentAck->sValue);
					}
				}
			}
			

			MIhmHttpRspRefresh * pRsp = new MIhmHttpRspRefresh(sSessionID);
			

			if(getStatus() == enuInitialized) 
			{
				bool bIsReadOnlySession = (pSession != m_pSessionInContol);
				pSession->getVisibleObjectsResponse(pRsp, bIsReadOnlySession, bOnlyUpdated);
			}

			pSession->updateLastResponseTime();
			sendResponse(pClient,pRsp);	
			delete pRsp;	
		}

	}
	else
	{
		TRACE_WEB_W(QString("MIhmWebInterface::processRefreshAllReq: Unable to find session object %1 client session: %2!")
					.arg(sSessionID).arg(pHttpRequest->getClientSessionID()));

		MIhmHttpRspError *pRsp = new MIhmHttpRspError();
							
		if(isClientValid(pClient))
			sendResponse(pClient,pRsp);

		if(pRsp!=NULL)
			delete pRsp;


	}


}


void MIhmWebInterface::processReqMiniWebRefresh(MIhmHttpRequest *pHttpRequest,
												MClientSocket *pClient,
												bool bOnlyUpdated)
{
	QString sClientSessionID = pHttpRequest->getClientSessionID();
	TRACE_WEB_D(QString("MIhmWebInterface::processReqMiniWebRefresh: Session:%1!")
								.arg(sClientSessionID.toLatin1().data()));
	
	MIhmMiniWebSession * pMiniWebSession = MIhmMiniWebSession::findByClientSessionId(&m_lstMiniWebSessions, sClientSessionID);
	
	Q_ASSERT(pClient!=NULL);

	if(pMiniWebSession==NULL )
	{
		int iMaxMiniWebClients = MIhmConfigWeb::getCfg()->getCfgDynamic()->getMaxMiniWebClients();

		if(m_lstMiniWebSessions.count()<iMaxMiniWebClients)
		{
			TRACE_WEB_D(QString("MIhmWebInterface::processReqMiniWebRefresh: Creating new mini web session for sClientSessionID=[%1]")
									.arg(sClientSessionID.toLatin1().data()));

			pMiniWebSession = new MIhmMiniWebSession(this, m_pCommonVisibleObjLogic);
			pMiniWebSession->setInitData(pClient, pHttpRequest->isProxyUsed(), pHttpRequest->getClientSessionID());
			m_lstMiniWebSessions.append(pMiniWebSession);
			bOnlyUpdated = false;
		}
		else
		{
			TRACE_WEB_D(QString("MIhmWebInterface::processReqMiniWebRefresh:Max [%1] mini web sessions  reached!")
									.arg(iMaxMiniWebClients));

			MIhmHttpRspMiniWebError * pRsp = new MIhmHttpRspMiniWebError();
			sendResponse(pClient,pRsp);	
			delete pRsp;
			return;
		}
	}

	QString sMiniWebPage = getMiniWebPage();
	if(sMiniWebPage.isEmpty())
	{
		MIhmHttpRspMiniWebError * pRsp = new MIhmHttpRspMiniWebError();
		sendResponse(pClient,pRsp);	
		delete pRsp;
		return;

	}

	
	if(pHttpRequest->getSourcePage() != getMiniWebPage())
	{
		MIhmHttpRspRedirect * pRsp = new MIhmHttpRspRedirect(getMiniWebPage());
		sendResponse(pClient,pRsp);	
		delete pRsp;
	}
	else
	{
		
		if(bOnlyUpdated) //for optimization do not do in case of RefreshAll because it resets timestamps anyway
		{
			//If in IHM part is configured to use ack request as part of refresh requests 
			// it would send the content in IHM_WEB_REQUEST_PARAM_CTRL_ACK_LIST parameter
			// that is extracted to list
			QList <SHttpParameter *> * pAckLst = pHttpRequest->getAckObjectsList();
			SHttpParameter * pCurrentAck;

			for(int i=0;i<pAckLst->count();i++)
			{
				pCurrentAck = pAckLst->at(i);

				if(pCurrentAck!=NULL)
				{
					pMiniWebSession->updateVisibleObjectTimestamp(pCurrentAck->sName, pCurrentAck->sValue);
				}
			}
		}
		

		MIhmHttpRspRefresh * pRsp = new MIhmHttpRspRefresh(pMiniWebSession->getClientSessionID());
		pMiniWebSession->getVisibleObjectsResponse(pRsp, bOnlyUpdated);
		pMiniWebSession->updateLastResponseTime();
		sendResponse(pClient,pRsp);	
		delete pRsp;	
	}

}



void MIhmWebInterface::processRefreshAckReq(MIhmHttpRequest *pHttpRequest,
											MClientSocket *pClient)
{	
	Q_ASSERT(pClient!=NULL);

	QString sSessionID = pHttpRequest->getSessionID();
	MIhmWebSession * pSession = MIhmWebSession::findSessionObj(&m_lstSessions, sSessionID);
	
	if(pSession!=NULL && pSession->getClientSessionID() == pHttpRequest->getClientSessionID())
	{	
		pSession->updateVisibleObjectTimestamp(pHttpRequest->getAckObjectName(), pHttpRequest->getAckObjectTimestamp());

		MIhmHttpRspRefreshAckOK *pRsp = new MIhmHttpRspRefreshAckOK(sSessionID);
		sendResponse(pClient,pRsp);
		delete pRsp;
	}
	else
	{
		TRACE_WEB_W(QString("MIhmWebInterface::processRefreshAckReq: Unable to find session object %1!")
					.arg(sSessionID.toLatin1().data()));

		MIhmHttpRspError *pRsp = new MIhmHttpRspError();
							
		if(isClientValid(pClient))
			sendResponse(pClient,pRsp);

		if(pRsp!=NULL)
			delete pRsp;

	}

}


					
void MIhmWebInterface::processClickCommandReq(MIhmHttpRequest *pHttpRequest,
										 MClientSocket *pClient)
{

	QString sSessionID = pHttpRequest->getSessionID();
	MIhmWebSession * pSession = MIhmWebSession::findSessionObj(&m_lstSessions, sSessionID);
	
	Q_ASSERT(pClient!=NULL);

	if(pSession!=NULL && pSession->getClientSessionID() == pHttpRequest->getClientSessionID())
	{
		pSession->resetSessionIdleTimer();
		QString sCtrlId = pHttpRequest->getClickCtrlId();

		//first handle specific actions
		if(sCtrlId==BTN_VAL_WEB_TAKE_CTRL)
		{
			processTakeControlReq(pHttpRequest, pClient);
		}
		else if(sCtrlId==BTN_VAL_WEB_RETURN_CTRL)	//this one is return control if in control and log off if not in control 
		{
			processReturnControlReq(pHttpRequest, pClient);
		}
		else
		{ 
			if(pSession == m_pSessionInContol)
			{
				QString sAction;
				QString sParams;
				if(pSession->getActionForCtrlId(sCtrlId, sAction, sParams))
				{
					if(sAction!="")
						emit action(sSessionID, sAction, sParams);
				}

			}
			
			MIhmHttpRspRefreshAckOK *pRsp = new MIhmHttpRspRefreshAckOK(sSessionID);
			sendResponse(pClient,pRsp);
			delete pRsp;
		}

	}
	else
	{
		TRACE_WEB_W(QString("MIhmWebInterface::processClickCommandReq: Unable to find session object %1!")
					.arg(sSessionID.toLatin1().data()));

		MIhmHttpRspError *pRsp = new MIhmHttpRspError();
							
		if(isClientValid(pClient))
			sendResponse(pClient,pRsp);

		if(pRsp!=NULL)
			delete pRsp;

	}

}


void MIhmWebInterface::processGetDynImagePathReq(MIhmHttpRequest *pHttpRequest,
										 MClientSocket *pClient)
{
	bool bAuthOK = false;
	QString sDynImagePath;


	QString sSessionID = pHttpRequest->getClientSessionID();
	MIhmWebSession * pSession = MIhmWebSession::findByClientSessionId(&m_lstSessions, sSessionID);
	
	Q_ASSERT(pClient!=NULL);

	if(pSession!=NULL && pSession->getClientSessionID() == pHttpRequest->getClientSessionID())
	{
		if(pSession->getStatus()>MIhmWebSession::enuWebSessWaitingAuth)
		{

			bAuthOK = true;
			sDynImagePath = m_pCommonVisibleObjLogic->getDynImageFullPath(pHttpRequest->getDynImageObjectName());
			TRACE_WEB_D(QString("MIhmWebInterface::processGetDynImagePathReq: %1!")
												.arg(sDynImagePath.toLatin1().data()));
		}

	}

	MIhmHttpRspDynImagePath *pRsp = new MIhmHttpRspDynImagePath(bAuthOK, sDynImagePath);
	sendResponse(pClient,pRsp);
	delete pRsp;
}



void MIhmWebInterface::processCommentsUpdate(MIhmHttpRequest *pHttpRequest,
										 MClientSocket *pClient)
{

	QString sSessionID = pHttpRequest->getSessionID();
	MIhmWebSession * pSession = MIhmWebSession::findSessionObj(&m_lstSessions, sSessionID);
	
	Q_ASSERT(pClient!=NULL);

	if(pSession!=NULL && pSession->getClientSessionID() == pHttpRequest->getClientSessionID())
	{
		QString sData = pHttpRequest->getCommentsUpdateData();

		if(pSession == m_pSessionInContol)
		{
			pSession->resetSessionIdleTimer();
			//create complete result and emit 
			MIhmMsgAsyncInputRsp *pMsg = new MIhmMsgAsyncInputRsp();
			pMsg->m_sEncStrValues = sData;

			emit webInputDlgsRsp(sSessionID, pMsg);				
		}

		MIhmHttpRspRefreshAckOK *pRsp = new MIhmHttpRspRefreshAckOK(sSessionID);
		sendResponse(pClient,pRsp);
		delete pRsp;

	}
	else
	{
		TRACE_WEB_W(QString("MIhmWebInterface::processCommentsUpdate: Unable to find session object %1!")
					.arg(sSessionID.toLatin1().data()));

		MIhmHttpRspError *pRsp = new MIhmHttpRspError();
							
		if(isClientValid(pClient))
			sendResponse(pClient,pRsp);

		if(pRsp!=NULL)
			delete pRsp;

	}

}

bool MIhmWebInterface::verifyAndPrepareCommentsUpdate(QString &sData)
{
	bool bRetVal = false;

	MIhmVirtAsyncInput * pVirtObjAsync = 
					(MIhmVirtAsyncInput *)m_pWEBVirtObjectsModel->getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtAsyncInputsID);


	if(pVirtObjAsync!=NULL)
	{
		MInputAsyncExReq oReq = *(pVirtObjAsync->m_pAsyncReq);
		oReq.updateInputValues(sData);
		QString sFinalData = oReq.getEncStrValues();
		sData = sFinalData;
		bRetVal = true;
	}


	return bRetVal;
}




void MIhmWebInterface::processKeyDownReq(MIhmHttpRequest *pHttpRequest,
										 MClientSocket *pClient)
{

	QString sWebSessionID = pHttpRequest->getSessionID();
	MIhmWebSession * pSession = MIhmWebSession::findSessionObj(&m_lstSessions, sWebSessionID);
	
	Q_ASSERT(pClient!=NULL);

	if(pSession!=NULL && pSession->getClientSessionID() == pHttpRequest->getClientSessionID())
	{
		if(pSession == m_pSessionInContol)
		{
			pSession->resetSessionIdleTimer();
			int iKeyCode = pHttpRequest->getKeyCode();
			emit keyDetected(sWebSessionID, iKeyCode);
		}

		MIhmHttpRspRefreshAckOK *pRsp = new MIhmHttpRspRefreshAckOK(sWebSessionID);
		sendResponse(pClient,pRsp);
		delete pRsp;

	}
	else
	{
		TRACE_WEB_W(QString("MIhmWebInterface::processKeyDownReq: Unable to find session object %1!")
					.arg(sWebSessionID.toLatin1().data()));

		MIhmHttpRspError *pRsp = new MIhmHttpRspError();
							
		if(isClientValid(pClient))
			sendResponse(pClient,pRsp);

		if(pRsp!=NULL)
			delete pRsp;

	}

}


void MIhmWebInterface::processTakeControlReq(MIhmHttpRequest *pHttpRequest,
											MClientSocket *pClient)
{	
	Q_ASSERT(pClient!=NULL);

	QString sSessionID = pHttpRequest->getSessionID();
	MIhmWebSession * pSession = MIhmWebSession::findSessionObj(&m_lstSessions, sSessionID);
	TRACE_WEB_D(QString("MIhmWebInterface::processTakeControlReq: Session %1!")
											.arg(sSessionID.toLatin1().data()));
	
	if(pSession!=NULL && pSession->getClientSessionID() == pHttpRequest->getClientSessionID())
	{	
		pSession->updateLastResponseTime();

		MIhmHttpRspRefreshAckOK *pRsp = new MIhmHttpRspRefreshAckOK(sSessionID);
		sendResponse(pClient,pRsp);
		delete pRsp;
		
		emit takeControlReq(sSessionID);
	}
	else
	{
		TRACE_WEB_W(QString("MIhmWebInterface::processTakeControlReq: Unable to find session object %1!")
					.arg(sSessionID.toLatin1().data()));

		MIhmHttpRspError *pRsp = new MIhmHttpRspError();
							
		if(isClientValid(pClient))
			sendResponse(pClient,pRsp);

		if(pRsp!=NULL)
			delete pRsp;

	}

}


void MIhmWebInterface::processReturnControlReq(MIhmHttpRequest *pHttpRequest,
											MClientSocket *pClient)
{	
	Q_ASSERT(pClient!=NULL);

	QString sSessionID = pHttpRequest->getSessionID();
	MIhmWebSession * pSession = MIhmWebSession::findSessionObj(&m_lstSessions, sSessionID);
	
	TRACE_WEB_D(QString("MIhmWebInterface::processReturnControlReq: Session %1!")
											.arg(sSessionID.toLatin1().data()));
	
	if(pSession!=NULL && pSession->getClientSessionID() == pHttpRequest->getClientSessionID())
	{	
		pSession->updateLastResponseTime();
		
		if(m_pSessionInContol == pSession)
		{
			setSessionInControl(NULL);
			emit returnControlReq(sSessionID, (int)enuRETURN_CTRL_VOLUNTARY);
		}
		else
		{
			pSession->logOff();
		}

		MIhmHttpRspRefreshAckOK *pRsp = new MIhmHttpRspRefreshAckOK(sSessionID);
		sendResponse(pClient,pRsp);
		delete pRsp;

	}
	else
	{
		TRACE_WEB_W(QString("MIhmWebInterface::processReturnControlReq: Unable to find session object %1!")
					.arg(sSessionID.toLatin1().data()));

		MIhmHttpRspError *pRsp = new MIhmHttpRspError();
							
		if(isClientValid(pClient))
			sendResponse(pClient,pRsp);

		if(pRsp!=NULL)
			delete pRsp;

	}

}

//-------------------------------------
//		Dead sessions cleanup ...
//-------------------------------------

void MIhmWebInterface::onDeadSessionsCheckTimer()
{
	verifySessionTimeouts();
	verifyMiniWebSessionTimeouts();
}

void MIhmWebInterface::verifySessionTimeouts()
{
	MIhmWebSession * pSession;

	for(int i=0; i<m_lstSessions.size(); ++i)
	{
		pSession = m_lstSessions.at(i);

		if(pSession->isSessionDead(m_oWebConfig.getWebSessionTimeout()))
		{
			m_lstSessions.removeAt(i);
			
			QString sCurrentCtrlSessionID = pSession->getSessionID();
			emit endIdentificationReq(sCurrentCtrlSessionID, enuRETURN_CTRL_IDLE_TIMEOUT);
			
			if(m_pSessionInContol == pSession)
			{
				setSessionInControl(NULL);
			}
			
			TRACE_WEB_D(QString("MIhmWebInterface::verifySessionTimeouts: Deleting session object %1!")
					.arg(pSession->getSessionID().toLatin1().data()));

			delete pSession;
			i=i-1;
		}	
	}
}	


void MIhmWebInterface::verifyMiniWebSessionTimeouts()
{
	MIhmMiniWebSession * pMiniWebSession;

	for(int i=0; i<m_lstMiniWebSessions.size(); ++i)
	{
		pMiniWebSession = m_lstMiniWebSessions.at(i);

		if(pMiniWebSession->isSessionDead(m_oWebConfig.getWebMiniWebSessionTimeout()))
		{
			m_lstMiniWebSessions.removeAt(i);
			
			TRACE_WEB_D(QString("MIhmWebInterface::verifyMiniWebSessionTimeouts: Deleting session object %1!")
					.arg(pMiniWebSession->getClientSessionID().toLatin1().data()));

			delete pMiniWebSession;
			i=i-1;
		}	
	}
}	



void MIhmWebInterface::deleteSession(MIhmWebSession * pTargetSession, int iReason, bool bSendEndIdentif)
{
	MIhmWebSession * pSession;

	for(int i=0; i<m_lstSessions.size(); ++i)
	{
		pSession = m_lstSessions.at(i);

		if(pSession==pTargetSession)
		{
			if(pSession==m_pSessionInContol)
			{
				setSessionInControl(NULL);
				//should work since when in control the deleteSession is called only on idle timeout 
				//if not in control the return control have already been sent
				emit returnControlReq(pSession->getSessionID(), (int)iReason); 
			}
			
			if(bSendEndIdentif)
				emit endIdentificationReq(pSession->getSessionID(), (int)iReason);

			m_lstSessions.removeAt(i);
			
			TRACE_WEB_D(QString("MIhmWebInterface::deleteSession: Deleting session object %1!")
					.arg(pSession->getSessionID().toLatin1().data()));
			
			delete pSession;
			break;
		}	
	}
}	

int MIhmWebInterface::getNumInitializedSessions()
{
	int iCount = 0;
	MIhmWebSession * pSession;

	for(int i=0; i<m_lstSessions.size(); ++i)
	{
		pSession = m_lstSessions.at(i);

		if(pSession->getStatus()>MIhmWebSession::enuWebSessWaitingAuth)
		{
			iCount++;
		}	
	}

	return iCount;
}


bool MIhmWebInterface::sendResponse(MClientSocket *pClient, MIhmHttpResponse *pResponse)
{
	 
	Q_ASSERT(pClient!=NULL);

	bool bOK = false;

    QByteArray baResponse;
	
	if(!pResponse->getRspData(&baResponse))
	{
		TRACE_WEB_W(QString("MIhmWebInterface::sendResponse: invalid response %1 not sent").arg(baResponse.data()));
		bOK = false;
	}
	else
	{
		bOK = pClient->sendData(baResponse);
	}

	pClient->close();
	

	return bOK;
}


void MIhmWebInterface::onVirtualObjectsUpdated()
{
	m_pTCLInterface->getUpdatedVirtualObjectsForWeb(m_pWEBVirtObjectsModel);

	signalUpdateNeeded();
}

void MIhmWebInterface::signalUpdateNeeded()
{
	if(m_iPrivateUpdateEmmited<1)
	{
		m_iPrivateUpdateEmmited++;
		emit privateUpdateObjectsNeeded();
	}
}


void MIhmWebInterface::onPrivateUpdateObjects()
{
	int iObjectsProcessedInTurn = 0;
	m_iPrivateUpdateEmmited--;
	TRACE_WEB_D("MIhmWebInterface::onPrivateUpdateObjects...");

	QVector <MIhmVirtualObject*> * pVector = m_pWEBVirtObjectsModel->getVirtObjVector();

	for (int i = 0; i < pVector->size(); ++i) 
	{
		if(pVector->at(i)==NULL)
			continue;

		if(!pVector->at(i)->isUpdated())
				continue;

		//process virtual object
		processVirtualObject(pVector->at(i));
		
		if(m_bRefreshAllObjects)
				break;

		iObjectsProcessedInTurn++;

		if(iObjectsProcessedInTurn>=3)
		{
			signalUpdateNeeded();
			break;
		}
	}

	if(m_bRefreshAllObjects)
	{
		refreshAllVirtualObjects();
		m_bRefreshAllObjects = false;
	}

	TRACE_WEB_D("MIhmWebInterface::onPrivateUpdateObjects...Done!");

}





void MIhmWebInterface::refreshSessionObjects(MIhmWebSession * pSession)
{
	if(pSession==NULL)
	{
		TRACE_WEB_W("MIhmWebInterface::refreshSessionObjects: Ignoring since session is NULL!!!");
		return;
	}

	TRACE_WEB_D("MIhmWebInterface::refreshSessionObjects ...");

	MIhmVirtualObject *pVirtObj;
	QVector <MIhmVirtualObject*> * pVector = m_pWEBVirtObjectsModel->getVirtObjVector();

	for (int i = 0; i < pVector->size(); ++i) 
	{
		if(pVector->at(i)==NULL)
			continue;

		pVirtObj = pVector->at(i);
		
		if(pVirtObj->getId() == MIhmVirtualObject::enuIhmVirtConfLaneType ||
				pVirtObj->getId() == MIhmVirtualObject::enuIhmVirtConfLanguage)
					continue;
		
		//process virtual object
		if(pVirtObj->isInitialized() && ifShouldProcessInSessionObjects(pVirtObj))
				pSession->processVirtualObject(pVirtObj);
	}
	
}


void MIhmWebInterface::refreshAllVirtualObjects()
{
	TRACE_WEB_D("MIhmWebInterface::refreshAllVirtualObjects ...");
	
	if(m_bThreadStarted)
	{
		MIhmVirtualObject *pVirtObj;
		QVector <MIhmVirtualObject*> * pVector = m_pWEBVirtObjectsModel->getVirtObjVector();

		for (int i = 0; i < pVector->size(); ++i) 
		{
			if(pVector->at(i)==NULL)
				continue;

			pVirtObj = pVector->at(i);

			if(pVirtObj->getId() == MIhmVirtualObject::enuIhmVirtConfLaneType ||
				pVirtObj->getId() == MIhmVirtualObject::enuIhmVirtConfLanguage)
						continue;
			
			//process virtual object
			if(pVirtObj->isInitialized())
				processVirtualObject(pVirtObj);
		}

		//refresh mini web sessions since it can exist before the the LaneType object is updated
		//---------------------------------------------------------------------------
		MIhmMiniWebSession * pMiniSession;

		for(int i=0; i<m_lstMiniWebSessions.size(); ++i)
		{
			pMiniSession = m_lstMiniWebSessions.at(i);
			pMiniSession->initializeCompleteVisibleObjectLinks(m_pCommonVisibleObjLogic);
		}
		//---------------------------------------------------------------------------
	
	}

}



void MIhmWebInterface::processVirtualObject(MIhmVirtualObject *pVirtObj)
{
	if(m_pHttpServer==NULL||!m_bThreadStarted)
	{
		//not initialized yet !
		return; 
	}

	//If the web is not initialized yet 
	//reset the virtual object flag and do nothing 
	//The config MIhmVirtDynConfig / enuIhmVirtConfLaneType
	//will refresh all the virtual objects
	if(pVirtObj->getId() != MIhmVirtualObject::enuIhmVirtConfLaneType &&	
			getStatus() != enuInitialized)
	{
			pVirtObj->clearUpdated();
			return;
	}
	else if(pVirtObj->getId() == MIhmVirtualObject::enuIhmVirtConfLaneType )
	{
		TRACE_WEB_D("MIhmWebInterface::processVirtualObject:Processing enuIhmVirtConfLaneType!");
		m_bRefreshAllObjects = true;
		m_eStatus = enuInitialized;
	}

	if(pVirtObj->getId() == MIhmVirtualObject::enuIhmVirtConfLanguage)
	{
		processVirtConfigLanguage((MIhmVirtDynConfig*)pVirtObj);
		pVirtObj->clearUpdated();
		
		if(m_eStatus == enuInitialized)
			m_bRefreshAllObjects = true;
	}
	else
	{
		if(ifShouldProcessInSessionObjects(pVirtObj))
		{
			MIhmWebSession * pSession;

			for(int i=0; i<m_lstSessions.size(); ++i)
			{
				pSession = m_lstSessions.at(i);
				pSession->processVirtualObject(pVirtObj);
			}
		}

		if(ifShouldProcessInCommonObjects(pVirtObj))
		{
			m_pCommonVisibleObjLogic->processVirtualObject(pVirtObj);
		}

		pVirtObj->clearUpdated();
	}
}

bool MIhmWebInterface::ifShouldProcessInCommonObjects(MIhmVirtualObject * pVirtObj)
{
	switch(pVirtObj->getType())
	{
		case MIhmVirtualObject::enuIhmContainer:
		case MIhmVirtualObject::enuIhmIco:
		case MIhmVirtualObject::enuIhmText:
		case MIhmVirtualObject::enuIhmButton:
		case MIhmVirtualObject::enuIhmList:
		case MIhmVirtualObject::enuIhmDynStrDetection:
		case MIhmVirtualObject::enuIhmTableViewReq:
		case MIhmVirtualObject::enuIhmTabView:
		case MIhmVirtualObject::enuIhmAsyncInput:		//M_IHM_ASYNC_INPUT service handling
		case MIhmVirtualObject::enuIhmHMenuViewReq:		
		case MIhmVirtualObject::enuIhmVideoViewReq:		
		case MIhmVirtualObject::enuIhmDynConfig:
				return true;
			break;

		default:
				return false;
			break;
	}

}

bool MIhmWebInterface::ifShouldProcessInSessionObjects(MIhmVirtualObject * pVirtObj)
{
	switch(pVirtObj->getType())
	{
		case MIhmVirtualObject::enuIhmDynConfig:
		case MIhmVirtualObject::enuIhmInputDialogReq:
		case MIhmVirtualObject::enuIhmInputDlgExReq:
				return true;
			break;

		default:
				return false;
			break;
	}

}


void MIhmWebInterface::processVirtConfigLanguage(MIhmVirtDynConfig* pVirtObj)
{
	QString sNewLanguage;
	
	sNewLanguage = pVirtObj->m_sDynConfigSetting;

	//try load language
	if(!MIhmConfigWeb::getCfg()->getLanguages()->setLanguage(sNewLanguage))
	{
		//ignore the change and keep the old lane type
        TRACE_WEB_W(QString("MIhmDesktopIntf::processVirtConfigLanguage: Unable to initialize Language:%1")
						.arg(sNewLanguage));
	}
}

//to assure that the visible objects of the session are initialized
void MIhmWebInterface::initSessionVisibleObjects(MIhmWebSession *pSession)
{
	TRACE_WEB_D("MIhmWebInterface::initSessionVisibleObjects ...");

	pSession->setStatusInitializing();

	MIhmVirtualObject *pVirtObj;
	pVirtObj = m_pWEBVirtObjectsModel->getVirtualObjectByID( MIhmVirtualObject::enuIhmVirtConfLaneType);

	if(pVirtObj!=NULL)
		pSession->processVirtualObject(pVirtObj);
	else
	{	TRACE_WEB_D("MIhmWebInterface::initSessionVisibleObjects: No LANE TYPE initialized yet!");
		return;	
	}

	QVector <MIhmVirtualObject*> * pVector = m_pWEBVirtObjectsModel->getVirtObjVector();

	for (int i = 0; i < pVector->size(); ++i) 
	{
		if(pVector->at(i)==NULL)
			continue;

		pVirtObj = pVector->at(i);
		//process virtual object
		if(pVirtObj->getId()!=MIhmVirtualObject::enuIhmVirtConfLaneType && pVirtObj->isInitialized() && ifShouldProcessInSessionObjects(pVirtObj))
					pSession->processVirtualObject(pVirtObj);
	}
}



void MIhmWebInterface::setSessionInControl(MIhmWebSession * pNewCtrlSession)
{

	m_pSessionInContol = pNewCtrlSession;
	
	//update the opened sessions
	MIhmWebSession * pSession;

	for(int i=0; i<m_lstSessions.size(); ++i)
	{
		pSession = m_lstSessions.at(i);

		if(pSession!=NULL)
		{

			if(pSession->setInControl(pSession == m_pSessionInContol)) 
			{	//if in control status changed refresh all objects for the session
				refreshSessionObjects(pSession);
			}
		}
	}
	
}


//get lane type to resolve the parameter
MIhmLaneTypeSettings * MIhmWebInterface::getCurrentLaneTypeSettings()
{
	MIhmLaneTypeSettings * pSettings = NULL;
	QString sLaneType;

	MIhmVirtDynConfig * pVirtLaneType = (MIhmVirtDynConfig *)m_pWEBVirtObjectsModel->getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtConfLaneType);

	if(pVirtLaneType!=NULL)
	{
		sLaneType = pVirtLaneType->m_sDynConfigSetting;

		if(sLaneType=="") 
		{
			TRACE_WEB_W( QString("MIhmWebInterface::getCurrentLaneTypeSettings: Error lane type unknown!"));
			return NULL;
		}
	}
		

	pSettings = MIhmConfigWeb::getCfg()->getLaneTypeSetting(MIhmLaneTypeSettings::enuWEB, sLaneType);
	return pSettings;
}


QString MIhmWebInterface::getVisObjHtmlTemplate(QString sObjID)
{
	QString sTemplate;
	MIhmLaneTypeSettings * pSettings = getCurrentLaneTypeSettings();

	if(pSettings!=NULL)
	{
		sTemplate = pSettings->getVisObjHtmlTemplate(sObjID);

		if(sTemplate=="") 
			TRACE_WEB_W( QString("MIhmWebInterface::getVisObjHtmlTemplate: No Html template for object:[%1]!").arg(sTemplate));
	}

	return sTemplate;
}


//Functions for processing commands from ANI
void MIhmWebInterface::onOpenLinkReq(MIhmMsgOpenLinkReq * pReq)
{
	if(m_pSessionInContol!=NULL)
	{
		m_pSessionInContol->appendOpenLinkReq(pReq->m_sUrl, pReq->m_iTargetWin, pReq->m_bVisible);
	}
	
	delete pReq;
}

void MIhmWebInterface::onReloadWebDynConfig()
{
	MIhmConfigWeb::getCfg()->reloadDynamicConfig();

	//reset all sessions idle timers
	MIhmWebSession * pSession;

	for(int i=0; i<m_lstSessions.size(); ++i)
	{
		pSession = m_lstSessions.at(i);

		if(pSession!=NULL)
			pSession->resetSessionIdleTimer();
	}	

	refreshAllVirtualObjects();

}

//-------------------------------------------------------------------------------------------------




//End of file ------------------
