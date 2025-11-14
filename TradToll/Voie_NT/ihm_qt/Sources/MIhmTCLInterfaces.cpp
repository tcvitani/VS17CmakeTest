
#include <QObject>
#include <QUrl>
#include <QDesktopServices>
#include <QProcess>
#include <QTimer>

#include "MIhmTCLInterfaces.h"
#include "MIhmDesktopIntf.h"
#include "MIhmWebInterface.h"
#include "MIhmMainLogic.h"
#include "MIhmAction.h"
#include "MIhmSessionUserData.h"
#include "MIHMAuthAutomate.h"
#include "MTracer.h"
#include "MIhmMsg.h"
#include "MIhmMsgVideo.h"
//#include "MRVideoControlObject.h"
#include "MRVideoMsg.h"
#include "MIhmVirtObjectsModel.h"

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include "ihm.h"
}


MIhmTCLInterfaces::MIhmTCLInterfaces()
{
	m_pDesktopIntf = NULL;
	m_pWebIntf = NULL;
	m_bDskInitOK = false;
	m_pAuthAutomate = NULL; 
	m_pUserInControl = NULL;
	m_pUserTakingCtrl = NULL;

	m_tmCommTimer.setSingleShot(true);
	connect(&m_tmCommTimer,SIGNAL(timeout()), this, SLOT(onCommTimeout()));

	m_tmTakeOverTimer.setSingleShot(true);
	connect(&m_tmTakeOverTimer,SIGNAL(timeout()), this, SLOT(onTakeOverTimeout()));
	
	m_bCommStarted = false;
	m_bTakeOverStarted = false;

	//Input msgs frequency limitation data
	m_dtLastInput = QTime::currentTime();

	m_pWEBVirtObjectModel = new MIhmVirtObjectsModel();


}


MIhmTCLInterfaces::~MIhmTCLInterfaces()
{
	TRACE_D(QString("MIhmTCLInterfaces::~MIhmTCLInterfaces: closing app..."));

	if(m_pDesktopIntf != NULL)
	{
		TRACE_D(QString("MIhmTCLInterfaces::~MIhmTCLInterfaces: deleting desktop interface..."));
		delete m_pDesktopIntf;
	}

	if(m_pWebIntf != NULL)
	{
		TRACE_D(QString("MIhmTCLInterfaces::~MIhmTCLInterfaces: deleting web interface..."));
		if(m_pWebIntf->isRunning())
		{
			m_pWebIntf->quit();
			m_pWebIntf->wait();
		}

		delete m_pWebIntf;
	}

	if(m_pAuthAutomate != NULL)
	{
		delete m_pAuthAutomate;
	}

	while (!m_lstAuthRequestQueue.isEmpty())
		delete m_lstAuthRequestQueue.takeFirst();

	while (!m_lstIdentifiedUsers.isEmpty())
		delete m_lstIdentifiedUsers.takeFirst();

	//while (!m_lstRVideoCtrl.isEmpty())
	//	delete m_lstRVideoCtrl.takeFirst();
	
	delete m_pWEBVirtObjectModel;	
}





bool MIhmTCLInterfaces::initialize(MIhmMainLogic *pParent)
{
	TRACE_D(QString("MIhmTCLInterfaces::initialize..."));

	m_pMainLogic = pParent;

	
	m_pDesktopIntf = new MIhmDesktopIntf();
	
	QObject::connect(m_pDesktopIntf,SIGNAL(exitProgram()),this, SIGNAL(exitProgram()));
	QObject::connect(m_pDesktopIntf,SIGNAL(keyDetected(int)),this, SLOT(onDskKeyDetected(int)));
	QObject::connect(m_pDesktopIntf,SIGNAL(stringDetected(MIhmMsgStringDetection *)),
								this, SLOT(onDskStringDetected(MIhmMsgStringDetection *)));

	QObject::connect(this, SIGNAL(updateVirtObjDesktop()),m_pDesktopIntf, SLOT(onVirtualObjectsUpdated()),Qt::QueuedConnection);

	QObject::connect(m_pDesktopIntf,SIGNAL(msgToAni(MIhmMsg *)),
								this, SLOT(onDskMessageToAni(MIhmMsg *)));
	QObject::connect(m_pDesktopIntf,SIGNAL(inputDialogRsp(MIhmMsg *)),
								this, SLOT(onDskInputDialogResult(MIhmMsg *)));
	QObject::connect(m_pDesktopIntf,SIGNAL(asyncInputRsp(MIhmMsg *)),
								this, SLOT(onDskAsyncInputRsp(MIhmMsg *)));
	QObject::connect(m_pDesktopIntf,SIGNAL(videoCtlRsp(MIhmMsg *)),
								this, SLOT(onDskVideoCtlRsp(MIhmMsg *)));
		

	QObject::connect(m_pDesktopIntf,SIGNAL(action(QString, QString)),
								this, SLOT(onDskAction(QString, QString)));
	QObject::connect(m_pDesktopIntf,SIGNAL(aboutDlgAction(QString, QString)),
								this, SLOT(onDskAboutDlgAction(QString, QString)));
	
	
	QObject::connect(m_pDesktopIntf,SIGNAL(identificationReq(QString, QString, bool)),
								this, SLOT(onDskIdentificationReq(QString, QString, bool)));
	QObject::connect(m_pDesktopIntf,SIGNAL(returnCtrlReq(QString, int)),this, SLOT(onDskReturnCtrlReq(QString, int)));
	QObject::connect(m_pDesktopIntf,SIGNAL(endIdentificationReq(QString, int)),this, SLOT(onDskEndIdentificationReq(QString, int)));
	QObject::connect(m_pDesktopIntf,SIGNAL(authDlgCanceled(int)),this, SLOT(onDskAuthDlgCanceled(int)));


	QObject::connect(m_pDesktopIntf,SIGNAL(takeCtrlReq(QString)),this, SLOT(onDskTakeCtrlReq(QString)));
	
	QObject::connect(m_pDesktopIntf,SIGNAL(takeOverReject()),this, SLOT(onDskTakeOverReject()));
	QObject::connect(m_pDesktopIntf,SIGNAL(takeOverAccept()),this, SLOT(onDskTakeOverAccept()));
	QObject::connect(m_pDesktopIntf,SIGNAL(takeOverCancel()),this, SLOT(onDskTakeOverCancel()));


	m_bDskInitOK = m_pDesktopIntf->initialize();
	
	if(m_bDskInitOK)
	{
		TRACE_D(QString("MIhmTCLInterfaces::initialize...m_bDskInitOK = true"));
		//Web interface thread starting ...
		if(MIhmConfigGeneral::getCfg()->isWebEnabled())
		{
			TRACE_D(QString("MIhmTCLInterfaces::initialize...Web enabled! Initializing ..."));
			
			m_pWebIntf = new MIhmWebInterface();

			QObject::connect(m_pWebIntf,SIGNAL(initOK()),
									this, SLOT(onWebInitOK()), Qt::QueuedConnection);

			QObject::connect(this, SIGNAL(updateVirtObjWeb()),
									m_pWebIntf, SLOT(onVirtualObjectsUpdated()),Qt::QueuedConnection);
			QObject::connect(m_pWebIntf,SIGNAL(keyDetected(QString, int)),
									this, SLOT(onWebKeyDetected(QString, int)),Qt::QueuedConnection);
			QObject::connect(m_pWebIntf,SIGNAL(action(QString, QString, QString)),
									this, SLOT(onWebAction(QString, QString, QString)),Qt::QueuedConnection);

			//ASYNC INPUT SIGNAL
			//INPUT DIALOG COMMUNICATION SIGNALS (SAISIE_VISU)
			//INPUT DIALOG EX COMMUNICATION SIGNALS (SAISIE_VISU_EX)
			QObject::connect(m_pWebIntf,SIGNAL(webInputDlgsRsp(QString, MIhmMsg *)),
									this, SLOT(onWebInputDlgsMessage(QString, MIhmMsg *)),Qt::QueuedConnection);

			//-----------------------------------------
			QObject::connect(m_pWebIntf,SIGNAL(authorisationReq(QString, QString, bool, QString)),
									this, SLOT(onWebAuthorisationReq(QString, QString, bool, QString)),Qt::QueuedConnection);
			QObject::connect(m_pWebIntf,SIGNAL(returnControlReq(QString, int)),
									this, SLOT(onWebReturnControlReq(QString, int)),Qt::QueuedConnection);
			QObject::connect(m_pWebIntf,SIGNAL(endIdentificationReq(QString, int)),
									this, SLOT(onWebEndIdentificationReq(QString, int)),Qt::QueuedConnection);
			QObject::connect(m_pWebIntf,SIGNAL(takeControlReq(QString)),
									this, SLOT(onWebTakeControlReq(QString)),Qt::QueuedConnection);
			QObject::connect(m_pWebIntf,SIGNAL(takeOverCancel(QString)),
									this, SLOT(onWebTakeOverCancel(QString)), Qt::QueuedConnection);
			QObject::connect(m_pWebIntf,SIGNAL(takeOverReject(QString)),
									this, SLOT(onWebTakeOverReject(QString)), Qt::QueuedConnection);
			QObject::connect(m_pWebIntf,SIGNAL(takeOverAccept(QString)),
									this, SLOT(onWebTakeOverAccept(QString)), Qt::QueuedConnection);

			QObject::connect(this, SIGNAL(authorizationRsp(QString, bool, QString)),
									m_pWebIntf, SLOT(onAuthorisationResult(QString, bool, QString)),Qt::QueuedConnection);
			QObject::connect(this, SIGNAL(forceUnIdentification(QString)),
									m_pWebIntf, SLOT(onForcedUnIdentification(QString)),Qt::QueuedConnection);

			QObject::connect(this, SIGNAL(webUserInControl(QString)),
									m_pWebIntf, SLOT(onWebUserInControl(QString)),Qt::QueuedConnection);
			QObject::connect(this, SIGNAL(initWebTakeOver(bool , QString , QString)),
									m_pWebIntf, SLOT(onInitTakeOver(bool , QString , QString)),Qt::QueuedConnection);
			QObject::connect(this, SIGNAL(cancelWebTakeOver(QString)),
									m_pWebIntf, SLOT(onCancelTakeOver(QString)),Qt::QueuedConnection);
			QObject::connect(this, SIGNAL(showClientInfoMsg(QString, QString, QString)),
									m_pWebIntf, SLOT(onShowClientInfoMsg(QString, QString, QString)),Qt::QueuedConnection);
			QObject::connect(this, SIGNAL(hideClientInfoMsg(QString)),
									m_pWebIntf, SLOT(onHideClientInfoMsg(QString)),Qt::QueuedConnection);
			QObject::connect(this, SIGNAL(showConnectingMsg(QString, QString, QString)),
									m_pWebIntf, SLOT(onShowConnectingMsg(QString, QString, QString)),Qt::QueuedConnection);
			QObject::connect(this, SIGNAL(hideConnectingMsg(QString)),
									m_pWebIntf, SLOT(onHideConnectingMsg(QString)),Qt::QueuedConnection);

			
			QObject::connect(this, SIGNAL(reloadWebDynConfig()),
									m_pWebIntf, SLOT(onReloadWebDynConfig()),Qt::QueuedConnection);

			QObject::connect(this, SIGNAL(openLinkReq(MIhmMsgOpenLinkReq *)),
									m_pWebIntf, SLOT(onOpenLinkReq(MIhmMsgOpenLinkReq *)),Qt::QueuedConnection);

			
			if(m_pWebIntf->initialize(this, MIhmConfigGeneral::getCfg()->getModuleConfigKey()))
				m_pWebIntf->start(MIhmConfigGeneral::getCfg()->getWebPriority());

			
			TRACE_D(QString("MIhmTCLInterfaces::initialize...Web initialized ..."));

		}

		m_pAuthAutomate = new MIHMAuthAutomate();
		m_pAuthAutomate->initialize(this);

//		InitRVideoControlObjects();

		if(!MIhmConfigGeneral::getCfg()->isWebEnabled()) // if WebEnabled the signal will be sent when web interface initializes 
		{
			emit initOK();
		}
	}

	return m_bDskInitOK;
}


