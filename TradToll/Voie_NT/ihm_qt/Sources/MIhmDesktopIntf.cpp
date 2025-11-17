#include <QCoreApplication>
#include <QScreen>

#include "MIhmVirtualObject.h"
#include "MIhmVirtObjectsModel.h"
#include "MIhmDesktopIntf.h"
#include "MFormSplash.h"
#include "MFormMain.h"

#include "MIhmConfigGeneral.h"
#include "MIhmLaneTypeSettings.h"
#include "MIhmLanguages.h"

#include "MFormTakeOverDlg.h"

#include "MInputDialogExReq.h"
#include "MTracer.h"
#include "MIhmMsg.h"
#include "MIhmMsgVideo.h"

#define MAX_OBJECTS_PROCESSED_IN_TURN 5
 
extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include "ihm.h"
}



MIhmDesktopIntf::MIhmDesktopIntf()
{
	m_eAuthStatus = enuNotIdentified;
	m_eStatus = enuDskNotInitialized;

	m_pMainDialog = NULL;
	m_pSplashDialog = NULL;

	m_pGUIModel = new MIhmVirtObjectsModel();
}




MIhmDesktopIntf::~MIhmDesktopIntf()
{
	if(m_pSplashDialog!=NULL)
	{
		delete m_pSplashDialog;
		m_pSplashDialog = NULL;
	}

	
	if(m_pMainDialog!=NULL)
		delete m_pMainDialog;

	delete m_pGUIModel;
}

void MIhmDesktopIntf::setInControl(bool bInControl)
{
	switch(m_eAuthStatus)
	{
	case enuNotIdentified:
        if(bInControl) //if should set to in control status this is an error
		{
			TRACE_W(QString("MIhmDesktopIntf::setInControl: User not identified!"));
		} 
		//else //otherwise all is OK - desktop user is not identified and is out of control already no need to update status

		break;
	case enuIdentifiedReadonly:
			if(bInControl)
				setAuthStatus(enuInControl);
		break;
	case enuInControl:
			if(!bInControl)
				setAuthStatus(enuIdentifiedReadonly);
	    break;
	default:
        TRACE_W(QString("MIhmDesktopIntf::setInControl: Invalid status!"));
	    break;
	}
}

void MIhmDesktopIntf::setUserIdentified(bool bIdentified, QString sInternalSessionID)
{
	m_sIdentSessionID = sInternalSessionID;
	
	if(bIdentified)
		setAuthStatus(enuIdentifiedReadonly);
	else
		setAuthStatus(enuNotIdentified);

}

void MIhmDesktopIntf::setAuthStatus(enumAuthStatus eAuthStatus)
{
	if(m_eAuthStatus!=eAuthStatus)
	{
		m_eAuthStatus = eAuthStatus;

		if(m_pMainDialog!=NULL)
			m_pMainDialog->updateAuthStatus();

		if(eAuthStatus == enuInControl)
		{
			MIhmVirtualObject *pVirtObj = findVirtualObjectByID(MIhmVirtualObject::enuIhmVirtInputDialogID);
			
			if(pVirtObj!=NULL)
				pVirtObj->setUpdated();

			pVirtObj = findVirtualObjectByID(MIhmVirtualObject::enuIhmVirtInputDlgExID);
	
			if(pVirtObj!=NULL)
				pVirtObj->setUpdated();

			processUpdatedVirtualObjects();
		}
	}
}


void MIhmDesktopIntf::setStatus(enumIHMStatus eStatus)
{
	enumIHMStatus eOldStatus = m_eStatus;	

	TRACE_D(QString("MIhmDesktopIntf::setStatus: New status:%1").arg((int)m_eStatus));

	m_eStatus = eStatus;
	
	switch(m_eStatus)
	{
	case enuDskNotInitialized:
		break;
	case enuDskInitializing:
		{
			if(m_pSplashDialog!=NULL)
			{
				m_pSplashDialog->showSplash();
			}
		}
		break;
	case enuDskInitialized:
		{
			if(m_pSplashDialog!=NULL)
				m_pSplashDialog->hide();
			
			MIhmVirtDynConfig * pVirtMainDlgHidden = (MIhmVirtDynConfig *)findVirtualObjectByID(MIhmVirtualObject::enuIhmVirtConfMainDlgHidden);
			processConfigMainDlgHidden(pVirtMainDlgHidden);

			//if (m_pMainDialog!=NULL)
			//{
			//	TRACE_D(QString("MIhmDesktopIntf::setStatus: enuInitialized: Calling m_pMainDialog->show..."));
			//	m_pMainDialog->show();
			//}
			//else
			//{
			//	TRACE_W(QString("MIhmDesktopIntf::setStatus: enuInitialized, but m_pMainDialog==NULL!"));
			//}
		}
	    break;
	default:
	    break;
	}

}

