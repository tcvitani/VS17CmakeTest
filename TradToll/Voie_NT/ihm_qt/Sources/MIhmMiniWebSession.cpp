

#include "MIhmMiniWebSession.h"
#include "MIhmWebVisibleObject.h"
#include "MWebVisibleObjSessionLink.h"
#include "MIhmVirtualObject.h"
#include "MIhmMsgVideo.h"

#include "MIhmWebInterface.h"
#include "MIhmWebCommonObjLogic.h"

#include "MIhmHttpResponse.h"
#include "MInputDialogReq.h"
#include "MTracer.h"
#include "MIhmMsg.h"
#include "MHelpFuncs.h"
#include "MRowsCommand.h"

#include <QUuid>
#include <QString>
#include <QList>
#include <QRegularExpression>
#include <QDir>


MIhmMiniWebSession::MIhmMiniWebSession(MIhmWebInterface *parent, MIhmWebCommonObjLogic * pCommonVisibleObjLogic)
: m_pWebInterface(parent)
{
	m_bLoggedOff = false;
	m_pLoginClient = NULL;
	m_bIdleTimeout = false;
	connect(&m_tmInControlIdleCheck, SIGNAL(timeout()), this, SLOT(onSessionIdleTimeout()));

	initializeCompleteVisibleObjectLinks(pCommonVisibleObjLogic);
	updateLastResponseTime();
}


MIhmMiniWebSession::~MIhmMiniWebSession()
{
	//clear list of visible objects links 
	 while (!m_lstCompleteWebVisibleObjectsSessData.isEmpty())
		delete m_lstCompleteWebVisibleObjectsSessData.takeFirst();

}


void MIhmMiniWebSession::initializeCompleteVisibleObjectLinks(MIhmWebCommonObjLogic *pCommonVisibleObjLogic)
{
	//initialize common objects
	MWebVisibleObjSessionLink *pNewVisibleObjLink;
	MIhmWebVisibleObject * pCurrent;

	 while (!m_lstCompleteWebVisibleObjectsSessData.isEmpty())
		delete m_lstCompleteWebVisibleObjectsSessData.takeFirst();

	for (int i=MIhmWebVisibleObject::enuWebVisibleObjectUnknown+1; i<=MIhmWebVisibleObject::enuWebVisibleObjectLastCommon;i++)
	{
		pCurrent = pCommonVisibleObjLogic->getVisibleObject((MIhmWebVisibleObject::enumWebVisibleObjectId)i);
		
		if(pCurrent!=NULL)
			if(pCurrent->isUsedAtMiniWeb())
			{
				pNewVisibleObjLink = new MWebVisibleObjSessionLink();
				pNewVisibleObjLink->m_pVisibleObject = pCurrent;
				m_lstCompleteWebVisibleObjectsSessData.append(pNewVisibleObjLink);
			}
		
	}
	
}




MIhmMiniWebSession * MIhmMiniWebSession::findByClientSessionId(QList <MIhmMiniWebSession *> * plstSessions, QString sClientSessionID)
{
	MIhmMiniWebSession *pRetObj = NULL;

	for (int i = 0; i < plstSessions->size(); ++i) 
	{
		if(plstSessions->at(i)->getClientSessionID()==sClientSessionID)
		{
			pRetObj = plstSessions->at(i);
			break;
		}
	}

	return pRetObj;
}



bool MIhmMiniWebSession::removeSessionObj(QList <MIhmMiniWebSession *> * plstSessions, MClientSocket *pClient)
{
	MIhmMiniWebSession *pCurrent = NULL;

	for (int i = 0; i < plstSessions->size(); ++i) 
	{
		if(plstSessions->at(i)->getLoginClient()==pClient)
		{
			pCurrent = plstSessions->at(i);
			plstSessions->removeAt(i);
			delete pCurrent;
			return true;
		}
	}

	return false;
}




void MIhmMiniWebSession::setInitData(MClientSocket *pClient, 
									bool bProxyUsed, 
									QString sClientSessionID)
{
	m_pLoginClient = pClient;
	m_bProxyUsed = bProxyUsed;
	m_sClientSessionID = sClientSessionID;
}



//---------------------------------------------------------------------------------



bool MIhmMiniWebSession::isSessionDead(int iSessionTimeoutSecs)
{
	QDateTime dtNow =	QDateTime::currentDateTime();
	int iDiff = m_dtLastResponseTime.secsTo(dtNow);

	if(iDiff>iSessionTimeoutSecs)
			return true; // session is dead


	return false;
}