void MIhmTCLInterfaces::close()
{
	
	if(m_pWebIntf!=NULL)
	{
		m_pWebIntf->disconnect();

		if(m_pWebIntf->isRunning())
		{
			m_pWebIntf->quit();
			m_pWebIntf->wait();
		}
	}	
	
	m_pDesktopIntf->close();

	this->disconnect();
}

void MIhmTCLInterfaces::displayInitInfo(QString s)
{
	m_pDesktopIntf->displayInitInfo(s);
}

void MIhmTCLInterfaces::updateVirtualObjects(MIhmVirtObjectsModel * pTransportModel)
{
	//TRACE_D(QString("MIhmTCLInterfaces::updateVirtualObjects...Num objects to Update: %1").arg(pTransportModel->getNumUpdatedObjects()));
	
	//copy virtual objects for desktop...
	m_pDesktopIntf->updateVirtualObjects(pTransportModel);

	//copy virtual objects for web...
	m_WebMutex.lock();
		pTransportModel->copyUpdatedVirtObjects(m_pWEBVirtObjectModel, true);	//reset updated flag not to copy the same change twice	 
	m_WebMutex.unlock();

	emit updateVirtObjWeb();
 	emit updateVirtObjDesktop();
}

void MIhmTCLInterfaces::getUpdatedVirtualObjectsForWeb(MIhmVirtObjectsModel * pWebModel)
{
	m_WebMutex.lock();
		m_pWEBVirtObjectModel->copyUpdatedVirtObjects(pWebModel, true);		
	m_WebMutex.unlock();

}


void MIhmTCLInterfaces::onWebInitOK()
{
	if(m_bDskInitOK == true)
		emit initOK();
}


//****************************************************//
//AUTHORISATION / TAKE CONTROL FUNCTIONS 
//****************************************************//


void MIhmTCLInterfaces::startCommTimer()
{
	m_bCommStarted = true;
	int intAuthReqTimeout = MIhmConfigGeneral::getCfg()->getAuthReqTimeout();
	
	if(intAuthReqTimeout>0)
		m_tmCommTimer.start(intAuthReqTimeout);
}

void MIhmTCLInterfaces::stopCommTimer()
{
	m_bCommStarted = false;
	m_tmCommTimer.stop();
}

void MIhmTCLInterfaces::onCommTimeout()
{
	if(m_bCommStarted)
	{
		MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtCommTimeout); 
		m_pAuthAutomate->processEvent(pEvent);
	}
}


void MIhmTCLInterfaces::startTakeOverTimer()
{
	int dwTakeOverTimeout = MIhmConfigGeneral::getCfg()->getCfgDynamic()->getTakeCtrlTimeout();
	m_tmTakeOverTimer.start(dwTakeOverTimeout*1000); //it is in seconds so we convert it to [ms]
	m_bTakeOverStarted = true;
}

void MIhmTCLInterfaces::stopTakeOverTimer()
{
	m_bTakeOverStarted = false;
	m_tmTakeOverTimer.stop();
}

void MIhmTCLInterfaces::onTakeOverTimeout()
{
	if(m_bTakeOverStarted)
	{
		MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtTakeOverTimeout); 
		m_pAuthAutomate->processEvent(pEvent);
	}
}




bool MIhmTCLInterfaces::isDskUserInControl()
{
	bool bRet = false;

	if(m_pUserInControl!=NULL)
	{
		if(m_pUserInControl->m_bIsDesktopUser)
			bRet = true;
	}

	return bRet;
}


bool MIhmTCLInterfaces::isWebUserInControl(QString sWebSessionId)
{
	bool bRet = false;

	if(m_pUserInControl!=NULL)
	{
		if(!m_pUserInControl->m_bIsDesktopUser && m_pUserInControl->m_sWebSessionID == sWebSessionId)
			bRet = true;
	}

	return bRet;
}




void MIhmTCLInterfaces::setUserInControl(MIhmSessionUserData * pData)
{
	m_pUserInControl = pData;
	
	updateInControlStatus();
}


void MIhmTCLInterfaces::updateInControlStatus()
{
	if(m_pUserInControl!=NULL)
	{ 
		if(m_pUserInControl->m_bIsDesktopUser)
		{
			m_pDesktopIntf->setInControl(true);
			emit webUserInControl("");
		}
		else
		{	
 			emit webUserInControl(m_pUserInControl->m_sWebSessionID);
			m_pDesktopIntf->setInControl(false);
		}
	}
	else
	{
		m_pDesktopIntf->setInControl(false);
		emit webUserInControl("");

	}
}