bool MIhmDesktopIntf::initialize()
{
	TRACE_D(QString("MIhmDesktopIntf::initialize:"));


	if(MIhmConfigGeneral::getCfg()->m_bShowSplash)
		if ( initializeSplashDialog() == false )
		{
			TRACE_W(QString( "MIhmDesktopIntf::initialize: Error initializeSplashDialog returned false"));
			//return false; //- do not exit if faulty splash template
		}
	
	
	if(MIhmConfigGeneral::getCfg()->doUITemplatesVerification())
	{
		//read all LaneTypes configurations available in registry
		//verify if the templates  exist and if are correct
		if(!verifyMainDialogTemplates())
		{
			TRACE_W(QString( "MIhmDesktopIntf::initialize: Error initialize MainDialog returned false"));
			return false;
		}
	}

	setStatus(enuDskInitializing);

	//to be able to continue processing of virtual objects if exited
	//to refresh (clock and form repaint)
	QObject::connect(this, SIGNAL(privateUpdateVirtObj()),this, SLOT(onVirtualObjectsUpdatedPrivate()), Qt::QueuedConnection);

	//QObject::connect(qApp->desktop(), SIGNAL(resized(int)), this, SLOT(onDesktopWidgetChanged(int)), Qt::QueuedConnection);

	//TO DO replace with correct signal
	//QObject::connect(qApp->desktop(), SIGNAL(screenCountChanged(int)), this, SLOT(onDesktopWidgetChanged(int)), Qt::QueuedConnection);
	
	//QObject::connect(qApp->desktop(), SIGNAL(workAreaResized(int)), this, SLOT(onDesktopWidgetChanged(int)), Qt::QueuedConnection);

	return true;
}

void MIhmDesktopIntf::onDesktopWidgetChanged(int iData)
{
	QScreen *deskWidget = qApp->primaryScreen();
	QList<QScreen*> lstSecreens = qApp->screens();

	if (lstSecreens.count() <= 1) //if only one screen left... force hiding this IHM instance if required by configuration
	{
		if (MIhmConfigGeneral::getCfg()->getStartupHideIfSingleScreen())
		{
			MIhmVirtDynConfig * pVirtMainDlgHidden = (MIhmVirtDynConfig *)findVirtualObjectByID(MIhmVirtualObject::enuIhmVirtConfMainDlgHidden);
			pVirtMainDlgHidden->setDynConfigSetting("1"); 
		}
	}

	forceRefreshAll();
}


bool MIhmDesktopIntf::initializeSplashDialog()
{
	bool bRetVal = false;
	TRACE_D(QString("MIhmDesktopIntf::initializeSplashDialog..."));

	//initialize splash dialog
	m_pSplashDialog = new MFormSplash();

	bRetVal = m_pSplashDialog->loadTemplate(MIhmConfigGeneral::getCfg()->getUIFullPath(MIhmConfigGeneral::getCfg()->m_sSplashTemplate));
	
	if(!bRetVal)
	{
		delete m_pSplashDialog;
		m_pSplashDialog = NULL;
	}

	return bRetVal;
}

bool MIhmDesktopIntf::verifyMainDialogTemplates()
{
	bool bRetVal = false;
	bool bRet = true;
	int i;

	QString sLaneType;
    TRACE_D(QString( "MIhmDesktopIntf::verifyMainDialogTemplates ..."));
	

	//for each type of lane configured in registry
	for (i=0;i<MIhmConfigGeneral::getCfg()->m_lstLaneTypeSettings.size();++i)
	{
		//get lane type name
		sLaneType = MIhmConfigGeneral::getCfg()->m_lstLaneTypeSettings.at(i)->getLaneType();
		
		TRACE_D(QString("MIhmDesktopIntf::verifyMainDialogTemplates: Try initializing Lane Type %1")
									.arg(sLaneType));

		//try load template form for this lane type
		//initialize splash dialog
		if(m_pMainDialog!=NULL)
		{
			delete m_pMainDialog;
		}

		m_pMainDialog = new MFormMain();
		
		bRet = bRet && m_pMainDialog->initialize(this, sLaneType);
		
		delete m_pMainDialog;
		m_pMainDialog = NULL;
	}
	
	if(i>0) //if any entry result is bRet
		bRetVal = bRet;

	return bRetVal;
}





void MIhmDesktopIntf::displayInitInfo(QString s)
{
	if(m_eStatus == enuDskInitializing)
	{
		if(m_pSplashDialog!=NULL)
				m_pSplashDialog->displayInfo(s);
	}
}