void MIhmMiniWebSession::getVisibleObjectsResponse(MIhmHttpRspRefresh * pRsp, bool bOnlyUpdated)
{
	MWebVisibleObjSessionLink *pObj;
	const bool bIsSessionReadOnly = true;

	TRACE_WEB_D( QString("MIhmMiniWebSession::getVisibleObjectsResponse: ..."));
	
	
	//m_lstCompleteWebVisibleObjectsSessData
	for(int i=0;i<m_lstCompleteWebVisibleObjectsSessData.size();i++)
	{
		pObj = m_lstCompleteWebVisibleObjectsSessData.at(i);
		
		if(!pObj->isReady() || !pObj->isUsedAtMiniWeb())
				continue;


		if(bOnlyUpdated)
			if(!pObj->isUpdated()) 
				continue;

		
		if(pObj->getType()==MIhmWebVisibleObject::enuHtmlContainer ||
			pObj->getType()==MIhmWebVisibleObject::enuHtmlCtrlDependContainer)
		{
			pRsp->addStyleCommand(pObj->getName(),pObj->getStyle(bIsSessionReadOnly),pObj->getTimeStamp());
		}
		else if(pObj->getType()==MIhmWebVisibleObject::enuHtmlTextLabel)
		{
			pRsp->addHtmlCommand(pObj->getName(),pObj->getHtml(bIsSessionReadOnly),pObj->getTimeStamp());
		}
		else if(pObj->getType()==MIhmWebVisibleObject::enuHtmlImage ||
				pObj->getType()==MIhmWebVisibleObject::enuHtmlDynImage)
		{
			pRsp->addHtmlCommand(pObj->getName(),pObj->getHtml(bIsSessionReadOnly),pObj->getTimeStamp());
		}
		else if(pObj->getType()==MIhmWebVisibleObject::enuHtmlList)
		{
			pRsp->addHtmlCommand(pObj->getName(),pObj->getHtml(bIsSessionReadOnly),pObj->getTimeStamp());
		}
		else if(pObj->getType()==MIhmWebVisibleObject::enuHtmlTableView)
		{
			pRsp->addHtmlCommand(pObj->getName(),pObj->getHtml(bIsSessionReadOnly),pObj->getTimeStamp());
		}
		else if(pObj->getType()==MIhmWebVisibleObject::enuHtmlButton)
		{
			pRsp->addHtmlCommand(pObj->getName(),pObj->getHtml(bIsSessionReadOnly),pObj->getTimeStamp());
		}
		else if(pObj->getType()==MIhmWebVisibleObject::enuHtmlHMenuView)
		{
			pRsp->addHtmlCommand(pObj->getName(),pObj->getHtml(bIsSessionReadOnly),pObj->getTimeStamp());
		}
		else if(pObj->getType()==MIhmWebVisibleObject::enuHtmlTabView)
		{
			pRsp->addHtmlCommand(pObj->getName(),pObj->getHtml(bIsSessionReadOnly),pObj->getTimeStamp());
		}
		else if(pObj->getType()==MIhmWebVisibleObject::enuHtmlRVideoView)
		{
			pRsp->addHtmlCommand(pObj->getName(),pObj->getHtml(bIsSessionReadOnly),pObj->getTimeStamp());
		}
		else if(pObj->getType()==MIhmWebVisibleObject::enuHtmlGenericScript)
		{
			//NOTE getHtml is used (html templates logic is used to define the script to execute)
			pRsp->addExecScriptCommand(pObj->getName(),pObj->getHtml(bIsSessionReadOnly),pObj->getTimeStamp());

		}
		else if(pObj->getType()==MIhmWebVisibleObject::enuHtmlGenericCtrlDependentScript)
		{
			//NOTE getHtml is used (html templates logic is used to define the script to execute)
			pRsp->addExecScriptCommand(pObj->getName(),pObj->getHtml(bIsSessionReadOnly),pObj->getTimeStamp());

		}
		else
		{
			TRACE_WEB_W( QString("MIhmMiniWebSession::getVisibleObjectsResponse: unknown object type %1").arg(pObj->getType()));
		}
	}

	//update the last response time
	updateLastResponseTime();

	TRACE_WEB_D( QString("MIhmMiniWebSession::getVisibleObjectsResponse: ...Done!"));

}

void MIhmMiniWebSession::updateLastResponseTime()
{
	m_dtLastResponseTime = QDateTime::currentDateTime();
}


void MIhmMiniWebSession::updateVisibleObjectTimestamp(QString sObjectName, QString sTimeStamp)
{
	MWebVisibleObjSessionLink * pCurrent;

	pCurrent = MWebVisibleObjSessionLink::findVisibleObjectByName(&m_lstCompleteWebVisibleObjectsSessData, sObjectName);

	if(pCurrent!=NULL)
	{
		pCurrent->setAckTimeStamp(sTimeStamp);
	}
	else
	{
		TRACE_WEB_W(QString("MIhmMiniWebSession::updateVisibleObjectTimestamp:Error visible object not found:%1") 
					.arg(sObjectName));
	}

	return;
}


void MIhmMiniWebSession::onSessionIdleTimeout()
{	
	m_bIdleTimeout = true;
}

