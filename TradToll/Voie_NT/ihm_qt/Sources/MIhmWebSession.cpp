

#include "MIhmWebSession.h"
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


MIhmWebSession::MIhmWebSession(MIhmWebInterface *parent, MIhmWebCommonObjLogic * pCommonVisibleObjLogic)
: m_pWebInterface(parent),m_eStatus(enuWebSessStatusUnknown)
{
	m_bLoggedOff = false;
	m_pLoginClient = NULL;
	generateNewSessionID();
	m_bSessionIsInControl = false;
	m_bIdleTimeout = false;
	connect(&m_tmInControlIdleCheck, SIGNAL(timeout()), this, SLOT(onSessionIdleTimeout()));

	initializeSessionVisibleObjects();
	initializeCompleteVisibleObjectLinks(pCommonVisibleObjLogic);
	updateSessionInControlStatuses(m_bSessionIsInControl);
	
	updateLastResponseTime();
}


MIhmWebSession::~MIhmWebSession()
{
	//clear visible objects vector
	for(int i=MIhmWebVisibleObject::enuWebVisibleObjectUnknown+1;i<m_vectSessionWebVisibleObjects.size();i++)
	{
		if(m_vectSessionWebVisibleObjects.at(i)!=NULL)
			delete m_vectSessionWebVisibleObjects.at(i);
	}

	//clear link objects vector
	for(int i=MIhmWebVisibleObject::enuWebVisibleObjectUnknown+1;i<m_vectCompleteWebVisibleObjectsSessData.size();i++)
	{
		if(m_vectCompleteWebVisibleObjectsSessData.at(i)!=NULL)
			delete m_vectCompleteWebVisibleObjectsSessData.at(i);
	}

}

void MIhmWebSession::initializeCompleteVisibleObjectLinks(MIhmWebCommonObjLogic *pCommonVisibleObjLogic)
{
	m_vectCompleteWebVisibleObjectsSessData.fill(NULL, (int)MIhmWebVisibleObject::enuWebVisibleObjectLast);

	//initialize common objects
	MWebVisibleObjSessionLink *pNewVisibleObjLink;

	for (int i=MIhmWebVisibleObject::enuWebVisibleObjectUnknown+1; i<=MIhmWebVisibleObject::enuWebVisibleObjectLastCommon;i++)
	{
		pNewVisibleObjLink = new MWebVisibleObjSessionLink();
		pNewVisibleObjLink->m_pVisibleObject = pCommonVisibleObjLogic->getVisibleObject((MIhmWebVisibleObject::enumWebVisibleObjectId)i);
		
		m_vectCompleteWebVisibleObjectsSessData.replace(i, pNewVisibleObjLink);
	}
	

	//!!! Be careful not to overwrite the links to the common objects 
	// Also take care that the index of the  MIhmWebVisibleObject in all vectors is equal to its enumWebVisibleObjectId
	int iFirstSessionVisibleObject = MIhmWebVisibleObject::enuWebVisibleObjectLastCommon + 1;

	//initialize session specific objects
	for (int i=iFirstSessionVisibleObject; i<MIhmWebVisibleObject::enuWebVisibleObjectLast;i++)
	{
		pNewVisibleObjLink = new MWebVisibleObjSessionLink();
		pNewVisibleObjLink->m_pVisibleObject = m_vectSessionWebVisibleObjects.at(i);
		m_vectCompleteWebVisibleObjectsSessData.replace(i, pNewVisibleObjLink);
	}
	
}

void MIhmWebSession::generateNewSessionID()
{
	QUuid guid;
	guid = QUuid::createUuid();

	m_sSessionID = guid.toString(QUuid::Id128);

}


MIhmWebSession * MIhmWebSession::findSessionObj(QList <MIhmWebSession *> * plstSessions, QString sSessionID)
{
	MIhmWebSession *pRetObj = NULL;

	for (int i = 0; i < plstSessions->size(); ++i) 
	{
		if(plstSessions->at(i)->getSessionID()==sSessionID)
		{
			pRetObj = plstSessions->at(i);
			break;
		}
	}

	return pRetObj;
}