void MIhmDesktopIntf::close()
{

	if(m_pSplashDialog!=NULL)
	{
		m_pSplashDialog->disconnect();
		m_pSplashDialog->close();
	}

	if(m_pMainDialog!=NULL)
	{
		m_pMainDialog->disconnect();
		m_pMainDialog->close();
	}



}

MIhmVirtualObject *MIhmDesktopIntf::findVirtualObjectByID(MIhmVirtualObject::enumVirtualObjectId eID)
{
	MIhmVirtualObject *pVirtObj = NULL;
	
	pVirtObj = m_pGUIModel->getVirtualObjectByID(eID);

	return pVirtObj;
}



void MIhmDesktopIntf::updateVirtualObjects(MIhmVirtObjectsModel *pTransportModel)
{
	pTransportModel->copyUpdatedVirtObjects(m_pGUIModel, false);	
}


void MIhmDesktopIntf::onVirtualObjectsUpdated()
{
	onVirtualObjectsUpdatedPrivate();
}

void MIhmDesktopIntf::onVirtualObjectsUpdatedPrivate()
{
	m_bRefreshAllObjects = false;

	processUpdatedVirtualObjects();

	if(m_bRefreshAllObjects)
			processRefreshAllObjects();
}


void MIhmDesktopIntf::processUpdatedVirtualObjects()
{
	int iCountProcessed = 0;
	
	QVector <MIhmVirtualObject*> * pVector = m_pGUIModel->getVirtObjVector();

	for (int i = 0; i < pVector->size(); ++i) 
	{
		if(pVector->at(i)==NULL)
				continue;

		if(!pVector->at(i)->isUpdated())
				continue;
		
		if(iCountProcessed>MAX_OBJECTS_PROCESSED_IN_TURN)
		{
			TRACE_D(QString("MIhmDesktopIntf::processUpdatedVirtualObjects...emit privateUpdateVirtObj()... Schedule another processing step..."));
			emit privateUpdateVirtObj();
			break;
		}

		//process virtual object
		processVirtualObject(pVector->at(i));

		//If more than one virtual object is processed it should refresh the
		//clock to show activity
		if(m_pMainDialog!=NULL)
			m_pMainDialog->dateChanged();

		iCountProcessed++;
	}


}


void MIhmDesktopIntf::forceRefreshAll()
{
	MIhmVirtDynConfig * pVirtLaneType = (MIhmVirtDynConfig *)findVirtualObjectByID(MIhmVirtualObject::enuIhmVirtConfLaneType);
	pVirtLaneType->setUpdated();
	emit privateUpdateVirtObj();
}


void MIhmDesktopIntf::processRefreshAllObjects()
{
	TRACE_D(QString("MIhmDesktopIntf::processRefreshAllObjects..."));

	if(m_pMainDialog!=NULL)
		m_pMainDialog->updateAuthStatus();
	
	QVector <MIhmVirtualObject*> * pVector = m_pGUIModel->getVirtObjVector();

	for (int i = 0; i < pVector->size(); ++i) 
	{
		if(pVector->at(i)==NULL)
			continue;

		//must not refresh this
		if(pVector->at(i)->getId() == MIhmVirtualObject::enuIhmVirtConfLaneType ||
			pVector->at(i)->getId() == MIhmVirtualObject::enuIhmVirtConfLanguage)
				continue;
		
		//set to be updated flag so it will be updated in steps MAX_OBJECTS_PROCESSED_IN_TURN
		pVector->at(i)->setUpdated();
	}
	
	emit privateUpdateVirtObj();
}