void MIhmTCLInterfaces::setUserTakingControl(MIhmSessionUserData * pUserTakingCtrl)
{
	m_pUserTakingCtrl = pUserTakingCtrl;
}


void MIhmTCLInterfaces::showConnectingInProgress()
{
	if(m_pUserTakingCtrl!=NULL)
	{	
		//show the message to the one that is taking control
		if(m_pUserTakingCtrl->m_bIsDesktopUser)
		{
 			m_pDesktopIntf->showConnectingMsg("LABEL_MESSAGE_CONNECTING", "LABEL_TITLE_CONNECTING");
		}
		else
		{
			emit showConnectingMsg(m_pUserTakingCtrl->m_sWebSessionID, "LABEL_MESSAGE_CONNECTING", "LABEL_TITLE_CONNECTING");
		}
	}
	else
	{
			TRACE_W("MIhmTCLInterfaces::showConnectingInProgress: User Taking control is NULL!");
			//send event to the automate so it can move to correct state
			MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtNewClientTakeOverCancel); 
			m_pAuthAutomate->processEvent(pEvent);
	}
}


void MIhmTCLInterfaces::hideConnectingInProgress()
{
	if(m_pUserTakingCtrl!=NULL)
	{
		if(m_pUserTakingCtrl->m_bIsDesktopUser)
		{
 			m_pDesktopIntf->hideConnectingMsg();
		}
		else
		{
			emit hideConnectingMsg(m_pUserTakingCtrl->m_sWebSessionID);
		}
	}
}




void MIhmTCLInterfaces::setTakeOverInProgress()
{
	if(m_pUserTakingCtrl!=NULL)
	{	

		if(m_pUserInControl!=NULL)
		{ 
			//show the question to the one in controle
			if(m_pUserInControl->m_bIsDesktopUser)
			{	//in case when user in control is desktop user 
				// send the takeover question dialog to the desktop
				m_pDesktopIntf->initTakeOver(true, m_pUserTakingCtrl->m_sUserName);
			}
			else
			{
				emit initWebTakeOver(true, m_pUserInControl->m_sWebSessionID, m_pUserTakingCtrl->m_sUserName);
			}

			//show the message to the one that is taking control
			if(m_pUserTakingCtrl->m_bIsDesktopUser)
			{
				m_pDesktopIntf->initTakeOver(false);
			}
			else
			{
				emit initWebTakeOver(false, m_pUserTakingCtrl->m_sWebSessionID, "");
			}
		}
		else //if one of the users were NULL cancel the TakeOver
		{
			//send event to the automate so it can move to correct state
			MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtTakeOverAccept); 
			m_pAuthAutomate->processEvent(pEvent);

		}
	}
	else
	{
			TRACE_W("MIhmTCLInterfaces::setTakeOverInProgress: User Taking control is NULL!");
			//send event to the automate so it can move to correct state
			MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtNewClientTakeOverCancel); 
			m_pAuthAutomate->processEvent(pEvent);
	}
}

void MIhmTCLInterfaces::cancelTakeOverInProgress()
{
	if(m_pUserInControl!=NULL) 
	{ 
		if(m_pUserInControl->m_bIsDesktopUser)
		{
			m_pDesktopIntf->cancelTakeOver();
		}
		else
		{
			emit cancelWebTakeOver(m_pUserInControl->m_sWebSessionID);
		}
	}
	
	
	if(m_pUserTakingCtrl!=NULL)
	{
		if(m_pUserTakingCtrl->m_bIsDesktopUser)
		{
			m_pDesktopIntf->cancelTakeOver();
		}
		else
		{
			emit cancelWebTakeOver(m_pUserTakingCtrl->m_sWebSessionID);
		}
	}
}


//---------------------------------------------------------
// DESKTOP Take control slots
//---------------------------------------------------------

void MIhmTCLInterfaces::informRequestingUserFailure(QString sMsg)
{
	MIhmSessionUserData *pData = NULL;
	pData = getUserTakingControl();

	if(pData!=NULL)
	{
		if(pData->m_bIsDesktopUser == true)
		{
			dskDisplayClientInfoMsg(sMsg,IHM_LABEL_TAKE_CTRL_FAILURE_DLG_TITLE);
		}
		else
		{
			emit showClientInfoMsg(pData->m_sWebSessionID, sMsg, IHM_LABEL_TAKE_CTRL_FAILURE_DLG_TITLE);
		}
	}		
}


void MIhmTCLInterfaces::informRequestingUserRejected()
{
	MIhmSessionUserData *pData = NULL;
	pData = getUserTakingControl();

	if(pData!=NULL)
	{
		MIhmLanguages *pLang = MIhmConfigGeneral::getCfg()->getLanguages();

		if(pData->m_bIsDesktopUser)
		{
			dskDisplayClientInfoMsg(IHM_LABEL_TAKE_CTRL_REJECTED_BY_USER_MESSAGE,IHM_LABEL_TAKE_CTRL_FAILURE_DLG_TITLE);
		}
		else
		{
			emit showClientInfoMsg(pData->m_sWebSessionID, IHM_LABEL_TAKE_CTRL_REJECTED_BY_USER_MESSAGE,IHM_LABEL_TAKE_CTRL_FAILURE_DLG_TITLE);
		}
	}		
}


void MIhmTCLInterfaces::onDskTakeOverReject()
{
	TRACE_D("MIhmTCLInterfaces::onDskTakeOverReject: ");

	MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtTakeOverReject); 
	m_pAuthAutomate->processEvent(pEvent);
}

void MIhmTCLInterfaces::onDskTakeOverCancel()
{
	TRACE_D("MIhmTCLInterfaces::onDskTakeOverCancel: ");

	MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtNewClientTakeOverCancel); 
	m_pAuthAutomate->processEvent(pEvent);
}


void MIhmTCLInterfaces::onDskTakeOverAccept()
{
	TRACE_D("MIhmTCLInterfaces::onDskTakeOverAccept: ");

	MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtTakeOverAccept); 
	m_pAuthAutomate->processEvent(pEvent);

}

void MIhmTCLInterfaces::onDskIdentificationReq(QString sName, QString sSecCode, bool bLDAPOffline)
{
	TRACE_D(QString("MIhmTCLInterfaces::onDskIdentificationReq:user:%1, isPwd:%2").
						arg(sName).
						arg(!bLDAPOffline));

	//Add the request to the take control queue 
	//and auth message no matter what is the automate state
	MIhmSessionUserData *pNewUser = new MIhmSessionUserData();
	pNewUser->m_bIsDesktopUser = true;
	pNewUser->m_sUserId = sName;
	pNewUser->m_sSecCode = sSecCode;
	pNewUser->m_bIsSecCodePwd = !bLDAPOffline;
	
	//append request to the queue
	m_lstAuthRequestQueue.append(pNewUser);

		MIhmMsgAuthReq *pMsg = new MIhmMsgAuthReq();
		pMsg->m_usrData = *pNewUser;
		sendMessageToAni(pMsg);
}


void MIhmTCLInterfaces::onDskReturnCtrlReq(QString sDskInternalSessionID, int iReason)
{
	TRACE_D(QString("MIhmTCLInterfaces::onDskReturnCtrlReq:%1").
						arg(sDskInternalSessionID));

	if((m_pUserInControl!=NULL) && (m_pUserInControl->m_bIsDesktopUser))
	{
		MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtReturnCtrlReq); 
		pEvent->setReason(iReason);
		m_pAuthAutomate->processEvent(pEvent);
	}
	else
	{
		m_pDesktopIntf->setUserIdentified(false,"");
	}
}

void MIhmTCLInterfaces::onDskAuthDlgCanceled(int iReason)
{
	MIhmMsgAuthDlgCanceled *pMsg = new MIhmMsgAuthDlgCanceled();
	pMsg->m_usrData.m_bIsDesktopUser = true;
	pMsg->reason = (enum_ihm_auth_ret_ctrl_reason)iReason;
	sendMessageToAni(pMsg);
}

