#include <QtCore>
#include <QtGui>
//#include <QtGui/QHBoxLayout>
#include <QtUiTools>
#include <QFont>
#include <QFileInfo>

#include "MTracer.h"


#include "MFormMain.h"
#include "MIhmDskVisibleObject.h"
#include "MIhmVirtualObject.h"
#include "MIhmDesktopIntf.h"
#include "MIhmLaneTypeSettings.h"
#include "MIhmConfigGeneral.h"
#include "MIhmLanguages.h"

#include "MDigitalClock.h"
#include "MStringDetection.h"
#include "MDskAsyncInputs.h"
#include "MIhmMsg.h"
#include "MIhmMsgVideo.h"
#include "MLibToggleKeys.h"
#include "MFormAbout.h"
#include "MFormInput.h"
#include "MFormInputGen.h"
#include "MFormLogin.h"
#include "MHelpFuncs.h"
#include "MFormFileOpen.h"
#include "MFormTestKeyboard.h"
#include "MFormCustomPluginDlg.h"
#include "MHMenuView.h"
#include "MTabView.h"
#include "MListWidget.h"
#include "MRowsCommand.h"




MFormMain::MFormMain(QWidget *parent)
    : QWidget(parent)
{
	m_bTemplateLoaded = false;

	m_layout = NULL;
	m_mainFrame = NULL;
	m_pDskIntf = NULL;
	m_DigitalClock = NULL;
	m_ptimNumLock = NULL;
	m_pStringDetection = new MStringDetection();
	m_pDskAsyncInputs = new MDskAsyncInputs(this);

	m_pAboutDlg = NULL;
	m_pLoginDialog = NULL;
	m_pCurrentClientDlg = NULL;

	m_pTakeOverMsg = NULL; //enuTakeOverMsg
	m_pTakeOverQuestion = NULL; //enuTakeOverQuestion
	m_pClientInfoMsg = NULL; //enuClientInfoMsg
	m_pClientConnectingMsg = NULL; //enuClientConnectingMsg

	m_pInputDialog = NULL;
	m_pInputDlgEx = NULL;
	m_pModalWin = NULL;
	m_pIdleTimer = NULL;

	m_bDoNotFilter = false;
	
	if(MIhmConfigGeneral::getCfg()->m_bHideCursor)
		this->setCursor(Qt::BlankCursor);
	else
		this->setCursor(Qt::ArrowCursor);

	initializeVisibleObjects();

}

MFormMain::~MFormMain()
{
	setModalWindow(NULL);
	

	if(m_ptimNumLock != NULL)
	{
		m_ptimNumLock->stop();
		m_ptimNumLock = NULL;
		delete m_ptimNumLock;
	}

	if(m_pIdleTimer != NULL)
	{
		m_pIdleTimer->stop();
		m_pIdleTimer = NULL;	
		delete m_pIdleTimer;
	}

	//clear visible objects vector
	for(int i=MIhmDskVisibleObject::enuIhmVisibleObjUnknown+1;i<m_vectDskVisibleObjects.size();i++)
	{
		if(m_vectDskVisibleObjects.at(i)!=NULL)
			delete m_vectDskVisibleObjects.at(i);
	}
	
	m_vectDskVisibleObjects.clear();

 	if(m_pCurrentClientDlg!=NULL)
 	{
 		m_pCurrentClientDlg->close();
		m_pCurrentClientDlg = NULL;
	}

	if(m_pInputDialog!=NULL)
	{
		m_pInputDialog->close();
		delete m_pInputDialog;
		m_pInputDialog = NULL;
	}

	if(m_pInputDlgEx!=NULL)
	{
		m_pInputDlgEx->close();
		delete m_pInputDlgEx;
		m_pInputDlgEx = NULL;
	}
	
	if(m_pLoginDialog!=NULL)
	{
		m_pLoginDialog->close();
		delete m_pLoginDialog;
		m_pLoginDialog = NULL;
	}


	
	delete m_pTakeOverMsg;m_pTakeOverMsg = NULL; //enuTakeOverMsg
	delete m_pTakeOverQuestion; m_pTakeOverQuestion = NULL;//enuTakeOverQuestion
	delete m_pClientInfoMsg;m_pClientInfoMsg = NULL; //enuClientInfoMsg
	delete m_pClientConnectingMsg;m_pClientConnectingMsg = NULL; //enuClientConnectingMsg


	if(m_layout!=NULL)
	{
		delete m_layout;
		m_layout = NULL;
	}
	
	if(m_mainFrame!=NULL)
	{
		delete m_mainFrame;
		m_mainFrame = NULL;
	}

	if(m_pDskAsyncInputs!=NULL)
	{
		delete m_pDskAsyncInputs;
		m_pDskAsyncInputs = NULL;
	}

	if(m_pStringDetection!=NULL)
	{
		delete m_pStringDetection;
		m_pStringDetection = NULL;
	}

}


void MFormMain::center()
{
	//make the window appear at the center
	QScreen *deskWidget = qApp->primaryScreen();
	QRect screenRect = deskWidget->availableGeometry();
	QRect rectTarget = this->geometry();
	
	
	int x = (screenRect.width() - rectTarget.width())/2;
	int y = (screenRect.height() - rectTarget.height())/2; 
	
	x = (x>0)?x:0;
	y = (y>0)?y:0;

	TRACE_D(QString("MFormMain::center: Screen:%1 x %2, form:%3 x %4, position:X:%5 Y:%6")
									.arg(screenRect.width())
									.arg(screenRect.height())
									.arg(rectTarget.width())
									.arg(rectTarget.height())
									.arg(x)
									.arg(y));

	this->move(x,y);
}

void MFormMain::showMainDlg(bool bShow)
{

	//TO DO see what is actualy vidsible and handle its status...
	if (m_pAboutDlg != NULL)
		if(bShow)
			m_pAboutDlg->show();
		else 
			m_pAboutDlg->hide();

	if (m_pLoginDialog != NULL)
		if (bShow)
			m_pLoginDialog->show();
		else
			m_pLoginDialog->hide();

	if (m_pCurrentClientDlg != NULL)
		if (bShow)
			m_pCurrentClientDlg->show();
		else
			m_pCurrentClientDlg->hide();

	if (m_pInputDialog != NULL)
		if (bShow)
			m_pInputDialog->show();
		else
			m_pInputDialog->hide();

	if (m_pInputDlgEx != NULL)
		if (bShow)
			m_pInputDlgEx->show();
		else
			m_pInputDlgEx->hide();

	//Show only if template correctly loaded
	if (bShow && m_bTemplateLoaded)
		show();
	else
		hide();


}


bool MFormMain::initialize(MIhmDesktopIntf *pParent, QString sLaneType)
{
	bool bRetVal = false;
	QString sDialogUI;

	m_pDskIntf = pParent;
	
	TRACE_D(QString("MFormMain::initialize: Loading lane type settings: %1").arg(sLaneType));

	//get the template form name for this setting
	MIhmLaneTypeSettings * pSettings = MIhmConfigGeneral::getCfg()->getLaneTypeSetting(MIhmLaneTypeSettings::enuDESKTOP, sLaneType);
	
	if(pSettings!=NULL)
	{
		sDialogUI = pSettings->getParam(CFG_LANE_TYPE_PARAM_MAIN_DIALOG);
		sDialogUI = MIhmConfigGeneral::getCfg()->getUIFullPath(sDialogUI);
	}
	else
	{
		TRACE_W(QString( "MFormMain::initialize: Error loading lane type: %1")
									.arg(m_sLaneType));
		return false;
	}
	
	m_sLaneType = sLaneType;

	//cash this setting to speed up keys processing
	m_bFilterStringDetectedKeys = MIhmConfigGeneral::getCfg()->getFilterStringDetectedKeys();

	bRetVal = loadTemplate(sDialogUI);
	
	if(bRetVal == true)
	{
		setAllVisibleObjIniSettings();
		checkResolution();

		updateAllVisibleObjStaticProp();
		initTakeControlButtons();
		
		prepareClientInfoDialogs();

		//load the clock and start it
		createClock();
		
		m_pStringDetection->initialize(MIhmConfigGeneral::getCfg()->getModuleConfigKey());

		//load values for NUMLOCK timer and start it
		createNumlockTimer();
		//load desktop idle timer - will be started when in control
		createIdleTimer();

		this->installEventFilter(this);
		setMouseTracking(true);

	}
	else
	{
		TRACE_W(QString( "MFormMain::initialize: Error loading template: %1 or invalid [%2] parameter for the %3 lane type!")
									.arg(sDialogUI)
									.arg(CFG_LANE_TYPE_PARAM_MAIN_DIALOG)
									.arg(m_sLaneType));
		return false;
	}

	updateAuthStatus();	



	return bRetVal;
}



void MFormMain::createClock()
{
	MIhmContainer * pCurrent;
	pCurrent = (MIhmContainer *)MIhmDskVisibleObject::findVisibleObjectByID(&m_vectDskVisibleObjects, MIhmDskVisibleObject::enuCLK_TIME);
	
	if(	pCurrent!=NULL)
	{	
		QFrame * pFrame = pCurrent->getFrame();

		m_DigitalClock = new MDigitalClock(pFrame);
		QString sTimeFormat = MIhmConfigGeneral::getCfg()->getDefaultTimeFormat();
		m_DigitalClock->setTimeFormat(sTimeFormat);

		QObject::connect(m_DigitalClock,SIGNAL(dateChange()),this, SLOT(dateChanged()));
	
		QHBoxLayout *layout = new QHBoxLayout(pFrame);
		layout->addWidget(m_DigitalClock);
		layout->setSpacing(0);
		layout->setContentsMargins(0,0,0,0);

		//if not hidden by registry specific lane type setting
		pCurrent->setVisible(pCurrent->isConfVisible());
		
		dateChanged(); //show the date
	}
	else
	{
		TRACE_W(QString( "MFormMain::createClock: Error visible object : %1 not found ")
									.arg(CLK_REG_VAL_TIME));
		emit exitProgram();
	}
}	


void MFormMain::dateChanged()
{
	MIhmLabel * pCurrent;
	pCurrent = (MIhmLabel *)MIhmDskVisibleObject::findVisibleObjectByID(&m_vectDskVisibleObjects, MIhmDskVisibleObject::enuCLK_DATE);
	
	if(pCurrent!=NULL)
	{	
		QLabel * pLabel = pCurrent->getLabel();

		QDate dt = QDate::currentDate();
		
		QString sDateFormat = MIhmConfigGeneral::getCfg()->getDefaultDateFormat();
		
		pLabel->setText(dt.toString(sDateFormat));//"dd/MM/yyyy"

		//if not hidden by registry specific lane type setting
		pCurrent->setVisible(pCurrent->isConfVisible());

		//pCurrent->repaintWidget();
	}
	else
	{
		TRACE_W(QString("MFormMain::dateChanged: Error visible object : %1 not found ")
									.arg(CLK_REG_VAL_DATE));

	}
}

void MFormMain::setAllVisibleObjIniSettings()
{
	for(int i=MIhmDskVisibleObject::enuIhmVisibleObjUnknown+1;i<m_vectDskVisibleObjects.size();i++)
	{
		if (m_vectDskVisibleObjects.at(i) != NULL)
			setVisibleObjIniSettings(m_vectDskVisibleObjects.at(i));
	}	

}


//-----------------------------------
// START Container processing 
//-----------------------------------


void MFormMain::updateContainer(MIhmDskVisibleObject::enumVisibleObjectId eId, bool bVisible, MIhmVirtContainer* pVirtObj)
{
	TRACE_D(QString( "MFormMain::updateContainerVisibility: Visible object [%1]...")
					.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));

	MIhmContainer * pCurrent;
	pCurrent = (MIhmContainer *)MIhmDskVisibleObject::findVisibleObjectByID(&m_vectDskVisibleObjects, eId);

	if(pCurrent==NULL)
	{
		TRACE_W(QString( "MFormMain::updateContainer: findVisibleObjectByID return NULL for the visible object name %1")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->getType()!=MIhmDskVisibleObject::enuIhmContainer)
	{
		TRACE_W(QString( "MFormMain::updateContainer: invalid visible object type:[%1]!")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}
	
	if(pCurrent->isNotUsed()) 
	{
		TRACE_D(QString( "MFormMain::updateContainer: Visible object [%1] is flagged NOT_USED in configuration!")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}
		

	//process POLICE properties (only for foreground and background color)
	if (!(pVirtObj->m_oPoliceValue.isEmpty()))
	{
		QString sStyle = pVirtObj->m_oPoliceValue.getColorStyle();

		if (sStyle != "")
		{
			sStyle = QString("#%1{%2}").arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)).arg(sStyle);
			TRACE_D(QString("MFormMain::updateContainer: setStyleSheet: %1")
				.arg(sStyle));

			QFrame * pFrame = pCurrent->getFrame();
			if (pFrame)
				pFrame->setStyleSheet(sStyle);
		}
	}

	pCurrent->setVisible(bVisible);
	
}