MIhmWebSession * MIhmWebSession::findByClientSessionId(QList <MIhmWebSession *> * plstSessions, QString sClientSessionID)
{
	MIhmWebSession *pRetObj = NULL;

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




bool MIhmWebSession::removeSessionObj(QList <MIhmWebSession *> * plstSessions, QString sSessionID)
{
	MIhmWebSession *pCurrent = NULL;

	for (int i = 0; i < plstSessions->size(); ++i) 
	{
		if(plstSessions->at(i)->getSessionID()==sSessionID)
		{
			pCurrent = plstSessions->at(i);
			plstSessions->removeAt(i);
			delete pCurrent;
			return true;
		}
	}

	return false;
}

bool MIhmWebSession::removeSessionObj(QList <MIhmWebSession *> * plstSessions, MClientSocket *pClient)
{
	MIhmWebSession *pCurrent = NULL;

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

QString MIhmWebSession::getLoginName()
{
	return m_sLoginName;
}


void MIhmWebSession::setLoginData(QString sLoginName, 
									bool bTakeControl, 
									MClientSocket *pClient, 
									bool bProxyUsed, 
									QString sClientSessionID)
{
	m_pLoginClient = pClient;
	m_sLoginName = sLoginName;
	m_bLoginTakeControlFlag = bTakeControl;
	m_bProxyUsed = bProxyUsed;
	m_sClientSessionID = sClientSessionID;
	m_eStatus = enuWebSessWaitingAuth;
}


void MIhmWebSession::setStatusInitializing()
{ 
	if(m_eStatus == enuWebSessWaitingAuth) 
		m_eStatus = enuWebSessInitializing;
	else
	{
		TRACE_WEB_W(QString("MIhmWebSession::setStatusInitializing:  Invalid session status %1")
						.arg(m_eStatus));

	}

}



void MIhmWebSession::initTakeOver(bool bQuest, QString sTakeOverUserName)
{
	MIhmWebVisibleHPrioDialog * pCurrent;
	pCurrent = (MIhmWebVisibleHPrioDialog *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectSessionWebVisibleObjects, 
								MIhmWebVisibleObject::enuDLG_VAL_WEB_HPRIO_INPUT_DIALOG);

	if(	pCurrent!=NULL)
	{
		MIhmLanguages *pLang = MIhmConfigWeb::getCfg()->getLanguages();

		if(bQuest)
		{
			QString sTitle = pLang->getTakeOverQuestionTitleTranslation(MIhmLanguages::enuTranslTargetWeb);
			QString sMsg = pLang->getTakeOverQuestionTranslation(MIhmLanguages::enuTranslTargetWeb, sTakeOverUserName);
			QString sAcceptText = pLang->getTakeOverAcceptButtonTranslation(MIhmLanguages::enuTranslTargetWeb);
			QString sRejectText = pLang->getTakeOverRejectButtonTranslation(MIhmLanguages::enuTranslTargetWeb);

			pCurrent->initialize(MIhmWebVisibleHPrioDialog::enuTakeOverQuestion, sTitle, sMsg, sTakeOverUserName, sAcceptText, sRejectText);
		}
		else
		{
			QString sTitle = pLang->getTakeOverInfoTitleTranslation(MIhmLanguages::enuTranslTargetWeb);
			QString sMsg = pLang->getTakeOverInfoTranslation(MIhmLanguages::enuTranslTargetWeb);
			QString sDefaultBtnText = pLang->getLoginCancelButtonTranslation(MIhmLanguages::enuTranslTargetWeb);
			pCurrent->initialize(MIhmWebVisibleHPrioDialog::enuTakeOverMsg, sTitle, sMsg, "", sDefaultBtnText, "");
		}
		
		pCurrent->setDialogInProgress(true);
	}

}

//used to stop the taking over procedure
void MIhmWebSession::closeTakeCtrlClientDlg()
{
	MIhmWebVisibleHPrioDialog * pCurrent;
	pCurrent = (MIhmWebVisibleHPrioDialog *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectSessionWebVisibleObjects, 
								MIhmWebVisibleObject::enuDLG_VAL_WEB_HPRIO_INPUT_DIALOG);

	if(	pCurrent!=NULL)
	{
		if(pCurrent->getDlgType()!=MIhmWebVisibleHPrioDialog::enuTakeOverInfoMsg)
			pCurrent->setDialogInProgress(false);
	}

}

//used to close any high priority dialog
void MIhmWebSession::closeHighPrioClientDlg()
{
	MIhmWebVisibleHPrioDialog * pCurrent;
	pCurrent = (MIhmWebVisibleHPrioDialog *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectSessionWebVisibleObjects, 
								MIhmWebVisibleObject::enuDLG_VAL_WEB_HPRIO_INPUT_DIALOG);

	if(	pCurrent!=NULL)
	{
		pCurrent->setDialogInProgress(false);
	}
}


void MIhmWebSession::initClientLoginDlg(QString sLogin)
{
	MIhmWebVisibleHPrioDialog * pCurrent;
	pCurrent = (MIhmWebVisibleHPrioDialog *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectSessionWebVisibleObjects, 
								MIhmWebVisibleObject::enuDLG_VAL_WEB_HPRIO_INPUT_DIALOG);

	if(	pCurrent!=NULL)
	{
		MIhmLanguages *pLang = MIhmConfigWeb::getCfg()->getLanguages();

		QString sTitle = pLang->getLoginTitleTranslation(MIhmLanguages::enuTranslTargetWeb);
		QString sMsg = pLang->getLoginMessageTranslation(MIhmLanguages::enuTranslTargetWeb);
		QString sOKText = pLang->getLoginOKButtonTranslation(MIhmLanguages::enuTranslTargetWeb);
		QString sCancelText = pLang->getLoginCancelButtonTranslation(MIhmLanguages::enuTranslTargetWeb);

		pCurrent->initialize(MIhmWebVisibleHPrioDialog::enuClientLoginDlg, sTitle, sMsg, sLogin, sOKText, sCancelText);
		pCurrent->setDialogInProgress(true);
	}
}


void MIhmWebSession::initClientInfoDlg(QString sMsg, QString sMsgTitle)
{
	MIhmWebVisibleHPrioDialog * pCurrent;
	pCurrent = (MIhmWebVisibleHPrioDialog *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectSessionWebVisibleObjects, 
								MIhmWebVisibleObject::enuDLG_VAL_WEB_HPRIO_INPUT_DIALOG);

	if(	pCurrent!=NULL)
	{
		MIhmLanguages *pLang = MIhmConfigWeb::getCfg()->getLanguages();
		
		QString sDefaultBtnText = pLang->getClientCloseButtonTranslation(MIhmLanguages::enuTranslTargetWeb);
		pCurrent->initialize(MIhmWebVisibleHPrioDialog::enuTakeOverInfoMsg, sMsgTitle, sMsg, "", sDefaultBtnText, "");

		pCurrent->setDialogInProgress(true);
	}

}

void MIhmWebSession::initConnectingInfoDlg(QString sMsg, QString sMsgTitle)
{
	MIhmWebVisibleHPrioDialog * pCurrent;
	pCurrent = (MIhmWebVisibleHPrioDialog *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectSessionWebVisibleObjects, 
								MIhmWebVisibleObject::enuDLG_VAL_WEB_HPRIO_INPUT_DIALOG);

	if(	pCurrent!=NULL)
	{
		MIhmLanguages *pLang = MIhmConfigWeb::getCfg()->getLanguages();
		
		QString sCancelBtnText = pLang->getLoginCancelButtonTranslation(MIhmLanguages::enuTranslTargetWeb);
		pCurrent->initialize(MIhmWebVisibleHPrioDialog::enuClientConnectingMsg, sMsgTitle, sMsg, "", "", sCancelBtnText);

		pCurrent->setDialogInProgress(true);
	}

}


int MIhmWebSession::getHPrioDlgType()
{
	MIhmWebVisibleHPrioDialog * pCurrent;
	pCurrent = (MIhmWebVisibleHPrioDialog *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectSessionWebVisibleObjects, 
								MIhmWebVisibleObject::enuDLG_VAL_WEB_HPRIO_INPUT_DIALOG);

	if(	pCurrent!=NULL)
		return pCurrent->getDlgType();
	else
		return MIhmWebVisibleHPrioDialog::enuClientDlgNone;

}


void MIhmWebSession::initializeSessionVisibleObjects()
{
	MIhmWebVisibleObject * pNewObject;
	
	m_vectSessionWebVisibleObjects.fill(NULL, (int)MIhmWebVisibleObject::enuWebVisibleObjectLast);

	for (int i = MIhmWebVisibleObject::enuWebVisibleObjectLastCommon + 1; i<MIhmWebVisibleObject::enuWebVisibleObjectLast; i++)
	{
		switch(i)
		{
			case MIhmWebVisibleObject::enuBTN_WEB_TAKE_CTRL:
			case MIhmWebVisibleObject::enuBTN_WEB_RETURN_CTRL:
				pNewObject = new MIhmWebVisibleButton((MIhmWebVisibleObject::enumWebVisibleObjectId)i, true);
				m_vectSessionWebVisibleObjects.replace(i, pNewObject);	
			break;
	
			//------------------------------------------------------------
			//--------- Modal input dialog -------------------------------
			//------------------------------------------------------------
			case MIhmWebVisibleObject::enuDLG_WEB_INPUT_DIALOG:
				pNewObject = new MIhmWebVisibleDialog((MIhmWebVisibleObject::enumWebVisibleObjectId)i);
				m_vectSessionWebVisibleObjects.replace(i, pNewObject);	
			break;

			case MIhmWebVisibleObject::enuDLG_WEB_INPUT_DIALOG_EX:
				pNewObject = new MIhmWebVisibleDialogEx((MIhmWebVisibleObject::enumWebVisibleObjectId)i);
				m_vectSessionWebVisibleObjects.replace(i, pNewObject);	
			break;

			case MIhmWebVisibleObject::enuDLG_VAL_WEB_HPRIO_INPUT_DIALOG:
				pNewObject = new MIhmWebVisibleHPrioDialog((MIhmWebVisibleObject::enumWebVisibleObjectId)i);
				m_vectSessionWebVisibleObjects.replace(i, pNewObject);	
			break;

			//------------------------------------------------------------
			//----- For specific take over mode handling -----------------
			//------------------------------------------------------------
			//a command to the client to update the look of the form when
			// in control status change
			case MIhmWebVisibleObject::enuCMD_WEB_IN_CONTROL_MODE:
				pNewObject = new MIhmWebVisibleInControlMode((MIhmWebVisibleObject::enumWebVisibleObjectId)i);
				m_vectSessionWebVisibleObjects.replace(i, pNewObject);	
			break;
			//------------------------------------------------------------
			//----- For specific OpenLink command handling -----------------
			//------------------------------------------------------------
 			case MIhmWebVisibleObject::enuCMD_WEB_OPEN_LINK:
				pNewObject = new MIhmWebVisibleOpenLink((MIhmWebVisibleObject::enumWebVisibleObjectId)i);
				m_vectSessionWebVisibleObjects.replace(i, pNewObject);	
			break;
			//------------------------------------------------------------
			default:
				TRACE_W(QString("MIhmWebSession::initializeSessionVisibleObjects: Error unknown visible object ID:%1").arg(i));
				continue;
		}

	}

}

bool MIhmWebSession::setInControl(bool bInControl)
{	
	bool bStateChanged = false;

	if(m_bSessionIsInControl!=bInControl)
	{
		m_bSessionIsInControl = bInControl;
		updateSessionInControlStatuses(m_bSessionIsInControl);
		bStateChanged = true;
	}

	return bStateChanged;
}


bool MIhmWebSession::getActionForCtrlId(QString sCtrlId, QString &sActionID, QString &sParams)
{
	bool bFound;
	QString foundActionID;
	QString actParams;
	bool bRetVal = false;
	MWebVisibleObjSessionLink* pCurrent;
		
	//if the clicked sCtrlId control ID matches the name of a visible object
	// it is an object with single action
	pCurrent = MWebVisibleObjSessionLink::findVisibleObjectByName(&m_vectCompleteWebVisibleObjectsSessData, sCtrlId);	

	if(pCurrent!=NULL)
	{
		//to speed up quit if the ctrlId match the name of the visible object
		//meaning the object was found and have just one action (or none)
		bRetVal = pCurrent->getActionForCtrlId(sCtrlId, foundActionID,actParams);

		if(bRetVal)
		{	
			sActionID = foundActionID;
			sParams = actParams;
		}
	}
	else 
	{
		for(int i=MIhmWebVisibleObject::enuWebVisibleObjectUnknown+1;i<m_vectCompleteWebVisibleObjectsSessData.size();i++)
		{
			if(m_vectCompleteWebVisibleObjectsSessData.at(i)!=NULL)
			{
				pCurrent = m_vectCompleteWebVisibleObjectsSessData.at(i);

				if(pCurrent->getType()==MIhmWebVisibleObject::enuHtmlTabView || 
					pCurrent->getType()==MIhmWebVisibleObject::enuHtmlTableView || 
					pCurrent->getType()== MIhmWebVisibleObject::enuHtmlHMenuView)
				{
					bFound = pCurrent->getActionForCtrlId(sCtrlId, foundActionID, actParams);

					if(bFound)
					{
						sActionID = foundActionID;
						sParams = actParams;
						bRetVal = true;
						break;
					}
				}
			}
		}

	}


	return bRetVal;
}


void MIhmWebSession::updateSessionInControlStatuses(bool bInControl)
{
	//update all the visible objects that should change in 
	// the case of change from In Control to Read Only and vice versa
	MWebVisibleObjSessionLink * pCurrent;
	MIhmLanguages *pLang = MIhmConfigWeb::getCfg()->getLanguages();

	for(int i=MIhmWebVisibleObject::enuWebVisibleObjectUnknown+1;i<m_vectCompleteWebVisibleObjectsSessData.size();i++)
	{
		if(m_vectCompleteWebVisibleObjectsSessData.at(i)!=NULL)
		{
			pCurrent = m_vectCompleteWebVisibleObjectsSessData.at(i);

			if(i == (int)MIhmWebVisibleObject::enuBTN_WEB_TAKE_CTRL)
			{	
				MIhmWebVisibleButton* pVisObj = (MIhmWebVisibleButton*)pCurrent->m_pVisibleObject;

				pVisObj->setTextLabel(pLang->getBtnLabelTakeControl(MIhmLanguages::enuTranslTargetWeb));
				pVisObj->setEnabled(!bInControl); 
			}
			else if(i == (int)MIhmWebVisibleObject::enuBTN_WEB_RETURN_CTRL)
			{
				MIhmWebVisibleButton* pVisObj = (MIhmWebVisibleButton*)pCurrent->m_pVisibleObject;		
				
				if(!bInControl)
					pVisObj->setTextLabel(pLang->getBtnLabelEndIdentification(MIhmLanguages::enuTranslTargetWeb));
				else
					pVisObj->setTextLabel(pLang->getBtnLabelReturnControl(MIhmLanguages::enuTranslTargetWeb));
					
			}
			else
			{
				pCurrent->forceUpdateForSession();
			}
		
		}
	}
}



void MIhmWebSession::processVirtualObject(MIhmVirtualObject* pVirtObj)
{	
	//If the main form is not initialized yet 
	//reset the virtual object flag and do nothing 
	//since the config MIhmVirtDynConfig / enuIhmVirtConfLaneType
	//will refresh all the objects
	if(getStatus() != enuWebSessInitialized)
	{
		if(pVirtObj->getId() == MIhmVirtualObject::enuIhmVirtConfLaneType && 
			m_eStatus == enuWebSessInitializing)	
		{
			m_eStatus = enuWebSessInitialized;
			startSessionIdleTimer();
		}
		else
		{
			pVirtObj->clearUpdated();
			return;
		}
	}

	switch(pVirtObj->getType())
	{
// 		case MIhmVirtualObject::enuIhmContainer:
// 				processVirtContainer((MIhmVirtContainer*)pVirtObj);
// 			break;
// 		case MIhmVirtualObject::enuIhmIco:
// 				processVirtIcon((MIhmVirtIco*)pVirtObj);
// 			break;
// 		case MIhmVirtualObject::enuIhmText:
// 				processVirtText((MIhmVirtText*)pVirtObj);
// 			break;
// 		case MIhmVirtualObject::enuIhmButton:
// 				processVirtButton((MIhmVirtButton*)pVirtObj);
// 			break;
// 		case MIhmVirtualObject::enuIhmList:
// 				processVirtList((MIhmVirtList*)pVirtObj);
// 			break;
		case MIhmVirtualObject::enuIhmDynConfig:
				processVirtConfig((MIhmVirtDynConfig*)pVirtObj);
			break;
// 		case MIhmVirtualObject::enuIhmDynStrDetection:
// 				processVirtStrDetConfig((MIhmVirtStrDetectConfig*)pVirtObj);
// 			break;
		case MIhmVirtualObject::enuIhmInputDialogReq:
				processInputDialogReq((MIhmVirtInputDialogReq*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmInputDlgExReq:
				processInputDialogExReq((MIhmVirtInputDlgExReq*)pVirtObj);
			break;
// 		case MIhmVirtualObject::enuIhmTableViewReq:
// 				processFileViewReq((MIhmVirtTableViewReq*)pVirtObj);
// 			break;
// 		case MIhmVirtualObject::enuIhmTabView:
// 				processTabViewReq((MIhmVirtTabView*)pVirtObj);
// 			break;
// 		case MIhmVirtualObject::enuIhmAsyncInput:		//M_IHM_ASYNC_INPUT service handling
// 				processAsyncInput((MIhmVirtAsyncInput*)pVirtObj);
// 			break;
// 		case MIhmVirtualObject::enuIhmHMenuViewReq:		
// 				processHMenuViewReq((MIhmVirtHMenuViewReq*)pVirtObj);
// 			break;
// 		case MIhmVirtualObject::enuIhmVideoViewReq:		
// 				processVideoViewReq((MIhmVirtVideoViewReq*)pVirtObj);
// 			break;

		default:
			TRACE_WEB_W( QString("MIhmWebSession::processVirtualObject: Unknown object type %1").
										arg(pVirtObj->getType()));
			break;
	}	
}


bool MIhmWebSession::isInputDlgInProgress()
{
	bool bRetVal = false;

	MIhmWebVisibleDialog * pCurrent;
	pCurrent = (MIhmWebVisibleDialog *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectSessionWebVisibleObjects, MIhmWebVisibleObject::enuDLG_WEB_INPUT_DIALOG);

	if(pCurrent!=NULL)
		bRetVal = pCurrent->isInputInProgress();

	return bRetVal;

}


bool MIhmWebSession::isInputDlgExInProgress()
{
	bool bRetVal = false;

	MIhmWebVisibleDialogEx * pCurrentEx;
	pCurrentEx = (MIhmWebVisibleDialogEx *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectSessionWebVisibleObjects, MIhmWebVisibleObject::enuDLG_WEB_INPUT_DIALOG_EX);
	if(	pCurrentEx!=NULL)
		bRetVal = pCurrentEx->isInputInProgress();

	return bRetVal;
}

bool MIhmWebSession::isHPrioDlgInProgress()
{
	bool bRetVal = false;

	MIhmWebVisibleHPrioDialog * pCurrent;
	pCurrent = (MIhmWebVisibleHPrioDialog *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectSessionWebVisibleObjects, MIhmWebVisibleObject::enuDLG_VAL_WEB_HPRIO_INPUT_DIALOG);
	if(	pCurrent!=NULL)
		bRetVal = pCurrent->isInputInProgress();

	return bRetVal;
}


bool MIhmWebSession::isAnyInputDlgInProgress()
{
	return isInputDlgInProgress() || isInputDlgExInProgress() || isHPrioDlgInProgress();
}


void MIhmWebSession::processInputDialogReq(MIhmVirtInputDialogReq* pVirtObj)
{
	//first verify if the request list is empty to check if there was a 
	//SASIE_VISU ANNULE (CANCEL) request
	bool bInputDlgCancelReq = pVirtObj->isCancel();

	TRACE_WEB_D(QString("MIhmWebSession::processInputDialogReq: CancelReq:%1").arg(bInputDlgCancelReq?1:0));

	if(m_pWebInterface->getSessionInControl() != this)
	{
		//if the session is not in control do not raise dialogs
		return;
	}

	if(bInputDlgCancelReq)
	{
		//if it is an input cancel request 
		//close the dialog
		onInputDialogCMDCancel();
	}
	else if(!pVirtObj->isEmpty() && !isAnyInputDlgInProgress()) //if no dialog is up raise next one
	{	
		raiseNextInputDialog(pVirtObj);
	}
	else if(isInputDlgInProgress())
	{
		//verify if it is the correct one
// 		MInputDialogReq oReq;
// 		if(pVirtObj->getCurrentReq(&oReq))
// 		{
// 			if(!(oReq==m_oCurrentInputReq))
// 			{
// 				hideInputDialog();
// 				raiseNextInputDialog(pVirtObj);
// 			}
// 		}
// 		else
// 			hideInputDialog();
	}

}

void MIhmWebSession::processInputDialogExReq(MIhmVirtInputDlgExReq* pVirtObj)
{
	//first verify if the request list is empty to check if there was a 
	//SASIE_VISU ANNULE (CANCEL) request
	bool bInputDlgExCancelReq = pVirtObj->isCancel();

	TRACE_WEB_D(QString("MIhmWebSession::processInputDialogExReq: Is Cancel Req:%1").arg(bInputDlgExCancelReq?1:0));

	if(m_pWebInterface->getSessionInControl() != this)
	{
		TRACE_WEB_D(QString("MIhmWebSession::processInputDialogExReq: This session is not in control ..."));
		//if the session is not in control do not raise dialogs
		return;
	}


	if(bInputDlgExCancelReq)
	{
		//if it is an input cancel request close the dialog
		onInputDlgExCMDCancel();
	}
	else if(!pVirtObj->isEmpty() && !isAnyInputDlgInProgress()) //if no dialog is up raise next one
	{	
		raiseNextInputDialogEx(pVirtObj);
	}
	else if(isInputDlgExInProgress())
	{
		TRACE_WEB_D(QString("MIhmWebSession::processInputDialogExReq: Input dialog already in progress.Do nothing!"));

		//verify if it is the correct one
// 		MInputDialogExReq oReq;
// 		if(pVirtObj->getCurrentReq(&oReq))
// 		{
// 			if(!(oReq==m_oCurrentInputReq))
// 			{
// 				hideInputDialog();
// 				raiseNextInputDialogEx(pVirtObj);
// 			}
// 		}
// 		else
// 			hideInputDialog();
	}


}


//---------------------------------------------------------------------------------
//----------------- DlgInputDialog  ----------------------------------------
//---------------------------------------------------------------------------------
void MIhmWebSession::sendInputDialogInitErr()
{
	MIhmMsgInputDialogInitErr *pMsg = new MIhmMsgInputDialogInitErr();
	MInputDialogReq oReq;

	getCurrentInputDialog(&oReq);
	pMsg->oInputReq = oReq;
	emit inputDlgsMsg(pMsg);
}

void MIhmWebSession::sendInputDlgCanceled()
{
	MIhmMsgInputDlgCanceled *pMsg = new MIhmMsgInputDlgCanceled();
	emit inputDlgsMsg(pMsg);
}



bool MIhmWebSession::getCurrentInputDialog(MInputDialogReq *pReq)
{
	MIhmWebVisibleDialog * pCurrent;
	pCurrent = (MIhmWebVisibleDialog *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectSessionWebVisibleObjects, MIhmWebVisibleObject::enuDLG_WEB_INPUT_DIALOG);

	if(pCurrent!=NULL)
	{
		return pCurrent->getCurrentReq(pReq);
	}
	else
	{
		TRACE_WEB_W( "MIhmWebSession::getCurrentInputDialog: unable to find DLG_REG_VAL_INPUT_DIALOG");
	}

	return false;
}


void MIhmWebSession::onInputDialogCMDCancel()
{
	MIhmWebVisibleDialog * pCurrent;
	pCurrent = (MIhmWebVisibleDialog *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectSessionWebVisibleObjects, MIhmWebVisibleObject::enuDLG_WEB_INPUT_DIALOG);

	if(	pCurrent!=NULL)
	{
		if(pCurrent->isInputInProgress())
		{
			pCurrent->setDialogInProgress(false);
			
		}
	}

	sendInputDlgCanceled();

}


void MIhmWebSession::raiseNextInputDialog(MIhmVirtInputDialogReq* pVirtObj)
{
	if(pVirtObj==NULL)
	{
        TRACE_WEB_D( "MIhmWebSession::raiseNextInputDialog: pVirtObj==NULL!");
		return;
	}

	MIhmWebVisibleDialog * pCurrent;
	pCurrent = (MIhmWebVisibleDialog *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectSessionWebVisibleObjects, MIhmWebVisibleObject::enuDLG_WEB_INPUT_DIALOG);

	if(	pCurrent!=NULL)
	{	
		MInputDialogReq oCurrentInputReq;
		pCurrent->getCurrentReq(&oCurrentInputReq);

		MInputDialogReq oNewInputReq;
		
		if(pVirtObj->getCurrentReq(&oNewInputReq))
		{
				if(pCurrent->initialize(&oNewInputReq))
				{	
					pCurrent->setDialogInProgress(true);
					pCurrent->setReadOnly(m_pWebInterface->getSessionInControl() != this);
				}
				else
				{
					TRACE_WEB_W( QString("MIhmWebSession::raiseNextInputDialog: Unable to initialize:%1")
								.arg(oNewInputReq.getDescription().toLatin1().data()));
					sendInputDialogInitErr();
				}
		}
		else
		{
			TRACE_WEB_W("MIhmWebSession::raiseNextInputDialog: Unable to read next input request!");
 			sendInputDialogInitErr();
		}
	}
	else
	{
		TRACE_WEB_W(QString("MIhmWebSession::raiseNextInputDialog: findVisibleObjectByName return NULL for the visible object name %1")
										.arg(DLG_REG_VAL_WEB_INPUT_DIALOG));
	}

}

void MIhmWebSession::setDialogNotInProgress()
{
	MIhmWebVisibleDialog * pCurrent;
	pCurrent = (MIhmWebVisibleDialog *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectSessionWebVisibleObjects, MIhmWebVisibleObject::enuDLG_WEB_INPUT_DIALOG);
	if(	pCurrent!=NULL)
	{	
		pCurrent->setDialogInProgress(false);
	}
}

//---------------------------------------------------------------------------------
// ---- DlgInputDialogEx
//---------------------------------------------------------------------------------

void MIhmWebSession::setDialogExNotInProgress()
{
	MIhmWebVisibleDialogEx * pCurrent;
	pCurrent = (MIhmWebVisibleDialogEx *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectSessionWebVisibleObjects, MIhmWebVisibleObject::enuDLG_WEB_INPUT_DIALOG_EX);
	if(	pCurrent!=NULL)
	{	
		pCurrent->setDialogInProgress(false);
	}
}


void MIhmWebSession::sendInputDialogExInitErr(MInputDialogExReq *pReq)
{
	MIhmMsgInputDlgExInitErr *pMsg = new MIhmMsgInputDlgExInitErr();
	pMsg->oInputReq = *pReq;
	emit inputDlgsMsg(pMsg);
}


void MIhmWebSession::sendInputDlgExCanceled()
{
	MIhmMsgInputDlgExCanceled *pMsg = new MIhmMsgInputDlgExCanceled();
	emit inputDlgsMsg(pMsg);
}


bool MIhmWebSession::getCurrentInputDialogEx(MInputDialogExReq *pReq)
{
	MIhmWebVisibleDialogEx * pCurrent;

	pCurrent = (MIhmWebVisibleDialogEx *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectSessionWebVisibleObjects, MIhmWebVisibleObject::enuDLG_WEB_INPUT_DIALOG_EX);

	if(pCurrent!=NULL)
		return pCurrent->getCurrentExReq(pReq);
	else
	{
		TRACE_WEB_W( "MIhmWebSession::getCurrentInputDialogEx: unable to find DLG_REG_VAL_WEB_INPUT_DIALOG_EX");
	}

	return false;
}


void MIhmWebSession::onInputDlgExCMDCancel()
{
	MIhmWebVisibleDialogEx * pCurrent;
	pCurrent = (MIhmWebVisibleDialogEx *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectSessionWebVisibleObjects, MIhmWebVisibleObject::enuDLG_WEB_INPUT_DIALOG_EX);

	if(	pCurrent!=NULL)
	{
		if(pCurrent->isInputInProgress())
		{
			pCurrent->setDialogInProgress(false);
		}
	}

	sendInputDlgExCanceled();
}



void MIhmWebSession::raiseNextInputDialogEx(MIhmVirtInputDlgExReq* pVirtObj)
{
	if(pVirtObj==NULL)
	{
        TRACE_WEB_D( "MIhmWebSession::raiseNextInputDialogEx: pVirtObj==NULL!");
		return;
	}

	MIhmWebVisibleDialogEx * pCurrent;
	pCurrent = (MIhmWebVisibleDialogEx *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectSessionWebVisibleObjects, MIhmWebVisibleObject::enuDLG_WEB_INPUT_DIALOG_EX);

	if(pCurrent!=NULL)
	{	
		MInputDialogExReq oNewInputReq;
		
		if(pVirtObj->getCurrentExReq(&oNewInputReq))
		{
				if(oNewInputReq.getDialogType() == MInputDialogExReq::enuSAISIE_EX_SEL_FILE || 
					oNewInputReq.getDialogType() == MInputDialogExReq::enuSAISIE_EX_SEL_FOLDER)
				{
					TRACE_WEB_W(QString("MIhmWebSession::raiseNextInputDialogEx: File or folder cant be processed on web interface!"));
					sendInputDialogExInitErr(&oNewInputReq); 
				}
				else
				{
					MIhmLaneTypeSettings * pSetings = m_pWebInterface->getCurrentLaneTypeSettings();
					QString sDialogTemplateContent;

					if(pSetings!=NULL)
					{
						QString sHtmlTemplateRoot = pSetings->getParam(CFG_LANE_TYPE_PARAM_WEB_HTMLTemplateRoot);
						QString sObjTemplate = oNewInputReq.getWebUITemplate();
						QString sPath = QDir::cleanPath(sHtmlTemplateRoot + "/" + sObjTemplate);
						
						sDialogTemplateContent = MHelpFuncs::loadFileContent(sPath);

						if(sDialogTemplateContent=="")
						{
							TRACE_WEB_W(QString("MIhmWebSession::raiseNextInputDialogEx: DialogId %1 Unable to load template file [%2] or the file is empty!")
										.arg(oNewInputReq.getDialogId().toLatin1().data())
										.arg(sPath.toLatin1().data()));
							sendInputDialogExInitErr(&oNewInputReq);
						}
						else 
						{
							if(pCurrent->initialize(&oNewInputReq, sDialogTemplateContent))
							{	
								pCurrent->setDialogInProgress(true);
								pCurrent->setReadOnly(m_pWebInterface->getSessionInControl() != this);
							}
							else
							{
								TRACE_WEB_W( QString("MIhmWebSession::raiseNextInputDialogEx: Unable to initialize:%1")
											.arg(oNewInputReq.getDialogId().toLatin1().data()));
								sendInputDialogExInitErr(&oNewInputReq);
							}
						}
					}
					else
					{
						TRACE_WEB_W(QString("MIhmWebSession::raiseNextInputDialogEx: Unable to select current LaneTypeSettings!"));
						sendInputDialogExInitErr(&oNewInputReq);
					}
				}
			

		}
		else
		{
			TRACE_WEB_W("MIhmWebSession::raiseNextInputDialogEx: Unable to read next input request!");
		}
	}
	else
	{
		TRACE_WEB_W(QString("MIhmWebSession::raiseNextInputDialog: findVisibleObjectByName return NULL for the visible object name %1")
										.arg(DLG_REG_VAL_WEB_INPUT_DIALOG_EX));
	}

}

//---------------------------------------------------------------------------------



bool MIhmWebSession::isSessionDead(int iSessionTimeoutSecs)
{
	if(getStatus() == enuWebSessInitialized)
	{
		QDateTime dtNow =	QDateTime::currentDateTime();

		if(m_dtLastResponseTime.secsTo(dtNow)>iSessionTimeoutSecs)
			return true; // session is dead
	}

	return false;
}


void MIhmWebSession::getVisibleObjectsResponse(MIhmHttpRspRefresh * pRsp, bool bIsSessionReadOnly, bool bOnlyUpdated)
{
	MWebVisibleObjSessionLink *pObj;
	
	TRACE_WEB_D( QString("MIhmWebSession::getVisibleObjectsResponse: ..."));
	
	if(getStatus() != enuWebSessInitialized)
	{
		TRACE_WEB_W( QString("MIhmWebSession::getVisibleObjectsResponse: Visible objects not initialized!"));
		return;
	}
	
	
	//m_vectCompleteWebVisibleObjectsSessData
	for(int i=MIhmWebVisibleObject::enuWebVisibleObjectUnknown+1;i<m_vectCompleteWebVisibleObjectsSessData.size();i++)
	{
		if(m_vectCompleteWebVisibleObjectsSessData.at(i)==NULL)
		{
			TRACE_WEB_W( QString("MIhmWebSession::getVisibleObjectsResponse: Visible object %1 does not exist!").arg(i));
			continue;
		}

		pObj = m_vectCompleteWebVisibleObjectsSessData.at(i);
		
		if(!pObj->isReady())
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
		else if(pObj->getType()==MIhmWebVisibleObject::enuHtmlInputDialog)
		{
			MIhmWebVisibleDialog * pDlg = (MIhmWebVisibleDialog*)pObj->m_pVisibleObject;

				if(pDlg->isInputInProgress())
				{
					if(!bIsSessionReadOnly)
					{
						pDlg->setReadOnly(false);
						pRsp->addShowDlgCommand(pObj->getName(),pObj->getHtml(bIsSessionReadOnly),pObj->getTimeStamp());
					}
					else
					{
						pDlg->setReadOnly(true);
						pRsp->addHideDlgCommand(pObj->getName(),pObj->getTimeStamp());
					}
				}
				else
					pRsp->addHideDlgCommand(pObj->getName(),pObj->getTimeStamp());
		}
		else if(pObj->getType()==MIhmWebVisibleObject::enuHtmlInputDialogEx)
		{
			MIhmWebVisibleDialogEx * pDlg = (MIhmWebVisibleDialogEx*)pObj->m_pVisibleObject;

				if(pDlg->isInputInProgress())
				{
					if(!bIsSessionReadOnly)
					{
						pDlg->setReadOnly(false);
						pRsp->addShowDlgCommand(pObj->getName(),pObj->getHtml(bIsSessionReadOnly),pObj->getTimeStamp());
					}
					else
					{
						pDlg->setReadOnly(true);
						pRsp->addHideDlgCommand(pObj->getName(),pObj->getTimeStamp());
					}
				}
				else
					pRsp->addHideDlgCommand(pObj->getName(),pObj->getTimeStamp());
		}
		else if(pObj->getType()==MIhmWebVisibleObject::enuHtmlHPrioInputDialog)
		{
			MIhmWebVisibleHPrioDialog * pDlg = (MIhmWebVisibleHPrioDialog*)pObj->m_pVisibleObject;

				if(pDlg->isInputInProgress())
				{
					pRsp->addShowHiPrioDlgCommand(pObj->getName(),pObj->getHtml(bIsSessionReadOnly),pObj->getTimeStamp());
				}
				else
					pRsp->addHideHiPrioDlgCommand(pObj->getName(),pObj->getTimeStamp());
		}
		else if(pObj->getType()==MIhmWebVisibleObject::enuHtmlAsyncInput)
		{
			pRsp->addHtmlCommand(pObj->getName(),pObj->getHtml(bIsSessionReadOnly),pObj->getTimeStamp());
		}
		else if(pObj->getType()==MIhmWebVisibleObject::enuHtmlInControlMode)
		{
		//poslat na promjenu kad se prominilo iz incontrol u readonly
			MIhmWebVisibleInControlMode * pInControlMode = (MIhmWebVisibleInControlMode*)pObj->m_pVisibleObject;

			if(pInControlMode->isUpdated())
					pRsp->addChangeModeCommand(pInControlMode->getMode(bIsSessionReadOnly),pInControlMode->getTimeStamp());

		}
		else if(pObj->getType()==MIhmWebVisibleObject::enuHtmlOpenLink)
		{
			MIhmWebVisibleOpenLink * pOpenLink = (MIhmWebVisibleOpenLink*)pObj->m_pVisibleObject;

			if(!bIsSessionReadOnly)
			{
				MOpenLinkReq * pReq = pOpenLink->getOpenLinkReq();

				if(pReq!=NULL)
				{
						pRsp->addOpenLinkCommand(pObj->getName(),
											pReq->m_sUrl,
											pReq->m_iIndex,
											pReq->m_sButtonText,
											pReq->m_sTimeStamp,
											pReq->m_bClose);
				}
			
			}
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
			TRACE_WEB_W( QString("MIhmWebSession::getVisibleObjectsResponse: unknown object type %1").arg(pObj->getType()));
		}
	}

	//update the last response time
	updateLastResponseTime();

	TRACE_WEB_D( QString("MIhmWebSession::getVisibleObjectsResponse: ...Done!"));

}

void MIhmWebSession::updateLastResponseTime()
{
	m_dtLastResponseTime = QDateTime::currentDateTime();

}




void MIhmWebSession::processVirtConfig(MIhmVirtDynConfig* pVirtObj)
{

	switch(pVirtObj->getId())
	{
		case MIhmVirtualObject::enuIhmVirtConfProjectVersion:
			//No need to update since this one belongs to the common objects group now
// 				processConfigPrjVersion(pVirtObj);
 			break;		
 		case MIhmVirtualObject::enuIhmVirtConfProjectName:
			//No need to update since this one belongs to the common objects group now
// 				processConfigPrjName(pVirtObj);
 			break;		
		case MIhmVirtualObject::enuIhmVirtConfLaneType:
				processConfigLaneType(pVirtObj);
			break;		
 		case MIhmVirtualObject::enuIhmVirtConfShiftType:
			//No need to update since this one belongs to the common objects group now
// 				processConfigShiftType(pVirtObj);
 			break;		
		case MIhmVirtualObject::enuIhmVirtConfLanguage:
				//already processed at higher level in function:
				// MIhmWebInterface::processVirtConfigLanguage

			break;
		case MIhmVirtualObject::enuIhmVirtConfMainDlgHidden:
			//not implemented at web
			break;

		default:
			TRACE_WEB_W( QString("MIhmWebSession::processVirtConfig: unknown object id %1").arg(pVirtObj->getId()));
			break;

	}
}

void MIhmWebSession::processConfigShiftType(MIhmVirtDynConfig* pVirtObj)
{
	QString sNewShiftType;
	
	sNewShiftType = pVirtObj->m_sDynConfigSetting;
	
	QString sShiftColour;
	
	if(MIhmConfigWeb::getCfg()->getConfigColors()->getColourForShift(sNewShiftType, sShiftColour))
	{
		TRACE_WEB_D( QString("MIhmWebSession::processConfigShiftType: shift:%1, colour:%2") 
											.arg(sNewShiftType)
											.arg(sShiftColour));
		MIhmWebVisibleObject * pCurrent;

		pCurrent = MIhmWebVisibleObject::findVisibleObjectByID(&m_vectSessionWebVisibleObjects, MIhmWebVisibleObject::enuCNT_WEB_MAIN_DIALOG);
		
		if(pCurrent==NULL)
		{
			TRACE_WEB_W( QString("MIhmWebSession::processConfigShiftType:  findVisibleObjectByName %1 return NULL").arg(CNT_REG_VAL_WEB_MAIN_DIALOG));
		}
		else
			pCurrent->setStyle(	MIhmConfigColor::createBkgColorStyle(sShiftColour));
	}
	else
	{
		TRACE_WEB_W( QString("MIhmWebSession::processConfigShiftType: ERROR No shift colour defined for %1")
							.arg(sNewShiftType));
	}

}


void MIhmWebSession::processConfigPrjName(MIhmVirtDynConfig* pVirtObj)
{
	//processed in common objects
}

void MIhmWebSession::processConfigPrjVersion(MIhmVirtDynConfig* pVirtObj)
{
	//processed in common objects
}





void MIhmWebSession::processConfigLaneType(MIhmVirtDynConfig* pVirtObj)
{
	QString sNewLaneType;
	
	sNewLaneType = pVirtObj->m_sDynConfigSetting;

	MIhmConfigWeb *pCfg = MIhmConfigWeb::getCfg();

	MIhmLaneTypeSettings *pLaneSettings = pCfg->getLaneTypeSetting(MIhmLaneTypeSettings::enuWEB,sNewLaneType);

	if(pLaneSettings!=NULL)
	{
		//load new HTML templates to all objects
		for(int i=MIhmWebVisibleObject::enuWebVisibleObjectLastCommon+1;i<m_vectSessionWebVisibleObjects.size();i++)
		{
			if(m_vectSessionWebVisibleObjects.at(i)==NULL)
				continue;

			MIhmWebVisibleObject * pCurrent = m_vectSessionWebVisibleObjects.at(i);

			QString sObjName = pCurrent->getName();

			//init initial visibility style 
			QString sVisible = pLaneSettings->getVisObjParamValue(sObjName, CFG_VISIBLE_OBJ_ATTRIBUTE_VISIBLE);
			
			if(sVisible.compare("0",Qt::CaseInsensitive) == 0 || sVisible.compare("false",Qt::CaseInsensitive)==0)
				pCurrent->setVisible(false);
			else
				pCurrent->setVisible(true);

			//init NOT_USED flag
			QString sNotUsed = pLaneSettings->getVisObjParamValue(sObjName, CFG_VISIBLE_OBJ_ATTRIBUTE_NOT_USED);
			
			if(sNotUsed.compare("1",Qt::CaseInsensitive) == 0 || sNotUsed.compare("true",Qt::CaseInsensitive)==0)
				pCurrent->setNotUsed(true);
			else
				pCurrent->setNotUsed(false);

			//check for action_id if any
			QString sActionId = pLaneSettings->getVisObjParamValue(sObjName, CFG_VISIBLE_OBJ_ATTRIBUTE_ACTION_ID);

			if(!sActionId.isEmpty())
			{
				pCurrent->setSingleAction(sActionId);
			}

			if(pCurrent->getType() == MIhmWebVisibleObject::enuHtmlImage ||
				pCurrent->getType() == MIhmWebVisibleObject::enuHtmlDynImage ||
				pCurrent->getType() == MIhmWebVisibleObject::enuHtmlTextLabel||
				pCurrent->getType() == MIhmWebVisibleObject::enuHtmlRVideoView )
			{
				QString sHtmlTemplate = pLaneSettings->getVisObjHtmlTemplate(sObjName);
				pCurrent->initTemplateHtml(sHtmlTemplate);
			}
			else if(pCurrent->getType() == MIhmWebVisibleObject::enuHtmlList||
				pCurrent->getType() == MIhmWebVisibleObject::enuHtmlInputDialog ||
				pCurrent->getType() == MIhmWebVisibleObject::enuHtmlTableView ||
				pCurrent->getType() == MIhmWebVisibleObject::enuHtmlHMenuView ||
				pCurrent->getType() == MIhmWebVisibleObject::enuHtmlButton ||
				pCurrent->getType() == MIhmWebVisibleObject::enuHtmlHPrioInputDialog ||
				pCurrent->getType() == MIhmWebVisibleObject::enuHtmlTabView)
			{ 
				  //if the web visible object is more complex we 
				  //also need to initialize sub templates
				QString sHtmlTemplate = pLaneSettings->getVisObjHtmlTemplate(sObjName);
				pCurrent->initTemplateHtml(sHtmlTemplate);
				pCurrent->initSubTemplates(pLaneSettings, sObjName);
			}	
			else if(pCurrent->getType() == MIhmWebVisibleObject::enuHtmlInputDialogEx)
			{
				//No need to initialize DlgInputDialogEx template because 
				//each DialogId (defined in registry InputEx) contains a different template file
				//Names from definition and values xml files are used 
				//to generate the final structure of html for the dialog 
			}	
			else if(pCurrent->getType() == MIhmWebVisibleObject::enuHtmlInControlMode)
			{
				//NO NEED to initialize. No style nor html for this object only current mode is kept to 
				// send ChangeMode command when updated!
			}
			else if(pCurrent->getType() == MIhmWebVisibleObject::enuHtmlOpenLink)
			{
				//NO NEED to initialize. No style nor html for this object only current mode is kept to 
				// send OpenLink command when updated!
			}
			else if(pCurrent->getType() == MIhmWebVisibleObject::enuHtmlContainer || 
					pCurrent->getType() == MIhmWebVisibleObject::enuHtmlCtrlDependContainer)
			{
				//NO NEED to initialize: For containers only additional style is updated 
				;
			}
			else if(pCurrent->getType() == MIhmWebVisibleObject::enuHtmlAsyncInput)
			{
				QString sHtmlTemplate = pLaneSettings->getVisObjHtmlTemplate(sObjName);
				pCurrent->initTemplateHtml(sHtmlTemplate);
			}
			else
			{
				TRACE_WEB_W(QString("MIhmWebSession::processConfigLaneType:Error unknown web visible object type:%1") 
								.arg((int)pCurrent->getType()));

			}
							
		}	

	}
	else
	{
		TRACE_WEB_W(QString("MIhmWebSession::processConfigLaneType:Error loading LaneType:%1") 
					.arg(sNewLaneType.toLatin1().data()));
	}

}



void MIhmWebSession::updateVisibleObjectTimestamp(QString sObjectName, QString sTimeStamp)
{
	MWebVisibleObjSessionLink * pCurrent;

	pCurrent = MWebVisibleObjSessionLink::findVisibleObjectByName(&m_vectCompleteWebVisibleObjectsSessData, sObjectName);

	if(pCurrent!=NULL)
	{
		pCurrent->setAckTimeStamp(sTimeStamp);
	}
	else
	{
		TRACE_WEB_W(QString("MIhmWebSession::updateVisibleObjectTimestamp:Error visible object not found:%1") 
					.arg(sObjectName));
	}

	return;
}



void MIhmWebSession::appendOpenLinkReq(QString sUrl, int iTargetWin, bool bOpen)
{
	MIhmWebVisibleOpenLink * pCurrent;

	pCurrent = (MIhmWebVisibleOpenLink *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectSessionWebVisibleObjects, 
						MIhmWebVisibleObject::enuCMD_WEB_OPEN_LINK);

	if(pCurrent==NULL)
	{
		TRACE_WEB_W( "MIhmWebSession::appendOpenLinkReq: unable to find CMD_REG_VAL_WEB_OPEN_LINK");
	}
	else
	{
		QString sTaskButtonText;
		MIhmLaneTypeSettings * pSetting = m_pWebInterface->getCurrentLaneTypeSettings();
		QString sTypeList = pSetting->getParam(CFG_LANE_TYPE_PARAM_TSKBAR_DLG_TYPES);
		QStringList lstpDlgList = sTypeList.split(CFG_LANE_TYPE_PARAM_SEPARATOR);
		
		if(sTypeList !="" && lstpDlgList.size()>iTargetWin)
		{
			sTaskButtonText = lstpDlgList.at(iTargetWin);

			if(sTaskButtonText=="")
				sTaskButtonText = "NOT DEFINED";

			MIhmLanguages * pLang = MIhmConfigWeb::getCfg()->getLanguages();
			pLang->getLabelTranslation(sTaskButtonText, MIhmLanguages::enuTranslTargetWeb, sTaskButtonText);

			pCurrent->appendOpenLinkReq(sUrl, iTargetWin, sTaskButtonText, bOpen);
		}
		else
		{
			QString sMsg = QString("MIhmWebSession::appendOpenLinkReq:");
			sMsg = QString("The window with index %1 is not defined in the LaneType parameter %2!").
										arg(iTargetWin).arg(CFG_LANE_TYPE_PARAM_TSKBAR_DLG_TYPES);
			TRACE_WEB_W(sMsg);
		}
	}
}


void MIhmWebSession::resetSessionIdleTimer()
{
	stopSessionIdleTimer();
	startSessionIdleTimer();
}

void MIhmWebSession::stopSessionIdleTimer()
{
	if(m_tmInControlIdleCheck.isActive())
			m_tmInControlIdleCheck.stop();
}


void MIhmWebSession::startSessionIdleTimer()
{
	int iIdleTimeout = MIhmConfigWeb::getCfg()->getCfgDynamic()->getIdleTimeoutWeb();
	
	//disable timeout if timeout == 0
	if(iIdleTimeout>0)
		m_tmInControlIdleCheck.start(iIdleTimeout*1000); //it is in seconds so we convert it to [ms]
}

void MIhmWebSession::onSessionIdleTimeout()
{	
	m_bIdleTimeout = true;
}