void MIhmTCLInterfaces::onDskEndIdentificationReq(QString sDskInternalSessionID, int iReason)
{
	TRACE_D(QString("MIhmTCLInterfaces::onDskEndIdentificationReq:%1 Reason:%2").
						arg(sDskInternalSessionID).arg(iReason));

	if(m_pUserInControl!=NULL)
		if(m_pUserInControl->m_sInternalSessionID == sDskInternalSessionID)
		{
			onDskReturnCtrlReq(sDskInternalSessionID, iReason);	
		}
	
	if(m_pUserTakingCtrl!=NULL)
		if(m_pUserTakingCtrl->m_sInternalSessionID == sDskInternalSessionID)
	{
		MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtNewClientTakeOverCancel); 
		pEvent->setReason(iReason);
		m_pAuthAutomate->processEvent(pEvent);
	}
	
	// now process the un identification	
	//find the user request in the list and delete it
	MIhmSessionUserData * pReq = MIhmSessionUserData::findByIntSessionID(&m_lstIdentifiedUsers, sDskInternalSessionID);

	if(pReq!=NULL)
	{
		MIhmMsgAuthLogOffReq *pMsg = new MIhmMsgAuthLogOffReq();
		pMsg->m_usrData = *pReq;
		pMsg->reason = (enum_ihm_auth_ret_ctrl_reason)iReason;
		sendMessageToAni(pMsg);

		removeIHMSession(pReq);
	}	
		
	m_pDesktopIntf->setUserIdentified(false,"");

}

void MIhmTCLInterfaces::onDskTakeCtrlReq(QString sDskInternalSessionID)
{
	TRACE_D("MIhmTCLInterfaces::onDskTakeCtrlReq:...");

	//initiate take control for the desktop user
	if(m_pDesktopIntf->isIdentified())
	{
		QString sDskInternalSessionID = m_pDesktopIntf->getDskIdentSessionID();
		//find the user request in the list
		MIhmSessionUserData * pReq = MIhmSessionUserData::findByIntSessionID(&m_lstIdentifiedUsers, sDskInternalSessionID);
	
		if(pReq!=NULL)
		{
			MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtInitTakeOver); 
			pEvent->setData(pReq);
			m_pAuthAutomate->processEvent(pEvent);
			
		}
		else
		{
			TRACE_W("MIhmTCLInterfaces::onDskTakeCtrlReq: Error desktop user not found");
				
		}

	}	
}

//---------------------------------------------------------
// END DESKTOP Take control slots
//---------------------------------------------------------

//---------------------------------------------------------
// WEB Take control slots
//---------------------------------------------------------

void MIhmTCLInterfaces::onWebAuthorisationReq(QString sUserId, QString sPwd, bool bTakeControl, QString sWebSessionID)
{
	TRACE_D("MIhmTCLInterfaces::onWebAuthorisationReq: ...");
	//Add the request to the take control queue no matter what is the automate state
	MIhmSessionUserData *pNewUser = new MIhmSessionUserData();
	pNewUser->m_bIsDesktopUser = false;
	pNewUser->m_sUserId = sUserId;
	pNewUser->m_sSecCode = sPwd;
	pNewUser->m_bIsSecCodePwd= true;
	pNewUser->m_sWebSessionID = sWebSessionID;

	TRACE_D(QString("MIhmTCLInterfaces::onWebAuthorisationReq: %1:%2 take control:%3 sWebSessionID:%4;\n internal sessionId:%5").
						arg(sUserId).
						arg(sPwd).
						arg(bTakeControl).
						arg(sWebSessionID).
						arg(pNewUser->m_sInternalSessionID));
	
	//append request to the queue
	m_lstAuthRequestQueue.append(pNewUser);

	MIhmMsgAuthReq *pMsg = new MIhmMsgAuthReq();
	pMsg->m_usrData = *pNewUser;
	sendMessageToAni(pMsg);
}

void MIhmTCLInterfaces::removeIHMSession(MIhmSessionUserData * pReq)
{
	TRACE_D("MIhmTCLInterfaces::removeIHMSession: ...");

	//TODO check that the AuthAtm is in the correct state

	if(getUserTakingControl()==pReq)
		setUserTakingControl(NULL);
	
	if(m_pUserInControl)
		if(m_pUserInControl == pReq)
		{
			MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtForceReturnCtrl); 
			m_pAuthAutomate->processEvent(pEvent);

			MIhmMsgAuthLogOffReq *pMsg2 = new MIhmMsgAuthLogOffReq();
			pMsg2->m_usrData = *(m_pUserInControl);
			pMsg2->reason = enuRETURN_CTRL_FORCED;
			sendMessageToAni(pMsg2);
		}

	MIhmSessionUserData::removeIhmSession(&m_lstIdentifiedUsers,  pReq, true);

}


void MIhmTCLInterfaces::onWebEndIdentificationReq(QString sWebSessionId, int iReason)
{
	TRACE_D(QString("MIhmTCLInterfaces::onWebEndIdentificationReq:Websession:%1;Reason:%2").
						arg(sWebSessionId).arg(iReason));

	//find the user request in the list and delete it
	MIhmSessionUserData * pReq = MIhmSessionUserData::findByWebSessionID(&m_lstIdentifiedUsers, sWebSessionId);
	if(pReq!=NULL)
	{
		//Assure that the user that is gone is not waiting for some other timer in auth automate
		if(m_pUserTakingCtrl!=NULL)
			if(m_pUserTakingCtrl->m_sInternalSessionID == pReq->m_sInternalSessionID)
		{
			MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtNewClientTakeOverCancel); 
			pEvent->setReason(iReason);
			m_pAuthAutomate->processEvent(pEvent);
		}

		if(m_pUserInControl!=NULL)
			if(m_pUserInControl->m_sInternalSessionID == pReq->m_sInternalSessionID)
		{
			MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtForceReturnCtrl); 
			pEvent->setReason(iReason);
			m_pAuthAutomate->processEvent(pEvent);
		}

		MIhmMsgAuthLogOffReq *pMsg = new MIhmMsgAuthLogOffReq();
		pMsg->m_usrData = *pReq;
		pMsg->reason = (enum_ihm_auth_ret_ctrl_reason)iReason;
		sendMessageToAni(pMsg);

		//todo emit signal to web interface to remove web session
		emit forceUnIdentification(sWebSessionId);

		removeIHMSession(pReq);
	}
}

void MIhmTCLInterfaces::onWebTakeControlReq(QString sWebSessionId)
{
	TRACE_D("MIhmTCLInterfaces::onWebTakeControlReq: ");

	MIhmSessionUserData * pReq = MIhmSessionUserData::findByWebSessionID(&m_lstIdentifiedUsers, sWebSessionId);

	if(pReq!=NULL)
	{
		MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtInitTakeOver); 
		pEvent->setData(pReq);
		m_pAuthAutomate->processEvent(pEvent);
	}
	else
	{
		TRACE_W("MIhmTCLInterfaces::onWebTakeControlReq: Error web user not found");
			
	}


}


void MIhmTCLInterfaces::onWebReturnControlReq(QString sWebSessionId, int iReason)
{
	TRACE_D(QString("MIhmTCLInterfaces::onWebReturnControlReq:Websession:%1;Reason:%2").
						arg(sWebSessionId).arg(iReason));

	if(m_pUserInControl!=NULL)
		if(!m_pUserInControl->m_bIsDesktopUser && m_pUserInControl->m_sWebSessionID == sWebSessionId)
		{
			MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtReturnCtrlReq); 
			pEvent->setReason(iReason);
			m_pAuthAutomate->processEvent(pEvent);
		}

}


void MIhmTCLInterfaces::onWebTakeOverReject(QString sWebSessionId)
{
	TRACE_D("MIhmTCLInterfaces::onWebTakeOverReject: ");


	MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtTakeOverReject); 
	m_pAuthAutomate->processEvent(pEvent);
}


void MIhmTCLInterfaces::onWebTakeOverCancel(QString)
{
	TRACE_D("MIhmTCLInterfaces::onWebTakeOverCancel: ");

	MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtNewClientTakeOverCancel); 
	m_pAuthAutomate->processEvent(pEvent);

}

