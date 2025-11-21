



#include <QApplication.h>
#include <MKybMainLogic.h>
#include <MKybAniThread.h>
#include <MKybMsg.h>
#include <MKybAniInitData.h>
#include <MKybFormMain.h>

#include <MTracer.h>

MKybMainLogic::MKybMainLogic():
	m_pAniThread(NULL),m_bModuleInitOK(false),m_pMainDialog(NULL)

{
	m_bStandaloneMode = false;
}


MKybMainLogic::~MKybMainLogic()
{
	TRACE_D(QString("MKybMainLogic::~MKybMainLogic: Closing the main logic object ..."));

	if(m_pAniThread!=NULL)
	{
		if(m_pAniThread->isRunning())
		{
			m_pAniThread->quit();
			
			Sleep(200);
			delete m_pAniThread;
		}
	}

	if(m_pMainDialog!=NULL)
	{
		delete m_pMainDialog;
		m_pMainDialog = NULL;
	}
	
	deinitTrace();

}


void MKybMainLogic::initTrace(QString sModuleMbox)
{
	// Initialiser la trace
	MTracer::Init(sModuleMbox);

	TRACE_D(QString("MKybMainLogic::initialize: Trace initialised"));
}

void MKybMainLogic::deinitTrace()
{
	MTracer::Deinit();
}




void MKybMainLogic::exitProgram()
{
    TRACE_D(QString("MKybMainLogic::exitProgram: Initiated ******!"));
	
	if(m_pAniThread!=NULL)
	{
		m_pAniThread->disconnect();

		if(m_pAniThread->isRunning())
		{
			m_pAniThread->stop();
			m_pAniThread->wait();
			delete m_pAniThread;
			m_pAniThread = NULL;
		}
	}
	
	qApp->exit(0);
}


void MKybMainLogic::sendMessageToAni(MKybMsg * pMsg)
{
	m_pAniThread->sendMessageToAni(pMsg);
}

enum_instance_result MKybMainLogic::Initialize(QString sBlName, QString sMode)
{
    TRACE_D(QString("MKybMainLogic::initialize: pcBalName = %1 mode=%2").
						arg(sBlName).arg(sMode));

	// Read general data
    if ( m_oConfGenData.loadConfigFromRegistry(sBlName) == false )
    {
        TRACE_W(QString( "MKybMainLogic::initialize: ERREUR loadConfigGeneralData retourne false"));
        return INST_INIT_ERR_REGISTRE;
    }


    TRACE_D(QString("MKybMainLogic::initializing: MKybAniThread ..."));

	noyau_bal_id iBalId;
	m_pAniThread = new MKybAniThread();
	m_pAniThread->Initialize(sBlName, m_oConfGenData.getModuleConfigKey());
	QObject::connect(m_pAniThread,SIGNAL(stopRequested()),this, SLOT(onAniStopRequested()), Qt::QueuedConnection);
	QObject::connect(m_pAniThread,SIGNAL(messageFromAni()),this, SLOT(onMessageFromAni()), Qt::QueuedConnection);
	m_pAniThread->start(m_oConfGenData.getAniPriority());

    // Attendre l'initialisation de la BAL du module par ANI
	iBalId = AttendBAL( sBlName.toLatin1().data());
    
	if ( iBalId <= 0 )
    {
		TRACE_W(QString("MKybMainLogic::initializing: Error starting MKybAniThread... "));
		return INST_INIT_ERR_LANCE;
	}
	else
	{
		TRACE_D(QString("MKybMainLogic::initializing: MKybAniThread initialized OK... "));
	}

	if(!sMode.isEmpty())	
	{
		QStringList sLst = sMode.trimmed().split("=");
		
		if((sLst.size()==2) && (sLst.at(0).toUpper()=="STANDALONE"))
		{
			m_bStandaloneMode = true;

			if(processConfigKeyboardType(sLst.at(1)))
				processConfigShow(true);
		}
	}

	return INST_INIT_OK;
}

void MKybMainLogic::onAniStopRequested()
{
	TRACE_W(QString( "MKybMainLogic::onAniStopRequested..."));

	exitProgram();
}


void MKybMainLogic::onKybFormStopRequested()
{
	TRACE_W(QString( "MKybMainLogic::onKybFormStopRequested..."));
	
	if(m_bStandaloneMode)
		exitProgram();
}


void MKybMainLogic::onMessageFromAni()
{
	QList <MKybMsg*> lstAniMsgs;

	if(m_pAniThread!=NULL)
		if(m_pAniThread->getMessagesFromAni(&lstAniMsgs))
		{
			while (!lstAniMsgs.isEmpty())
			{
				MKybMsg* pMsg = lstAniMsgs.takeFirst();

				if(pMsg!=NULL)
				{
					processMessageFromAni(pMsg);
					delete pMsg;
				}
			}
		}

}