void MIhmDesktopIntf::processVirtualObject(MIhmVirtualObject* pVirtObj)
{
	TRACE_D(QString("MIhmDesktopIntf::processVirtualObject:%1:%2")
									.arg(pVirtObj->getId())
									.arg(MIhmVirtualObject::getNameForId(pVirtObj->getId())));

	//If the main form is not initialized yet 
	//reset the virtual object flag and do nothing 
	//since the config MIhmVirtDynConfig / enuIhmVirtConfLaneType
	//will refresh all the objects
	if(pVirtObj->getId() != MIhmVirtualObject::enuIhmVirtConfLaneType)	
		if(getStatus() != enuDskInitialized)
		{
			pVirtObj->clearUpdated();
			return;
		}


	switch(pVirtObj->getType())
	{
		case MIhmVirtualObject::enuIhmContainer:
				processContainer((MIhmVirtContainer*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmIco:
				processIcon((MIhmVirtIco*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmText:
				processText((MIhmVirtText*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmButton:
				processButton((MIhmVirtButton*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmList:
				processList((MIhmVirtList*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmDynConfig:
				processConfig((MIhmVirtDynConfig*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmDynStrDetection:
				processStrDetConfig((MIhmVirtStrDetectConfig*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmTableViewReq:
				processFileViewReq((MIhmVirtTableViewReq*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmHMenuViewReq:
				processHMenuViewReq((MIhmVirtHMenuViewReq*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVideoViewReq:
				processVideoViewReq((MIhmVirtVideoViewReq*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmTabView:
				processTabView((MIhmVirtTabView*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmAsyncInput:
				processAsyncInput((MIhmVirtAsyncInput*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmInputDialogReq:
				processInputDialogReq((MIhmVirtInputDialogReq*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmInputDlgExReq:
				processInputDlgExReq((MIhmVirtInputDlgExReq*)pVirtObj);
			break;


		default:
			break;
	}
}



void MIhmDesktopIntf::processContainer(MIhmVirtContainer* pVirtObj)
{
	TRACE_D(QString("MIhmDesktopIntf::processContainer: ..."));
	if(m_pMainDialog!=NULL)
	{
		m_pMainDialog->processContainer(pVirtObj);
	}

	pVirtObj->clearUpdated();
	

}

				
void MIhmDesktopIntf::processTabView(MIhmVirtTabView* pVirtObj)
{
	TRACE_D(QString("MIhmDesktopIntf::processTabView:..."));
	if(m_pMainDialog!=NULL)
	{
		m_pMainDialog->processTabView(pVirtObj);
	}

	pVirtObj->clearUpdated();
	

}

void MIhmDesktopIntf::processIcon(MIhmVirtIco* pVirtObj)
{
	TRACE_D(QString("MIhmDesktopIntf::processIcon:... "));
	//load icon if not loaded and show or hide icon
	if(m_pMainDialog!=NULL)
	{
		m_pMainDialog->processIcon(pVirtObj);
	}

	pVirtObj->clearUpdated();
}

void MIhmDesktopIntf::processText(MIhmVirtText* pVirtObj)
{
	TRACE_D(QString( "MIhmDesktopIntf::processText:... "));
	//load text if not loaded and show or hide
	if(m_pMainDialog!=NULL)
	{
		m_pMainDialog->processText(pVirtObj);
	}

	pVirtObj->clearUpdated();

}


void MIhmDesktopIntf::processButton(MIhmVirtButton* pVirtObj)
{
	TRACE_D(QString( "MIhmDesktopIntf::processButton: ... "));

	//load text if not loaded and show or hide
	if(m_pMainDialog!=NULL)
	{
		m_pMainDialog->processButton(pVirtObj);
	}

	pVirtObj->clearUpdated();

}

void MIhmDesktopIntf::processList(MIhmVirtList* pVirtObj)
{
	TRACE_D(QString( "MIhmDesktopIntf::processList: ... "));
	//load text if not loaded and show or hide
	if(m_pMainDialog!=NULL)
	{
		m_pMainDialog->processList(pVirtObj);
	}

	pVirtObj->clearUpdated();

}



void MIhmDesktopIntf::processStrDetConfig(MIhmVirtStrDetectConfig* pVirtObj)
{
	TRACE_D(QString( "MIhmDesktopIntf::processStrDetConfig: ... "));

	//load text if not loaded and show or hide
	if(m_pMainDialog!=NULL)
	{
		m_pMainDialog->processStrDetConfig(pVirtObj);
	}

	pVirtObj->clearUpdated();

}


void MIhmDesktopIntf::processFileViewReq(MIhmVirtTableViewReq* pVirtObj)
{
	TRACE_D(QString("MIhmDesktopIntf::processFileViewReq..."));
	
	if(m_pMainDialog!=NULL)
	{
		m_pMainDialog->processFileViewReq(pVirtObj);
	}

	pVirtObj->clearUpdated();
}


void MIhmDesktopIntf::processHMenuViewReq(MIhmVirtHMenuViewReq* pVirtObj)
{
	TRACE_D(QString("MIhmDesktopIntf::processHMenuViewReq..."));
	
	if(m_pMainDialog!=NULL)
	{
		m_pMainDialog->processHMenuViewReq(pVirtObj);
	}
	
	pVirtObj->clearUpdated();
}

void MIhmDesktopIntf::processVideoViewReq(MIhmVirtVideoViewReq* pVirtObj)
{
	TRACE_D(QString("MIhmDesktopIntf::processVideoViewReq..."));
	
	if(m_pMainDialog!=NULL)
	{
		m_pMainDialog->processVideoViewReq(pVirtObj);
	}
	
	pVirtObj->clearUpdated();
}


void MIhmDesktopIntf::processOpenLink(int iTargetDlg, QString sUrl, bool bVisible)
{
	TRACE_D(QString("MIhmDesktopIntf::processOpenLink..."));
	
	if(m_pMainDialog!=NULL)
	{
		m_pMainDialog->processOpenLink(iTargetDlg, sUrl, bVisible);
	}
}

void MIhmDesktopIntf::processOpenAboutDlgReq(QString sAboutDialogCfg)
{
	if (m_pMainDialog != NULL)
		m_pMainDialog->openAboutDlg(sAboutDialogCfg);
}

void MIhmDesktopIntf::processCloseAboutDlgReq()
{
	if (m_pMainDialog != NULL)
		m_pMainDialog->closeAboutDlg();
}



void MIhmDesktopIntf::processOpenLoginDlgReq(QString sLoginId, bool bLoginFieldEnabled, bool bLDAPOffline, bool bShow)
{
	TRACE_D(QString("MIhmDesktopIntf::processOpenLoginDlgReq..."));
	
	if(m_pMainDialog!=NULL && !isIdentified() && (!isInControl()||!bShow))
	{
		//used to init take control from the main application
		//when the user uses the ID card, tag or bar code
		m_pMainDialog->openLoginDlg(sLoginId, bLoginFieldEnabled, bLDAPOffline, bShow);
	}
	else
	{
		if(m_pMainDialog!=NULL)
		{
			TRACE_W(QString("MIhmDesktopIntf::processOpenLoginDlgReq: Command IGNORED! It is received while the desktop user is identified!"));
		}
		else
		{
			TRACE_W(QString("MIhmDesktopIntf::processOpenLoginDlgReq: Command IGNORED! Main dialog not initialized!"));
		}
	}
	
}


//-------------------------------------------------------------------------
void MIhmDesktopIntf::processConfig(MIhmVirtDynConfig* pVirtObj)
{
	TRACE_D(QString("MIhmDesktopIntf::processConfig:VirtObjId:%1").arg(pVirtObj->getId()));

	switch(pVirtObj->getId())
	{
		case MIhmVirtualObject::enuIhmVirtConfProjectName:
		case MIhmVirtualObject::enuIhmVirtConfProjectVersion:
				//project name and/or version updated
				processConfigPrjNameAndVersion();
			break;		
		case MIhmVirtualObject::enuIhmVirtConfLanguage:
				processConfigLanguage(pVirtObj);
			break;		
		case MIhmVirtualObject::enuIhmVirtConfLaneType:
				processConfigLaneType(pVirtObj);
			break;		
		case MIhmVirtualObject::enuIhmVirtConfShiftType:
				processConfigShiftType(pVirtObj);
			break;		
		case MIhmVirtualObject::enuIhmVirtConfMainDlgHidden:
				processConfigMainDlgHidden(pVirtObj);
			break;
		default:
			break;

	}
}

void MIhmDesktopIntf::processConfigShiftType(MIhmVirtDynConfig* pVirtObj)
{
	QString sNewShiftType;
	
	sNewShiftType = pVirtObj->m_sDynConfigSetting;

	if(m_pMainDialog!=NULL)
	{
		m_pMainDialog->processNewShiftType(sNewShiftType);
	}
	
	pVirtObj->clearUpdated();
}

void MIhmDesktopIntf::processConfigPrjNameAndVersion()
{
	QString sNewPrjName;
	QString sNewPrjVersion;
	MIhmVirtDynConfig* pPrjName;
	MIhmVirtDynConfig* pPrjVersion;


	pPrjName = (MIhmVirtDynConfig*)findVirtualObjectByID(MIhmVirtualObject::enuIhmVirtConfProjectName);

	if(pPrjName!=NULL)
	{
		sNewPrjName = pPrjName->m_sDynConfigSetting;
		pPrjName->clearUpdated();
	}
	
	pPrjVersion = (MIhmVirtDynConfig*)findVirtualObjectByID(MIhmVirtualObject::enuIhmVirtConfProjectVersion);

	if(pPrjVersion!=NULL)
	{
		sNewPrjVersion = pPrjVersion->m_sDynConfigSetting;
		pPrjVersion->clearUpdated();
	}

	if(m_pMainDialog!=NULL)
	{
		m_pMainDialog->processPrjNameAndVersion(sNewPrjName,sNewPrjVersion);
	}

}

void MIhmDesktopIntf::processConfigLanguage(MIhmVirtDynConfig* pVirtObj)
{
	QString sNewLanguage;
	
	sNewLanguage = pVirtObj->m_sDynConfigSetting;

	//try load language
	if(MIhmConfigGeneral::getCfg()->getLanguages()->setLanguage(sNewLanguage))
	{
		//set the flag to refresh all objects
		m_bRefreshAllObjects = true;
	}
	else
	{
		//ignore the change and keep the old lane type
        TRACE_W(QString( "MIhmDesktopIntf::processConfigLanguage: Unable to initialize Language:%1")
									.arg(sNewLanguage));

	}

	pVirtObj->clearUpdated();
}

void MIhmDesktopIntf::processConfigLaneType(MIhmVirtDynConfig* pVirtObj)
{
	bool bRetVal;
	QString sNewLaneType;
	
	sNewLaneType = pVirtObj->m_sDynConfigSetting;
	
	if(sNewLaneType=="")
	{
        TRACE_W(QString("MIhmDesktopIntf::processConfigLaneType: Unable to initialize empty LaneType!"));
		return;
	}
	
    TRACE_D(QString("MIhmDesktopIntf::processConfigLaneType: Initializing LaneType:[%1]!").arg(sNewLaneType));

	//recreate the 	dialog
	if(m_pMainDialog!=NULL)
	{
		delete m_pMainDialog;
		m_pMainDialog = NULL;
	}

//------------------------------------------------------------------------------
	int target_screen_index = MIhmConfigGeneral::getCfg()->getStartupScreenIndex();//1 is an example, this is 2nd screen, because screens are numbered from 0
	
	QScreen *parent_screen = qApp->primaryScreen();  //deskWidget;
	QList<QScreen*> lstSecreens = qApp->screens();

	if (lstSecreens.count() <= target_screen_index)
	{
		target_screen_index = 0;
	}
	
	if (lstSecreens.count()>1)
	{
		TRACE_D(QString("MIhmDesktopIntf::processConfigLaneType: For Virtual desktop - parent screen is NULL!"));
		
		QScreen* screen = NULL;
		
		for (int i=0; i < lstSecreens.count();i++)
		{
			screen = lstSecreens.at(i);

			if (screen != NULL)
			{
				QRect rect = screen->availableGeometry();
				TRACE_D(QString("MIhmDesktopIntf::processConfigLaneType: screen index:%1 - screen geometry:x:%2 y:%3 w:%4 h:%5 ")
					.arg(i)
					.arg(rect.x()).arg(rect.y())
					.arg(rect.width()).arg(rect.height()));

				if (target_screen_index == i)
				{
					parent_screen = screen;
					break;
				}

			}
		}
		

	}


	m_pMainDialog = new MFormMain();
	m_pMainDialog->setScreen(parent_screen);
//---------------------------------------------------------------------



	//m_pMainDialog = new MFormMain();
	QObject::connect(m_pMainDialog,SIGNAL(exitProgram()),this, SIGNAL(exitProgram()));
	QObject::connect(m_pMainDialog,SIGNAL(keyDetected(int)),this, SIGNAL(keyDetected(int)));
	QObject::connect(m_pMainDialog,SIGNAL(stringDetected(MIhmMsgStringDetection *)),this, SIGNAL(stringDetected(MIhmMsgStringDetection *)));
	QObject::connect(m_pMainDialog,SIGNAL(action(QString, QString)),this, SLOT(onAction(QString, QString)));
	QObject::connect(m_pMainDialog,SIGNAL(aboutDlgAction(QString, QString)),this, SIGNAL(aboutDlgAction(QString, QString)));
	
	QObject::connect(m_pMainDialog,SIGNAL(refreshReq()),this, SLOT(onRefreshAllReq()));

	QObject::connect(m_pMainDialog,SIGNAL(idleTimeout()),this, SLOT(onDskIdleTimeout()));
	QObject::connect(m_pMainDialog,SIGNAL(identificationReq(QString,QString,bool)),this, SLOT(onIdentificationReq(QString,QString,bool)));
	QObject::connect(m_pMainDialog,SIGNAL(authDlgCanceled(int)),this, SIGNAL(authDlgCanceled(int)));

	
	QObject::connect(m_pMainDialog,SIGNAL(takeOverReject()),this, SLOT(onTakeOverReject()));
	QObject::connect(m_pMainDialog,SIGNAL(takeOverAccept()),this, SLOT(onTakeOverAccept()));
	QObject::connect(m_pMainDialog,SIGNAL(takeOverCancel()),this, SLOT(onTakeOverCancel()));
	

	//try load template form for this lane type
	bRetVal = m_pMainDialog->initialize(this, sNewLaneType);
	
	if(bRetVal)
	{
		//set the flag to refresh all objects
		m_bRefreshAllObjects = true;
		
		//if splash visible hide it and show main form
		setStatus(enuDskInitialized);
	}
	else
	{
		//ignore the change and keep the old lane type
        TRACE_W(QString( "MIhmDesktopIntf::processConfigLaneType: Unable to initialize Lane Type:%1")
									.arg(sNewLaneType));
		
		delete m_pMainDialog;
		m_pMainDialog = NULL;
		setStatus(enuDskInitializing);
	}

	pVirtObj->clearUpdated();
}

void MIhmDesktopIntf::processConfigMainDlgHidden(MIhmVirtDynConfig* pVirtObj)
{
	QString sNewState;

	sNewState = pVirtObj->m_sDynConfigSetting;

	if (sNewState.toInt() == 1)
	{
		if (m_pMainDialog != NULL)
		{
			TRACE_D(QString("MIhmDesktopIntf::setStatus: enuInitialized: Calling m_pMainDialog->hide..."));
			m_pMainDialog->showMainDlg(false);
		}
	}
	else
	{
		if (m_pMainDialog != NULL)
		{
			TRACE_D(QString("MIhmDesktopIntf::setStatus: enuInitialized: Calling m_pMainDialog->show..."));
			m_pMainDialog->showMainDlg(true);
		}
	}

	pVirtObj->clearUpdated();
}


void MIhmDesktopIntf::processInputDialogReq(MIhmVirtInputDialogReq* pVirtObj)
{
	if(m_pMainDialog!=NULL && isInControl())
	{
		m_pMainDialog->processInputDialogReq(pVirtObj);
	}

	pVirtObj->clearUpdated();
}

void MIhmDesktopIntf::sendMsgToAni(MIhmMsg *pMsg)
{
	emit msgToAni(pMsg);
}

void MIhmDesktopIntf::sendInputDialogResponse(MIhmMsg *pMsg)
{
	emit inputDialogRsp(pMsg);
}

void MIhmDesktopIntf::sendAsyncInputResponse(MIhmMsg *pMsg)
{
	emit asyncInputRsp(pMsg);
}

void MIhmDesktopIntf::sendVideoCtlResponse(MIhmMsg *pMsg)
{
	emit videoCtlRsp(pMsg);
}



void MIhmDesktopIntf::processInputDlgExReq(MIhmVirtInputDlgExReq* pVirtObj)
{
	if(m_pMainDialog!=NULL && isInControl())
	{
		m_pMainDialog->processInputDlgExReq(pVirtObj);
	}

	pVirtObj->clearUpdated();
}


void MIhmDesktopIntf::processAsyncInput(MIhmVirtAsyncInput* pVirtObj)
{
	if(m_pMainDialog!=NULL)
	{
		m_pMainDialog->processAsyncInput(pVirtObj);
	}

	pVirtObj->clearUpdated();
}



/*!
	Triggered on refresh key pressed. Refreshes all objects.
*/
void MIhmDesktopIntf::onRefreshAllReq()
{
    TRACE_D(QString("MIhmDesktopIntf::onRefreshAllReq:Refreshing all object on Refresh key request!"));
	processRefreshAllObjects();
}

void MIhmDesktopIntf::onAction(QString sAction, QString sParams)
{
	if(sAction == SPECIFIC_ACTION_BTN_END_IDENTIF) //for return control request
	{
		if(isReturnCtrlBtnEnabled())
		{
			if(m_eAuthStatus == enuInControl)
				emit returnCtrlReq(m_sIdentSessionID, (int)enuRETURN_CTRL_VOLUNTARY);
			else
				emit endIdentificationReq(m_sIdentSessionID, (int)enuRETURN_CTRL_VOLUNTARY);
		}
	}
	else if(sAction == SPECIFIC_ACTION_BTN_IDENTIFICATION)
	{
		if(m_eAuthStatus == enuNotIdentified)
		{	//verify that the TAKE control button is enabled (since the VALID button can also be used to initiate take control)
			if(isTakeCtrlBtnEnabled())
				emit action(sAction, sParams);
		}
		else if(enuIdentifiedReadonly)
			takeCtrlReq(m_sIdentSessionID);
	}
	else
		emit action(sAction, sParams);
}

bool MIhmDesktopIntf::isTakeCtrlBtnEnabled()
{
	MIhmVirtButton* pVirtObjTakeCtrl = (MIhmVirtButton*)findVirtualObjectByID(MIhmVirtualObject::enuIhmVirtBtnTakeCtrl);
	bool bTakeCtrlEnabled = true;

	if(pVirtObjTakeCtrl!=NULL)
		bTakeCtrlEnabled = pVirtObjTakeCtrl->isEnabled();

	return bTakeCtrlEnabled;

}

bool MIhmDesktopIntf::isReturnCtrlBtnEnabled()
{
	MIhmVirtButton* pVirtObjTakeCtrl = (MIhmVirtButton*)findVirtualObjectByID(MIhmVirtualObject::enuIhmVirtBtnReturnCtrl);
	bool bTakeCtrlEnabled = true;
	
	if(pVirtObjTakeCtrl!=NULL)
		bTakeCtrlEnabled = pVirtObjTakeCtrl->isEnabled();
	
	return bTakeCtrlEnabled;
	
}


void MIhmDesktopIntf::onIdentificationReq(QString sName, QString sSecCode, bool bLDAPOffline)
{
	if(!isIdentified() && !isInControl())
	{
		emit identificationReq(sName, sSecCode, bLDAPOffline);
	}
	else
	{
		TRACE_W(QString( "MIhmDesktopIntf::onIdentificationReq: Error:Already identified or in control!"));
	}
}

void MIhmDesktopIntf::onDskIdleTimeout()
{

	if(m_eAuthStatus != enuNotIdentified)
	{
		TRACE_D(QString( "MIhmDesktopIntf::onDskIdleTimeout:..."));
		emit endIdentificationReq(m_sIdentSessionID, (int)enuRETURN_CTRL_IDLE_TIMEOUT);
	}
	else
	{
		TRACE_W(QString( "MIhmDesktopIntf::onDskIdleTimeout: Error:Not identified or in control!"));
	}

}


//-----------------------------------------------
//   Taking control ...
//-----------------------------------------------


void MIhmDesktopIntf::cancelTakeOver()
{
	if(m_pMainDialog!=NULL)
	{
		m_pMainDialog->cancelTakeOver();
	}
}



void MIhmDesktopIntf::initTakeOver(bool bQuest, QString sTakeOverUserName)
{
	if(m_pMainDialog!=NULL)
	{
		m_pMainDialog->initTakeOver(bQuest, sTakeOverUserName);
	}
}


void MIhmDesktopIntf::showClientInfoMsg(QString sMsg, QString sMsgTitle)
{
	if(m_pMainDialog!=NULL)
	{
		m_pMainDialog->showClientInfoMsg(sMsg, sMsgTitle);
	}
}

void MIhmDesktopIntf::hideClientInfoMsg()
{
	if(m_pMainDialog!=NULL)
	{
		m_pMainDialog->hideClientInfoMsg();
	}
}

void MIhmDesktopIntf::showConnectingMsg(QString sMsg, QString sMsgTitle)
{
	if(m_pMainDialog!=NULL)
	{
		m_pMainDialog->showConnectingMsg(sMsg, sMsgTitle);
	}
}

void MIhmDesktopIntf::hideConnectingMsg()
{
	if(m_pMainDialog!=NULL)
	{
		m_pMainDialog->hideConnectingMsg();
	}
}



void MIhmDesktopIntf::onTakeOverCancel()
{
	emit takeOverCancel();
}

void MIhmDesktopIntf::onTakeOverReject()
{
	emit takeOverReject();
}

void MIhmDesktopIntf::onTakeOverAccept()
{
	emit takeOverAccept();
}


//get lane type to resolve the parameter
QString MIhmDesktopIntf::getCurrentLaneTypeParam(QString sParamName)
{
	QString sRetVal;QString sLaneType;

	MIhmVirtDynConfig * pVirtLaneType = (MIhmVirtDynConfig *)findVirtualObjectByID(MIhmVirtualObject::enuIhmVirtConfLaneType);

	if(pVirtLaneType!=NULL)
	{
		sLaneType = pVirtLaneType->m_sDynConfigSetting;

		if(sLaneType.isEmpty())
			return QString();
	}
		

	MIhmLaneTypeSettings * pSettings = 
		MIhmConfigGeneral::getCfg()->getLaneTypeSetting(MIhmLaneTypeSettings::enuDESKTOP, sLaneType);
	
	if(pSettings!=NULL)
	{
		sRetVal = pSettings->getParam(sParamName);
	}
	else
	{
		TRACE_W(QString( "MIhmDesktopIntf::getLaneTypeParam: Error loading lane type: %1")
									.arg(sLaneType));
		return QString();
	}
	
	return sRetVal;
}


QString MIhmDesktopIntf::getLaneTypeParam(QString sLaneType, QString sParamName)
{
	QString sRetVal;

	MIhmLaneTypeSettings * pSettings = 
		MIhmConfigGeneral::getCfg()->getLaneTypeSetting(MIhmLaneTypeSettings::enuDESKTOP, sLaneType);
	
	if(pSettings!=NULL)
	{
		sRetVal = pSettings->getParam(sParamName);
	}
	else
	{
		TRACE_W(QString("MIhmDesktopIntf::getLaneTypeParam: Error loading lane type: %1")
									.arg(sLaneType));
		return "";
	}
	
	return sRetVal;
}


void MIhmDesktopIntf::resetIdleTimer()
{
	if(m_pMainDialog!=NULL)
	{
		m_pMainDialog->resetIdleTimer();
	}
}