void MIhmTCLInterfaces::onWebTakeOverAccept(QString)
{
	TRACE_D("MIhmTCLInterfaces::onWebTakeOverAccept: ");

	MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtTakeOverAccept); 
	m_pAuthAutomate->processEvent(pEvent);

}


//---------------------------------------------------------
// END WEB Take control slots
//---------------------------------------------------------

//-----------------------------------------------------------
// Processing messages from/to ANI 
//-----------------------------------------------------------

void MIhmTCLInterfaces::sendMessageToAni(MIhmMsg * pMsg)
{
	m_pMainLogic->sendMessageToAni(pMsg);
}

void MIhmTCLInterfaces::processMessageFromAni(MIhmMsg* pMsg)
{
	TRACE_D("MIhmTCLInterfaces::processMessageFromAni: ...");
	
	if(pMsg->getType()==MIhmMsg::enuIhmMsgAuthRsp)
	{
		MIhmMsgAuthRsp* pAuthRsp = (MIhmMsgAuthRsp*)pMsg;
		processAuthorisationRsp(pAuthRsp);
	}
	else if(pMsg->getType()==MIhmMsg::enuIhmMsgTakeCtrlRsp)
	{
		MIhmMsgTakeCtrlRsp* pAuthRsp = (MIhmMsgTakeCtrlRsp*)pMsg;
		processTakeControlRsp(pAuthRsp);
	}
	else if(pMsg->getType()==MIhmMsg::enuIhmMsgReturnCtrlRsp)
	{
		MIhmMsgReturnCtrlRsp* pAuthRsp = (MIhmMsgReturnCtrlRsp*)pMsg;
		processReturnControlRsp(pAuthRsp);
	}
	else if(pMsg->getType()==MIhmMsg::enuIhmMsgOpenLinkReq)
	{
		MIhmMsgOpenLinkReq* pReq = (MIhmMsgOpenLinkReq*)pMsg;
		processOpenLinkReq(pReq);
	}
	else if(pMsg->getType()==MIhmMsg::enuIhmMsgOpenAboutDlgReq)
	{
		MIhmMsgOpenAboutDlgReq* pReq = (MIhmMsgOpenAboutDlgReq*)pMsg;
		processOpenAboutDlgReq(pReq);
	}
	else if(pMsg->getType()==MIhmMsg::enuIhmMsgCloseAboutDlgReq)
	{
		MIhmMsgCloseAboutDlgReq* pReq = (MIhmMsgCloseAboutDlgReq*)pMsg;
		processCloseAboutDlgReq(pReq);
	}
	else if(pMsg->getType()==MIhmMsg::enuIhmMsgOpenLoginDlgReq)
	{
		MIhmMsgOpenLoginDlgReq* pReq = (MIhmMsgOpenLoginDlgReq*)pMsg;
		processOpenLoginDlgReq(pReq);
	}
	else if(pMsg->getType()==MIhmMsg::enuIhmMsgVideo)
	{
		processVideoMsgReq((MIhmMsgVideo*)pMsg);
	}
	else if(pMsg->getType()==MIhmMsg::enuIhmMsgReloadDynConfigReq)
	{
		processReloadDynConfig();
	}
	else
	{
		TRACE_W(QString("MIhmTCLInterfaces::processMessageFromAni: Unknown message type %1!").
						arg(pMsg->getType()));
	}

}

//------------------------------------------------------------------
// Processing video request messages
//------------------------------------------------------------------
void MIhmTCLInterfaces::processVideoMsgReq(MIhmMsgVideo* pReq)
{
	//Depending on type of target object process the commands here (for the restream video objects)
	// or forward the command to be processed at the desktop (CAM video objects)
	if(pReq->getTargetVirtObject() == MIhmVirtualObject::enuIhmVirtRestreamVideoView1 ||
		pReq->getTargetVirtObject() == MIhmVirtualObject::enuIhmVirtRestreamVideoView2	)
	{
		processRVideoMsg(pReq);
	}

}


bool MIhmTCLInterfaces::processRVideoMsg(MIhmMsgVideo* pMsg)
{
	bool bRetVal = false;

	//MRVideoControlObject * pRVideoControlObject = getRVideoControlObject(pMsg->getTargetVirtObject()); 
	//
	//if(pRVideoControlObject!=NULL)
	//{

	//	switch(pMsg->getVideoMsgType())
	//	{
	//		case MIhmMsgVideo::enuIhmMsgVideoSelectSourceReq:
	//			{
	//				TRACE_D(QString("MIhmTCLInterfaces::processRVideoMsg: Message type enuIhmMsgVideoSelectSourceReq, Source:%1 !").
	//												arg(((MIhmMsgVideoSelectSrc*)pMsg)->m_iNewSource));

	//				pRVideoControlObject->activate(((MIhmMsgVideoSelectSrc*)pMsg)->m_iNewSource);

	//				MIhmMsgRVideoUpdateSrc * pNewTclMsg = new MIhmMsgRVideoUpdateSrc(pMsg->getTargetVirtObject());
	//				pNewTclMsg->m_sStreamSourceUrlLocal = pRVideoControlObject->getActivatedStreamSourceUrlLocal();
	//				pNewTclMsg->m_sStreamSourceUrlRemote = pRVideoControlObject->getActivatedStreamSourceUrlRemote();
	//				pNewTclMsg->m_sBaseUrl = pRVideoControlObject->getActivatedStreamBaseUrl();
	//				sendMessageToAni(pNewTclMsg);
	//				bRetVal = true;
	//			}
	//			break;
	//		case MIhmMsgVideo::enuIhmMsgVideoOnReq:
	//			{
	//				TRACE_D(QString("MIhmTCLInterfaces::processRVideoMsg: Message type enuIhmMsgVideoOnReq!"));

	//				pRVideoControlObject->reactivate();

	//				MIhmMsgRVideoUpdateOnOff * pNewTclMsg = new MIhmMsgRVideoUpdateOnOff(pMsg->getTargetVirtObject());
	//				pNewTclMsg->m_bVideoOn = true;
	//				sendMessageToAni(pNewTclMsg);
	//				bRetVal = true;
	//			}
	//			break;
	//		case MIhmMsgVideo::enuIhmMsgVideoOffReq:
	//			{
	//				TRACE_D(QString("MIhmTCLInterfaces::processRVideoMsg: Message type enuIhmMsgVideoOffReq!"));
	//				pRVideoControlObject->deactivate();

	//				MIhmMsgRVideoUpdateOnOff * pNewTclMsg = new MIhmMsgRVideoUpdateOnOff(pMsg->getTargetVirtObject());
	//				pNewTclMsg->m_bVideoOn = false;
	//				sendMessageToAni(pNewTclMsg);
	//				bRetVal = true;
	//			}
	//			break;
	//		case MIhmMsgVideo::enuIhmMsgVideoFreezeReq:
	//				pRVideoControlObject->grabb();
	//				bRetVal = true;
	//			break;
	//		case MIhmMsgVideo::enuIhmMsgVideoUnfreezeReq:
	//				pRVideoControlObject->reactivate();
	//				bRetVal = true;
	//			break;
	//		case MIhmMsgVideo::enuIhmMsgVideoSaveReq:
	//			{	
	//				QString sTargetSavePath = MIhmConfigGeneral::getCfg()->createDefaultVideoFullPath(((MIhmMsgVideoSave*)pMsg)->m_sTargetFileName);

	//				pRVideoControlObject->save(sTargetSavePath, 
	//										   ((MIhmMsgVideoSave*)pMsg)->m_bSaveJpg);
	//				bRetVal = true;
	//			}
	//			break;

	//		case MIhmMsgVideo::enuIhmMsgVideoZoomReq:
	//			break;
	//		default:
	//			TRACE_W(QString("MIhmTCLInterfaces::processRVideoMsg: Unknown video message type %1!").
	//							arg(pMsg->getVideoMsgType()));
	//			break;
	//	}
	//}
	//else
	//{
	//		TRACE_W(QString("MIhmTCLInterfaces::processRVideoMsg: Unable to find RVideoControlObject for %1!").
	//						arg(pMsg->getTargetVirtObject()));
	//	
	//}

	return bRetVal;
}