void MKybMainLogic::processMessageFromAni(MKybMsg* pMsg)
{
	TRACE_D("MKybMainLogic::processMessageFromAni: ...");
	
	if(pMsg->getType()==MKybMsg::enuMsgKybConfig)
	{
		MKybMsgKybConfig* pConfigMsg = (MKybMsgKybConfig*)pMsg;
		processConfigKeyboardTypeMsg(pConfigMsg);
	}
	if(pMsg->getType()==MKybMsg::enuMsgKybShow)
	{
		MKybMsgKybShow* pMsgShow = (MKybMsgKybShow*)pMsg;
		processConfigShowMsg(pMsgShow);
	}
	else if(pMsg->getType()==MKybMsg::enuMsgKybMove)
	{
		MKybMsgKybMove* pMsgMove = (MKybMsgKybMove*)pMsg;
		processConfigMove(pMsgMove);
	}
	else
	{
		TRACE_W(QString("MKybMainLogic::processMessageFromAni: Unknown message type %1!").
			arg(pMsg->getType()));
	}
}

void MKybMainLogic::processConfigMove(MKybMsgKybMove* pMsgMove)
{
	if(m_sCurrentKyboardTypeID != "")
	{
		m_ptCurrentPosition.setX(pMsgMove->dwPosX);
		m_ptCurrentPosition.setY(pMsgMove->dwPosY);

		if(m_pMainDialog!=NULL)
			m_pMainDialog->move(m_ptCurrentPosition);
	}
}



void MKybMainLogic::processConfigShowMsg(MKybMsgKybShow* pMsgShow)
{
	processConfigShow(pMsgShow->m_bShow);
}


void MKybMainLogic::processConfigShow(bool bShow)
{
	if(m_sCurrentKyboardTypeID != "")
	{
		if(m_pMainDialog!=NULL)
			if(bShow)
			{
				m_pMainDialog->showMainDlg();
				m_pMainDialog->move(m_ptCurrentPosition);
			}
			else
				m_pMainDialog->hide();
	}
}


void MKybMainLogic::processConfigKeyboardTypeMsg(MKybMsgKybConfig* pConfigMsg)
{
	processConfigKeyboardType(pConfigMsg->m_sKyboardTypeID);
}

bool MKybMainLogic::processConfigKeyboardType(QString sNewKeybType)
{
	bool bRetVal = false;
	
	if(QString::compare(m_sCurrentKyboardTypeID, sNewKeybType,Qt::CaseInsensitive) != 0)
	{
		m_sCurrentKyboardTypeID = sNewKeybType;
		MKybTypeSettings * pNewSettings = m_oConfGenData.getKybTypeSettings(m_sCurrentKyboardTypeID);
		
		if(pNewSettings!=NULL)
		{
			TRACE_D(QString("MKybDesktopIntf::processConfigKeyboardType: Initializing sKeyboardTypeID:[%1]!").arg(m_sCurrentKyboardTypeID));
			
			//recreate the 	dialog
			if(m_pMainDialog!=NULL)
			{
				delete m_pMainDialog;
				m_pMainDialog = NULL;
			}
			
			m_pMainDialog = new MKybFormMain(pNewSettings->showTitleBar());
			
			//try load template form for this lane type
			bRetVal = m_pMainDialog->initialize(pNewSettings);
			if(bRetVal)
			{
				m_ptCurrentPosition = pNewSettings->getInitialPos();
				m_pMainDialog->move(m_ptCurrentPosition);
				
				if(m_bStandaloneMode)
				{
					QObject::connect(m_pMainDialog,SIGNAL(stopRequested()),this, SLOT(onKybFormStopRequested()));
				}
			}
			
		}else
		{
			bRetVal = false;	
			TRACE_W(QString("MKybDesktopIntf::processConfigKeyboardType: sKeyboardTypeID:[%1] not found!").arg(m_sCurrentKyboardTypeID));

		}
	}
	else
	{
		bRetVal = true;	
		TRACE_D(QString("MKybDesktopIntf::processConfigKeyboardType: sKeyboardTypeID:[%1] already initialized!").arg(m_sCurrentKyboardTypeID));
	}
	
	if(bRetVal==false)
	{
		//ignore the change and keep the old lane type
		TRACE_W(QString( "MKybDesktopIntf::processConfigLaneType: Unable to initialize sKeyboardTypeID:%1")
			.arg(m_sCurrentKyboardTypeID));
		
		if(m_pMainDialog!=NULL)
		{
			delete m_pMainDialog;
			m_pMainDialog = NULL;
		}

		m_sCurrentKyboardTypeID = "";
	}

	return bRetVal;
}
