



#include <QApplication.h>
#include <MIhmMainLogic.h>
#include <MIhmAniThread.h>
#include <MIhmMsg.h>
#include <MIhmAniInitData.h>
#include <MIhmTCLInterfaces.h>
#include <QTextCodec>
#include <MTracer.h>
#include "MIhmLoadedPlugins.h"


MIhmMainLogic::MIhmMainLogic():
	m_pAniThread(NULL),m_pTCLInterfaces(NULL),m_bModuleInitOK(false)

{
	m_pTransVirtObjectModel = new MIhmVirtObjectsModel();
}


MIhmMainLogic::~MIhmMainLogic()
{
	TRACE_D(QString("MIhmMainLogic::~MIhmMainLogic: Closing the main logic object ..."));

	if(m_pAniThread!=NULL)
	{
		if(m_pAniThread->isRunning())
		{
			m_pAniThread->quit();
			
			Sleep(200);
			delete m_pAniThread;
		}
	}

	if(m_pTCLInterfaces!=NULL)
	{
		delete m_pTCLInterfaces;
	}
	
	deinitTrace();

	delete m_pTransVirtObjectModel;
	
}


void MIhmMainLogic::initTrace(QString sModuleMbox)
{


	// Initialiser la trace
	MTracer::Init(sModuleMbox);

	TRACE_D(QString("MIhmMainLogic::initialize: Trace initialised"));
}

void MIhmMainLogic::deinitTrace()
{
	MTracer::Deinit();
}




void MIhmMainLogic::exitProgram()
{
    TRACE_D(QString("MIhmMainLogic::exitProgram: Initiated ******!"));
	
	if(m_pTCLInterfaces!=NULL)
	{
		m_pTCLInterfaces->close();
		delete m_pTCLInterfaces;
		m_pTCLInterfaces = NULL;
	}

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


void MIhmMainLogic::onInitOK()
{
	TRACE_D(QString( "MIhmMainLogic::onInitOK()..."));
	m_bModuleInitOK = true;
}


void MIhmMainLogic::onStartTimerTimeout()
{
	 if(!m_bModuleInitOK)
	 {
		 TRACE_W(QString("MIhmMainLogic::onStartTimerTimeout: The module is closing because it is not initialized correctly."));
		 exitProgram();
	 }
}

bool MIhmMainLogic::InitializeTCLInterfaces()
{
	bool bRetVal = false;

	m_pTCLInterfaces = new MIhmTCLInterfaces();
	QObject::connect(m_pTCLInterfaces,SIGNAL(exitProgram()),this, SLOT(exitProgram()), Qt::QueuedConnection);
	QObject::connect(m_pTCLInterfaces,SIGNAL(initOK()),this, SLOT(onInitOK()), Qt::QueuedConnection);

	bRetVal = m_pTCLInterfaces->initialize(this);
	
	m_oTimerStart.setSingleShot(true);
	QObject::connect(&m_oTimerStart,SIGNAL(timeout()),this,SLOT(onStartTimerTimeout()));
	m_oTimerStart.start(m_oConfGenData.m_dwStartTimeout);

	return bRetVal;
}


void MIhmMainLogic::sendMessageToAni(MIhmMsg * pMsg)
{
	m_pAniThread->sendMessageToAni(pMsg);
}

enum_instance_result MIhmMainLogic::Initialize(QString sBlName)
{
    TRACE_D(QString("MIhmMainLogic::initialize: pcBalName = %1").
						arg(sBlName));

	// Read general data
    if ( m_oConfGenData.loadConfigFromRegistry(sBlName) == false )
    {
        TRACE_W(QString( "MIhmMainLogic::initialize: ERREUR loadConfigGeneralData retourne false"));
        return INST_INIT_ERR_REGISTRE;
    }

	if(m_oConfGenData.m_sSystemEncoding!="")
	{
		QTextCodec *codec = QTextCodec::codecForName(m_oConfGenData.m_sSystemEncoding.toLatin1().data());
		if(codec!=NULL)
			QTextCodec::setCodecForLocale(codec);
		else
			TRACE_W(QString("MIhmMainLogic::initialize: ERREUR installing codec %1").
						arg(m_oConfGenData.m_sSystemEncoding));
	}

	
	//Initialize Desktop dialogs and TCP server to listen for requests 
	if(!InitializeTCLInterfaces())
			return INST_INIT_ERR_LANCE;

	m_pTCLInterfaces->displayInitInfo("Launching tasks ...");

	MIhmLoadedPlugins * pGlobalPlugins = new MIhmLoadedPlugins();
	if (!pGlobalPlugins->InitPlugins(m_oConfGenData.getModuleConfigKey()))
	{
		TRACE_W(QString("MIhmMainLogic::initializing: Error initializing MIhmLoadedPlugins::InitPlugins for [%1]").arg(m_oConfGenData.getModuleConfigKey()));
		return INST_INIT_ERR_LANCE;
	}



    TRACE_D(QString("MIhmMainLogic::initializing: MIhmAniThread ..."));

	noyau_bal_id iBalId;
	m_pAniThread = new MIhmAniThread();
	m_pAniThread->Initialize(sBlName, m_oConfGenData.getModuleConfigKey());
	QObject::connect(m_pAniThread,SIGNAL(virtualObjUpdated()),this, SLOT(onAniVirtualObjUpdated()), Qt::QueuedConnection);
	QObject::connect(m_pAniThread,SIGNAL(stopRequested()),this, SLOT(onAniStopRequested()), Qt::QueuedConnection);
	QObject::connect(m_pAniThread,SIGNAL(messageFromAni()),this, SLOT(onMessageFromAni()), Qt::QueuedConnection);
	m_pAniThread->start(m_oConfGenData.getAniPriority());

    // Attendre l'initialisation de la BAL du module par ANI
	iBalId = AttendBAL( sBlName.toLatin1().data());
    
	if ( iBalId <= 0 )
    {
		TRACE_W(QString("MIhmMainLogic::initializing: Error starting MIhmAniThread... "));
		return INST_INIT_ERR_LANCE;
	}
	else
	{
		TRACE_D(QString("MIhmMainLogic::initializing: MIhmAniThread initialized OK... "));
	}

	m_pTCLInterfaces->displayInitInfo("Waiting for configuration data from application ...");

	return INST_INIT_OK;
}

void MIhmMainLogic::onAniStopRequested()
{
	TRACE_W(QString( "MIhmMainLogic::onAniStopRequested..."));

	exitProgram();
}


void MIhmMainLogic::onAniVirtualObjUpdated()
{
	//check if any priority message sent from ANI to assure priority
	onMessageFromAni(); 
	
	if(m_pAniThread->copyUpdatedVirtObjects(m_pTransVirtObjectModel))
	{
		m_pTCLInterfaces->updateVirtualObjects(m_pTransVirtObjectModel);
	}
}


void MIhmMainLogic::onMessageFromAni()
{
	QList <MIhmMsg*> lstAniMsgs;

	if(m_pAniThread!=NULL)
		if(m_pAniThread->getMessagesFromAni(&lstAniMsgs))
		{
			while (!lstAniMsgs.isEmpty())
			{
				MIhmMsg* pMsg = lstAniMsgs.takeFirst();

				if(pMsg!=NULL)
				{
					m_pTCLInterfaces->processMessageFromAni(pMsg);
					delete pMsg;
				}
			}
		}
}