void MIhmTCLInterfaces::onRVideoResponseMessage(MRVideoMsg* pRVMsg, int eSourceVirtObjet)
{
	MIhmMsgVideo * pMsg = NULL;

	switch(pRVMsg->getType())
	{
	case MRVideoMsg::enuRVideoFreezeRsp:
		{
			pMsg = new MIhmMsgVideoFreezeRsp(eSourceVirtObjet);
			((MIhmMsgVideoFreezeRsp*)pMsg)->m_iResult = ((MRVideoMsgVideoFreezeRsp*)pRVMsg)->m_bOK?1:0;
		}
		break;
	case MRVideoMsg::enuRVideoSaveRsp:
		{
			pMsg = new MIhmMsgVideoSaveRsp(eSourceVirtObjet);
			((MIhmMsgVideoSaveRsp*)pMsg)->m_iResult = ((MRVideoMsgVideoSaveRsp*)pRVMsg)->m_bOK?1:0;
		}
	    break;
	default:
		TRACE_W(QString("MRVideoControlObject::onOutputRVideoMessage: Unknown MRVideoMsg type %1!").
			arg(pRVMsg->getType()));
	    break;
	}

	if(pMsg!=NULL)
		m_pMainLogic->sendMessageToAni(pMsg);

	delete pRVMsg;
}




//void MIhmTCLInterfaces::InitRVideoControlObjects()
//{
	//MRVideoControlObject * pNewObj;
	//
	//QString sRegVideoSrcs = MIhmConfigGeneral::getCfg()->getModuleConfigKey();
	//sRegVideoSrcs.append("\\RestreamVideoSources");


	//for(int iVirtId = MIhmVirtualObject::enuIhmVirtRestreamVideoView1; iVirtId<=MIhmVirtualObject::enuIhmVirtRestreamVideoView2; iVirtId++)
	//{
	//	pNewObj = new MRVideoControlObject(iVirtId, MIhmVirtualObject::getNameForId((enum MIhmVirtualObject::enumVirtualObjectId)iVirtId));
	//	
	//	if(pNewObj->init(sRegVideoSrcs))	
	//	{
	//		connect(pNewObj, SIGNAL(newOutputRVideoMessage(MRVideoMsg*, int)), this, SLOT(onRVideoResponseMessage(MRVideoMsg*, int)));
	//		m_lstRVideoCtrl.append(pNewObj);
	//	}
	//	else
	//		delete pNewObj;
	//}
//}

//MRVideoControlObject* MIhmTCLInterfaces::getRVideoControlObject(int eTargetVirtObjet)
//{
//	MRVideoControlObject* pRetVal = NULL;
//
//	for(int i=0; i< m_lstRVideoCtrl.size();i++)
//	{
//		if (m_lstRVideoCtrl.at(i)->getVirtObject() == eTargetVirtObjet)
//		{
//			pRetVal = m_lstRVideoCtrl.at(i);
//			break;
//		}
//	}
//	
//	return pRetVal;
//	
//}


//------------------------------------------------------------------
//------------------------------------------------------------------

void MIhmTCLInterfaces::processOpenLinkReq(MIhmMsgOpenLinkReq * pReq)
{
	TRACE_D(QString("MIhmTCLInterfaces::processOpenLinkReq: int:%1 url[%2]").
						arg(pReq->m_iTargetWin).
						arg(pReq->m_sUrl));

	if(m_pUserInControl!=NULL)
	{
		if(m_pUserInControl->m_bIsDesktopUser)
		{
			//process desktop open link req
			m_pDesktopIntf->processOpenLink(pReq->m_iTargetWin, pReq->m_sUrl, pReq->m_bVisible);

		}
		else
		{ //process web open link req
			MIhmMsgOpenLinkReq *pNew = new MIhmMsgOpenLinkReq();
			*pNew = *pReq;
			emit openLinkReq(pNew);
		}
	}
}






void MIhmTCLInterfaces::processOpenAboutDlgReq(MIhmMsgOpenAboutDlgReq * pReq)
{
	TRACE_D(QString("MIhmTCLInterfaces::processOpenAboutDlgReq: %1").
		arg(pReq->m_sAboutDlgCnf));
	
	m_pDesktopIntf->processOpenAboutDlgReq(pReq->m_sAboutDlgCnf);
}

void MIhmTCLInterfaces::processCloseAboutDlgReq(MIhmMsgCloseAboutDlgReq * pReq)
{
	TRACE_D(QString("MIhmTCLInterfaces::processCloseAboutDlgReq... "));
	
	m_pDesktopIntf->processCloseAboutDlgReq();
}


void MIhmTCLInterfaces::processOpenLoginDlgReq(MIhmMsgOpenLoginDlgReq * pReq)
{
	TRACE_D(QString("MIhmTCLInterfaces::processOpenLoginDlgReq: sLoginId[%1]").
						arg(pReq->m_sLoginId));

	//process desktop open login dlg req
	m_pDesktopIntf->processOpenLoginDlgReq(pReq->m_sLoginId, pReq->m_bManualLogin, pReq->m_bLDAPOffline, pReq->m_bShow);

}



void MIhmTCLInterfaces::processAuthorisationRsp(MIhmMsgAuthRsp * pAuthRsp)
{
	TRACE_D(QString("MIhmTCLInterfaces::processAuthorisationRsp: Internal Session:%1 Result: %2").
						arg(pAuthRsp->m_usrData.m_sInternalSessionID).
						arg((pAuthRsp->m_iAuthRsp == enuAUTH_OK)?"true":"false"));

					
	if(pAuthRsp->m_iAuthRsp == enuAUTH_OK)
	{
		//find the user request in the list
		MIhmSessionUserData * pReq = MIhmSessionUserData::findByIntSessionID(&m_lstAuthRequestQueue, pAuthRsp->m_usrData.m_sInternalSessionID);
	
		if(pReq!=NULL)
		{
			pReq->m_sUserName = pAuthRsp->m_usrData.m_sUserName;
			
			//remove the auth request from the list, but do not delete 
			MIhmSessionUserData::removeIhmSession(&m_lstAuthRequestQueue, pReq, false);
			m_lstIdentifiedUsers.append(pReq);


			if(!pReq->m_bIsDesktopUser)	
			{
				webSendWebAuthRsp(pReq->m_sWebSessionID,true,"");
			}
			else
			{
				m_pDesktopIntf->setUserIdentified(true, pReq->m_sInternalSessionID);	
			}
			
		}
		else if(pAuthRsp->m_usrData.m_bIsDesktopUser)
		{
			pReq = MIhmSessionUserData::findByIntSessionID(&m_lstIdentifiedUsers, pAuthRsp->m_usrData.m_sInternalSessionID);
			
			if(pReq == NULL)
			{
				TRACE_D(QString("MIhmTCLInterfaces::processAuthorisationRsp: Authorisation response for the name:[%1] sesion_id:[%2] not found! Creating new entry for desktop.").
					arg(pAuthRsp->m_usrData.m_sUserId).
					arg(pAuthRsp->m_usrData.m_sInternalSessionID));

				pReq = new MIhmSessionUserData();
				pReq->m_bIsDesktopUser = true;
				pReq->m_sUserId = pAuthRsp->m_usrData.m_sUserId;
				pReq->m_sInternalSessionID = pAuthRsp->m_usrData.m_sInternalSessionID;
				m_lstIdentifiedUsers.append(pReq);
			}

			m_pDesktopIntf->setUserIdentified(true, pReq->m_sInternalSessionID);	
		}
		else
		{
			TRACE_W(QString("MIhmTCLInterfaces::processAuthorisationRsp: Authorisation request for the name:[%1] sesion_id:[%2] not found! Forced authorization is not supported for web.").
				arg(pAuthRsp->m_usrData.m_sUserId).
				arg(pAuthRsp->m_usrData.m_sInternalSessionID));

			return;
		}

		Q_ASSERT(pReq!=NULL);

		//if no one is in control auto start the taking over 
		if(m_pUserInControl==NULL && m_pUserTakingCtrl == NULL)
		{
			MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtInitTakeOver); 
			pEvent->setData(pReq);
			m_pAuthAutomate->processEvent(pEvent);
		}

	}
	else
	{
		//find the user request in the request list
		MIhmSessionUserData * pReq = MIhmSessionUserData::findByIntSessionID(&m_lstAuthRequestQueue, pAuthRsp->m_usrData.m_sInternalSessionID);
		
		if(pReq!=NULL)
		{
			QString sErrMsg = pAuthRsp->m_sError;
			if(sErrMsg=="")
					sErrMsg = IHM_LABEL_MSG_INVALID_USER_OR_PWD;
				
			if(pReq->m_bIsDesktopUser)
			{
				dskDisplayClientInfoMsg(sErrMsg, IHM_LABEL_TAKE_CTRL_FAILURE_DLG_TITLE);
			}
			else 
			{
				webSendWebAuthRsp(pReq->m_sWebSessionID,false,sErrMsg);
			}

			MIhmSessionUserData::removeIhmSession(&m_lstAuthRequestQueue, pReq, true);
		}
		else //if not found in the request list test if it is one of already identified
		{
			MIhmSessionUserData * pReq = MIhmSessionUserData::findByIntSessionID(&m_lstIdentifiedUsers, pAuthRsp->m_usrData.m_sInternalSessionID);
			
			if(pReq!=NULL)
			{ //the user is identified ...

				TRACE_D(QString("MIhmTCLInterfaces::processAuthorisationRsp: End identification forced by main app!"));
				
				//check if it is desktop or web
				if(pReq->m_bIsDesktopUser)
				{
					onDskEndIdentificationReq(pReq->m_sInternalSessionID, enuRETURN_CTRL_FORCED);
				}
				else 
				{
					onWebEndIdentificationReq(pReq->m_sWebSessionID, enuRETURN_CTRL_FORCED);
				}
				
			}
		}
	}

}

