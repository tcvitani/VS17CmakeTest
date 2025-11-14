
#include <QSettings>
#include <QDir>
#include "MTracer.h"
#include "MRVideoControlObject.h"
#include "MRVideoCmdThread.h"
#include "MRVideoMsg.h"

MRVideoControlObject::MRVideoControlObject(int eTargetVirtObj, QString sTargetRVideoObjId)
{
	m_eTargetVirtObject = eTargetVirtObj;
	m_sTargetRVideoObjId = sTargetRVideoObjId;
	m_bInitialized = false;
	m_iActivatedSource = -1;
	m_bTraceAll = false;
	m_pCmdThread = NULL;
}
	
	
MRVideoControlObject::~MRVideoControlObject()
{
	if(m_pCmdThread!=NULL)
	{
		delete m_pCmdThread;
		m_pCmdThread = NULL;
	}
}

bool MRVideoControlObject::init(QString sRSourcesRegKey)
{
	//initialize configuration data
	unsigned long ulCounter;
	QString sRegSubKey;
	QString sRegBaseKey;
	sRegBaseKey = QString("HKEY_LOCAL_MACHINE\\%1").arg(sRSourcesRegKey);
	QSettings regBaseValuesIHM(sRegBaseKey, QSettings::NativeFormat);
	
	int iTrace = regBaseValuesIHM.value("Trace",0).toUInt();
	m_bTraceAll = (iTrace!=0);

	for(ulCounter=0; ulCounter<MAX_RVIDEO_SOURCES; ulCounter ++)
	{
		m_aRVideoSourceParameters[ulCounter].bValidSource = false;
		sRegSubKey = QString("%1").arg(ulCounter);

		if(regBaseValuesIHM.contains(sRegSubKey + "/StreamSourceUrl"))
		{
			QString sRegFullKey = QDir::toNativeSeparators(sRegBaseKey+ "/"+ sRegSubKey); 
			QSettings regSourceValuesIHM(sRegFullKey, QSettings::NativeFormat);

			m_aRVideoSourceParameters[ulCounter].sStreamSourceUrlLocal = regSourceValuesIHM.value("StreamSourceUrlLocal").toString();
			m_aRVideoSourceParameters[ulCounter].sStreamSourceUrlRemote = regSourceValuesIHM.value("StreamSourceUrl").toString();

			m_aRVideoSourceParameters[ulCounter].sStreamSourceUrlLocal.append(QString("&RESSClientID=CLI%1").arg(m_sTargetRVideoObjId));
			m_aRVideoSourceParameters[ulCounter].sStreamSourceUrlRemote.append(QString("&RESSClientID=CLI%1").arg(m_sTargetRVideoObjId));

			m_aRVideoSourceParameters[ulCounter].sBaseUrl = regSourceValuesIHM.value("BaseUrl").toString();
			m_aRVideoSourceParameters[ulCounter].sUser = regSourceValuesIHM.value("User").toString();
			m_aRVideoSourceParameters[ulCounter].sPwd = regSourceValuesIHM.value("Pwd").toString();
			
			m_aRVideoSourceParameters[ulCounter].sHost = regSourceValuesIHM.value("Host").toString();
			m_aRVideoSourceParameters[ulCounter].uiPort = regSourceValuesIHM.value("Port",0).toUInt();

			m_aRVideoSourceParameters[ulCounter].sRestreamServerStreamId = regSourceValuesIHM.value("StreamId").toString();
			
			if(m_aRVideoSourceParameters[ulCounter].sStreamSourceUrlLocal.isEmpty())
				m_aRVideoSourceParameters[ulCounter].sStreamSourceUrlLocal = 
						m_aRVideoSourceParameters[ulCounter].sStreamSourceUrlRemote;

			if(!m_aRVideoSourceParameters[ulCounter].sStreamSourceUrlRemote.isEmpty()&&
				!m_aRVideoSourceParameters[ulCounter].sHost.isEmpty()&&
				!m_aRVideoSourceParameters[ulCounter].sBaseUrl.isEmpty()&&
				!m_aRVideoSourceParameters[ulCounter].sRestreamServerStreamId.isEmpty()&&
				m_aRVideoSourceParameters[ulCounter].uiPort!=0)
			{

				m_aRVideoSourceParameters[ulCounter].bValidSource = true;
				m_bInitialized = true;
			}
			else
			{
				TRACE_W(QString("MRVideoControlObject::init: Unable to initialize restream source for source %1!").
							arg(ulCounter));
				m_aRVideoSourceParameters[ulCounter].bValidSource = false;
			}
		}
	}

	if(m_bInitialized)
		initRVideoCtrlTread();

	return m_bInitialized;
}
	