void MFormMain::processContainer(MIhmVirtContainer* pVirtObj)
{
	TRACE_D(QString( "MFormMain::processContainer: ..."));

	switch(pVirtObj->getId())
	{
		case MIhmVirtualObject::enuIhmVirtCntHeaderGroup:
			updateContainer(MIhmDskVisibleObject::enuCNT_HEADER_GROUP, pVirtObj->isVisible(), pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtCntInstructionGroup:
			updateContainer(MIhmDskVisibleObject::enuCNT_INSTR_GROUP, pVirtObj->isVisible(), pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtCntModeGroup:
			updateContainer(MIhmDskVisibleObject::enuCNT_MODE_GROUP, pVirtObj->isVisible(), pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtCntPaymentGroup:
			updateContainer(MIhmDskVisibleObject::enuCNT_PAYMENT_GROUP, pVirtObj->isVisible(), pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtCntTrsGroup:
			updateContainer(MIhmDskVisibleObject::enuCNT_TRS_GROUP, pVirtObj->isVisible(), pVirtObj);
			break;

		case MIhmVirtualObject::enuIhmVirtCntComments:
			{	
				//this container should be visible only if client is in control
				bool bVisible = pVirtObj->isVisible() && m_pDskIntf->isInControl();
				updateContainer(MIhmDskVisibleObject::enuCNT_CNT_COMMENTS, bVisible, pVirtObj);
			}
			break;
		case MIhmVirtualObject::enuIhmVirtCntCamera:
			{	
				//this container should be visible only if client is in control
				bool bVisible = pVirtObj->isVisible() && m_pDskIntf->isInControl();
				updateContainer(MIhmDskVisibleObject::enuCNT_CNT_CAMERA, bVisible, pVirtObj);
			}
			break;
		case MIhmVirtualObject::enuIhmVirtCntScanner:
			{	
				//this container should be visible only if client is in control
				bool bVisible = pVirtObj->isVisible() && m_pDskIntf->isInControl();
				updateContainer(MIhmDskVisibleObject::enuCNT_CNT_SCANNER, bVisible, pVirtObj);
			}
			break;

		case MIhmVirtualObject::enuIhmVirtCntSpare1:updateContainer(MIhmDskVisibleObject::enuCNT_CNT_SPARE1, pVirtObj->isVisible(), pVirtObj); break;
		case MIhmVirtualObject::enuIhmVirtCntSpare2:updateContainer(MIhmDskVisibleObject::enuCNT_CNT_SPARE2, pVirtObj->isVisible(), pVirtObj); break;
		case MIhmVirtualObject::enuIhmVirtCntSpare3:updateContainer(MIhmDskVisibleObject::enuCNT_CNT_SPARE3, pVirtObj->isVisible(), pVirtObj); break;
		case MIhmVirtualObject::enuIhmVirtCntSpare4:updateContainer(MIhmDskVisibleObject::enuCNT_CNT_SPARE4, pVirtObj->isVisible(), pVirtObj); break;
		case MIhmVirtualObject::enuIhmVirtCntSpare5:updateContainer(MIhmDskVisibleObject::enuCNT_CNT_SPARE5, pVirtObj->isVisible(), pVirtObj); break;
		case MIhmVirtualObject::enuIhmVirtCntSpare6:updateContainer(MIhmDskVisibleObject::enuCNT_CNT_SPARE6, pVirtObj->isVisible(), pVirtObj); break;
		case MIhmVirtualObject::enuIhmVirtCntSpare7:updateContainer(MIhmDskVisibleObject::enuCNT_CNT_SPARE7, pVirtObj->isVisible(), pVirtObj); break;
		case MIhmVirtualObject::enuIhmVirtCntSpare8:updateContainer(MIhmDskVisibleObject::enuCNT_CNT_SPARE8, pVirtObj->isVisible(), pVirtObj); break;
		case MIhmVirtualObject::enuIhmVirtCntSpare9:updateContainer(MIhmDskVisibleObject::enuCNT_CNT_SPARE9, pVirtObj->isVisible(), pVirtObj); break;
		case MIhmVirtualObject::enuIhmVirtCntSpare10:updateContainer(MIhmDskVisibleObject::enuCNT_CNT_SPARE10, pVirtObj->isVisible(), pVirtObj); break;
		case MIhmVirtualObject::enuIhmVirtCntSpare11:updateContainer(MIhmDskVisibleObject::enuCNT_CNT_SPARE11, pVirtObj->isVisible(), pVirtObj); break;
		case MIhmVirtualObject::enuIhmVirtCntSpare12:updateContainer(MIhmDskVisibleObject::enuCNT_CNT_SPARE12, pVirtObj->isVisible(), pVirtObj); break;
		case MIhmVirtualObject::enuIhmVirtCntSpare13:updateContainer(MIhmDskVisibleObject::enuCNT_CNT_SPARE13, pVirtObj->isVisible(), pVirtObj); break;
		case MIhmVirtualObject::enuIhmVirtCntSpare14:updateContainer(MIhmDskVisibleObject::enuCNT_CNT_SPARE14, pVirtObj->isVisible(), pVirtObj); break;
		case MIhmVirtualObject::enuIhmVirtCntSpare15:updateContainer(MIhmDskVisibleObject::enuCNT_CNT_SPARE15, pVirtObj->isVisible(), pVirtObj); break;
		case MIhmVirtualObject::enuIhmVirtCntSpare16:updateContainer(MIhmDskVisibleObject::enuCNT_CNT_SPARE16, pVirtObj->isVisible(), pVirtObj); break;
		case MIhmVirtualObject::enuIhmVirtCntSpare17:updateContainer(MIhmDskVisibleObject::enuCNT_CNT_SPARE17, pVirtObj->isVisible(), pVirtObj); break;
		case MIhmVirtualObject::enuIhmVirtCntSpare18:updateContainer(MIhmDskVisibleObject::enuCNT_CNT_SPARE18, pVirtObj->isVisible(), pVirtObj); break;
		case MIhmVirtualObject::enuIhmVirtCntSpare19:updateContainer(MIhmDskVisibleObject::enuCNT_CNT_SPARE19, pVirtObj->isVisible(), pVirtObj); break;
		case MIhmVirtualObject::enuIhmVirtCntSpare20:updateContainer(MIhmDskVisibleObject::enuCNT_CNT_SPARE20, pVirtObj->isVisible(), pVirtObj); break;

		case MIhmVirtualObject::enuIhmVirtCntMiniWeb:
			updateContainer(MIhmDskVisibleObject::enuCNT_CNT_MINI_WEB, pVirtObj->isVisible(), pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtCntAlarmsGroup:
			updateContainer(MIhmDskVisibleObject::enuCNT_CNT_ALARMS_GROUP, pVirtObj->isVisible(), pVirtObj);
			break;
			
		default:
			TRACE_W(QString( "MFormMain::processContainer: Unknown virtual object id:%1")
									.arg(pVirtObj->getId()));

			break;
	}


}
//-----------------------------------
// END Container processing 
//-----------------------------------




//-----------------------------------
// START TabView processing 
//-----------------------------------


void MFormMain::updateTabView(MIhmDskVisibleObject::enumVisibleObjectId eId, QStringList& lstTabText, int iCurrentIndex ,bool bVisible)
{
	TRACE_D(QString( "MFormMain::updateTabView: ..."));

	MIhmTabView * pCurrent;
	pCurrent = (MIhmTabView *)MIhmDskVisibleObject::findVisibleObjectByID(&m_vectDskVisibleObjects, eId);

	if(pCurrent==NULL)
	{
		TRACE_W(QString( "MFormMain::updateTabView: findVisibleObjectByID return NULL for the visible object name %1")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->getType()!=MIhmDskVisibleObject::enuIhmTabView)
	{
		TRACE_W(QString("MFormMain::updateTabView: invalid visible object type:[%1]!")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}
	
	if(pCurrent->isNotUsed()) 
	{
		TRACE_D(QString("MFormMain::updateTabView: Visible object [%1] is flagged NOT_USED in configuration!")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	pCurrent->updateTab(lstTabText,iCurrentIndex);
	pCurrent->setVisible(bVisible);
	
}

void MFormMain::processTabView(MIhmVirtTabView* pVirtObj)
{
	TRACE_D(QString( "MFormMain::processTabView: ..."));

	switch(pVirtObj->getId())
	{
		case MIhmVirtualObject::enuIhmVirtTabControl1:
				updateTabView(MIhmDskVisibleObject::enuCNT_TAB_VIEW1, pVirtObj->m_slstTabText, pVirtObj->m_iCurrentIndex , pVirtObj->isVisible());
			break;
		case MIhmVirtualObject::enuIhmVirtTabControl2:
			updateTabView(MIhmDskVisibleObject::enuCNT_TAB_VIEW2, pVirtObj->m_slstTabText, pVirtObj->m_iCurrentIndex , pVirtObj->isVisible());
			break;
		case MIhmVirtualObject::enuIhmVirtTabControl3:
			updateTabView(MIhmDskVisibleObject::enuCNT_TAB_VIEW3, pVirtObj->m_slstTabText, pVirtObj->m_iCurrentIndex , pVirtObj->isVisible());
			break;
		case MIhmVirtualObject::enuIhmVirtTabControl4:
			updateTabView(MIhmDskVisibleObject::enuCNT_TAB_VIEW4, pVirtObj->m_slstTabText, pVirtObj->m_iCurrentIndex , pVirtObj->isVisible());
			break;
		default:
			TRACE_W(QString( "MFormMain::processTabView: Unknown virtual object id:%1")
									.arg(pVirtObj->getId()));

			break;
	}


}
//-----------------------------------
// END TabView processing 
//-----------------------------------

//-----------------------------------
// START Icons processing 
//-----------------------------------

void MFormMain::updateStaticImage(MIhmDskVisibleObject::enumVisibleObjectId eId, MIhmVirtIco* pVirtObj)
{
	TRACE_D(QString( "MFormMain::setStaticIconImage: ..."));

	MIhmIco * pCurrent;
	pCurrent = (MIhmIco *)MIhmDskVisibleObject::findVisibleObjectByID(&m_vectDskVisibleObjects, eId);

	if(pCurrent==NULL)
	{
		TRACE_W(QString( "MFormMain::updateStaticImage: findVisibleObjectByID return NULL for the visible object name %1")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}
	
	if(pCurrent->getType()!=MIhmDskVisibleObject::enuIhmIco)
	{
		TRACE_W(QString( "MFormMain::updateStaticImage: invalid visible object type:[%1]!")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->isNotUsed()) 
	{
		TRACE_D(QString( "MFormMain::updateStaticImage: Visible object [%1] is flagged NOT_USED in configuration!")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}
		
		
	QString sImageID = pVirtObj->m_sImageID;
	pCurrent->m_sImageName = sImageID;

	QLabel * pLabel = pCurrent->getLabel();
	if(pLabel!=NULL)
	{
		MIhmLanguages * pLang = MIhmConfigGeneral::getCfg()->getLanguages();
		
		QString sToolTip;
		pLang->getLabelTranslation(pVirtObj->getToolTip(), MIhmLanguages::enuTranslTargetDesktop, sToolTip);
		pLabel->setToolTip(sToolTip);
	}


	pCurrent->resetIcon();
	bool bIconLoaded = false;

	//Load pixmap and show icon at label if the visible object image name is correct
	//and if it points to the correct image file otherwise hide the label
	if(pCurrent->m_sImageName!="")
	{
		MIhmConfigImages * pImages = MIhmConfigGeneral::getCfg()->getDskConfigImages();
		
		bool bAnimate = false;
		QString sPath = pImages->getImageFullPath(pCurrent->m_sImageName, &bAnimate);

		TRACE_D(QString( "MFormMain::setStaticIconImage: loading image %1")
									.arg(sPath));

		//this should automatically add the pixmap to the  QPixmapCache 
		//if performance issues appear we should use QPixmapCache explicitly
		if(pCurrent->loadIcon(sPath, bAnimate))
		{
			bIconLoaded = true;
		}
		else
		{
			TRACE_W(QString( "MFormMain::setStaticIconImage: Error loading image: [%1] for the visible object name %2")
									.arg(sPath)
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));

			QList<QByteArray> sLst;

			sLst =	QImageReader::supportedImageFormats();

			QString sFormats;
			for(int i=0;i<sLst.size();i++)
			{
				sFormats += QString(sLst.at(i));
				sFormats += ", ";
			}
			
			TRACE_D(QString( "MFormMain::setStaticIconImage: supported image formats are: %1")
									.arg(sFormats));
		}

		TRACE_D(QString( "MFormMain::setStaticIconImage: ... Done..."));
	}

	//show if anything to show
	if(bIconLoaded && pVirtObj->isVisible())
	{
		//if not hidden by registry specific lane type setting
		pCurrent->setVisible(true);
	}
	else
		pCurrent->setVisible(false);

}


void MFormMain::processIcon(MIhmVirtIco* pVirtObj)
{
	TRACE_D(QString( "MFormMain::processIcon: ..."));
	
	switch(pVirtObj->getId())
	{
		case MIhmVirtualObject::enuIhmVirtIcoLaneStatus:
			updateStaticImage(MIhmDskVisibleObject::enuICO_STATUS, pVirtObj);
			break;

		case MIhmVirtualObject::enuIhmVirtIcoLaneMode:
			updateStaticImage(MIhmDskVisibleObject::enuICO_MODE, pVirtObj);

			break;
		case MIhmVirtualObject::enuIhmVirtIcoCollector:
			updateStaticImage(MIhmDskVisibleObject::enuICO_COLLECTOR, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtIcoEntryGate:
			updateStaticImage(MIhmDskVisibleObject::enuICO_ENTRYGATE, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtIcoExitGate:
			updateStaticImage(MIhmDskVisibleObject::enuICO_EXITGATE, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtIcoCar:
			updateStaticImage(MIhmDskVisibleObject::enuICO_CAR, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtIcoTrafficLight:
			updateStaticImage(MIhmDskVisibleObject::enuICO_TRAFFIC, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtIcoViolation:
			updateStaticImage(MIhmDskVisibleObject::enuICO_VIOLATION, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtIcoEntryLoop:
			updateStaticImage(MIhmDskVisibleObject::enuICO_ENTRYLOOP, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtIcoExitLoop:
			updateStaticImage(MIhmDskVisibleObject::enuICO_EXITLOOP, pVirtObj);
			break;

		case MIhmVirtualObject::enuIhmVirtIcoBeacon:
			updateStaticImage(MIhmDskVisibleObject::enuICO_ICO_BEACON, pVirtObj);
			break;

		case MIhmVirtualObject::enuIhmVirtIcoEntOpticalBarrier:
			updateStaticImage(MIhmDskVisibleObject::enuICO_ICO_OPT_BARR, pVirtObj);
			break;

		case MIhmVirtualObject::enuIhmVirtIcoSpare1: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE1, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare2: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE2, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare3: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE3, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare4: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE4, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare5: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE5, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare6: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE6, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare7: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE7, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare8: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE8, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare9: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE9, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare10: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE10, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare11: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE11, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare12: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE12, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare13: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE13, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare14: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE14, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare15: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE15, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare16: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE16, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare17: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE17, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare18: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE18, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare19: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE19, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare20: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE20, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare21: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE21, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare22: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE22, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare23: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE23, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare24: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE24, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare25: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE25, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare26: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE26, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare27: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE27, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare28: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE28, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare29: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE29, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare30: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE30, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare31: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE31, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare32: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE32, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare33: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE33, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare34: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE34, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare35: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE35, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare36: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE36, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare37: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE37, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare38: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE38, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare39: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE39, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare40: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE40, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare41: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE41, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare42: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE42, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare43: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE43, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare44: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE44, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare45: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE45, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare46: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE46, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare47: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE47, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare48: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE48, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare49: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE49, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare50: updateStaticImage(MIhmDskVisibleObject::enuICO_SPARE50, pVirtObj);	break;

		case MIhmVirtualObject::enuIhmVirtIcoCamera1:
			updateStaticImage(MIhmDskVisibleObject::enuICO_CAMERA1, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtIcoCamera2:
			updateStaticImage(MIhmDskVisibleObject::enuICO_CAMERA2, pVirtObj);
			break;

		case MIhmVirtualObject::enuIhmVirtIcoBeacon2:
			updateStaticImage(MIhmDskVisibleObject::enuICO_BEACON2, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtIcoSignalLight:
			updateStaticImage(MIhmDskVisibleObject::enuICO_SIGNAL_LIGHT, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtIcoExtOpticalBarrier:
			updateStaticImage(MIhmDskVisibleObject::enuICO_EXT_OPT_BARR, pVirtObj);
			break;

			//special image objects to be loaded directly from file not by ID
			// and to take care of show hide of container and label itself 
		case MIhmVirtualObject::enuIhmVirtImage:
			updateDynImage(pVirtObj->m_sImageID, pVirtObj->isVisible(), 
						MIhmDskVisibleObject::enuCNT_CNT_DYN_PICTURE, MIhmDskVisibleObject::enuCNT_LBL_DYN_IMAGE);
			break;
		case MIhmVirtualObject::enuIhmVirtImage2:
			updateDynImage(pVirtObj->m_sImageID, pVirtObj->isVisible(), 
						MIhmDskVisibleObject::enuCNT_CNT_DYN_PICTURE2, MIhmDskVisibleObject::enuCNT_LBL_DYN_IMAGE2);
			break;
		default:
			TRACE_W(QString( "MFormMain::processIcon: Unknown virtual object id:%1")
									.arg(pVirtObj->getId()));

			break;
	}
}


void MFormMain::updateDynImage(const QString &sImagePath, bool bShow, 
								MIhmDskVisibleObject::enumVisibleObjectId eVisContainerId, MIhmDskVisibleObject::enumVisibleObjectId eVisLabelId)
{
	TRACE_D(QString( "MFormMain::updateDynImage: in container %1")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eVisContainerId)));

	MIhmContainer * pPictureBox;
	pPictureBox = (MIhmContainer *)MIhmDskVisibleObject::findVisibleObjectByID(&m_vectDskVisibleObjects, eVisContainerId);

	MIhmIco * pImage;
	pImage = (MIhmIco *)MIhmDskVisibleObject::findVisibleObjectByID(&m_vectDskVisibleObjects, eVisLabelId);

	if(pPictureBox==NULL)
	{
		TRACE_W(QString("MFormMain::updateDynImage: findVisibleObjectByID return NULL for the visible object name %1")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eVisContainerId)));
		return;
	}
	else if(pImage==NULL)
	{
		TRACE_W(QString("MFormMain::updateDynImage: findVisibleObjectByID return NULL for the visible object name %1")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eVisLabelId)));
		return;
	}
	else if(pImage->getType()!=MIhmDskVisibleObject::enuIhmIco)
	{
		TRACE_W(QString("MFormMain::updateDynImage: invalid visible object type:[%1]!")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eVisLabelId)));
		return;
	}
	else if(pImage->isNotUsed() || pPictureBox->isNotUsed()) 
	{
		TRACE_D(QString( "MFormMain::updateDynImage: Visible objects [%1] or [%2] are flagged NOT_USED in configuration!")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eVisContainerId))
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eVisLabelId)));
		return;
	}
	else
	{
		pPictureBox->setVisible(bShow);

		if(bShow)
		{
			QLabel * pLabel = ((MIhmIco*)pImage)->getLabel();
			
			if(pLabel!=NULL)
			{
				pLabel->setScaledContents(true);
				pLabel->setVisible(true);
				bool bLoaded = false;

				if(sImagePath!="")
				{
					QFileInfo fi(sImagePath);
					
					if(fi.exists())
					{
						QPixmap oPixmap;
						bLoaded = oPixmap.load(sImagePath);
							
						if(bLoaded)
							pLabel->setPixmap(oPixmap);
						else					
						{
							TRACE_W(QString( "MFormMain::updateDynImage: Error loading picture: [%1]! QPixmap load failed!")
										.arg(sImagePath));
						}
					}
					else					
					{
						TRACE_W(QString( "MFormMain::updateDynImage: Error loading picture: [%1]! The file does not exist!")
									.arg(sImagePath));
					}
				}
				
				if(!bLoaded) //if nothing loaded load the default image
				{
					MIhmConfigImages * pImages = MIhmConfigGeneral::getCfg()->getDskConfigImages();
					QString sNoImagePath = pImages->getNoPictureImgPath();

					TRACE_D(QString("MFormMain::setStaticIconImage: loading default picture %1")
									.arg(sNoImagePath));

					QPixmap oPixmap;
					if(oPixmap.load(sNoImagePath))	
					{
						pLabel->setPixmap(oPixmap);
					}
					else
					{
						TRACE_W(QString( "MFormMain::updateDynImage: Error loading default picture:[%1] ")
									.arg(sNoImagePath));
					}
				}
			}
		}
	}
}



//-----------------------------------
// END Icons processing 
//-----------------------------------

//-----------------------------------
// START Buttons processing 
//-----------------------------------

void MFormMain::processButton(MIhmVirtButton* pVirtObj)
{
	switch(pVirtObj->getId())
	{
		case MIhmVirtualObject::enuIhmVirtBtnClass:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_CLASS, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnEntryPoint:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_ENTRY_POINT, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnEntryPointNbr:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_ENTRY_POINT_NBR, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnMode:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_MODE, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnArrowOn:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_ARROW_ON, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnArrowOff:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_ARROW_OFF, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnCrossOn:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_CROSS_ON, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnCrossOff:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_CROSS_OFF, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnDsrcOn:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_DSRC_ON, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnDsrcOff:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_DSRC_OFF, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnMagneticOn:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_MAGNET_ON, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnMagneticOff:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_MAGNET_OFF, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnCollectorOn:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_COLLECTOR_ON, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnCollectorOff:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_COLLECTOR_OFF, pVirtObj);
			break;

		case MIhmVirtualObject::enuIhmVirtBtnCamera:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_CAMERA, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnScanner:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SCANNER, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnComment:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_COMMENT, pVirtObj);
			break;

		case MIhmVirtualObject::enuIhmVirtBtnGabarite:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_GABARITE, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnEntryGate:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_ENTRY_GATE, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnExitGate:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_EXIT_GATE, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnCar3:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_CAR3, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnCar2:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_CAR2, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnCar:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_CAR, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnEntryLoop:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_ENTRY_LOOP, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnExitLoop:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_EXIT_LOOP, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnTblBeacon:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_TBL_BEACON, pVirtObj);
			break;

		case MIhmVirtualObject::enuIhmVirtBtnCamGrab:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_CAM_GRAB, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnCamHide:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_CAM_HIDE, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnScanGrab:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SCAN_GRAB, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnScanHide:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SCAN_HIDE, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnScanSend:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SCAN_SEND, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnScanSave:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SCAN_SAVE, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnCommHide:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_COMM_HIDE, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnExternalDisplay:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_EXTERNAL_DISPLAY, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnExternalDisplay2:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_EXTERNAL_DISPLAY2, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnTakeCtrl:
		case MIhmVirtualObject::enuIhmVirtBtnReturnCtrl:
				updateTakeControlButtons();
			break;

		case MIhmVirtualObject::enuIhmVirtButton1:
				updateButton(MIhmDskVisibleObject::enuCNT_PRODUCT_BUTTON1, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtButton2:
				updateButton(MIhmDskVisibleObject::enuCNT_PRODUCT_BUTTON2, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtButton3:
				updateButton(MIhmDskVisibleObject::enuCNT_PRODUCT_BUTTON3, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtButton4:
				updateButton(MIhmDskVisibleObject::enuCNT_PRODUCT_BUTTON4, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtButton5:
				updateButton(MIhmDskVisibleObject::enuCNT_PRODUCT_BUTTON5, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpeedLimitOn:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPEED_LIMIT_ON, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpeedLimitOff:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPEED_LIMIT_OFF, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnWarningOn:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_WARNING_ON, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnWarningOff:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_WARNING_OFF, pVirtObj);
			break;

		case MIhmVirtualObject::enuIhmVirtBtnDsrcTSAOn:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_DSRC_TSA_ON, pVirtObj);
			break;

		case MIhmVirtualObject::enuIhmVirtBtnDsrcTSAOff:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_DSRC_TSA_OFF, pVirtObj);
			break;

		case MIhmVirtualObject::enuIhmVirtBtnSpare1:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_1, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare2:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_2, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare3:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_3, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare4:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_4, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare5:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_5, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare6:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_6, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare7:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_7, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare8:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_8, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare9:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_9, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare10:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_10, pVirtObj);
			break;

		case MIhmVirtualObject::enuIhmVirtBtnSpare11:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_11, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare12:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_12, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare13:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_13, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare14:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_14, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare15:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_15, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare16:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_16, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare17:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_17, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare18:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_18, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare19:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_19, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare20:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_20, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare21:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_21, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare22:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_22, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare23:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_23, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare24:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_24, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare25:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_25, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare26:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_26, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare27:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_27, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare28:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_28, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare29:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_29, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare30:
				updateButton(MIhmDskVisibleObject::enuCNT_BTN_SPARE_30, pVirtObj);
			break;

		default:
			TRACE_W(QString( "MFormMain::processButton: unknown object ID: %1")
									.arg(pVirtObj->getId()));

			break;
	}	
}

void MFormMain::updateButton(MIhmDskVisibleObject::enumVisibleObjectId eId, MIhmVirtButton* pVirtObj)
{
	MIhmButton * pCurrent;

	pCurrent = (MIhmButton *)MIhmDskVisibleObject::findVisibleObjectByID(&m_vectDskVisibleObjects, eId);

	if(pCurrent==NULL)
	{
		TRACE_W(QString( "MFormMain::updateButton: findVisibleObjectByID return NULL for the visible object name %1")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}
	
	if(pCurrent->getType()!=MIhmDskVisibleObject::enuIhmButton)
	{
		TRACE_W(QString( "MFormMain::updateButton: invalid visible object type:[%1]!")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->isNotUsed()) 
	{
		TRACE_D(QString( "MFormMain::updateButton: Visible object [%1] is flagged NOT_USED in configuration!")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	MIhmLanguages * pLang = MIhmConfigGeneral::getCfg()->getLanguages();
	QString sTranslation;
	pLang->getLabelTranslation(pVirtObj->m_sTextID, MIhmLanguages::enuTranslTargetDesktop, sTranslation);


	MIhmConfigImages * pImages = MIhmConfigGeneral::getCfg()->getDskConfigImages();
	QString sImgPath = pImages->getImageFullPath(pVirtObj->m_sImageID);

	QString sToolTip;
	pLang->getLabelTranslation(pVirtObj->getToolTip(), MIhmLanguages::enuTranslTargetDesktop, sToolTip);

	pCurrent->updateButton(sTranslation, sToolTip, sImgPath, pVirtObj->isEnabled());
	pCurrent->setVisible(pVirtObj->isVisible());

	TRACE_D(QString("MFormMain::updateButton: %1 Transl:[%2] ToolTip:[%3] Img:[%4] Enabled[%5] Visible[%6] ")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId))
									.arg(sTranslation)
									.arg(sToolTip)
									.arg(sImgPath)
									.arg(pVirtObj->isEnabled())
									.arg(pVirtObj->isVisible()));

}

//-----------------------------------
// END Buttons processing 
//-----------------------------------


//-----------------------------------
// START file view processing 
//-----------------------------------

void MFormMain::processFileViewReq(MIhmVirtTableViewReq* pVirtObj)
{
	switch(pVirtObj->getId())
	{
		case MIhmVirtualObject::enuIhmVirtTableView1:
					updateFileView(MIhmDskVisibleObject::enuCNT_FILE_VIEW1, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTableView2:
					updateFileView(MIhmDskVisibleObject::enuCNT_FILE_VIEW2, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTableView3:
					updateFileView(MIhmDskVisibleObject::enuCNT_FILE_VIEW3, pVirtObj);
				break;
		default:
			TRACE_W(QString("MFormMain::processFileViewReq: unknown object ID: %1")
									.arg(pVirtObj->getId()));

			break;
	}	
}

void MFormMain::updateFileView(MIhmDskVisibleObject::enumVisibleObjectId eId, MIhmVirtTableViewReq* pVirtObj)
{
	MIhmFileView * pCurrent;
	TRACE_D(QString( "MFormMain::updateFileView: %1")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));

	pCurrent = (MIhmFileView *)MIhmDskVisibleObject::findVisibleObjectByID(&m_vectDskVisibleObjects, eId);

	if(pCurrent==NULL)
	{
		TRACE_W(QString( "MFormMain::updateFileView: findVisibleObjectByID return NULL for the visible object name %1")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->getType()!=MIhmDskVisibleObject::enuIhmFileView)
	{
		TRACE_W(QString("MFormMain::updateFileView: invalid visible object type:[%1]!")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->isNotUsed()) 
	{
		TRACE_D(QString("MFormMain::updateFileView: Visible object [%s] is flagged NOT_USED in configuration!")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}
	

	//verify if the visible object is up to date with the virtual object
	if(pVirtObj->getLastUpdate() != pCurrent->getLastCommandTimestamp() || 
		pVirtObj->getLastUpdate() == 0 ) // if last update is 0 it means that it should be refreshed
	{
		//process the commands 
		QList <MRowsCommand *> * pLst = pVirtObj->getRowCommandList();
		MRowsCommand * pCurrentCommand;

		if(pLst->count() > 0)
		{
			int i=0;
			bool bFoundTheLastUpdatedRow = false;
			quint64 uiCurrentLastUpdate = pCurrent->getLastCommandTimestamp();
			
			if(pVirtObj->getLastUpdate()!=0)
				if(uiCurrentLastUpdate!=0)
				{
					for(i=0; i<pLst->count();i++ )
					{
						pCurrentCommand = pLst->at(i);

						if(pCurrentCommand->m_uiCommandTimestamp == uiCurrentLastUpdate)
						{
							bFoundTheLastUpdatedRow = true;
							break;
						}
					}
				}
			
			if(bFoundTheLastUpdatedRow)
			{
				//update only rows that were changed in period from the last update
				i++;
				for(; i<pLst->count();i++ )
				{
					pCurrentCommand = pLst->at(i);
					pCurrent->executeRowCommand(&pCurrentCommand->m_lstRows, pCurrentCommand->m_eCmd, pCurrentCommand->m_uiCommandTimestamp);
				}
			}
			else
			{
				//reset the table and update all rows
				pCurrent->executeRowCommand(NULL, 	enuIHM_AFF_TABLE_CMD_RESET_ROWS, Q_UINT64_C(0));
				
				for(i=0; i<pLst->count();i++ )
				{
					pCurrentCommand = pLst->at(i);
					pCurrent->executeRowCommand(&pCurrentCommand->m_lstRows, pCurrentCommand->m_eCmd, pCurrentCommand->m_uiCommandTimestamp);
				}
			}
		}
		else
		{
			pCurrent->executeRowCommand(NULL, 	enuIHM_AFF_TABLE_CMD_RESET_ROWS, pVirtObj->getLastUpdate());
		}
	}
	
	pCurrent->setVisible(pVirtObj->isVisible());
}

//-----------------------------------
// END file view processing 
//-----------------------------------




//-----------------------------------
// START Hmenu view processing 
//-----------------------------------

void MFormMain::processHMenuViewReq(MIhmVirtHMenuViewReq* pVirtObj)
{
	switch(pVirtObj->getId())
	{
	case MIhmVirtualObject::enuIhmVirtHMenuView1:
		updateHMenuView(MIhmDskVisibleObject::enuCNT_HMENU_VIEW1, pVirtObj);
		break;
	case MIhmVirtualObject::enuIhmVirtHMenuView2:
		updateHMenuView(MIhmDskVisibleObject::enuCNT_HMENU_VIEW2, pVirtObj);
		break;
	default:
		TRACE_W(QString( "MFormMain::processHMenuViewReq: no action defined for the object ID: %1")
									.arg(pVirtObj->getId()));
		
		break;
	}	
}

void MFormMain::updateHMenuView(MIhmDskVisibleObject::enumVisibleObjectId eId, MIhmVirtHMenuViewReq* pVirtObj)
{
	MIhmHMenuView * pCurrent;
	TRACE_D(QString( "MFormMain::updateHMenuView: %1")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
	
	pCurrent = (MIhmHMenuView *)MIhmDskVisibleObject::findVisibleObjectByID(&m_vectDskVisibleObjects, eId);
	
	if(pCurrent==NULL)
	{
		TRACE_W(QString( "MFormMain::updateHMenuView: findVisibleObjectByID return NULL for the visible object name %1")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->getType()!=MIhmDskVisibleObject::enuIhmHMenuView)
	{
		TRACE_W(QString( "MFormMain::updateHMenuView: invalid visible object type:[%1]!")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->isNotUsed()) 
	{
		TRACE_D(QString( "MFormMain::updateHMenuView: Visible object [%1] is flagged NOT_USED in configuration!")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	
	pCurrent->updateMenu(pVirtObj->m_sMenuContent, pVirtObj->m_bUseExternalFile, pVirtObj->m_bDoNotReloadMenu, pVirtObj->isVisible());
	
}

//-----------------------------------
// END menu view processing 
//-----------------------------------

//-----------------------------------
// START video view processing 
//-----------------------------------

MIhmDskVisibleObject * MFormMain::getVisibleVideoObject(int eVirtObjet)
{
	MIhmDskVisibleObject *  pCurrent = NULL;

	MIhmDskVisibleObject::enumVisibleObjectId eTargetVisibleObjId = MIhmDskVisibleObject::enuIhmVisibleObjUnknown;

	switch((enum_ihm_objets)eVirtObjet)
	{
		case MIhmVirtualObject::enuIhmVirtRestreamVideoView1:
				eTargetVisibleObjId = MIhmDskVisibleObject::enuCNT_RVIDEO_VIEW1;
			break;
		case MIhmVirtualObject::enuIhmVirtRestreamVideoView2:
				eTargetVisibleObjId = MIhmDskVisibleObject::enuCNT_RVIDEO_VIEW2;
			break;
		default:
			TRACE_W(QString( "MFormMain::getVisibleVideoObject: invalid virt object id:[%1]!")
										.arg((int)eVirtObjet));
	}

	if(eTargetVisibleObjId != MIhmDskVisibleObject::enuIhmVisibleObjUnknown)
	{
		pCurrent = MIhmDskVisibleObject::findVisibleObjectByID(&m_vectDskVisibleObjects, eTargetVisibleObjId);
		
		if(pCurrent==NULL)
		{
			TRACE_W(QString( "MFormMain::getVisibleVideoObject: findVisibleObjectByID return NULL for the visible object name %1")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eTargetVisibleObjId)));
		}
		else if(pCurrent->isNotUsed()) 
		{
			TRACE_W(QString( "MFormMain::getVisibleVideoObject: Visible object [%1] is flagged NOT_USED in configuration!")
										.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eTargetVisibleObjId)));
			pCurrent = NULL;	
		}
		else if(pCurrent->getType()!=MIhmDskVisibleObject::enuIhmVideoView 
			&& pCurrent->getType()!=MIhmDskVisibleObject::enuIhmRestreamVideoView)
		{
			TRACE_W(QString( "MFormMain::getVisibleVideoObject: invalid visible object type:[%1]!")
										.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eTargetVisibleObjId)));
			pCurrent = NULL;
		}
	}
	else
	{
		TRACE_W(QString( "MFormMain::getVisibleVideoObject: Unable to find visible object for virtual object:[%1]!")
						.arg(MIhmVirtualObject::getNameForId((MIhmVirtualObject::enumVirtualObjectId)eVirtObjet)));	
	}
	
	return pCurrent;
}

void MFormMain::processVideoViewReq(MIhmVirtVideoViewReq* pVirtObj)
{
	switch(pVirtObj->getId())
	{
	case MIhmVirtualObject::enuIhmVirtRestreamVideoView1:
		updateRVideoView(MIhmDskVisibleObject::enuCNT_RVIDEO_VIEW1, pVirtObj);
		break;
	case MIhmVirtualObject::enuIhmVirtRestreamVideoView2:
		updateRVideoView(MIhmDskVisibleObject::enuCNT_RVIDEO_VIEW2, pVirtObj);
		break;
	default:
		TRACE_W(QString( "MFormMain::processVideoViewReq: no action defined for the object ID: %1")
									.arg(pVirtObj->getId()));
		break;
	}	
}


//------------------------------------------------------------------------------------
//    Restream video desktop implementation
//------------------------------------------------------------------------------------

void MFormMain::updateRVideoView(MIhmDskVisibleObject::enumVisibleObjectId eId, MIhmVirtVideoViewReq* pVirtObj)
{
	TRACE_D(QString( "MFormMain::updateRVideoView: %1 visible:%2, local source:[%3], remote source:[%4], videoOn:%5")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId))
									.arg((int)pVirtObj->isVisible())
									.arg(pVirtObj->m_sStreamSourceUrlLocal)
									.arg(pVirtObj->m_sStreamSourceUrlRemote)
									.arg((int)pVirtObj->m_bVideoOn));

	MIhmRestreamVideoView * pCurrent = (MIhmRestreamVideoView*)getVisibleVideoObject(pVirtObj->getId());
	
	if(pCurrent->getType()==MIhmDskVisibleObject::enuIhmRestreamVideoView)
	{
		pCurrent->setVisible(pVirtObj->isVisible());
		
		if(pCurrent->getSourceUrl()!=pVirtObj->m_sStreamSourceUrlLocal)
		{
			pCurrent->setSource(pVirtObj->m_sStreamSourceUrlLocal);
		}

		if(pVirtObj->m_bVideoOn)
			pCurrent->showVideo();
		else
			pCurrent->hideVideo();
	}
}

//-----------------------------------
// END video view processing 
//-----------------------------------


//-----------------------------------
// START Text labels processing 
//-----------------------------------
void MFormMain::processText(MIhmVirtText* pVirtObj)
{
	switch(pVirtObj->getId())
	{
		case MIhmVirtualObject::enuIhmVirtPlazaName:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_PLAZA_NAME, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtPlazaName1:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_PLAZA_NAME1, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtPlazaName2:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_PLAZA_NAME2, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtPlazaName3:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_PLAZA_NAME3, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtPlazaNameTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_PLAZA_NAME_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtPlazaNbr:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_PLAZA_NUMBER, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtPlazaNbrTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_PLAZA_NUMBER_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtLaneNbr:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_LANE_NUMBER, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtLaneNbrTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_LANE_NUMBER_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtLaneName:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_LANE_NAME, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtLaneName1:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_LANE_NAME1, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtLaneName2:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_LANE_NAME2, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtLaneNameTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_LANE_NAME_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtCollectorID:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_COL_ID, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtCollectorName:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_COL_NAME, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtTxtSpare1:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE1, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare2:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE2, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare3:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE3, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare4:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE4, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare5:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE5, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare6:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE6, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare7:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE7, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare8:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE8, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare9:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE9, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare10:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE10, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare11:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE11, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare12:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE12, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare13:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE13, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare14:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE14, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare15:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE15, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare16:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE16, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare17:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE17, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare18:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE18, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare19:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE19, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare20:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE20, pVirtObj);
				break;
//----------------------------------------------------------------
		case MIhmVirtualObject::enuIhmVirtTxtSpare21:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE21, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare22:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE22, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare23:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE23, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare24:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE24, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare25:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE25, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare26:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE26, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare27:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE27, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare28:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE28, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare29:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE29, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare30:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE30, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare31:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE31, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare32:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE32, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare33:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE33, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare34:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE34, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare35:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE35, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare36:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE36, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare37:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE37, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare38:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE38, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare39:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE39, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare40:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE40, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare41:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE41, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare42:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE42, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare43:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE43, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare44:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE44, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare45:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE45, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare46:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE46, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare47:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE47, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare48:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE48, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare49:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE49, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare50:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE50, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare51:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE51, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare52:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE52, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare53:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE53, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare54:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE54, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare55:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE55, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare56:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE56, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare57:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE57, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare58:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE58, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare59:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE59, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare60:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SPARE60, pVirtObj);
				break;




		case MIhmVirtualObject::enuIhmVirtTransactionNbr:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_TRANSACTIONNBR, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTransactionNbrTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_TRANSACTIONNBR_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtReceiptNbr:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_RECEIPTNBR, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtReceiptNbrTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_RECEIPTNBR_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtAxles:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_AXLES, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtAxlesTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_AXLES_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBalance:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_BALANCE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBalanceTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_BALANCE_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtClass:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_CLASS, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtClassTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_CLASS_TITLE, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtAddRevenue:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_ADD_REVENUE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtAddRevenueTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_ADD_REVENUE_TITLE, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtNbCar:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_NBCAR, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtEntry:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_ENTRY, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtEntryTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_ENTRY_TITLE, pVirtObj);
				break;
		
//NOT USED
// 		case MIhmVirtualObject::enuIhmVirtCurrency:
// 					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_CURRENCY, pVirtObj);
// 				break;
// 		case MIhmVirtualObject::enuIhmVirtCurrencyTitle:
// 					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_CURRENCY_TITLE, pVirtObj);
// 				break;

		case MIhmVirtualObject::enuIhmVirtFare:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_FARE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtFareTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_FARE_TITLE, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtSale:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SALE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtSaleTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SALE_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtSaleDue:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SALEDUE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtSaleDueTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SALEDUE_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtSaleDuePaid:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_SALEDUE_PAID, pVirtObj);
				break;


		case MIhmVirtualObject::enuIhmVirtTransactionType:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_TYPE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTransactionTypeTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_TYPE_TITLE, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtFareType:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_FARE_TYPE, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtFareTypeTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_FARE_TYPE_TITLE, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtTxtModeTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_TXT_MODE_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtMode:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_TXT_MODE, pVirtObj);
				break;
	
		case MIhmVirtualObject::enuIhmVirtTxtCollectorTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_TXT_COLLECTOR_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtInCtrlUserTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_TXT_IN_CTRL_USER_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtInCtrlUser:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_TXT_IN_CTRL_USER, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtInCtrlUser1:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_TXT_IN_CTRL_USER1, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtTrsContainerTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_TXT_TRS_CONTAINER_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtAlarmsTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_TXT_ALARMS_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtTrsHistoryTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_TXT_TRS_HISTORY_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtAlertesTitle:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_TXT_ALERTES_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtCamNumPic:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_TXT_CAM_NUM_PIC, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtScanNumPic:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_TXT_SCAN_NUM_PIC, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtNbCarSAS:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_TXT_NB_CARS_SAS, pVirtObj);
				break;

		//INSTRUCTIONS
		case MIhmVirtualObject::enuIhmVirtErrPayment:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_ERR_PAYMENT, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtPayment:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_PAYMENT, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtRemark:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_REMARK, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtInstruction:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_INSTR, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtInstruction2:
					updateTextLabel(MIhmDskVisibleObject::enuTEXT_LABEL_INSTR_2, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtCurrentDate:
					TRACE_W(QString( "MFormMain::processText: no action is to be performed for enuIhmVirtCurrentDate! It is updated with clock."));
				break;

		default:
			TRACE_W(QString( "MFormMain::processText: no action defined for the object ID: %1")
									.arg(pVirtObj->getId()));

			break;
	}	
}

void MFormMain::updateTextLabel(MIhmDskVisibleObject::enumVisibleObjectId eId, MIhmVirtText* pVirtObj)
{
	MIhmLabel * pCurrent;
	TRACE_D(QString( "MFormMain::updateTextLabel: %1 : visible:%2")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)).arg(pVirtObj->isVisible()));

	pCurrent = (MIhmLabel *)MIhmDskVisibleObject::findVisibleObjectByID(&m_vectDskVisibleObjects, eId);

	if(pCurrent==NULL)
	{
		TRACE_W(QString( "MFormMain::updateTextLabel: findVisibleObjectByID return NULL for the visible object name %1")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->getType()!=MIhmDskVisibleObject::enuIhmLabel)
	{
		TRACE_W(QString( "MFormMain::updateTextLabel: invalid visible object type:[%1]!")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}
	
	if(pCurrent->isNotUsed()) 
	{
		TRACE_D(QString( "MFormMain::updateTextLabel: Visible object [%1] is flagged NOT_USED in configuration!")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	MIhmLanguages * pLang = MIhmConfigGeneral::getCfg()->getLanguages();
	QString sTranslation;
	pLang->getLabelTranslation(pVirtObj->m_sTextID, MIhmLanguages::enuTranslTargetDesktop,sTranslation);
	
	QString sToolTip;
	pLang->getLabelTranslation(pVirtObj->getToolTip(), MIhmLanguages::enuTranslTargetDesktop, sToolTip);

	QLabel *lbl = pCurrent->getLabel();

	if(lbl!=NULL)
	{
		TRACE_D(QString( "MFormMain::updateTextLabel setText:[%1],translation:[%2]")
									.arg(pVirtObj->m_sTextID)
									.arg(sTranslation));
		lbl->setText(sTranslation);
		
		lbl->setToolTip(sToolTip);

		if(!(pVirtObj->m_oPoliceValue.isEmpty()))
		{
			QString sStyle;	

			if(pVirtObj->m_oPoliceValue.isFontValid())
			{
				sStyle = MIhmConfigGeneral::getCfg()->getPoliceFontStyle(&(pVirtObj->m_oPoliceValue));
			}

			QString sColorStyle = pVirtObj->m_oPoliceValue.getColorStyle();
			sStyle += sColorStyle;
			
			if(sStyle!="")
			{
				sStyle = QString("#%1{%2}").arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)).arg(sStyle);
				TRACE_D(QString( "MFormMain::updateTextLabel setStyleSheet: %1")
									.arg(sStyle));


				lbl->setStyleSheet(sStyle);
			}
		}

		//make the text object visible if any correct affichage was received
		if(pVirtObj->isVisible())
		{	
			lbl->setVisible(true);
		}
		else
			lbl->setVisible(false);
	}
	
}



//------------------------------------

//------------------------------------
// START List objects 
//------------------------------------

void MFormMain::processList(MIhmVirtList* pVirtObj)
{
	switch(pVirtObj->getId())
	{
		case MIhmVirtualObject::enuIhmVirtLstAlarms:
					updateListObject(MIhmDskVisibleObject::enuLIST_LIST_ALARMS, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtLstWarnings:
					updateListObject(MIhmDskVisibleObject::enuLIST_LIST_WARNINGS, pVirtObj);
				break;

		default:
			TRACE_W(QString( "MFormMain::processList: no action defined for the object ID: %1")
									.arg(pVirtObj->getId()));

			break;
	}

}

void MFormMain::updateListObject(MIhmDskVisibleObject::enumVisibleObjectId eId, MIhmVirtList* pVirtObj)
{
	MIhmList * pCurrent;
	TRACE_D(QString( "MFormMain::updateListObject: %1")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));

	pCurrent = (MIhmList *)MIhmDskVisibleObject::findVisibleObjectByID(&m_vectDskVisibleObjects, eId);

	if(pCurrent==NULL)
	{
		TRACE_W(QString( "MFormMain::updateListObject: findVisibleObjectByID return NULL for the visible object name %1")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->getType()!=MIhmDskVisibleObject::enuIhmList)
	{
		TRACE_W(QString( "MFormMain::updateListObject: invalid visible object type:[%1]!")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->isNotUsed()) 
	{
		TRACE_D(QString( "MFormMain::updateListObject: Visible object [%1] is flagged NOT_USED in configuration!")
									.arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}


	QListWidget *pListWidget = (QListWidget *)pCurrent->getList();

	if(pCurrent!=NULL)
	{
		pCurrent->setVisible(pVirtObj->isVisible());
			
		//Then update the list items
		QStringList lstToAdd; //items to add
		QStringList lstToRemove;
		bool bAddItems = false;

		//find items to be added ...
		for(int i=0;i<pVirtObj->m_slistIDs.size();++i)
		{
			if(pCurrent->m_sItemList.contains(pVirtObj->m_slistIDs.at(i)) == false)
			{
				lstToAdd.append(pVirtObj->m_slistIDs.at(i));//item added
				bAddItems = true;
			}
		}
		
		
		if(bAddItems)
		{
			TRACE_D(QString( "MFormMain::updateListObject: Items to be added: %1..").arg(lstToAdd.join(";")));
			pCurrent->addItemsToListView(lstToAdd);
		}
		
		bool bRecreate = false;
		//find items to be removed
		for(int i=0;i<pCurrent->m_sItemList.size();++i)
		{
			if(pVirtObj->m_slistIDs.contains(pCurrent->m_sItemList.at(i)) == false)
			{
				TRACE_D(QString( "MFormMain::updateListObject: Found item to be removed:%1. Doing recreate...").arg(pCurrent->m_sItemList.at(i)));
				pCurrent->m_sItemList.removeAt(i);
				i=i-1;
				bRecreate = true;
				break;
			}
		}
		
		if (!bAddItems && !bRecreate) //meaning nothing is added and nothing is removed => it might be refresh request because of language change
			bRecreate = true;

		if(bRecreate)
		{
			pCurrent->clear();
			TRACE_D(QString( "MFormMain::updateListObject: All items to be added: %1..").arg(pVirtObj->m_slistIDs.join(";")));
			pCurrent->addItemsToListView(pVirtObj->m_slistIDs);
		}


		
		//process POLICE properties (only for foreground and background color)
		if(!(pVirtObj->m_oPoliceValue.isEmpty()))
		{
			QString sStyle = pVirtObj->m_oPoliceValue.getColorStyle();

			if(sStyle!="")
			{
				sStyle = QString("#%1{%2}").arg(MIhmDskVisibleObject::getVisibleObjNameForID(eId)).arg(sStyle);
				TRACE_D(QString( "MFormMain::updateListObject: setStyleSheet: %1")
									.arg(sStyle));

				pListWidget->setStyleSheet(sStyle);
			}
		}

	}

		
}



//------------------------------------
// END List objects 
//------------------------------------



void MFormMain::processNewShiftType(QString sNewShiftType)
{
	
	if(sNewShiftType.size()>0)
	{
		QString sNewColor;
		bool bOK = false;
		
		if(sNewShiftType.at(0) == IHM_REG_SHIFT_COL_PREFIX)
		{
			sNewColor = sNewShiftType.mid(1);
		}
		else
		{
			MIhmConfigColor *pColors = MIhmConfigGeneral::getCfg()->getConfigColors();
			bOK = pColors->getColourForShift(sNewShiftType, sNewColor);
		}

		//update the shift type
		if(bOK)
		{
			QString s = QString(" QWidget#MFormMainWidget{%1}")
									.arg(MIhmConfigColor::createBkgColorStyle(sNewColor));

			if(s!="")
			{
				QString sNewStyle = m_sDynaFormStyleSheet + s;
				m_dynFormWidget->setStyleSheet(sNewStyle);
			}
		}
	}

}


void MFormMain::checkResolution()
{
	bool bResolutionOK = true;
	

	TRACE_D(QString( "MFormMain::checkResolution: ..." ));
	
	QRect rectTarget(0,0, MIhmConfigGeneral::getCfg()->getResolutionX(), MIhmConfigGeneral::getCfg()->getResolutionY());

	QScreen *deskWidget = qApp->primaryScreen();
	QRect screenRect = deskWidget->availableGeometry();


	if(screenRect!=rectTarget)
		bResolutionOK = false;
	else
		bResolutionOK = true;
	

//	if(MIhmConfigGeneral::getCfg()->isNoResolutionCheck() == false)
//	{
//		//if normal working with resolution check...
//		if(rectTarget.isValid() && bResolutionOK)
//		{	
//			TRACE_D(QString("MFormMain::checkResolution: Resolution OK" ));
//			
//			if(MIhmConfigGeneral::getCfg()->m_bShowTitleBar)
//			{	
//				TRACE_D(QString("MFormMain::checkResolution: ShowTitleBar" ));
//
//				setWindowFlags(Qt::FramelessWindowHint | Qt::WindowTitleHint);
//			}
//			else
//			{
//				TRACE_D(QString("MFormMain::checkResolution: show no title bar" ));
//
//				setWindowFlags(Qt::SplashScreen);
//			}
//
//		}
//		else
//		{
//			TRACE_W(QString("MFormMain::initialize: Error setting resolution to: %1,%2")
//									.arg(MIhmConfigGeneral::getCfg()->getResolutionX())
//									.arg(MIhmConfigGeneral::getCfg()->getResolutionY()));
//			emit exitProgram();
//			return;
//		}
//	}
//	else
//	{
		//if working without resolution check...
		TRACE_D(QString( "MFormMain::checkResolution: NoResolutionCheck = 1..."));
		
		if(rectTarget.isValid())
		{

			this->setWindowState(Qt::WindowNoState);
			//this->setGeometry(rectTarget);
			
			if(MIhmConfigGeneral::getCfg()->m_bShowTitleBar)
			{	
				TRACE_D(QString( "MFormMain::checkResolution: ShowTitleBar"));
				setWindowFlags(Qt::FramelessWindowHint | Qt::WindowTitleHint); //
			}
			else
			{
				TRACE_D(QString( "MFormMain::checkResolution: show no title bar"));
				setWindowFlags(Qt::SplashScreen);
				//this->setWindowState(Qt::WindowFullScreen);
			}
			
		}
		else
		{
			TRACE_W(QString( "MFormMain::initialize: Error setting resolution to: %1,%2")
									.arg(MIhmConfigGeneral::getCfg()->getResolutionX())
									.arg(MIhmConfigGeneral::getCfg()->getResolutionY()));
			emit exitProgram();
			return;
		}
//	}

	this->resize(rectTarget.size());


	if(MIhmConfigGeneral::getCfg()->isManualStartupPosition())
	{
		int iLeft = MIhmConfigGeneral::getCfg()->getStartupPositionLeft();
		int iTop = MIhmConfigGeneral::getCfg()->getStartupPositionTop();
		int target_screen_index = MIhmConfigGeneral::getCfg()->getStartupScreenIndex();//1 is an example, this is 2nd screen, because screens are numbered from 0
		
		QPoint top_left = QPoint(iLeft, iTop);

		//if (deskWidget->screenCount() <= target_screen_index)
		//{
		//	target_screen_index = 0;
		//}

		//bool is_virtual_desktop = deskWidget->isVirtualDesktop();
		QList<QScreen*> lstSecreens = qApp->screens();

		//if (is_virtual_desktop)
		{
			QRect rect = deskWidget->availableGeometry();

			top_left = rect.topLeft() + QPoint(iLeft, iTop);
			TRACE_D(QString("MIhmDesktopIntf::processConfigLaneType: For Virtual desktop - Target screen is %1, target Left:%2, target Top:%3 - Result posx:%4 posy:%5!")
				.arg(target_screen_index)
				.arg(iLeft).arg(iTop)
				.arg(top_left.x()).arg(top_left.y()));
		}
		//else
		//{
		//	TRACE_D(QString("MIhmDesktopIntf::processConfigLaneType:  For NON Virtual desktop (parent_screen != NULL) -  Target screen is %1 posx:%2 posx:%3 ")
		//		.arg(target_screen_index)
		//		.arg(top_left.x()).arg(top_left.y()));
		//}

		this->move(top_left);
	}
	else
	{	//make the window appear at the center
			this->center();
	}
}

void MFormMain::updateAllVisibleObjStaticProp()
{
	for(int i=MIhmDskVisibleObject::enuIhmVisibleObjUnknown+1;i<m_vectDskVisibleObjects.size();i++)
	{
		if(m_vectDskVisibleObjects.at(i)!=NULL)
			m_vectDskVisibleObjects.at(i)->setVisible(m_vectDskVisibleObjects.at(i)->isConfVisible());
	}

}

//Assure that take control and return control have the correct hardcoded actions
// (which might have been overwritten by LaneTypes definition settings of the current config)
void MFormMain::initTakeControlButtons()
{
	MIhmButton * pCurrent;
	pCurrent = (MIhmButton *)MIhmDskVisibleObject::findVisibleObjectByID(&m_vectDskVisibleObjects, MIhmDskVisibleObject::enuCNT_BTN_TAKE_CTRL);
	
	if(pCurrent!=NULL)
	{	
		// to handle specifically the button pressed signal in onAction slot
		pCurrent->setActionID(SPECIFIC_ACTION_BTN_IDENTIFICATION);  
	}
	else
	{
		TRACE_W(QString("MFormMain::initTakeControlButtons: Error visible object : %1 not found ")
									.arg(CNT_REG_VAL_BTN_TAKE_CTRL));
	}

	pCurrent = (MIhmButton *)MIhmDskVisibleObject::findVisibleObjectByID(&m_vectDskVisibleObjects, MIhmDskVisibleObject::enuCNT_BTN_RETURN_CTRL);
	
	if(pCurrent!=NULL)
	{	
		// to handle specifically the button pressed signal in onAction slot
		pCurrent->setActionID(SPECIFIC_ACTION_BTN_END_IDENTIF);  
	}
	else
	{
		TRACE_W(QString("MFormMain::initTakeControlButtons: Error visible object:%1 not found ")
									.arg(CNT_REG_VAL_BTN_RETURN_CTRL));
	}
	

}

void MFormMain::updateTakeControlButtons()
{
	MIhmLanguages * pLang = MIhmConfigGeneral::getCfg()->getLanguages();
	MIhmButton * pCurrent = NULL;
	pCurrent = (MIhmButton *)MIhmDskVisibleObject::findVisibleObjectByID(&m_vectDskVisibleObjects, MIhmDskVisibleObject::enuCNT_BTN_TAKE_CTRL);
	QString sText;

	bool bTakeCtrlEnabled = m_pDskIntf->isTakeCtrlBtnEnabled();

	if(pCurrent!=NULL)
	{	
		if(m_pDskIntf->isInControl())
		{
			pCurrent->setEnabled(false);
			sText = pLang->getBtnLabelTakeControl(MIhmLanguages::enuTranslTargetDesktop);
			pCurrent->setText(sText);
		}
		else if(!m_pDskIntf->isInControl() && m_pDskIntf->isIdentified())
		{
			pCurrent->setEnabled(true);
			sText = pLang->getBtnLabelTakeControl(MIhmLanguages::enuTranslTargetDesktop);
			pCurrent->setText(sText);
		}
		else
		{
			pCurrent->setEnabled(bTakeCtrlEnabled);
			sText = pLang->getBtnLabelIdentification(MIhmLanguages::enuTranslTargetDesktop);
			pCurrent->setText(sText);
		}
	}
	else
	{
		TRACE_W(QString( "MFormMain::updateTakeControlButtons: Error visible object:%1 not found!")
									.arg(CNT_REG_VAL_BTN_TAKE_CTRL));
	}

	pCurrent = (MIhmButton *)MIhmDskVisibleObject::findVisibleObjectByID(&m_vectDskVisibleObjects, MIhmDskVisibleObject::enuCNT_BTN_RETURN_CTRL);
	

	bool bReturnCtrlEnabled = m_pDskIntf->isReturnCtrlBtnEnabled();

	if(pCurrent!=NULL)
	{	
		if(m_pDskIntf->isInControl())
		{
			pCurrent->setEnabled(bReturnCtrlEnabled);
			sText = pLang->getBtnLabelReturnControl(MIhmLanguages::enuTranslTargetDesktop);
			pCurrent->setText(sText);
		}
		else if(!m_pDskIntf->isInControl() && m_pDskIntf->isIdentified())
		{
			pCurrent->setEnabled(true);
			sText = pLang->getBtnLabelEndIdentification(MIhmLanguages::enuTranslTargetDesktop);
			pCurrent->setText(sText);
		}
		else
		{
			pCurrent->setEnabled(false);
			sText = pLang->getBtnLabelEndIdentification(MIhmLanguages::enuTranslTargetDesktop);
			pCurrent->setText(sText);
		}
	}
	else
	{
		TRACE_W(QString( "MFormMain::updateTakeControlButtons: Error visible object:%1 not found!")
									.arg(CNT_REG_VAL_BTN_RETURN_CTRL));
	}
	
}



bool MFormMain::loadTemplate(QString sUIFilePath)
{
	bool bRetVal = false;
	QUiLoader loader;

	TRACE_D(QString("MFormMain::loadTemplate: UI from file %1")
									.arg(sUIFilePath));

	QFile file(sUIFilePath);
	
	if(file.open(QFile::ReadOnly))
	{
		m_layout = new QHBoxLayout(this);
		m_mainFrame = new QFrame(this);
		m_mainFrame->setFrameShape(QFrame::NoFrame);
		m_mainFrame->setLineWidth(0);

		m_dynFormWidget = loader.load(&file, m_mainFrame);
		file.close();
		
		//keep the original style sheet to be able to dynamically add style
		m_sDynaFormStyleSheet = m_dynFormWidget->styleSheet();	

		m_layout->addWidget(m_mainFrame);
		m_layout->setSpacing(0);

		this->setContentsMargins(0,0,0,0);
		m_mainFrame->setContentsMargins(0,0,0,0);
		m_layout->setContentsMargins(1,1,1,1);

		if(connectVisibleObjects())
		{	
			m_bTemplateLoaded = true;
			bRetVal = true;
		}
		else
		{
			TRACE_W(QString("MFormMain::loadTemplate: Error connectVisibleObjects %1")
									.arg(sUIFilePath));
			return false;
		}

	}
	else
	{
		TRACE_W(QString("MFormMain::loadTemplate: Error loading %1")
									.arg(sUIFilePath));
		return false;
		
	}

	return bRetVal;
}




bool MFormMain::connectVisibleObjects()
{
	bool bRetVal = true;
	MIhmDskVisibleObject * pCurrent;
	QWidget *pWidget;

	for(int i=MIhmDskVisibleObject::enuIhmVisibleObjUnknown+1;i<m_vectDskVisibleObjects.size();i++)
	{
		pCurrent = m_vectDskVisibleObjects.at(i);

		if (pCurrent == NULL)
		{
			TRACE_W(QString( "MFormMain::connectVisibleObjects: Error geting visible object num: %1")
									.arg(i));
			continue;
		}
		
		pWidget = NULL;

		switch(pCurrent->getType())
		{
		case MIhmDskVisibleObject::enuIhmContainer:
		case MIhmDskVisibleObject::enuIhmFileView:
			pWidget = this->findChild<QFrame*>(pCurrent->getName());
			pCurrent->setWidget(pWidget);
			break;
		case MIhmDskVisibleObject::enuIhmVideoView:
		case MIhmDskVisibleObject::enuIhmRestreamVideoView:
			pWidget = this->findChild<QFrame*>(pCurrent->getName());
			pCurrent->setWidget(pWidget);
			break;
		case MIhmDskVisibleObject::enuIhmHMenuView:
			pWidget = this->findChild<QFrame*>(pCurrent->getName());
			pCurrent->setWidget(pWidget);
			((MIhmHMenuView*)pCurrent)->initialize();
			break;
		case MIhmDskVisibleObject::enuIhmIco:
			pWidget = this->findChild<QLabel*>(pCurrent->getName());
			pCurrent->setWidget(pWidget);
			break;
		case MIhmDskVisibleObject::enuIhmLabel:
			pWidget = this->findChild<QLabel*>(pCurrent->getName());
			pCurrent->setWidget(pWidget);
			break;
		case MIhmDskVisibleObject::enuIhmList:
			{
				QFrame *  pListFrame = this->findChild<QFrame*>(pCurrent->getName());
				pWidget = pListFrame;

				QString sTooltipLabelName = ((MIhmList*)pCurrent)->getToolTipLabelName();
				QLabel * pToolTipLabel = this->findChild<QLabel*>(sTooltipLabelName);

				MIhmLaneTypeSettings * pSettings = MIhmConfigGeneral::getCfg()->getLaneTypeSetting(MIhmLaneTypeSettings::enuDESKTOP, m_sLaneType);
				LaneTypeVisObjParams * pVisObjParams = NULL;

				if(pSettings!=NULL)
					pVisObjParams = pSettings->getVisObjParams(pCurrent->getName());

				QString sIsIconMode; 

				if(pVisObjParams!=NULL)
					sIsIconMode = pVisObjParams->getParam(CFG_VISIBLE_OBJ_ATTRIBUTE_IS_ICON_MODE);
				
				bool bIconMode = (sIsIconMode=="1")?true:false;

				QString sShowText, sShowIcon, sShowTooltipShowLastItem;

				if(pVisObjParams!=NULL)
					sShowText = pVisObjParams->getParam(CFG_VISIBLE_OBJ_ATTRIBUTE_SHOW_TEXT);

				if(pVisObjParams!=NULL)
					sShowIcon = pVisObjParams->getParam(CFG_VISIBLE_OBJ_ATTRIBUTE_SHOW_ICON);

				if (pVisObjParams != NULL)
					sShowTooltipShowLastItem = pVisObjParams->getParam(CFG_VISIBLE_OBJ_ATTRIBUTE_TOOLTIP_SHOW_LAST_ITEM);

				bool bShowText = (sShowText=="1")?true:false;
				bool bShowIcon = (sShowIcon=="1")?true:false;
				bool bTooltipShowLastItem = (sShowTooltipShowLastItem == "1") ? true : false;
				
				((MIhmList*)pCurrent)->initialize(pListFrame, pToolTipLabel, bIconMode, bShowText, bShowIcon, bTooltipShowLastItem);
			}
			break;
		case MIhmDskVisibleObject::enuIhmButton:
			{
				pWidget = this->findChild<QFrame*>(pCurrent->getName());
				pCurrent->setWidget(pWidget);


				bool bInvariantToSessionState = false;

				if(pCurrent->getName()==CNT_REG_VAL_BTN_TAKE_CTRL||
					pCurrent->getName()==CNT_REG_VAL_BTN_RETURN_CTRL)
					bInvariantToSessionState = true;

				((MIhmButton*)pCurrent)->initialize(this,bInvariantToSessionState);	
			}
			break;
		case MIhmDskVisibleObject::enuIhmTabView:
			{
				QFrame * pFrame = this->findChild<QFrame*>(pCurrent->getName());
				pWidget = pFrame;
				
				QString sPaneFrameName = ((MIhmTabView*)pCurrent)->getContentFrameName();
				QFrame * pPaneFrame = this->findChild<QFrame*>(sPaneFrameName);

				((MIhmTabView*)pCurrent)->initialize((QFrame *)pFrame, pPaneFrame);	
			}
			break;

		case MIhmDskVisibleObject::enuIhmTaskBarView:
			{
				QFrame * pFrame = this->findChild<QFrame*>(pCurrent->getName());
				pWidget = pFrame;

				QString sTypeList = m_pDskIntf->getLaneTypeParam(m_sLaneType, CFG_LANE_TYPE_PARAM_TSKBAR_DLG_TYPES);
				QStringList lstDlgList = sTypeList.split(CFG_LANE_TYPE_PARAM_SEPARATOR);

				

				QString sDlgUITemplatePath = m_pDskIntf->getLaneTypeParam(m_sLaneType,CFG_LANE_TYPE_PARAM_TSKBAR_BROWSER_DLG_TEMPLATE);
				if(sDlgUITemplatePath!="")	
					sDlgUITemplatePath = MIhmConfigGeneral::getCfg()->getUIFullPath(sDlgUITemplatePath);
				
				//if(sTypeList!="" && sDlgUITemplatePath!="")
				//{
				//	((MIhmTaskBarView*)pCurrent)->initialize(pFrame, this, sDlgUITemplatePath);	
				//	
				//	QString sGeometry;
				//	QString sTemplateStr = CFG_LANE_TYPE_PARAM_TSKBAR_DLG_GEOMETRY;
				//	QString sParameName;

				//	for (int i=0;i<lstDlgList.size();i++)
				//	{
				//		sParameName = QString(sTemplateStr).arg(i);
				//		sGeometry = m_pDskIntf->getLaneTypeParam(m_sLaneType,sParameName);

				//		QRect rectGeometry = MHelpFuncs::convertStrToRect(sGeometry);

				//		((MIhmTaskBarView*)pCurrent)->initDialog(i, lstDlgList.at(i), rectGeometry);
				//	}
				//}
			}
			break;

		default:
			TRACE_W(QString( "MFormMain::connectVisibleObjects: Unknown visible object type %1")
									.arg(pCurrent->getType()));
			break;
		}
		
		
		if(pWidget == NULL)
		{
			TRACE_W(QString( "MFormMain::connectVisibleObjects: Unable to find template object %1")
									.arg(pCurrent->getName()));
			bRetVal = false;
		}
		else
		{
			//the widget is not visible, but it would be visible if the parent window(this) is visible?
			 pCurrent->setConfVisible(pWidget->isVisibleTo(this));
			
		}
	}
	
	return bRetVal;
}



void MFormMain::initializeVisibleObjects()
{
	MIhmDskVisibleObject * pNewObject;
	
	m_vectDskVisibleObjects.fill(NULL, (int)MIhmDskVisibleObject::enuIhmVisibleObjLast);

	for (int i = MIhmDskVisibleObject::enuIhmVisibleObjUnknown + 1; i<MIhmDskVisibleObject::enuIhmVisibleObjLast; i++)
	{
		switch(i)
		{
			case MIhmDskVisibleObject::enuTEXT_LABEL_CLASS:
			case MIhmDskVisibleObject::enuTEXT_LABEL_CLASS_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_FARE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_FARE_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_ADD_REVENUE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_ADD_REVENUE_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_AXLES:
			case MIhmDskVisibleObject::enuTEXT_LABEL_AXLES_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_TYPE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_TYPE_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SALE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SALE_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SALEDUE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SALEDUE_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SALEDUE_PAID:
			case MIhmDskVisibleObject::enuTEXT_LABEL_BALANCE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_BALANCE_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_ENTRY:
			case MIhmDskVisibleObject::enuTEXT_LABEL_ENTRY_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_TRANSACTIONNBR:
			case MIhmDskVisibleObject::enuTEXT_LABEL_TRANSACTIONNBR_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_RECEIPTNBR:
			case MIhmDskVisibleObject::enuTEXT_LABEL_RECEIPTNBR_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_COL_ID:
			case MIhmDskVisibleObject::enuTEXT_LABEL_COL_NAME:
			case MIhmDskVisibleObject::enuTEXT_LABEL_PLAZA_NAME:
			case MIhmDskVisibleObject::enuTEXT_LABEL_PLAZA_NAME1:
			case MIhmDskVisibleObject::enuTEXT_LABEL_PLAZA_NAME2:
			case MIhmDskVisibleObject::enuTEXT_LABEL_PLAZA_NAME3:
			case MIhmDskVisibleObject::enuTEXT_LABEL_PLAZA_NAME_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_LANE_NAME_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_LANE_NAME:
			case MIhmDskVisibleObject::enuTEXT_LABEL_LANE_NAME1:
			case MIhmDskVisibleObject::enuTEXT_LABEL_LANE_NAME2:
			case MIhmDskVisibleObject::enuTEXT_LABEL_PLAZA_NUMBER_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_PLAZA_NUMBER:
			case MIhmDskVisibleObject::enuTEXT_LABEL_LANE_NUMBER_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_LANE_NUMBER:
	
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE1:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE2:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE3:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE4:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE5:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE6:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE7:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE8:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE9:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE10:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE11:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE12:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE13:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE14:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE15:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE16:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE17:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE18:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE19:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE20:

			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE21:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE22:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE23:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE24:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE25:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE26:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE27:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE28:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE29:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE30:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE31:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE32:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE33:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE34:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE35:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE36:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE37:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE38:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE39:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE40:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE41:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE42:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE43:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE44:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE45:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE46:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE47:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE48:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE49:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE50:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE51:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE52:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE53:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE54:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE55:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE56:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE57:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE58:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE59:
			case MIhmDskVisibleObject::enuTEXT_LABEL_SPARE60:



			case MIhmDskVisibleObject::enuTEXT_LABEL_FARE_TYPE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_FARE_TYPE_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_TXT_MODE_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_TXT_MODE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_TXT_COLLECTOR_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_TXT_IN_CTRL_USER_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_TXT_IN_CTRL_USER:
			case MIhmDskVisibleObject::enuTEXT_LABEL_TXT_IN_CTRL_USER1:
			case MIhmDskVisibleObject::enuTEXT_LABEL_TXT_TRS_CONTAINER_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_TXT_ALARMS_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_TXT_TRS_HISTORY_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_TXT_ALERTES_TITLE:
			case MIhmDskVisibleObject::enuTEXT_LABEL_TXT_CAM_NUM_PIC:
			case MIhmDskVisibleObject::enuTEXT_LABEL_TXT_SCAN_NUM_PIC:
			case MIhmDskVisibleObject::enuTEXT_LABEL_TXT_NB_CARS_SAS:
		//Instructions
			case MIhmDskVisibleObject::enuTEXT_LABEL_INSTR:
			case MIhmDskVisibleObject::enuTEXT_LABEL_INSTR_2:
			case MIhmDskVisibleObject::enuTEXT_LABEL_PAYMENT:
			case MIhmDskVisibleObject::enuTEXT_LABEL_ERR_PAYMENT:
			case MIhmDskVisibleObject::enuTEXT_LABEL_REMARK:
			//------------------------------------
			//separate object not updated by virtual objects, 
			// but depending on "in control" status of the interface
			case MIhmDskVisibleObject::enuTEXT_LABEL_IN_CONTROL_STATUS:
			case MIhmDskVisibleObject::enuTEXT_LABEL_NBCAR:
				pNewObject = new MIhmLabel((MIhmDskVisibleObject::enumVisibleObjectId)i);
				m_vectDskVisibleObjects.replace(i, pNewObject);	
			break;

		//Lists
			case MIhmDskVisibleObject::enuLIST_LIST_ALARMS:
				pNewObject = new MIhmList((MIhmDskVisibleObject::enumVisibleObjectId)i);
				((MIhmList*)pNewObject)->setToolTipLabelName(LABEL_REG_VAL_TXT_ALARMS_TOOLTIP);
				m_vectDskVisibleObjects.replace(i, pNewObject);	
			break;

			case MIhmDskVisibleObject::enuLIST_LIST_WARNINGS:
				pNewObject = new MIhmList((MIhmDskVisibleObject::enumVisibleObjectId)i);
				((MIhmList*)pNewObject)->setToolTipLabelName(LABEL_REG_VAL_TXT_WARNINGS_TOOLTIP);
				m_vectDskVisibleObjects.replace(i, pNewObject);	
			break;



		//Icons
		//------------------------------------
			case MIhmDskVisibleObject::enuICO_CAR:
			case MIhmDskVisibleObject::enuICO_COLLECTOR:
			case MIhmDskVisibleObject::enuICO_ENTRYGATE:
			case MIhmDskVisibleObject::enuICO_ENTRYLOOP:
			case MIhmDskVisibleObject::enuICO_EXITGATE:
			case MIhmDskVisibleObject::enuICO_EXITLOOP:
			case MIhmDskVisibleObject::enuICO_MODE:
			case MIhmDskVisibleObject::enuICO_STATUS:
			case MIhmDskVisibleObject::enuICO_TRAFFIC:
			case MIhmDskVisibleObject::enuICO_VIOLATION:
			case MIhmDskVisibleObject::enuICO_ICO_BEACON:
			case MIhmDskVisibleObject::enuICO_ICO_OPT_BARR:
			case MIhmDskVisibleObject::enuICO_SPARE1:
			case MIhmDskVisibleObject::enuICO_SPARE2:
			case MIhmDskVisibleObject::enuICO_SPARE3:
			case MIhmDskVisibleObject::enuICO_SPARE4:
			case MIhmDskVisibleObject::enuICO_SPARE5:
			case MIhmDskVisibleObject::enuICO_SPARE6:
			case MIhmDskVisibleObject::enuICO_SPARE7:
			case MIhmDskVisibleObject::enuICO_SPARE8:
			case MIhmDskVisibleObject::enuICO_SPARE9:
			case MIhmDskVisibleObject::enuICO_SPARE10:
			case MIhmDskVisibleObject::enuICO_SPARE11:
			case MIhmDskVisibleObject::enuICO_SPARE12:
			case MIhmDskVisibleObject::enuICO_SPARE13:
			case MIhmDskVisibleObject::enuICO_SPARE14:
			case MIhmDskVisibleObject::enuICO_SPARE15:
			case MIhmDskVisibleObject::enuICO_SPARE16:
			case MIhmDskVisibleObject::enuICO_SPARE17:
			case MIhmDskVisibleObject::enuICO_SPARE18:
			case MIhmDskVisibleObject::enuICO_SPARE19:
			case MIhmDskVisibleObject::enuICO_SPARE20:
			case MIhmDskVisibleObject::enuICO_SPARE21:
			case MIhmDskVisibleObject::enuICO_SPARE22:
			case MIhmDskVisibleObject::enuICO_SPARE23:
			case MIhmDskVisibleObject::enuICO_SPARE24:
			case MIhmDskVisibleObject::enuICO_SPARE25:
			case MIhmDskVisibleObject::enuICO_SPARE26:
			case MIhmDskVisibleObject::enuICO_SPARE27:
			case MIhmDskVisibleObject::enuICO_SPARE28:
			case MIhmDskVisibleObject::enuICO_SPARE29:
			case MIhmDskVisibleObject::enuICO_SPARE30:
			case MIhmDskVisibleObject::enuICO_SPARE31:
			case MIhmDskVisibleObject::enuICO_SPARE32:
			case MIhmDskVisibleObject::enuICO_SPARE33:
			case MIhmDskVisibleObject::enuICO_SPARE34:
			case MIhmDskVisibleObject::enuICO_SPARE35:
			case MIhmDskVisibleObject::enuICO_SPARE36:
			case MIhmDskVisibleObject::enuICO_SPARE37:
			case MIhmDskVisibleObject::enuICO_SPARE38:
			case MIhmDskVisibleObject::enuICO_SPARE39:
			case MIhmDskVisibleObject::enuICO_SPARE40:
			case MIhmDskVisibleObject::enuICO_SPARE41:
			case MIhmDskVisibleObject::enuICO_SPARE42:
			case MIhmDskVisibleObject::enuICO_SPARE43:
			case MIhmDskVisibleObject::enuICO_SPARE44:
			case MIhmDskVisibleObject::enuICO_SPARE45:
			case MIhmDskVisibleObject::enuICO_SPARE46:
			case MIhmDskVisibleObject::enuICO_SPARE47:
			case MIhmDskVisibleObject::enuICO_SPARE48:
			case MIhmDskVisibleObject::enuICO_SPARE49:
			case MIhmDskVisibleObject::enuICO_SPARE50:
			case MIhmDskVisibleObject::enuICO_CAMERA1:
			case MIhmDskVisibleObject::enuICO_CAMERA2:
			case MIhmDskVisibleObject::enuICO_BEACON2:
			case MIhmDskVisibleObject::enuICO_SIGNAL_LIGHT:
			case MIhmDskVisibleObject::enuICO_EXT_OPT_BARR:
			case MIhmDskVisibleObject::enuCNT_LBL_DYN_IMAGE:
			case MIhmDskVisibleObject::enuCNT_LBL_DYN_IMAGE2:
				pNewObject = new MIhmIco((MIhmDskVisibleObject::enumVisibleObjectId)i);
				m_vectDskVisibleObjects.replace(i, pNewObject);	
			break;




		//Containers
			case MIhmDskVisibleObject::enuCNT_PAYMENT_GROUP:
			case MIhmDskVisibleObject::enuCNT_INSTR_GROUP:
			case MIhmDskVisibleObject::enuCNT_TRS_GROUP:
			case MIhmDskVisibleObject::enuCNT_MODE_GROUP:
			case MIhmDskVisibleObject::enuCNT_HEADER_GROUP:
			case MIhmDskVisibleObject::enuCNT_CNT_DYN_PICTURE:
			case MIhmDskVisibleObject::enuCNT_CNT_DYN_PICTURE2:
			case MIhmDskVisibleObject::enuCNT_CNT_COMMENTS:
			case MIhmDskVisibleObject::enuCNT_CNT_CAMERA:
			case MIhmDskVisibleObject::enuCNT_CNT_SCANNER:
			case MIhmDskVisibleObject::enuCNT_CNT_SPARE1:
			case MIhmDskVisibleObject::enuCNT_CNT_SPARE2:
			case MIhmDskVisibleObject::enuCNT_CNT_SPARE3:
			case MIhmDskVisibleObject::enuCNT_CNT_SPARE4:
			case MIhmDskVisibleObject::enuCNT_CNT_SPARE5:
			case MIhmDskVisibleObject::enuCNT_CNT_SPARE6:
			case MIhmDskVisibleObject::enuCNT_CNT_SPARE7:
			case MIhmDskVisibleObject::enuCNT_CNT_SPARE8:
			case MIhmDskVisibleObject::enuCNT_CNT_SPARE9:
			case MIhmDskVisibleObject::enuCNT_CNT_SPARE10:
			case MIhmDskVisibleObject::enuCNT_CNT_SPARE11:
			case MIhmDskVisibleObject::enuCNT_CNT_SPARE12:
			case MIhmDskVisibleObject::enuCNT_CNT_SPARE13:
			case MIhmDskVisibleObject::enuCNT_CNT_SPARE14:
			case MIhmDskVisibleObject::enuCNT_CNT_SPARE15:
			case MIhmDskVisibleObject::enuCNT_CNT_SPARE16:
			case MIhmDskVisibleObject::enuCNT_CNT_SPARE17:
			case MIhmDskVisibleObject::enuCNT_CNT_SPARE18:
			case MIhmDskVisibleObject::enuCNT_CNT_SPARE19:
			case MIhmDskVisibleObject::enuCNT_CNT_SPARE20:
			case MIhmDskVisibleObject::enuCNT_CNT_MINI_WEB:
			case MIhmDskVisibleObject::enuCNT_CNT_ALARMS_GROUP:
				pNewObject = new MIhmContainer((MIhmDskVisibleObject::enumVisibleObjectId)i);
				m_vectDskVisibleObjects.replace(i, pNewObject);	
			break;

		//Buttons
			case MIhmDskVisibleObject::enuCNT_BTN_TAKE_CTRL:
			case MIhmDskVisibleObject::enuCNT_BTN_RETURN_CTRL:
			case MIhmDskVisibleObject::enuCNT_BTN_CLASS:
			case MIhmDskVisibleObject::enuCNT_BTN_ENTRY_POINT:
			case MIhmDskVisibleObject::enuCNT_BTN_ENTRY_POINT_NBR:
			case MIhmDskVisibleObject::enuCNT_BTN_EXTERNAL_DISPLAY:
			case MIhmDskVisibleObject::enuCNT_BTN_EXTERNAL_DISPLAY2:
			case MIhmDskVisibleObject::enuCNT_BTN_MODE:
			case MIhmDskVisibleObject::enuCNT_BTN_ARROW_ON:
			case MIhmDskVisibleObject::enuCNT_BTN_ARROW_OFF:
			case MIhmDskVisibleObject::enuCNT_BTN_CROSS_ON:
			case MIhmDskVisibleObject::enuCNT_BTN_CROSS_OFF:
			case MIhmDskVisibleObject::enuCNT_BTN_DSRC_ON:
			case MIhmDskVisibleObject::enuCNT_BTN_DSRC_OFF:
			case MIhmDskVisibleObject::enuCNT_BTN_MAGNET_ON:
			case MIhmDskVisibleObject::enuCNT_BTN_MAGNET_OFF:
			case MIhmDskVisibleObject::enuCNT_BTN_COLLECTOR_ON:
			case MIhmDskVisibleObject::enuCNT_BTN_COLLECTOR_OFF:
			case MIhmDskVisibleObject::enuCNT_BTN_CAMERA:
			case MIhmDskVisibleObject::enuCNT_BTN_SCANNER:
			case MIhmDskVisibleObject::enuCNT_BTN_COMMENT:
			case MIhmDskVisibleObject::enuCNT_PRODUCT_BUTTON1:
			case MIhmDskVisibleObject::enuCNT_PRODUCT_BUTTON2:
			case MIhmDskVisibleObject::enuCNT_PRODUCT_BUTTON3:
			case MIhmDskVisibleObject::enuCNT_PRODUCT_BUTTON4:
			case MIhmDskVisibleObject::enuCNT_PRODUCT_BUTTON5:
			case MIhmDskVisibleObject::enuCNT_BTN_SPEED_LIMIT_ON:
			case MIhmDskVisibleObject::enuCNT_BTN_SPEED_LIMIT_OFF:
			case MIhmDskVisibleObject::enuCNT_BTN_WARNING_ON:
			case MIhmDskVisibleObject::enuCNT_BTN_WARNING_OFF:
			case MIhmDskVisibleObject::enuCNT_BTN_DSRC_TSA_ON:
			case MIhmDskVisibleObject::enuCNT_BTN_DSRC_TSA_OFF:

			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_1:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_2:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_3:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_4:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_5:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_6:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_7:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_8:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_9:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_10:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_11:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_12:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_13:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_14:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_15:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_16:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_17:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_18:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_19:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_20:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_21:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_22:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_23:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_24:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_25:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_26:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_27:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_28:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_29:
			case MIhmDskVisibleObject::enuCNT_BTN_SPARE_30:


			case MIhmDskVisibleObject::enuCNT_BTN_GABARITE:
			case MIhmDskVisibleObject::enuCNT_BTN_ENTRY_GATE:
			case MIhmDskVisibleObject::enuCNT_BTN_EXIT_GATE:
			case MIhmDskVisibleObject::enuCNT_BTN_CAR3:
			case MIhmDskVisibleObject::enuCNT_BTN_CAR2:
			case MIhmDskVisibleObject::enuCNT_BTN_CAR:
			case MIhmDskVisibleObject::enuCNT_BTN_ENTRY_LOOP:
			case MIhmDskVisibleObject::enuCNT_BTN_EXIT_LOOP:
			case MIhmDskVisibleObject::enuCNT_BTN_TBL_BEACON:
			case MIhmDskVisibleObject::enuCNT_BTN_CAM_GRAB:
			case MIhmDskVisibleObject::enuCNT_BTN_CAM_HIDE:
			case MIhmDskVisibleObject::enuCNT_BTN_SCAN_GRAB:
			case MIhmDskVisibleObject::enuCNT_BTN_SCAN_HIDE:
			case MIhmDskVisibleObject::enuCNT_BTN_SCAN_SEND:
			case MIhmDskVisibleObject::enuCNT_BTN_SCAN_SAVE:
			case MIhmDskVisibleObject::enuCNT_BTN_COMM_HIDE:				
				pNewObject = new MIhmButton((MIhmDskVisibleObject::enumVisibleObjectId)i);
				m_vectDskVisibleObjects.replace(i, pNewObject);	
			break;
		//------------------------ Time and Date -------------------------------
			case MIhmDskVisibleObject::enuCLK_TIME:
				pNewObject = new MIhmContainer((MIhmDskVisibleObject::enumVisibleObjectId)i);
				m_vectDskVisibleObjects.replace(i, pNewObject);	
			break;

			case MIhmDskVisibleObject::enuCLK_DATE:
				pNewObject = new MIhmLabel((MIhmDskVisibleObject::enumVisibleObjectId)i);
				m_vectDskVisibleObjects.replace(i, pNewObject);	
			break;
		//------------------------------------------------------------

			case MIhmDskVisibleObject::enuCNT_FILE_VIEW1:
			case MIhmDskVisibleObject::enuCNT_FILE_VIEW2:
			case MIhmDskVisibleObject::enuCNT_FILE_VIEW3:
				pNewObject = new MIhmFileView((MIhmDskVisibleObject::enumVisibleObjectId)i);
				m_vectDskVisibleObjects.replace(i, pNewObject);	
			break;
		//------------------------------------------------------------
			case MIhmDskVisibleObject::enuCNT_HMENU_VIEW1:
			case MIhmDskVisibleObject::enuCNT_HMENU_VIEW2:
				pNewObject = new MIhmHMenuView((MIhmDskVisibleObject::enumVisibleObjectId)i);
				m_vectDskVisibleObjects.replace(i, pNewObject);	
			break;
		//------------------------------------------------------------
			case MIhmDskVisibleObject::enuCNT_RVIDEO_VIEW1:
				pNewObject = new MIhmRestreamVideoView((MIhmDskVisibleObject::enumVisibleObjectId)i, MIhmVirtualObject::enuIhmVirtRestreamVideoView1);
				m_vectDskVisibleObjects.replace(i, pNewObject);	
			break;

			case MIhmDskVisibleObject::enuCNT_RVIDEO_VIEW2:
				pNewObject = new MIhmRestreamVideoView((MIhmDskVisibleObject::enumVisibleObjectId)i, MIhmVirtualObject::enuIhmVirtRestreamVideoView2);
				m_vectDskVisibleObjects.replace(i, pNewObject);	
			break;
		//------------------------------------------------------------

			case MIhmDskVisibleObject::enuCNT_TAB_VIEW1:
				pNewObject = new MIhmTabView((MIhmDskVisibleObject::enumVisibleObjectId)i);
				((MIhmTabView*)pNewObject)->setContentFrameName(CNT_REG_VAL_TAB_VIEW1_FRAME);
				m_vectDskVisibleObjects.replace(i, pNewObject);	
			break;

			case MIhmDskVisibleObject::enuCNT_TAB_VIEW2:
				pNewObject = new MIhmTabView((MIhmDskVisibleObject::enumVisibleObjectId)i);
				((MIhmTabView*)pNewObject)->setContentFrameName(CNT_REG_VAL_TAB_VIEW2_FRAME);
				m_vectDskVisibleObjects.replace(i, pNewObject);	
			break;

			case MIhmDskVisibleObject::enuCNT_TAB_VIEW3:
				pNewObject = new MIhmTabView((MIhmDskVisibleObject::enumVisibleObjectId)i);
				((MIhmTabView*)pNewObject)->setContentFrameName(CNT_REG_VAL_TAB_VIEW3_FRAME);
				m_vectDskVisibleObjects.replace(i, pNewObject);	
				break;
				
			case MIhmDskVisibleObject::enuCNT_TAB_VIEW4:
				pNewObject = new MIhmTabView((MIhmDskVisibleObject::enumVisibleObjectId)i);
				((MIhmTabView*)pNewObject)->setContentFrameName(CNT_REG_VAL_TAB_VIEW4_FRAME);
				m_vectDskVisibleObjects.replace(i, pNewObject);	
				break;

			case MIhmDskVisibleObject::enuCNT_CNT_TASK_BAR:
			//	pNewObject = new MIhmTaskBarView((MIhmDskVisibleObject::enumVisibleObjectId)i);
			//	m_vectDskVisibleObjects.replace(i, pNewObject);	
			break;
				
			default:
				TRACE_W(QString("MIhmDskVisibleObject::initializeVisibleObjects: Error unknown visible object ID:%1").arg(i));
				continue;
		}

	}
}



bool MFormMain::setVisibleObjIniSettings(MIhmDskVisibleObject * pObject)
{
	bool bRet = false;
	QString sConfigString;

	MIhmLaneTypeSettings * pSettings;
	pSettings = MIhmConfigGeneral::getCfg()->getLaneTypeSetting(MIhmLaneTypeSettings::enuDESKTOP, m_sLaneType);
	
	MIhmConfigActions *pCfgActions = MIhmConfigGeneral::getCfg()->getActions();

	if(pObject!=NULL)
	{
		LaneTypeVisObjParams * pVisParams = pSettings->getVisObjParams(pObject->getName());	
	
		if(pVisParams!=NULL)
		{
			pObject->initSignalReceiver(this);
			bRet = pObject->setIniSettings(pVisParams);
			
			//if ActionId set is <> empty string
			// init specific pointing cursor if specified for that action_id
			QString sActionId = pObject->getActionID();
			
			if(sActionId!="")
			{
				QString sCursorId = pCfgActions->getCursorIdForAction(sActionId);
				
				if(sCursorId!="")
				{
					QCursor oCursor = MIhmConfigGeneral::getCfg()->getCursor(sCursorId);
					pObject->setCursor(oCursor);
				}
				else
				{
					if (MIhmConfigGeneral::getCfg()->m_bHideCursor)
						pObject->setCursor(QCursor(Qt::BlankCursor));
				}

			}

		}
		
		if(bRet)
		{
			if(pObject->getType()== MIhmDskVisibleObject::enuIhmFileView)
			{
				MTableModelView * p = ((MIhmFileView*)pObject)->getTableModelView();
				if(p!=NULL)
				{
					connect((QObject *)p, SIGNAL(action(QString, QString)), this, SLOT(onAction(QString, QString)));
				}

				MTableView * pWidget = ((MIhmFileView*)pObject)->getTableViewWidget();

				if(pWidget!=NULL)
				{
					pWidget->installEventFilter(this);
					pWidget->setMouseTracking(true);
				}
			}
			else if(pObject->getType()== MIhmDskVisibleObject::enuIhmHMenuView)
			{
				MHMenuView * p = ((MIhmHMenuView*)pObject)->getMenuViewWidget();
				if(p!=NULL)
				{
					connect((QObject *)p, SIGNAL(action(QString, QString)), this, SLOT(onAction(QString, QString)));
					p->installEventFilter(this);
					p->setMouseTracking(true);
				
				}
			}
			else if(pObject->getType()== MIhmDskVisibleObject::enuIhmList)
			{
				MListWidget * p = (MListWidget*)(pObject->getClickableWidget());
				if(p!=NULL)
				{
					p->installEventFilter(this);
					p->setMouseTracking(true);
				}

			}
			else if(pObject->getType()== MIhmDskVisibleObject::enuIhmButton || 
					pObject->getType()== MIhmDskVisibleObject::enuIhmContainer ||
					pObject->getType()== MIhmDskVisibleObject::enuIhmTaskBarView || 
					pObject->getType()== MIhmDskVisibleObject::enuIhmLabel || 
					pObject->getType()== MIhmDskVisibleObject::enuIhmIco ||
					pObject->getType()== MIhmDskVisibleObject::enuIhmVideoView)
			{
				QWidget * p = pObject->getClickableWidget();
				if(p!=NULL)
				{
					p->installEventFilter(this);
					p->setMouseTracking(true);
				}
			}
			else if(pObject->getType()== MIhmDskVisibleObject::enuIhmTabView)
			{
				MTabView * p = ((MIhmTabView*)pObject)->getTabView();
				if(p!=NULL)
				{
					connect((QObject*)p, SIGNAL(clicked(QString, int)), this, SLOT(onTabClickedAction(QString ,int)));
				}

				QWidget * pW = pObject->getClickableWidget();
				if (pW != NULL)
				{
					pW->installEventFilter(this);
					pW->setMouseTracking(true);
				}

			}



		}
		else
		{
			TRACE_W(QString( "MFormMain::setVisibleObjIniSettings: invalid default parameters for %1 [%2] ")
									.arg(pObject->getName())
									.arg(sConfigString));
		}

	}

	return bRet;
}



void MFormMain::processPrjNameAndVersion(QString sName, QString sVersion)
{
	this->setWindowTitle(QString("%1 %2").arg(sName).arg(sVersion));
}


void MFormMain::processOpenLink(int iTargetDlg, QString sUrl, bool bVisible)
{
	//MIhmTaskBarView * pCurrent;
	//pCurrent = (MIhmTaskBarView *)MIhmDskVisibleObject::findVisibleObjectByID(&m_vectDskVisibleObjects, 
	//													MIhmDskVisibleObject::enuCNT_CNT_TASK_BAR);
	//
	//if(pCurrent==NULL)
	//{
	//	TRACE_W(QString( "MFormMain::processOpenLink: findVisibleObjectByID return NULL for the visible object name %1")
	//								.arg(MIhmDskVisibleObject::getVisibleObjNameForID(MIhmDskVisibleObject::enuCNT_CNT_TASK_BAR)));
	//	return;
	//}

	//if (bVisible)
	//	pCurrent->openLink(iTargetDlg, sUrl);
	//else
	//	pCurrent->closeDlg(iTargetDlg);


}


//-------------------------------------
//Keyboard handling ...
//-------------------------------------
#define IHM_KEYCODE_INS 256      // touche INS
#define IHM_KEYCODE_DEL 257      // touche DEL
#define IHM_KEYCODE_ALT 258      // touche ALT
#define IHM_KEYCODE_CTRL 259     // touche CTRL
#define IHM_KEYCODE_BKSPC 8      // touche BACKSPACE
#define IHM_KEYCODE_FCT 261      // 1ere touche de fonction (F1)
#define IHM_KEYCODE_LEFT 280      // touche LEFT
#define IHM_KEYCODE_UP 281        // touche UP
#define IHM_KEYCODE_RIGHT 282     // touche RIGHT
#define IHM_KEYCODE_DOWN 283      // touche DOWN


void MFormMain::saveEvent(QObject *target, QKeyEvent * event)
{
	if(m_pSavedEvent!=NULL)
	{
		delete m_pSavedEvent;
	}

	m_pSavedEventTarget = target;

	m_pSavedEvent = new QKeyEvent(event->type(),
										event->key(), 
										event->modifiers(), 
										event->text(), 
										event->isAutoRepeat(),
										event->count());
}

void MFormMain::reemitSavedEvent()
{
	if(m_pSavedEventTarget!=NULL && m_pSavedEvent!=NULL)
	{
		doNotFilterNextEvent(m_pSavedEventTarget, m_pSavedEvent);
		QCoreApplication::sendEvent(m_pSavedEventTarget, m_pSavedEvent);

		m_pSavedEvent = NULL;
		m_pSavedEventTarget = NULL;
	}
}


void MFormMain::doNotFilterNextEvent(QObject *target, QKeyEvent * pEvent)
{
	if(target!=this)
	{
		m_bDoNotFilter = true;
		QCoreApplication::sendEvent(target, pEvent);
		return;
	}
}

//--------------------------------------------------------
//keyboard testing ...
//--------------------------------------------------------

void MFormMain::setKeyboardTestingInProgress()
{
	//Activate all str detection couple IDs
	m_pStringDetection->activateAllPairs();
}

void MFormMain::unsetKeyboardTestingInProgress()
{
	//Return str detection to old state
	MIhmVirtualObject * pVirtObj = m_pDskIntf->findVirtualObjectByID(MIhmVirtualObject::enuIhmVirtStrDetectConfigID);	
	
	if(pVirtObj!=NULL)
		processStrDetConfig((MIhmVirtStrDetectConfig*)pVirtObj);
	else
	{
		TRACE_W(QString("MFormMain::unsetKeyboardTestingInProgress: Unable to find MIhmVirtualObject::enuIhmVirtStrDetectConfigID!"));
		ExitBad();
	}
}


bool MFormMain::isKeyboardTestingInProgress()
{
	if(isInputDlgExInProgress())
		if(m_oCurrentInputExReq.getDialogType() == MInputDialogExReq::enuSAISIE_EX_KEYBOARD_TEST)
			return true;

	return false;
}

void MFormMain::processKeyTest(bool bIsTollKey, bool bIsStringDetection, int iAskedKey, int iAsciiCode, QString sDetectedStr, QString sCoupleId)
{
	if(isKeyboardTestingInProgress())
	{
		if(!bIsStringDetection)
		{
			if(!bIsTollKey)
			{
				((MFormTestKeyboard*)m_pInputDlgEx)->onKeyDetected(iAskedKey, iAsciiCode);
			}
			else
				((MFormTestKeyboard*)m_pInputDlgEx)->onTollKeyDetected(iAskedKey, sDetectedStr);		
		}
		else
		{
			((MFormTestKeyboard*)m_pInputDlgEx)->onStringDetected(sCoupleId, sDetectedStr);
		}
	}
}
//--------------------------------------------------------

//If the entered key is to be filtered the function should return true
bool MFormMain::keyPressEventHandler(QObject *target, QKeyEvent * event )
{
	Q_UNUSED(target)
	//Used to explicitly force some events not to be processed by this handler function
	//see functions: saveEvent and reemitSavedEvent which set the flag by doNotFilterNextEvent call
	if(m_bDoNotFilter) 
	{
		m_bDoNotFilter = false;
		return false; //do not filter
	}
	
	bool bIsTollKey = false;
	int iScanCode = event->nativeScanCode();
	int iVirtualCode = event->nativeVirtualKey();
	int iKey = event->key();
	QString s = event->text();
	unsigned char cAscii = 0;
	if(s.size()>0) 
		cAscii = (unsigned char)s.at(0).toLatin1();

	TRACE_D(QString("MFormMain::keyPressEvent: ScanCode:%1 VirtualCode:%2 Key:%3 Text:%4 Ascii(%5) - Obj:%6")
									.arg(iScanCode)
									.arg(iVirtualCode)
									.arg(iKey)
									.arg(s)
									.arg(cAscii)
									.arg(target->metaObject()->className()));

	//The internal code that is used as our "ihm" key code (values from 1-255, 255-299, 301-999)
	int iAskedKey = 0;
	
	
	// verify if special toll key entering or other device reading is in progress
	if(m_pStringDetection->checkIfStringDetection(s, iScanCode))
	{
		if(m_pStringDetection->isStringComplete())
		{
			if(m_pStringDetection->isTollKey())
			{
				if(isKeyboardTestingInProgress())
				{
					//should read before getTollKeyDetection because it resets detected string
					QString sDetectedStr = m_pStringDetection->getCompleteStringDetectionWithPrePostAmbules();
					processKeyTest(true, false, m_pStringDetection->getTollKeyDetection(), 0, sDetectedStr, QString());
					return true;
				}

				int iDetectedCode =	m_pStringDetection->getTollKeyDetection();
				TRACE_D(QString("MFormMain::keyPressEvent: getTollKeyDetection returned %1")
									.arg(iDetectedCode));

				iAskedKey = iDetectedCode;
				bIsTollKey = true;
			}
			else
			{
				MIhmMsgStringDetection * pMsg = new MIhmMsgStringDetection();
				
				//Since activated and non activated pairs are processed
				//we have to ask if the pair is actually active
				bool bStringDetPairActive = false;


				if(m_pStringDetection->getCompleteStringDetection(pMsg, &bStringDetPairActive))
				{	
					TRACE_D(QString("MFormMain::keyPressEvent: getCompleteString returned: Pair:%1 String:%2 Complete:%3")
									.arg(pMsg->sCoupleId)
									.arg(pMsg->sDetectedString)
									.arg(pMsg->bStringComplete?1:0));

					if(isKeyboardTestingInProgress())
					{
						processKeyTest(false, true, 0,0, pMsg->sDetectedString, pMsg->sCoupleId);
						delete pMsg;
					}
					else
					{
						if(bStringDetPairActive)
							emit stringDetected(pMsg);
						else
							delete pMsg;
					}
				}
				else
				{
					TRACE_W(QString("MFormMain::keyPressEvent: Error getCompleteString returned false!"));
					delete pMsg;
				}

				return m_bFilterStringDetectedKeys; //filter the last key
			}
		}
		else
			return m_bFilterStringDetectedKeys; //if string detection is in progress but not complete yet
	}
	else
	{

			//If key is some of these, convert it to internal IHM code
		   switch(iKey)
		   {
				case Qt::Key_F1:
				case Qt::Key_F2:
				case Qt::Key_F3:
				case Qt::Key_F4:
				case Qt::Key_F5:
				case Qt::Key_F6:
				case Qt::Key_F7:
				case Qt::Key_F8:
				case Qt::Key_F9:
				case Qt::Key_F10:
				case Qt::Key_F11:
				case Qt::Key_F12:
	   					iAskedKey = (iKey - Qt::Key_F1 + IHM_KEYCODE_FCT);
					break;

				case Qt::Key_Insert:
						iAskedKey = IHM_KEYCODE_INS;
   					break;

				case Qt::Key_Delete:
						iAskedKey = IHM_KEYCODE_DEL;
					break;

				case Qt::Key_Alt:
						iAskedKey = IHM_KEYCODE_ALT;
					break;
				
				case Qt::Key_Control:
						iAskedKey = IHM_KEYCODE_CTRL;
					break;

				case Qt::Key_Left:
						iAskedKey = IHM_KEYCODE_LEFT;
					break;
				case Qt::Key_Up:
						iAskedKey = IHM_KEYCODE_UP;
					break;
				case Qt::Key_Right:
						iAskedKey = IHM_KEYCODE_RIGHT;
					break;
				case Qt::Key_Down:
						iAskedKey = IHM_KEYCODE_DOWN;
					break;
			   default:
				   break;
		   }

			if(iAskedKey == 0)
			{
				iAskedKey = cAscii;

				if(iAskedKey <= 0) //ignore other keys with ascii == 0
					return false;
			}

		if(isKeyboardTestingInProgress())
		{
			processKeyTest(false, false, iAskedKey, (int)cAscii , QString(), QString());
			return true;
		}

	}

		if(iAskedKey <= 0) //ignore keys with ascii <= 0
			return false;

//-------------------------------------
//process detected iAskedKey
//-------------------------------------
		int iAboutKey = MIhmConfigGeneral::getCfg()->getAboutKey();
		int iRefreshKey = MIhmConfigGeneral::getCfg()->getRefreshKey();
		int iReturnControlKey = MIhmConfigGeneral::getCfg()->getReturnControlKey();
		int iTakeControlKey = MIhmConfigGeneral::getCfg()->getTakeControlKey();
		int iSynchronyzeNumLockKey = MIhmConfigGeneral::getCfg()->getNumLockSynchroKey();
		int iValidKey = MIhmConfigGeneral::getCfg()->getValidKey();
		int iMenuKey = MIhmConfigGeneral::getCfg()->getMenuKey();

		//handle the help and refresh key and NumLock forcing 
		if(iAboutKey!=0 &&  iAboutKey == iAskedKey)
		{
			openAboutDlg(""); //open the default about dialog if enabled with "AboutKey" != 0 registry value
			return true;
		}
		else if(iRefreshKey!=0 &&  iRefreshKey == iAskedKey)
		{
			emit refreshReq();
			return true;
		}
		else if(iReturnControlKey!=0 &&  iReturnControlKey == iAskedKey)
		{	
			emit action(SPECIFIC_ACTION_BTN_END_IDENTIF, "");			
			return true;
		}
		else if (iTakeControlKey != 0 && iTakeControlKey == iAskedKey)
		{
			emit action(SPECIFIC_ACTION_BTN_IDENTIFICATION, "");
			return true;
		}
		else if(iSynchronyzeNumLockKey != 0 && iAskedKey == iSynchronyzeNumLockKey)
		{
			MLibToggleKeys::SetToggleKeyState(VK_NUMLOCK, true, true);
			MLibToggleKeys::SetToggleKeyState(VK_NUMLOCK, true, true);
			if(m_ptimNumLock != NULL && m_ptimNumLock->isActive())
				onNumLockTimer();
			return true;
		}
		else if(iValidKey != 0 && iAskedKey == iValidKey)
		{
			if(m_pDskIntf->isInControl())
			{//if in control
				if(m_pCurrentClientDlg!=NULL)
				{
					m_pCurrentClientDlg->pressedKeyValidate();
					return true;
				}
				else if(isInputDlgExInProgress()) //If VALID key and dialog ex in progress
				{	
					m_pInputDlgEx->pressedKeyValidate();
					return true;
				}
			}
			else 
			{	
				if(m_pLoginDialog!=NULL)
				{
					m_pLoginDialog->pressedKeyValidate();
					return true;
				}
				else if(m_pCurrentClientDlg!=NULL)
				{
					m_pCurrentClientDlg->pressedKeyValidate();
					return true;
				}
			}
		}
		else if(iMenuKey!=0 &&  iMenuKey == iAskedKey)
		{
			if(m_pLoginDialog==NULL && m_pCurrentClientDlg==NULL)
			{	//signal DESKTOP take control request
				TRACE_D(QString("MFormMain::keyPressEvent: iMenuKey pressed:(%1) - sending SPECIFIC_ACTION_BTN_IDENTIFICATION!")
								.arg(iAskedKey));
				emit action(SPECIFIC_ACTION_BTN_IDENTIFICATION, "");			
				return true;
			}
		}

		if(processSetFocusKey(iAskedKey))
		{
			return true;
		}
		else if(processKeyInObjectInFocus(iAskedKey))
		{
			return true;
		}
		else 
		{
			//Send the key code to parent
			TRACE_D(QString("MFormMain::keyPressEvent: emit key pressed %1 ")
									.arg(iAskedKey));
			emit keyDetected(iAskedKey);

			if(bIsTollKey)
				return m_bFilterStringDetectedKeys; //to filter the last char from the detected toll key string 
			else
				return false;
		}
}

//Check if some visible object is configured with the detected key (iAskedKey) for focusing
// It is used to force focus to the visible object's widget
//The function return true if the key is used to force focus to avoid further processing of the key
bool MFormMain::processSetFocusKey(int iAskedKey)
{
	bool bProcessed = false;

	//find visible object with the selected set focus key
	MIhmDskVisibleObject *pRetObj = NULL;

	for(int i=MIhmDskVisibleObject::enuIhmVisibleObjUnknown+1;i<m_vectDskVisibleObjects.size();i++)
	{
		if(m_vectDskVisibleObjects.at(i)!=NULL)
			if(m_vectDskVisibleObjects.at(i)->getType() == MIhmDskVisibleObject::enuIhmFileView && 
				m_vectDskVisibleObjects.at(i)->getForceFocusKey()!=0 && 
				m_vectDskVisibleObjects.at(i)->getForceFocusKey() == iAskedKey)
			{
				pRetObj = m_vectDskVisibleObjects.at(i);
				break;
			}
	}


	if(pRetObj!=NULL)
	{
		pRetObj->forceFocus();
		TRACE_D(QString("MFormMain::processSetFocusKey: Key %1 processed as force focus to object %2!")
									.arg(iAskedKey)
									.arg(pRetObj->getName()));

		bProcessed = true;
	}

	return bProcessed;
}


bool MFormMain::processKeyInObjectInFocus(int iAskedKey)
{
	bool bProcessed = false;

	//List all the visible objects and check which one has focus
	// The one that is in keyboard focus should be informed (using virtual function) about the detected key code
	// the function should return result if the key code should be emit in key detected
	QWidget *pWInFocus = QApplication::focusWidget();

	MIhmDskVisibleObject *pObjInFocus = NULL;

	pObjInFocus = MIhmDskVisibleObject::findVisibleObjectByWidget(&m_vectDskVisibleObjects, pWInFocus);

	if(pObjInFocus!=NULL)
	{
		bProcessed = pObjInFocus->onKeyDetected(iAskedKey);	
	}

	return bProcessed;
}



void MFormMain::openAboutDlg(QString sCfgFileName)
{
	if(m_pAboutDlg==NULL)
	{
		m_pAboutDlg = new MFormAbout(this);
		connect(m_pAboutDlg, SIGNAL(action(QString, QString)), this, SLOT(onAboutDlgAction(QString, QString)));
		connect(m_pAboutDlg, SIGNAL(rejected()), this, SLOT(onAboutDlgClosed()));
		connect(m_pAboutDlg, SIGNAL(accepted()), this, SLOT(onAboutDlgClosed()));

		if(m_pAboutDlg->initialize(sCfgFileName))
			m_pAboutDlg->show();
		else
			onAboutDlgClosed();
	}
}


void MFormMain::onAboutDlgClosed()
{
	TRACE_D(QString("MFormMain::onAboutDlgClosed: ..."))
	
	if (m_pAboutDlg != NULL)
	{
		MFormAbout * pDlg = m_pAboutDlg;
		m_pAboutDlg = NULL;
		delete pDlg;

		this->activateWindow();
	}
}

void MFormMain::closeAboutDlg()
{
	if(m_pAboutDlg!=NULL)
	{
		m_pAboutDlg->close();
	}
}


void MFormMain::processStrDetConfig(MIhmVirtStrDetectConfig* pVirtObj)
{
	m_pStringDetection->updateStrDetectionPairs(&pVirtObj->m_lstActivePairs);
}

void MFormMain::createNumlockTimer()
{
	// get period for timer, connect signal and start timer
	if(MIhmConfigGeneral::getCfg()->getNumLockPeriod() > 0)
	{
		m_ptimNumLock = new QTimer(this);
		QObject::connect(m_ptimNumLock,SIGNAL(timeout()),this, SLOT(onNumLockTimer()));
		m_ptimNumLock->start(MIhmConfigGeneral::getCfg()->getNumLockPeriod());
	}
}

void MFormMain::onNumLockTimer()
{
	MLibToggleKeys::SetToggleKeyState(VK_NUMLOCK, true, false);
}


void MFormMain::createIdleTimer()
{
	m_pIdleTimer = new QTimer(this);
	QObject::connect(m_pIdleTimer,SIGNAL(timeout()),this, SLOT(onIdleTimer()));

}

void MFormMain::startIdleTimer()
{
	if(m_pIdleTimer!=NULL)
	{
		int iIdleTimeout = MIhmConfigGeneral::getCfg()->getCfgDynamic()->getIdleTimeoutDsk();
		
		if(m_pIdleTimer->isActive())
			m_pIdleTimer->stop();
		
		//if iIdleTimeout == 0 - idle timer is disabled
		if(iIdleTimeout>0)
			m_pIdleTimer->start(iIdleTimeout*1000);//it is in seconds so we convert it to [ms]

	}
}

void MFormMain::stopIdleTimer()
{
	if(m_pIdleTimer!=NULL)
		m_pIdleTimer->stop();

}


void MFormMain::resetIdleTimer()
{
	if(m_pDskIntf->isInControl()||m_pDskIntf->isIdentified())	
	{
		stopIdleTimer();
		startIdleTimer();
	}
}

void MFormMain::onIdleTimer()
{
	if(m_pDskIntf->isInControl()||m_pDskIntf->isIdentified())
		emit idleTimeout();
}


void MFormMain::onAction(QString sActionID, QString sActionParams)
{	
	TRACE_D(QString("MFormMain::onAction: ActionId:%1 , params:%2")
									.arg(sActionID)
									.arg(sActionParams));

	emit action(sActionID, sActionParams);
}

void MFormMain::onAboutDlgAction(QString sActionID, QString sActionParams)
{	
	TRACE_D(QString("MFormMain::onAboutDlgAction: ActionId:%1 , params:%2")
		.arg(sActionID)
		.arg(sActionParams));
	
	emit aboutDlgAction(sActionID, sActionParams);
}



void MFormMain::onTabClickedAction(QString sActionName, int iTabIndex)
{	
	TRACE_D(QString("MFormMain::onTabClickedAction: sName:%1 , tab_index:%2")
									.arg(sActionName)
									.arg(iTabIndex));
	
	QString sActionParams = QString("%1").arg(iTabIndex);
	emit action(sActionName, sActionParams);
}


void MFormMain::onAsyncInputDataChanged()
{

	MIhmMsgAsyncInputRsp *pMsg = new MIhmMsgAsyncInputRsp();

	if(m_pDskAsyncInputs!=NULL)
		pMsg->m_sEncStrValues = m_pDskAsyncInputs->getEncStrValues();

	m_pDskIntf->sendAsyncInputResponse(pMsg);	
}

void MFormMain::handleMouseClick(QObject *obj) 
{	
	QWidget *pWdg = dynamic_cast<QWidget*>(obj);

	MIhmDskVisibleObject *pObj	= MIhmDskVisibleObject::findVisibleObjectByWidget(&m_vectDskVisibleObjects, pWdg);

	if(pObj!=NULL)
	{
		if(pObj->isReadOnly())
					return;

		QString sActionID;

		if(pObj->getType()==MIhmDskVisibleObject::enuIhmIco
			||pObj->getType()==MIhmDskVisibleObject::enuIhmLabel)
		{
			sActionID = pObj->getActionID();
			
			if(sActionID!="")
				onAction(sActionID, "");
		}
	}
}


bool MFormMain::eventFilter(QObject *obj, QEvent *event)
{
	if(event->type() == QEvent::FocusIn)
	{
		//QFocusEvent *focusEvent = static_cast<QFocusEvent *>(event);
		//TRACE_D(QString("MFormMain::eventFilter: Object in focus %1 ")
		//							.arg(obj->objectName()));
	}

	if (event->type() == QEvent::MouseMove || 
		event->type() == QEvent::KeyRelease || 
		event->type() == QEvent::MouseButtonRelease) 
	{
		resetIdleTimer();
	}

	if(event->type() == QEvent::MouseButtonPress) 
	{
		resetIdleTimer();
		QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

		if(mouseEvent->button()==Qt::LeftButton)
		handleMouseClick(obj);

		return false;
	}
	else if(event->type() == QEvent::KeyPress) 
	{
		resetIdleTimer();
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		return keyPressEventHandler(obj, keyEvent);
	}
	else {
		
		 // standard event processing
		 return QObject::eventFilter(obj, event);
	}
}

void MFormMain::closeEvent(QCloseEvent * event)
{
   event->ignore();
}



void MFormMain::updateCtrlStatusLabel(bool bInControl)
{
	MIhmLabel * pCurrent;
	pCurrent = (MIhmLabel *)MIhmDskVisibleObject::findVisibleObjectByID(&m_vectDskVisibleObjects,
											MIhmDskVisibleObject::enuTEXT_LABEL_IN_CONTROL_STATUS);
	
	if(pCurrent==NULL)
	{
		TRACE_W(QString( "MFormMain::updateCtrlStatusLabel: findVisibleObjectByID return NULL for the visible object name %1")
									.arg(LABEL_REG_VAL_IN_CONTROL_STATUS));
		return;
	}

	if(pCurrent->getType()!=MIhmDskVisibleObject::enuIhmLabel)
	{
		TRACE_W(QString( "MFormMain::updateCtrlStatusLabel: invalid visible object type:[%1]!")
									.arg(LABEL_REG_VAL_IN_CONTROL_STATUS));
		return;
	}

	if(pCurrent->isNotUsed()) 
	{
		TRACE_D(QString( "MFormMain::updateCtrlStatusLabel: Visible object [%1] is flagged NOT_USED in configuration!")
									.arg(LABEL_REG_VAL_IN_CONTROL_STATUS));
		return;
	}

	QLabel *lbl = pCurrent->getLabel();

	if(lbl!=NULL)
	{
		MIhmLanguages * pLang = MIhmConfigGeneral::getCfg()->getLanguages();
		
		QString sText;
		if(bInControl)
		{
			sText = pLang->getInControlTranslation(MIhmLanguages::enuTranslTargetDesktop);
		}
		else
		{
			sText = pLang->getNotInControlTranslation(MIhmLanguages::enuTranslTargetDesktop);
		}

		lbl->setText(sText);
		lbl->setVisible(true);
	}
}



void MFormMain::updateInCtrlVisibleObjects(bool bInControl)
{
	MIhmDskVisibleObject * pCurrent;

	for(int i=MIhmDskVisibleObject::enuIhmVisibleObjUnknown+1;i<m_vectDskVisibleObjects.size();i++)
	{
		
		if(m_vectDskVisibleObjects.at(i)!=NULL)
		{
			pCurrent = m_vectDskVisibleObjects.at(i);

			if(i == MIhmDskVisibleObject::enuCNT_CNT_COMMENTS || 
				i == MIhmDskVisibleObject::enuCNT_CNT_CAMERA || 
				i == MIhmDskVisibleObject::enuCNT_CNT_SCANNER)
			{
				MIhmVirtualObject * pVirtObj = NULL;

				if(pCurrent->getName() == CNT_REG_VAL_CNT_COMMENTS)
				{
					pVirtObj = m_pDskIntf->findVirtualObjectByID(MIhmVirtualObject::enuIhmVirtCntComments);
				}
				else if(pCurrent->getName() == CNT_REG_VAL_CNT_CAMERA)
				{
					pVirtObj = m_pDskIntf->findVirtualObjectByID(MIhmVirtualObject::enuIhmVirtCntCamera);
				}
				else if(pCurrent->getName() == CNT_REG_VAL_CNT_SCANNER)
				{
					pVirtObj = m_pDskIntf->findVirtualObjectByID(MIhmVirtualObject::enuIhmVirtCntScanner);
				}

				//this container should be visible only if client is in control
				if(pVirtObj!=NULL)
				{
					bool bVisible = pVirtObj->isVisible() && bInControl;
					updateContainer((MIhmDskVisibleObject::enumVisibleObjectId)i, bVisible, (MIhmVirtContainer*)pVirtObj);
				}
			}
			else
				pCurrent->setReadOnly(!bInControl);
		}		
	}	
}



void MFormMain::updateAuthStatus()
{
	TRACE_D(QString( "MFormMain::updateAuthStatus:..."));

	if(m_pDskIntf->isInControl()||m_pDskIntf->isIdentified())
		startIdleTimer();
	else
		stopIdleTimer();

	updateCtrlStatusLabel(m_pDskIntf->isInControl());
	
	//Update in control status to all visible objects
	updateInCtrlVisibleObjects(m_pDskIntf->isInControl());

	//Hide visible modal dialogs (if any) since it does not belong to the visible object list
	if(!m_pDskIntf->isInControl())
	{
		if(isInputDialogInProgress())
			hideInputDialog();

		if(isInputDlgExInProgress())
			hideInputDlgEx();
	}

	updateTakeControlButtons();	
}


bool MFormMain::isAnyInputDlgInProgress()
{
	return isInputDialogInProgress() || isInputDlgExInProgress();
}


//-------------------------------------------------------------------------
//----------		Processing of MIhmVirtInputDialogReq	---------------
//-------------------------------------------------------------------------

void MFormMain::processInputDialogReq(MIhmVirtInputDialogReq* pVirtObj)
{
	//first verify if the request list is empty to check if there was a 
	//SASIE_VISU ANNULE (CANCEL) request
	bool bInputCancelReq = pVirtObj->isCancel();

	TRACE_D(QString("MFormMain::processInputDialogReq: CancelReq:%1")
									.arg(bInputCancelReq?1:0));

	if(bInputCancelReq )
	{
		//if it is an input cancel request 
		//close the dialog
		onInputDialogCMDCancel();
	}
	else if(!pVirtObj->isEmpty() && !isAnyInputDlgInProgress()) //if no dialog is up raise next one
	{	
		raiseNextInputDialog(pVirtObj);
	}
	else if(isInputDialogInProgress())
	{
		MInputDialogReq oReq;
		if(pVirtObj->getCurrentReq(&oReq))
		{
			if(!(oReq==m_oCurrentInputReq))
			{
				hideInputDialog();
				raiseNextInputDialog(pVirtObj);
			}
		}
		else
			hideInputDialog();
	}

}

bool MFormMain::isInputDialogInProgress()
{
	return (m_pInputDialog!=NULL);
}


void MFormMain::raiseNextInputDialog(MIhmVirtInputDialogReq* pVirtObj)
{
	if(pVirtObj==NULL)
	{
        TRACE_W(QString("MFormMain::raiseNextInputDialog: pVirtObj==NULL!"));
		return;
	}

	bool bOK = false;

	m_pInputDialog = new MFormInput(this);
	m_pInputDialog->installEventFilter(this);
	QObject::connect(m_pInputDialog,SIGNAL(accepted()),this,SLOT(onInputDialogOK()));
	QObject::connect(m_pInputDialog,SIGNAL(rejected()),this,SLOT(onInputDialogUSRCancel()));
	setModalWindow(m_pInputDialog);
	
	if(pVirtObj->getCurrentReq(&m_oCurrentInputReq))
	{
		TRACE_D(QString("MFormMain::raiseNextInputDialog: getCurrentReq:%1")
									.arg(m_oCurrentInputReq.getDefinition()));
		
		if(m_pInputDialog->initialize(&m_oCurrentInputReq))
		{	
			m_pInputDialog->show();
			bOK = true;
		}
		else
		{
			TRACE_W(QString("MFormMain::raiseNextInputDialog: Unable to initialize:%1")
									.arg(m_oCurrentInputReq.getDescription()));
			sendInputDialogInitErr();
		}
	}
	else
	{
		TRACE_W(QString("MFormMain::raiseNextInputDialog: Unable to read next input request!"));
		sendInputDialogInitErr();
	}

	if(!bOK)
		deleteModalWindow((QWidget**)&m_pInputDialog);
		
}


void MFormMain::onInputDialogCMDCancel()
{
	if(m_pInputDialog!=NULL) 
	{
		deleteModalWindow((QWidget**)&m_pInputDialog);
	}	
	
	sendInputDlgCanceled();
}

void MFormMain::hideInputDialog()
{
	if(m_pInputDialog!=NULL) 
	{
		deleteModalWindow((QWidget**)&m_pInputDialog);
	}
}



void MFormMain::onInputDialogOK()
{
    TRACE_D(QString( "MFormMain::onInputDialogOK"));

	QString sResult = m_pInputDialog->getResult();
	deleteModalWindow((QWidget**)&m_pInputDialog);
	sendInputDialogRsp(false, sResult);

}

void MFormMain::onInputDialogUSRCancel()
{
    TRACE_D(QString( "MFormMain::onInputDialogUsrCancel"));

	QString sResult = m_pInputDialog->getResult();
	deleteModalWindow((QWidget**)&m_pInputDialog);
	sendInputDialogRsp(true, sResult);
}


void MFormMain::sendInputDialogRsp(bool bCanceled, QString sInputRes)
{
	MIhmMsgInputDialogRes *pMsg = new MIhmMsgInputDialogRes();

	pMsg->oInputReq = m_oCurrentInputReq;
	pMsg->bCanceled = bCanceled;
	pMsg->sInputResult = sInputRes;

	m_pDskIntf->sendInputDialogResponse(pMsg);
}

void MFormMain::sendInputDialogInitErr()
{
	MIhmMsgInputDialogInitErr *pMsg = new MIhmMsgInputDialogInitErr();

	pMsg->oInputReq = m_oCurrentInputReq;
	m_pDskIntf->sendInputDialogResponse(pMsg);
}


void MFormMain::sendInputDlgCanceled()
{
	MIhmMsgInputDlgCanceled *pMsg = new MIhmMsgInputDlgCanceled();
	m_pDskIntf->sendInputDialogResponse(pMsg);
}




void MFormMain::onVideoGrabbed(int iVirtObjId, bool bResult)
{
	TRACE_D(QString("MFormMain::onVideoGrabbed: VirtObj:%1;  Result:%2")
				.arg(MIhmVirtualObject::getNameForId((MIhmVirtualObject::enumVirtualObjectId)iVirtObjId))
				.arg(bResult));

	MIhmMsgVideoFreezeRsp *pMsg = new MIhmMsgVideoFreezeRsp(iVirtObjId);
	pMsg->m_iResult = bResult?1:0;

	m_pDskIntf->sendVideoCtlResponse(pMsg);
}

void MFormMain::onVideoSaved(int iVirtObjId, bool bResult)
{
	TRACE_D(QString("MFormMain::onVideoSaved: VirtObj:%1;  Result:%2")
				.arg(MIhmVirtualObject::getNameForId((MIhmVirtualObject::enumVirtualObjectId)iVirtObjId))
				.arg(bResult));

	MIhmMsgVideoSaveRsp *pMsg = new MIhmMsgVideoSaveRsp(iVirtObjId);
	pMsg->m_iResult = bResult?1:0;
	
	m_pDskIntf->sendVideoCtlResponse(pMsg);
}





//-------------------------------------------------------------------------
//----------END of processing of MIhmVirtInputDialogReq	---------------
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
//----------Processing of MIhmVirtInputDlgExReq				---------------
//-------------------------------------------------------------------------


void MFormMain::processInputDlgExReq(MIhmVirtInputDlgExReq* pVirtObj)
{
	//first verify if the request list is empty to check if there was a 

	TRACE_D(QString("MFormMain::processInputDlgExReq: CancelReq:%1")
									.arg(pVirtObj->isCancel()?1:0));

	if(pVirtObj->isCancel())
	{
		//if it is an input cancel request 
		onInputDlgExCMDCancel();
	}
	else if(!pVirtObj->isEmpty() && !isAnyInputDlgInProgress())
	{	
		raiseNextInputDlgEx(pVirtObj);
	}
	else if(isInputDlgExInProgress())
	{

		MInputDialogExReq oReq;
		if(pVirtObj->getCurrentExReq(&oReq))
		{
			if(!(oReq == m_oCurrentInputExReq))
			{
				TRACE_W(QString("MFormMain::processInputDlgExReq: It seems that the current request in the model is different from the one that is executed to the client!"));
				hideInputDlgEx();
				raiseNextInputDlgEx(pVirtObj);
			}
		}
		else
			hideInputDlgEx();

	}

}

bool MFormMain::isInputDlgExInProgress()
{
	return (m_pInputDlgEx!=NULL);
}




void MFormMain::raiseNextInputDlgEx(MIhmVirtInputDlgExReq* pVirtObj)
{
	if(pVirtObj==NULL)
	{
        TRACE_W(QString("MFormMain::raiseNextInputDlgEx: pVirtObj==NULL!"));
		return;
	}

	bool bOK = false;

	if(pVirtObj->getCurrentExReq(&m_oCurrentInputExReq))
	{
		switch(m_oCurrentInputExReq.getDialogType())
		{
		case MInputDialogExReq::enuSAISIE_EX_GENERIC:
				m_pInputDlgEx = new MFormInputGen(this);
			break;
		case MInputDialogExReq::enuSAISIE_EX_SEL_FILE:
		case MInputDialogExReq::enuSAISIE_EX_SEL_FOLDER:
				m_pInputDlgEx = new MFormFileOpen(this, m_oCurrentInputExReq.getDialogType());
			break;
		case MInputDialogExReq::enuSAISIE_EX_KEYBOARD_TEST:
				m_pInputDlgEx = new MFormTestKeyboard(this);
				setKeyboardTestingInProgress();
			break;
		
		case  MInputDialogExReq::enuSAISIE_EX_CUSTOM_PLUGIN_DLG:
			m_pInputDlgEx = new MFormCustomPluginDlg(this);

		case MInputDialogExReq::enuSAISIE_EX_UNKNOWN:
		default:
				if(m_pInputDlgEx==NULL)
				{
					TRACE_W(QString("MFormMain::raiseNextInputDlgEx: Unable to initialize Dialog_id:%1! Invalid dialog type!")
									.arg(m_oCurrentInputExReq.getDialogId()));
					sendInputDlgExInitErr();

				}	
			break;
		}
		
		//m_pInputDlgEx->installEventFilter(this);
		QObject::connect(m_pInputDlgEx,SIGNAL(accepted()),this,SLOT(onInputDlgExOK()));
		QObject::connect(m_pInputDlgEx,SIGNAL(rejected()),this,SLOT(onInputDlgExUSRCancel()));

		setModalWindow(m_pInputDlgEx);

		TRACE_D(QString("MFormMain::raiseNextInputDlgEx: Dialog_id:%1")
									.arg(m_oCurrentInputExReq.getDialogId()));
		
 		if(m_pInputDlgEx->initialize(&m_oCurrentInputExReq))
		{	
			m_pInputDlgEx->show();
			sendInputDlgExOppened();
			
			bOK = true;
		}
		else
		{
			TRACE_W(QString("MFormMain::raiseNextInputDlgEx: Unable to initialize Dialog_id:%1")
									.arg(m_oCurrentInputExReq.getDialogId()));
			sendInputDlgExInitErr();
		}
	}
	else
	{
		TRACE_W(QString("MFormMain::raiseNextInputDlgEx: Unable to read next input request!"));
		sendInputDlgExInitErr();
	}

	if(!bOK)
		deleteModalWindow((QWidget**)&m_pInputDlgEx);

}

void MFormMain::onInputDlgExCMDCancel()
{
    TRACE_D(QString("MFormMain::onInputDlgExCMDCancel"));

	if(m_pInputDlgEx!=NULL) 
	{	
		deleteModalWindow((QWidget**)&m_pInputDlgEx);
	}
	
	sendInputDlgExCanceled();
}


void MFormMain::hideInputDlgEx()
{
	if(m_pInputDlgEx!=NULL) 
	{	
		m_pInputDlgEx->close();
		deleteModalWindow((QWidget**)&m_pInputDlgEx);
	}
}


void MFormMain::onInputDlgExOK()
{
    TRACE_D(QString("MFormMain::onInputDlgExOK"));

	QString sResult = m_pInputDlgEx->getResult();
	deleteModalWindow((QWidget**)&m_pInputDlgEx);
	sendInputDlgExRsp(false, sResult);

}

void MFormMain::onInputDlgExUSRCancel()
{
    TRACE_D(QString("MFormMain::onInputDlgExUSRCancel"));

	deleteModalWindow((QWidget**)&m_pInputDlgEx);
	sendInputDlgExRsp(true, "");
}


void MFormMain::sendInputDlgExRsp(bool bCanceled, QString sInputRes)
{
	MIhmMsgInputDlgExRes *pMsg = new MIhmMsgInputDlgExRes();

	pMsg->oInputReq = m_oCurrentInputExReq;
	pMsg->bCanceled = bCanceled;
	pMsg->sInputResult = sInputRes;

	m_pDskIntf->sendInputDialogResponse(pMsg);
}

void MFormMain::sendInputDlgExInitErr()
{
	MIhmMsgInputDlgExInitErr *pMsg = new MIhmMsgInputDlgExInitErr();

	pMsg->oInputReq = m_oCurrentInputExReq;

	m_pDskIntf->sendInputDialogResponse(pMsg);
}


void MFormMain::sendInputDlgExCanceled()
{
	MIhmMsgInputDlgExCanceled *pMsg = new MIhmMsgInputDlgExCanceled();

	m_pDskIntf->sendInputDialogResponse(pMsg);
}

void MFormMain::sendInputDlgExOppened()
{
	MIhmMsgInputDlgExOpened *pMsg = new MIhmMsgInputDlgExOpened();
	pMsg->oInputReq = m_oCurrentInputExReq;
	
	m_pDskIntf->sendInputDialogResponse(pMsg);
}

void MFormMain::sendLoginDlgOppened()
{
	MIhmMsgLoginDlgOpened *pMsg = new MIhmMsgLoginDlgOpened();
	m_pDskIntf->sendMsgToAni(pMsg);
}

void MFormMain::sendLoginDlgClosed()
{
	MIhmMsgLoginDlgClosed *pMsg = new MIhmMsgLoginDlgClosed();
	m_pDskIntf->sendMsgToAni(pMsg);
}



//-------------------------------------------------------------------------
//----------END Processing of MIhmVirtInputDlgExReq		---------------
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
//----------Processing of MIhmVirtAsyncInput				---------------
//-------------------------------------------------------------------------


void MFormMain::processAsyncInput(MIhmVirtAsyncInput* pVirtObj)
{
	TRACE_D(QString("MFormMain::processAsyncInput..."));

	if(m_pDskAsyncInputs!=NULL)
	{
		//If not initialized initialize the async input objects on this form.
		// The first update is done by config lane type and this is the time
		// when the async input DEF file for this type of lane is loaded.
		// After that only fields values are updated
		if(!m_pDskAsyncInputs->isInitialized())
		{
			if(m_pDskAsyncInputs->initialize(pVirtObj->m_pAsyncReq, m_dynFormWidget))
			{
				connect(m_pDskAsyncInputs, SIGNAL(dataChanged()), this, SLOT(onAsyncInputDataChanged()));
			}
			else
			{
				TRACE_W(QString("MFormMain::processAsyncInput: Error initialize returned false!"));
			}
		}	
		else
		{
			if(!m_pDskAsyncInputs->updateValues(pVirtObj->m_pAsyncReq))
			{
				TRACE_W(QString("MFormMain::processAsyncInput: Error updateValues returned false!"));
			}			
		}

	}
	else
	{
		TRACE_W(QString("MFormMain::processAsyncInput: m_pDskAsyncInputs == NULL"));

	}
}

//-------------------------------------------------------------------------
//----------END Processing of MIhmVirtAsyncInput				---------------
//-------------------------------------------------------------------------


//-----------------------------------------------
//-----------------------------------------------

//THis one open and closes identification dialog on desktop UI
void MFormMain::openLoginDlg(QString sLoginName, bool bLoginFieldEnabled, bool bLDAPOffline, bool bShow)
{
	bool bDlgWasOpened = false;

	TRACE_D(QString( "MFormMain::openLoginDlg: %1").arg(sLoginName));

	//delete login dialog...
	if(m_pLoginDialog!=NULL)
	{	
		MFormLogin * p = m_pLoginDialog;
		m_pLoginDialog = NULL;

		bDlgWasOpened = true; 
		delete p;
	}

	if(m_pCurrentClientDlg!=NULL)
	{	
		onTakeOverReject(); //to force closing the take over dlg if any
	}


	if(!bShow) //if not show than it should be hidden
	{
		if(bDlgWasOpened)
		{
			TRACE_D(QString( "MFormMain::openLoginDlg: Login dialog closed by main app request!"));
			emit authDlgCanceled(enuRETURN_CTRL_FORCED);
		}
		return;	
	}
	else
	{

	
		QString sLoginTemplate = m_pDskIntf->getCurrentLaneTypeParam(CFG_LANE_TYPE_PARAM_LOGIN_DLG_TEMPLATE);

		if(sLoginTemplate.isEmpty())
		{
			TRACE_W(QString( "MFormMain::openLoginDlg: Undefined template file %1!")
										.arg(CFG_LANE_TYPE_PARAM_LOGIN_DLG_TEMPLATE));
			return;
		}
		
		QString sEmbedTo = m_pDskIntf->getCurrentLaneTypeParam( CFG_LANE_TYPE_PARAM_CLIENT_DLG_EMBEDED_TO);
		QString sCSSPath = m_pDskIntf->getCurrentLaneTypeParam( CFG_LANE_TYPE_PARAM_CLIENT_CSS_FILE);
		
		if(!sCSSPath.isEmpty())
			sCSSPath = MIhmConfigGeneral::getCfg()->getCSSFullPath(sCSSPath);


		m_pLoginDialog = new MFormLogin(this);
		connect(m_pLoginDialog, SIGNAL(accepted()), this, SLOT(onLoginDlgOK()));
		connect(m_pLoginDialog, SIGNAL(rejected()), this, SLOT(onLoginDlgCancel()));

		if(m_pLoginDialog->initialize(sLoginName, sLoginTemplate, sEmbedTo, sCSSPath, bLoginFieldEnabled, bLDAPOffline))
		{
			m_pLoginDialog->show();
			sendLoginDlgOppened();
		}	
		else
		{
			TRACE_W(QString( "MFormMain::openLoginDlg:Error initializing !!!"));
			emit exitProgram();
		}
	}

}


void MFormMain::onLoginDlgOK()
{
    TRACE_D(QString("MFormMain::onLoginDlgOK:..."));

	QString sName, sSecCode;
	bool bLDAPOffline;
	m_pLoginDialog->getLoginData(sName, sSecCode, bLDAPOffline);
	
	MFormLogin * p = m_pLoginDialog;
	m_pLoginDialog = NULL;
	delete p;

	this->activateWindow();
	
	emit identificationReq(sName, sSecCode, bLDAPOffline);
	sendLoginDlgClosed();
	
}

void MFormMain::onLoginDlgCancel()
{
    TRACE_D(QString("MFormMain::onLoginDlgCancel:... "));

	MFormLogin * p = m_pLoginDialog;
	m_pLoginDialog = NULL;
	delete p;

	this->activateWindow();
	
	emit authDlgCanceled(enuRETURN_CTRL_VOLUNTARY);
	sendLoginDlgClosed();

}

//--------------------------------------------------------------------------------
//   Taking control ...
//--------------------------------------------------------------------------------

void MFormMain::prepareClientInfoDialogs()
{
	QString sTakeOverTemplate = m_pDskIntf->getCurrentLaneTypeParam(CFG_LANE_TYPE_PARAM_TAKE_OVER_DLG_TEMPLATE);

	if(sTakeOverTemplate.isEmpty())	
	{
		TRACE_W(QString("MFormMain::initTakeOver: Undefined template file %1!")
									.arg(CFG_LANE_TYPE_PARAM_TAKE_OVER_DLG_TEMPLATE));
		return;
	}
	
	QString sEmbedTo = m_pDskIntf->getCurrentLaneTypeParam(CFG_LANE_TYPE_PARAM_CLIENT_DLG_EMBEDED_TO);
	QString sCSSPath = m_pDskIntf->getCurrentLaneTypeParam(CFG_LANE_TYPE_PARAM_CLIENT_CSS_FILE);
	if(!sCSSPath.isEmpty())
		sCSSPath = MIhmConfigGeneral::getCfg()->getCSSFullPath(sCSSPath);

	
	if(m_pTakeOverMsg!=NULL)
	{
		delete m_pTakeOverMsg; m_pTakeOverMsg = NULL;
	}

	m_pTakeOverMsg = new MFormTakeOverDlg(this);
	connect(m_pTakeOverMsg, SIGNAL(accepted()), this, SLOT(onTakeOverOK()));
	connect(m_pTakeOverMsg, SIGNAL(rejected()), this, SLOT(onTakeOverReject()));

	if(!m_pTakeOverMsg->initialize(MFormTakeOverDlg::enuTakeOverMsg,sTakeOverTemplate, sEmbedTo, sCSSPath))
	{
		TRACE_W(QString("MIhmDesktopIntf::initTakeOver:Unable to intialize  MFormTakeOverDlg enuTakeOverMsg!"));
		emit exitProgram();
	}


	if(m_pTakeOverQuestion!=NULL)
	{
		delete m_pTakeOverQuestion; m_pTakeOverQuestion = NULL;
	}

	m_pTakeOverQuestion= new MFormTakeOverDlg(this);
	connect(m_pTakeOverQuestion, SIGNAL(accepted()), this, SLOT(onTakeOverOK()));
	connect(m_pTakeOverQuestion, SIGNAL(rejected()), this, SLOT(onTakeOverReject()));

	if(!m_pTakeOverQuestion->initialize(MFormTakeOverDlg::enuTakeOverQuestion,sTakeOverTemplate, sEmbedTo, sCSSPath))
	{
		TRACE_W(QString("MIhmDesktopIntf::initTakeOver:Unable to intialize  MFormTakeOverDlg enuTakeOverQuestion!"));
		emit exitProgram();
	} 	


	if(m_pClientInfoMsg!=NULL)
	{
		delete m_pClientInfoMsg; m_pClientInfoMsg = NULL;
	}

	m_pClientInfoMsg= new MFormTakeOverDlg(this);
	connect(m_pClientInfoMsg, SIGNAL(accepted()), this, SLOT(onTakeOverOK()));
	connect(m_pClientInfoMsg, SIGNAL(rejected()), this, SLOT(onTakeOverReject()));

	if(!m_pClientInfoMsg->initialize(MFormTakeOverDlg::enuClientInfoMsg,sTakeOverTemplate, sEmbedTo, sCSSPath))
	{
		TRACE_W(QString("MIhmDesktopIntf::initTakeOver:Unable to intialize  MFormTakeOverDlg enuClientInfoMsg!"));
		emit exitProgram();
	} 	


	if(m_pClientConnectingMsg!=NULL)
	{
		delete m_pClientConnectingMsg; m_pClientConnectingMsg = NULL;
	}

	m_pClientConnectingMsg = new MFormTakeOverDlg(this);
	connect(m_pClientConnectingMsg, SIGNAL(accepted()), this, SLOT(onTakeOverOK()));
	connect(m_pClientConnectingMsg, SIGNAL(rejected()), this, SLOT(onTakeOverReject()));

	if(!m_pClientConnectingMsg->initialize(MFormTakeOverDlg::enuClientConnectingMsg, sTakeOverTemplate, sEmbedTo, sCSSPath))
	{
		TRACE_W(QString("MIhmDesktopIntf::initTakeOver:Unable to intialize  MFormTakeOverDlg!"));
		emit exitProgram();
	} 	

}

//Shows the message dialog with CLOSE button
//(no response from the user is expected)
void MFormMain::showClientInfoMsg(QString sMsg, QString sMsgTitle)
{
	MIhmLanguages *pLang = MIhmConfigGeneral::getCfg()->getLanguages();
	QString sTranslatedTitle;
	QString sTranslatedMsg;

	pLang->getLabelTranslation(sMsg, MIhmLanguages::enuTranslTargetDesktop, sTranslatedMsg);
	pLang->getLabelTranslation(sMsgTitle, MIhmLanguages::enuTranslTargetDesktop, sTranslatedTitle);

	showClientDialog(MFormTakeOverDlg::enuClientInfoMsg, sTranslatedTitle, sTranslatedMsg);
}

void MFormMain::hideClientInfoMsg()
{
	if(m_pCurrentClientDlg!=NULL)
	{
		MFormTakeOverDlg::enumDialogType eType = m_pCurrentClientDlg->getDlgType();
		if(eType == MFormTakeOverDlg::enuClientInfoMsg)
		{
			hideClientDialog();
		}
	}
}



void MFormMain::showConnectingMsg(QString sMsg, QString sMsgTitle)
{
	MIhmLanguages *pLang = MIhmConfigGeneral::getCfg()->getLanguages();
	QString sTranslatedTitle;
	QString sTranslatedMsg;

	pLang->getLabelTranslation(sMsg, MIhmLanguages::enuTranslTargetDesktop, sTranslatedMsg);
	pLang->getLabelTranslation(sMsgTitle, MIhmLanguages::enuTranslTargetDesktop, sTranslatedTitle);

	showClientDialog(MFormTakeOverDlg::enuClientConnectingMsg, sTranslatedTitle, sTranslatedMsg);
}

void MFormMain::hideConnectingMsg()
{
	if(m_pCurrentClientDlg!=NULL)
	{
		MFormTakeOverDlg::enumDialogType eType = m_pCurrentClientDlg->getDlgType();
		if(eType == MFormTakeOverDlg::enuClientConnectingMsg)
		{
			hideClientDialog();
		}
	}
}



void  MFormMain::cancelTakeOver()
{
	if(m_pCurrentClientDlg!=NULL)
	{
		if(m_pCurrentClientDlg->getDlgType()!=MFormTakeOverDlg::enuClientInfoMsg)
		{
			hideClientDialog();
		}
	}
}



void MFormMain::initTakeOver(bool bQuest, QString sTakeOverUserName)
{

	if(bQuest&&!m_pDskIntf->isInControl())	
	{
		TRACE_W(QString("MFormMain::initTakeOver: Invalid control state!"));
		return;
	}

	MIhmLanguages *pLang = MIhmConfigGeneral::getCfg()->getLanguages();
		
	if(!m_pDskIntf->isInControl())
	{
		QString sMsg = pLang->getTakeOverInfoTranslation(MIhmLanguages::enuTranslTargetDesktop);
		QString sMsgTitle = pLang->getTakeOverInfoTitleTranslation(MIhmLanguages::enuTranslTargetDesktop);

		showClientDialog(MFormTakeOverDlg::enuTakeOverMsg,sMsgTitle,sMsg);
	}
	else
	{
		
 		QString sQuestionMsg = pLang->getTakeOverQuestionTranslation(MIhmLanguages::enuTranslTargetDesktop, sTakeOverUserName);
		QString sQuestionTtile = pLang->getTakeOverQuestionTitleTranslation(MIhmLanguages::enuTranslTargetDesktop);
		showClientDialog(MFormTakeOverDlg::enuTakeOverQuestion,sQuestionTtile,sQuestionMsg);	
	}

}


void MFormMain::showClientDialog(MFormTakeOverDlg::enumDialogType eType, QString sMsgTitle, QString sMsg)
{
	MFormTakeOverDlg * p = NULL;

	if(eType==MFormTakeOverDlg::enuTakeOverMsg)
	{
		p = m_pTakeOverMsg;
	}
	else if(eType==MFormTakeOverDlg::enuTakeOverQuestion)
	{
		p = m_pTakeOverQuestion;
	}
	else if(eType==MFormTakeOverDlg::enuClientConnectingMsg)
	{
		p = m_pClientConnectingMsg;
	}
	else if(eType==MFormTakeOverDlg::enuClientInfoMsg)
	{
		p = m_pClientInfoMsg;
	}

	if (p != NULL)
	{
		p->update(sMsgTitle, sMsg);
		m_pCurrentClientDlg = p;
		m_pCurrentClientDlg->show();
	}
	else
	{
		 TRACE_W(QString("MFormMain::showClientDialog: Error showing dialog type %1... ").arg((int)eType));
	}
}

void MFormMain::hideClientDialog()
{
	MFormTakeOverDlg * p = m_pCurrentClientDlg;
	m_pCurrentClientDlg = NULL;
	p->hide();

	this->activateWindow();

}


void MFormMain::onTakeOverOK()
{
    TRACE_D(QString("MFormMain::onTakeOverOK: ... "));

	if(m_pCurrentClientDlg!=NULL)
	{
		MFormTakeOverDlg::enumDialogType eType = m_pCurrentClientDlg->getDlgType();
		hideClientDialog();

		if(eType==MFormTakeOverDlg::enuTakeOverMsg)
		{
			emit takeOverCancel();
		}
		else if(eType==MFormTakeOverDlg::enuTakeOverQuestion)
		{
			emit takeOverAccept();
		}
		else if(eType==MFormTakeOverDlg::enuClientConnectingMsg)
		{
			emit takeOverCancel();
		}
	}
	
}

void MFormMain::onTakeOverReject()
{
    TRACE_D(QString("MFormMain::onTakeOverReject: ... "));

	if(m_pCurrentClientDlg!=NULL)
	{
		MFormTakeOverDlg::enumDialogType eType = m_pCurrentClientDlg->getDlgType();
		hideClientDialog();

		if(eType==MFormTakeOverDlg::enuTakeOverMsg)
		{
			emit takeOverCancel();
		}
		else if(eType==MFormTakeOverDlg::enuTakeOverQuestion)
		{
			emit takeOverReject();
		}
		else if(eType==MFormTakeOverDlg::enuClientConnectingMsg)
		{
			emit takeOverCancel();
		}

	}

}

//-----------------------------------------------
//  END Taking control ...
//-----------------------------------------------


//-----------------------------------------------
//Simulate modality ...
//-----------------------------------------------

void MFormMain::setModalWindow(QWidget * p)
{
	m_pModalWin = p;
}

QWidget * MFormMain::getModalWindow()
{
	return m_pModalWin;
}

void MFormMain::deleteModalWindow(QWidget ** p)
{
	if(*p!=NULL)
	{
		if(isKeyboardTestingInProgress())
			unsetKeyboardTestingInProgress();

		//replace only if it is the current modal window
		if(*p == m_pModalWin) 
			m_pModalWin = NULL;
		else
		{
			TRACE_W(QString("MFormMain::deleteModalWindow:  Deleting dialog which is not modal!"));
		}
		
		delete *p;
		*p = NULL;
	}
}



void MFormMain::changeEvent(QEvent * event)
{
	if(event->type() == QEvent::ActivationChange)
	{
		//TRACE_D(QString("MFormMain::changeEvent: ActivationChange ... %1").arg((int)qApp->applicationState()));

		if (qApp->applicationState() == Qt::ApplicationActive)
		{
			if (m_pLoginDialog != NULL)
			{
				TRACE_D(QString("MFormMain::changeEvent: m_pLoginDialog != NULL"));
				m_pLoginDialog->raise();
				m_pLoginDialog->activateWindow();
			}
			else if (m_pCurrentClientDlg != NULL)
			{
				TRACE_D(QString("MFormMain::changeEvent: m_pCurrentClientDlg != NULL"));
				m_pCurrentClientDlg->raise();
				m_pCurrentClientDlg->activateWindow();
			}
			else if (m_pAboutDlg != NULL)
			{
				TRACE_D(QString("MFormMain::changeEvent: m_pAboutDlg != NULL"));
				m_pAboutDlg->raise();
				m_pAboutDlg->activateWindow();
			}
			else
			{ 
				QWidget * pModalWin = getModalWindow();

				if (pModalWin != NULL)
				{
					TRACE_D(QString("MFormMain::changeEvent: pModalWin != NULL"));
					pModalWin->raise();
					pModalWin->activateWindow();
				}
				else
				{
					if (!this->isActiveWindow())
					{
						TRACE_D(QString("MFormMain::changeEvent: main wnd not active - activating..."));
						this->activateWindow();
					}
					else
					{
						;// TRACE_D(QString("MFormMain::changeEvent: main window already activated!"));

					}
				}
			}

		}

	}
	
	QWidget::changeEvent(event);
}

//-----------------------------------------------
//END Simulate modality ...
//-----------------------------------------------