void MIhmTCLInterfaces::webSendWebAuthRsp(QString sWebSessionID, bool bAuthorized, QString sMsg)
{
	TRACE_D("MIhmTCLInterfaces::webSendWebAuthRsp:...");
	emit authorizationRsp(sWebSessionID,bAuthorized, sMsg);
}

void MIhmTCLInterfaces::dskDisplayClientInfoMsg(QString sMsg, QString sMsgTitle)
{
	m_pDesktopIntf->showClientInfoMsg(sMsg, sMsgTitle);
}


void MIhmTCLInterfaces::processTakeControlRsp(MIhmMsgTakeCtrlRsp * pAuthRsp)
{
	TRACE_D("MIhmTCLInterfaces::processTakeControlRsp:...");

	// if response is AUTH_NOK just check that it is not for the one that is in control
	if(pAuthRsp->m_iAuthRsp == enuAUTH_NOK)
	{
		if(m_pUserInControl != NULL)
		{
			if(m_pUserInControl->m_sInternalSessionID == pAuthRsp->m_usrData.m_sInternalSessionID)
				{
					MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtForceReturnCtrl); 
					m_pAuthAutomate->processEvent(pEvent);
				}
		}
		else if(m_pUserTakingCtrl != NULL && m_pUserTakingCtrl->m_sInternalSessionID == pAuthRsp->m_usrData.m_sInternalSessionID)
		{
			m_pUserTakingCtrl->m_sUserName = pAuthRsp->m_usrData.m_sUserName;
			MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtTakeCtrlRspNOK); 
			pEvent->setError(pAuthRsp->m_sError);
			m_pAuthAutomate->processEvent(pEvent);
		}
		else
		{
			TRACE_W(QString("MIhmTCLInterfaces::processTakeControlRsp:Received TakeCtrl response NOK for user that did not request it and is not in control![%1] ").
							arg(pAuthRsp->m_usrData.m_sInternalSessionID));

		}
		
	}
	else
	{

		// if response is AUTH_OK means that we should find the one with the session ID and put it in control state
		// if we fail to find the user with the session ID we should respond with return control message so
		// the main application knows that the this one is not in control
		if(m_pUserTakingCtrl != NULL && m_pUserTakingCtrl->m_sInternalSessionID == pAuthRsp->m_usrData.m_sInternalSessionID)
		{
			//if the response is for the user that is trying to take control  
			//(it is normal working)
			m_pUserTakingCtrl->m_sUserName = pAuthRsp->m_usrData.m_sUserName;
			MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtTakeCtrlRspOK); 
			m_pAuthAutomate->processEvent(pEvent);
		}
		else
		{	
			if(pAuthRsp->m_usrData.m_bIsDesktopUser)
			{
				MIhmSessionUserData *pReq = MIhmSessionUserData::findByIntSessionID(&m_lstIdentifiedUsers, pAuthRsp->m_usrData.m_sInternalSessionID);
				
				if(pReq!=NULL)
				{
					m_pUserTakingCtrl = pReq;
					MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::enuEvtTakeCtrlRspOK); 
					m_pAuthAutomate->processEvent(pEvent);
				}
				else
				{
					//return
					MIhmMsgReturnCtrlReq *pMsg = new MIhmMsgReturnCtrlReq();
					pMsg->m_usrData = pAuthRsp->m_usrData;	
					pMsg->m_iReason = (int)enuRETURN_CTRL_IDLE_TIMEOUT;
					sendMessageToAni(pMsg);
				}
			}
			else
			{
				//return
				MIhmMsgReturnCtrlReq *pMsg = new MIhmMsgReturnCtrlReq();
				pMsg->m_usrData = pAuthRsp->m_usrData;	
				pMsg->m_iReason = (int)enuRETURN_CTRL_IDLE_TIMEOUT;
				sendMessageToAni(pMsg);
			}
		}	
	}



}


void MIhmTCLInterfaces::processReturnControlRsp(MIhmMsgReturnCtrlRsp* pAuthRsp)
{
	TRACE_D("MIhmTCLInterfaces::processReturnControlRsp: NO NEED TO IMPLEMENT");
}


void MIhmTCLInterfaces::processReloadDynConfig()
{
	TRACE_D("MIhmTCLInterfaces::processReloadDynConfig: ...");
	
	MIhmConfigGeneral::getCfg()->reloadConfigDynamic();


	//reset DSK idle timers 
	m_pDesktopIntf->resetIdleTimer();
	
	m_pDesktopIntf->processRefreshAllObjects();

	//and reset web timers
	emit reloadWebDynConfig();
}		


//-----------------------------------------------------------
// END Processing messages from ANI 
//-----------------------------------------------------------




//-----------------------------------------------------------
// DESKTOP input interface functions
//-----------------------------------------------------------
void MIhmTCLInterfaces::onDskMessageToAni(MIhmMsg * pMsg)
{
	m_pMainLogic->sendMessageToAni(pMsg);
}

void MIhmTCLInterfaces::onDskInputDialogResult(MIhmMsg * pMsg)
{
	//verify if in control ...
	if(isDskUserInControl() && !m_pAuthAutomate->isTakeOverInProgress())
		m_pMainLogic->sendMessageToAni(pMsg);
	else
		delete pMsg;
}

void MIhmTCLInterfaces::onDskAsyncInputRsp(MIhmMsg * pMsg)
{
	//verify if in control ...
	if(isDskUserInControl() && !m_pAuthAutomate->isTakeOverInProgress())
		m_pMainLogic->sendMessageToAni(pMsg);
	else
		delete pMsg;
}


void MIhmTCLInterfaces::onDskVideoCtlRsp(MIhmMsg * pMsg)
{
	//send it no matter who is in control
	m_pMainLogic->sendMessageToAni(pMsg);
}


void MIhmTCLInterfaces::onDskAction(QString sAction, QString sParams)
{
	TRACE_D(QString("MIhmTCLInterfaces::onDskAction:Action:%1, params:%2 ").
						arg(sAction).
						arg(sParams));

	if(isDskUserInControl())
	{
		//execute the action
		executeDskAction(sAction, sParams);
	}
	else
	{
		if(sAction == SPECIFIC_ACTION_BTN_IDENTIFICATION)
			execCommandReq(SPECIFIC_ACTION_BTN_IDENTIFICATION, "");
	}
}

void MIhmTCLInterfaces::onDskAboutDlgAction(QString sAction, QString sParams)
{
	TRACE_D(QString("MIhmTCLInterfaces::onDskAboutDlgAction:Action:%1, params:%2 ").
		arg(sAction).
		arg(sParams));
	
	//execute the action
	executeDskAction(sAction, sParams);
}