void MRVideoControlObject::initRVideoCtrlTread()
{
	if(m_pCmdThread!=NULL)
	{
		delete m_pCmdThread;
		m_pCmdThread = NULL;
	}

	m_pCmdThread = new MRVideoCmdThread(m_sTargetRVideoObjId);

	if(m_bTraceAll)
		m_pCmdThread->setTraceAll();
	
	m_pCmdThread->start();
	connect(m_pCmdThread, SIGNAL(outputRVideoMessage(MRVideoMsg*)), this, SLOT(onOutputRVideoMessage(MRVideoMsg*)));
}

void MRVideoControlObject::onOutputRVideoMessage(MRVideoMsg* pRVMsg)
{
	if(pRVMsg!=NULL)
		emit newOutputRVideoMessage(pRVMsg, m_eTargetVirtObject);
}



QString MRVideoControlObject::getActivatedStreamSourceUrlLocal()
{
	if(m_iActivatedSource>=0 && m_aRVideoSourceParameters[m_iActivatedSource].bValidSource)
		return m_aRVideoSourceParameters[m_iActivatedSource].sStreamSourceUrlLocal;
	else
	{
		TRACE_W(QString("MRVideoControlObject::getActivatedStreamSourceUrlLocal: Invalid source %1!").
			arg(m_iActivatedSource));

		return QString("");
	}
}

QString MRVideoControlObject::getActivatedStreamSourceUrlRemote()
{
	if(m_iActivatedSource>=0 && m_aRVideoSourceParameters[m_iActivatedSource].bValidSource)
		return m_aRVideoSourceParameters[m_iActivatedSource].sStreamSourceUrlRemote;
	else
	{
		TRACE_W(QString("MRVideoControlObject::getActivatedStreamSourceUrlRemote: Invalid source %1!").
			arg(m_iActivatedSource));

		return QString("");
	}
}


QString MRVideoControlObject::getActivatedStreamBaseUrl()
{
	if(m_iActivatedSource>=0 && m_aRVideoSourceParameters[m_iActivatedSource].bValidSource)
	{
		return m_aRVideoSourceParameters[m_iActivatedSource].sBaseUrl;
	}
	else
	{
		TRACE_W(QString("MRVideoControlObject::getActivatedStreamBaseUrl: Invalid source %1!").
			arg(m_iActivatedSource));

		return QString("");
	}
}


void MRVideoControlObject::activate(int iSource)
{
	if(m_aRVideoSourceParameters[iSource].bValidSource)
	{
		m_iActivatedSource = iSource;
		m_pCmdThread->sendActivate(m_aRVideoSourceParameters[iSource].sHost, 
									m_aRVideoSourceParameters[iSource].uiPort, 
									m_aRVideoSourceParameters[iSource].sRestreamServerStreamId);

	}
}


void MRVideoControlObject::reactivate()
{
	m_pCmdThread->sendReactivate();
}

void MRVideoControlObject::deactivate()
{
	m_pCmdThread->sendDeactivate();
}


void MRVideoControlObject::grabb()
{
	m_pCmdThread->sendGrabb();
}


void MRVideoControlObject::save(QString sFilePath, bool bUseJPG)
{
	m_pCmdThread->sendGetImage(sFilePath, bUseJPG);

}