void MIhmTCLInterfaces::executeDskAction(QString sAction, QString sParams)
{
	
	MIhmConfigActions * pCfgActions = MIhmConfigGeneral::getCfg()->getActions();
	MIhmAction * pAction = pCfgActions->findAction(sAction);

	if(pAction!=NULL)
	{
		if(pAction->getType()==MIhmAction::enuOpenLink)
		{
			QString sLinkUrl =  pAction->getParamValue() + sParams;

			QDesktopServices::openUrl(QUrl(sLinkUrl, QUrl::TolerantMode));	
		}
		else if(pAction->getType()==MIhmAction::enuSendKey)
		{
			//emit both: staticaly defined part which is in the registry and 
			// the other dynamic part received in the additional parameters
			//This way we can use generic key down action 
			QString sKey = pAction->getParamValue();
			bool bOK;
			int iKeyCode = sKey.toInt(&bOK);
			if(bOK)
				execKeyPressed(iKeyCode);
			else
			{
				//if not emit the parameter key
				sKey = sParams;
				iKeyCode = sKey.toInt(&bOK);
				if(bOK)
					execKeyPressed(iKeyCode);
			}
			
		}
		else if(pAction->getType()==MIhmAction::enuExecuteCmd)
		{
			QString sProgram =  pAction->getParamValue() + sParams;
			QProcess::startDetached(sProgram);
		}
		else if(pAction->getType()==MIhmAction::enuSendCmdReq)
		{
			QString sCmd =  pAction->getParamValue();
			execCommandReq(sCmd, sParams);
		}
		else
			TRACE_D(QString("MIhmTCLInterfaces::executeDskAction:Undefined Action type:%1").
						arg(pAction->getType()));

	}
	else
	{
		TRACE_D(QString("MIhmTCLInterfaces::executeDskAction:Undefined Action: %1").
						arg(sAction));
	}

}

void MIhmTCLInterfaces::onDskKeyDetected(int iAskedIhmKeyCode)
{
   //Desktop IHM generated the key
	//verify who has the control (if control is not shared) and forward the detected key 
	//to the ANI thread (if the issuer is allowed to do the action)
	if(isDskUserInControl())
		execKeyPressed(iAskedIhmKeyCode);
	else
	{
		TRACE_D("MIhmTCLInterfaces::onDskKeyDetected: rejected! User not in control!");
	}
}



void MIhmTCLInterfaces::execKeyPressed(int iAskedIhmKeyCode)
{
	if(m_pAuthAutomate->isTakeOverInProgress())
	{
		TRACE_D("MIhmTCLInterfaces::execKeyPressed: Ignored, cause takeover is in progress!");
		return;
	}

	//to limit the input frequency	
	if(IsInputBufferOverflow())	return;

	MIhmMsgKeyAndCmdReq *pMsg;
	pMsg = new MIhmMsgKeyAndCmdReq();
	pMsg->m_iKeyPressed = iAskedIhmKeyCode;
	pMsg->m_type = enuIHM_TOUCHE_Key;
	m_pMainLogic->sendMessageToAni(pMsg);
}

void MIhmTCLInterfaces::execCommandReq(QString sCommand, QString sCommandParam)
{
	if(m_pAuthAutomate->isTakeOverInProgress())
	{
		TRACE_D("MIhmTCLInterfaces::execCommandReq: Ignored, cause takeover is in progress!");
		return;
	}

	//to limit the input frequency	
	if(IsInputBufferOverflow())	return;

	MIhmMsgKeyAndCmdReq *pMsg;
	pMsg = new MIhmMsgKeyAndCmdReq();
	pMsg->m_sCommand = sCommand;
	pMsg->m_sCommandParam = sCommandParam;
	pMsg->m_type = enuIHM_TOUCHE_Command;
	m_pMainLogic->sendMessageToAni(pMsg);
}


void MIhmTCLInterfaces::onDskStringDetected(MIhmMsgStringDetection * pMsg)
{
	if(m_pAuthAutomate->isTakeOverInProgress())
	{
		TRACE_D("MIhmTCLInterfaces::onDskStringDetected: Ignored, cause takeover is in progress!");
		return;
	}

	//to limit the input frequency	
	if(IsInputBufferOverflow())	return;

    //Desktop IHM detected a string
	//Do not verify who has the control and forward the detected string 
	//to the ANI thread 
	//(We need it to be able to take control from desktop using Zip card)
	m_pMainLogic->sendMessageToAni(pMsg);
}


bool MIhmTCLInterfaces::IsInputBufferOverflow()
{
	bool bBufferOverflow = false;

	if(MIhmConfigGeneral::getCfg()->getMaxInputMsgPerSec()>0)
	{
		QTime dt = QTime::currentTime();
		int iDifMsecs = m_dtLastInput.msecsTo(dt);
		
		if(iDifMsecs <1000 && iDifMsecs > 0)
		{
			m_iNumMsgsInLastSecond = m_iNumMsgsInLastSecond + 1;
		
			if(m_iNumMsgsInLastSecond > MIhmConfigGeneral::getCfg()->getMaxInputMsgPerSec()) //limit 
			{
				TRACE_D("MIhmTCLInterfaces::IsInputBufferOverflow: Input message filtered because of input freq. limitation!");
				bBufferOverflow = true;
			}
		}
		else
		{
			m_iNumMsgsInLastSecond = 0;
			m_dtLastInput = dt;
		}
	}

	return bBufferOverflow;
}

//-----------------------------------------------------------
// END DESKTOP input interface functions
//-----------------------------------------------------------



//-----------------------------------------------------------------------------------
// WEB input interface functions
//-----------------------------------------------------------------------------------

void MIhmTCLInterfaces::onWebKeyDetected(QString sWebSession, int iKeyCode)
{
	TRACE_D(QString("MIhmTCLInterfaces::onWebKeyDetected: key:%1").
						arg(iKeyCode));
	
	if(isWebUserInControl(sWebSession))
	{
		execKeyPressed(iKeyCode);
	}
	else
	{
		TRACE_W("MIhmTCLInterfaces::onWebKeyDetected: Rejected! Desktop user is in control!");
	}
}


void MIhmTCLInterfaces::onWebInputDlgsMessage(QString sWebSession, MIhmMsg * pMsg)
{
	if(isWebUserInControl(sWebSession))
	{
		m_pMainLogic->sendMessageToAni(pMsg);		
	}
	else
	{
		delete pMsg;
		TRACE_W("MIhmTCLInterfaces::onWebInputDlgsMessage: Msg Rejected! Desktop user in control!");
	}
}





void MIhmTCLInterfaces::onWebAction(QString sWebSession, QString sAction, QString sParams)
{
	TRACE_D(QString("MIhmTCLInterfaces::onWebAction:Action:%1, params:%2 ").
						arg(sAction).
						arg(sParams));

	if(isWebUserInControl(sWebSession))
	{
		//execute the action
		executeWebAction(sAction, sParams);
	}			
}

void MIhmTCLInterfaces::executeWebAction(QString sAction, QString sParams)
{
	MIhmConfigActions * pCfgActions = MIhmConfigGeneral::getCfg()->getActions();
	
	MIhmAction * pAction = pCfgActions->findAction(sAction);

	if(pAction!=NULL)
	{
		if(pAction->getType()==MIhmAction::enuSendKey)
		{
			//emit both: staticaly defined part which is in the registry and 
			// the other dynamic part received in the additional parameters
			//This way we can use generic key down action 
			QString sKey = pAction->getParamValue();
			bool bOK;
			int iKeyCode = sKey.toInt(&bOK);
			if(bOK)
				execKeyPressed(iKeyCode);
			else
			{
				//if not emit the parameter key
				sKey = sParams;
				iKeyCode = sKey.toInt(&bOK);
				if(bOK)
					execKeyPressed(iKeyCode);
			}
			
		}
		else if(pAction->getType()==MIhmAction::enuSendCmdReq)
		{
			QString sCmd =  pAction->getParamValue();
			execCommandReq(sCmd, sParams);
		}
		else
			TRACE_W(QString("MIhmTCLInterfaces::executeWebAction:Undefined Action type:%1 ").
						arg(pAction->getType()));

	}
	else
	{
		TRACE_W(QString("MIhmTCLInterfaces::executeWebAction:Undefined Action: %1").
						arg(sAction));
	}

}


//-----------------------------------------------------------------------------------
// END WEB input interface functions
//-----------------------------------------------------------------------------------


