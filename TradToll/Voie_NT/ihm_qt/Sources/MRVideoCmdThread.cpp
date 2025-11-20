

#include <QByteArrayMatcher>
//#include <QHttpResponseHeader>
#include <QPixmap>

#include "MRVideoCmdThread.h"
#include "MTracer.h"
#include "MRVideoMsg.h"

MRVideoCmdThread::MRVideoCmdThread(QString sTargetRVideoObjId)
{
	m_sTargetRVideoObjId = sTargetRVideoObjId;
	m_iPoolTimeout = 1000;
	m_bTraceAll = false;
	m_pHttpNewManager = NULL;
}


MRVideoCmdThread::~MRVideoCmdThread()
{
	quit();
    wait();

	while (!m_lstInputMessages.isEmpty())
		delete m_lstInputMessages.takeFirst();


}

void MRVideoCmdThread::setPoolTimerTimeout(int iPoolTimeout)
{
	if(iPoolTimeout>0)
		m_iPoolTimeout = iPoolTimeout;
	else
		iPoolTimeout = 1000;

	m_bPoolStarted = false;
	
}


void MRVideoCmdThread::run()
{
	if(getTraceAll())
		TRACE_D("MRVideoCmdThread::run:..." );
	
	if(initialize())
		this->exec();
	else
	{ 
		TRACE_W("MRVideoCmdThread::Error initializing video cmd thread!" );
	}
}


bool MRVideoCmdThread::initialize()
{
	bool bRetVal = false;
	

	if(m_RVideoAut.initialize(this))
	{
		m_bPoolStarted = false;
		connect(this, SIGNAL(checkMessages()), this, SLOT(onCheckInputMessages()), Qt::QueuedConnection);
		connect(&m_tmPoolTimer, SIGNAL(timeout()), this, SLOT(onPoolTimeout()), Qt::QueuedConnection);

		m_RVideoAut.reset();

		bRetVal = true;
	}


	return bRetVal;
}

void MRVideoCmdThread::sendOutputRVideoMessage(MRVideoMsg* pMsg)
{
	emit outputRVideoMessage(pMsg);
}


void MRVideoCmdThread::putRVideoMessage(MRVideoMsg* pMsg)
{
	//MRVideoMsg *pCurrMsg = NULL;

	m_Mutex.lock();
// 		//To prohibit input messages >1
// 		if(m_lstInputMessages.count()>0)
// 		{
// 			//remove current first message
// 			pCurrMsg = m_lstInputMessages.takeFirst();
// 			delete pCurrMsg;
// 		}
		
		m_lstInputMessages.append(pMsg);
	m_Mutex.unlock();

	emit checkMessages();
}


void MRVideoCmdThread::sendMyselfACheckMessagesSignal()
{
	emit checkMessages();
}

void MRVideoCmdThread::onCheckInputMessages()
{
	MRVideoMsg *pMsg = NULL;
	bool bHasInputMsg = false;
	bool bMsgIsDeactivate = false;

	//first get if the next message is deactivate
	m_Mutex.lock();
		if(m_lstInputMessages.count()>0)
		{
			bHasInputMsg = true;
			pMsg = m_lstInputMessages.at(0);

			if(pMsg->getType()==MRVideoMsg::enuRVideoDeactivateReq)
				bMsgIsDeactivate = true;
		}
	m_Mutex.unlock();

	if(bHasInputMsg)
	{
		//because the Deactivate can be handled in all atm states but other commands only in specific states
		if(bMsgIsDeactivate || m_RVideoAut.ifCanProcessNewCommand())
		{
			m_Mutex.lock();
				if(m_lstInputMessages.count()>0)
					pMsg = m_lstInputMessages.takeFirst();
			m_Mutex.unlock();

			if(pMsg!=NULL)
			{
				processMessage(pMsg);
				delete pMsg;
			}
		}

	}
	// else no input messages to execute	

}

void MRVideoCmdThread::processMessage(MRVideoMsg* pMsg)
{

	if(getTraceAll())	
		TRACE_D(QString("MRVideoCmdThread::processMessage: %1...")
							.arg(pMsg->getMessageDescription()));

	
	switch(pMsg->getType())
	{
		case MRVideoMsg::enuRVideoActivateSourceReq:
		{
			QString sNewHost = ((MRVideoMsgVideoActivateSrc*)pMsg)->m_sNewHost;;
			QString sNewSourceId = ((MRVideoMsgVideoActivateSrc*)pMsg)->m_sNewSourceId;
			quint16 uiNewPort = ((MRVideoMsgVideoActivateSrc*)pMsg)->m_uiNewPort;

			if(m_sCurrentSourceId != sNewSourceId || m_sCurrentHost != sNewHost || m_uiPort != uiNewPort )
			{
				m_sCurrentHost = sNewHost;
				m_sCurrentSourceId = sNewSourceId;
				m_uiPort = uiNewPort;
				
				MRVideoCmdAtmEvent * pEvent = new MRVideoCmdAtmEvent(MRVideoCmdAutomate::enuEvtACTIVATEReq); 
				m_RVideoAut.processEvent(pEvent);
			}
		}
		break;
		case MRVideoMsg::enuRVideoReactivateReq:
		{
			if(m_sCurrentSourceId != "" || m_sCurrentHost != "" )
			{
				MRVideoCmdAtmEvent * pEvent = new MRVideoCmdAtmEvent(MRVideoCmdAutomate::enuEvtACTIVATEReq); 
				m_RVideoAut.processEvent(pEvent);
			}

		}
		break;

		case MRVideoMsg::enuRVideoDeactivateReq:
		{
			MRVideoCmdAtmEvent * pEvent = new MRVideoCmdAtmEvent(MRVideoCmdAutomate::enuEvtDEACTIVATEReq); 
			m_RVideoAut.processEvent(pEvent);
		}
		break;

		case MRVideoMsg::enuRVideoFreezeReq:
		{
			MRVideoCmdAtmEvent * pEvent = new MRVideoCmdAtmEvent(MRVideoCmdAutomate::enuEvtGRABBReq); 
			m_RVideoAut.processEvent(pEvent);
		}
		break;

		case MRVideoMsg::enuRVideoUnfreezeReq:
		{
			MRVideoCmdAtmEvent * pEvent = new MRVideoCmdAtmEvent(MRVideoCmdAutomate::enuEvtUNFREEZEReq); 
			m_RVideoAut.processEvent(pEvent);
		}
		break;

		case MRVideoMsg::enuRVideoSaveReq:
		{
			MRVideoCmdAtmEvent * pEvent = new MRVideoCmdAtmEvent(MRVideoCmdAutomate::enuEvtSAVEReq); 

			m_sTargetFileName = ((MRVideoMsgVideoSave*)pMsg)->m_sTargetFileName;;
			m_bSaveJpg = ((MRVideoMsgVideoSave*)pMsg)->m_bSaveJpg;

			m_RVideoAut.processEvent(pEvent);
		}
		break;

		default:
			TRACE_W(QString("MRVideoCmdThread::processMessage: Message type:%1 not identified!")
							.arg(pMsg->getType()));

	}

}

void MRVideoCmdThread::startPoolTimer()
{
	if(m_iPoolTimeout>0)
	{
		m_tmPoolTimer.start(m_iPoolTimeout);
		m_bPoolStarted = true;
	}
}

void MRVideoCmdThread::stopPoolTimer()
{
	m_tmPoolTimer.stop();
	m_bPoolStarted = false;

}

void MRVideoCmdThread::onPoolTimeout()
{
	if(m_bPoolStarted)
	{
		MRVideoCmdAtmEvent * pEvent = new MRVideoCmdAtmEvent(MRVideoCmdAutomate::enuEvtPoolTimeout); 
		m_RVideoAut.processEvent(pEvent);
	}
}

//-----------------------------------------------------------


QString MRVideoCmdThread::generateCommandPath(enuRVStreamHttpCommand eCmdId, QString sRVideoStreamId)
{
	QString sResult;	
	QString sCommand;	

	switch(eCmdId)
	{
	case enuHttpCmdGetStatus:
			sCommand = "get_status";
		break;
// 	case enuHttpCmdActivate:
// 			sCommand = "activate";
// 		break;
// 	case enuHttpCmdDeactivate:
// 			sCommand = "deactivate";
// 		break;
	case enuHttpCmdGrabb:
			sCommand = "grab";
	    break;
	case enuHttpCmdResume:
			sCommand = "resume";
	    break;
	case enuHttpCmdGetImage:
			sCommand = "get_captured_image";
	    break;
	default:
	    break;
	}

	sResult = QString("/command?id=%1&stream=%2&RESSClientID=CLI%3").arg(sCommand)
																.arg(sRVideoStreamId)
																.arg(m_sTargetRVideoObjId);

	return sResult;
}


void MRVideoCmdThread::sendHttpCommand(enuRVStreamHttpCommand eCmdId)
{
	if(m_pHttpNewManager ==NULL)
	{
		m_pHttpNewManager = new QNetworkAccessManager(this);
		connect(m_pHttpNewManager, &QNetworkAccessManager::finished, this, &MRVideoCmdThread::onHttpRequestFinished);  //, SLOT(onHttpRequestFinished(QNetworkReply*)));
	}
	
	//m_pHttp->setHost(m_sCurrentHost, m_uiPort);
	QString sCmd = generateCommandPath(eCmdId, m_sCurrentSourceId);

	QString sURL = QString("http://%1:%2/%3").arg(m_sCurrentHost).arg(m_uiPort).arg(sCmd);

	QNetworkRequest req;
	req.setUrl(QUrl(sURL));

	m_pLastNetworkReply = m_pHttpNewManager->get(req);
 
	if(getTraceAll())
		TRACE_D(QString("MRVideoCmdThread::sendHttpCommand: Sending get to: host:[%1], path:[%2]").arg(req.url().toString()).arg(sCmd));
}


void MRVideoCmdThread::abortHttp()
{
	if(getTraceAll())
		TRACE_D(QString("MRVideoCmdThread::abortHttp: Aborting http req!"));
	
	if(m_pLastNetworkReply!=NULL)
		m_pLastNetworkReply->abort();
}


void MRVideoCmdThread::onHttpRequestFinished(QNetworkReply* reply)
{
	////first verify that this is the last get I sent
	//if(m_iLastHttpRspId == idHttp)
	//{
		
	if (getTraceAll())
	{
		QString sMsg = QString("MRVideoCmdThread::onHttpRequestFinished: url:%1 error:%2!").arg(reply->url().toString()).arg(reply->errorString());
		TRACE_D(sMsg)
	}

		if(reply->error() != QNetworkReply::TimeoutError)
		{
			//parse HTTP response and generate event to send to automate
			MAutEvent * pEvent = parseResponse(reply);
			m_RVideoAut.processEvent(pEvent);
		}
		else
		{
			MAutEvent * pEvent = new MRVideoCmdAtmEvent(MRVideoCmdAutomate::enuEvtErrOrCommTimeout); 
			m_RVideoAut.processEvent(pEvent);
		}

		emit checkMessages();
	//}
}


MAutEvent * MRVideoCmdThread::parseResponse(QNetworkReply* reply)
{
	MAutEvent * pEvent = NULL;

	if(getTraceAll())
		TRACE_D(QString("MRVideoCmdThread::parseResponse:url:%1 error:%2!").arg(reply->url().toString()).arg(reply->errorString()));

	if(reply->error() == QNetworkReply::NoError)
	{
		
		QVariant vRspHeader = reply->header(QNetworkRequest::ContentTypeHeader);
		QString sType = vRspHeader.toString();

		if(sType.indexOf("text/html", 0, Qt::CaseInsensitive)>=0)
		{
			QString sMsg = "Response data:";
			QByteArray baRsp = reply->readAll();
			sMsg.append(baRsp);
			if(m_bTraceAll)
				TRACE_D(sMsg);

			int iStreamStatus;

			if(extractRspCode(baRsp, &iStreamStatus))
			{

				if(getTraceAll())
					TRACE_D(QString("MRVideoCmdThread::parseResponse: Extracted code %1").arg(iStreamStatus));

				switch(iStreamStatus)
				{
// 					case Inactive://    = 10,
// 					{
// 						pEvent = new MRVideoCmdAtmEvent(MRVideoCmdAutomate::enuEvtRspOKInactive); 
// 					}
// 					break;
					case Stopped://      = 11,
					{
						pEvent = new MRVideoCmdAtmEvent(MRVideoCmdAutomate::enuEvtRspOKStopped); 
					}
					break;
					case Running://      = 12,
					case WaitingForImage://      = 15,
					case ErrorRecovery://   = 16,
					{
						pEvent = new MRVideoCmdAtmEvent(MRVideoCmdAutomate::enuEvtRspOKRunning); 
					}
						break;
					case Grabbing://     = 13,
					{
						pEvent = new MRVideoCmdAtmEvent(MRVideoCmdAutomate::enuEvtRspOKGrabbing); 
					}
						break;
					case Paused://       = 14
					{
						pEvent = new MRVideoCmdAtmEvent(MRVideoCmdAutomate::enuEvtRspOKGrabbed); 
					}
					break;

					case Unknown:  //0 - invalid configuration or unknown stream id
						TRACE_W(QString("MRVideoCmdThread::onHttpRequestFinished:  Stream status [Unknown = 0] Invalid configuration or unknown stream id!"));
						pEvent = new MRVideoCmdAtmEvent(MRVideoCmdAutomate::enuEvtRspNOK); 
					break;

					default:
					{
						TRACE_W(QString("MRVideoCmdThread::onHttpRequestFinished: Unknown stream status %1").arg(iStreamStatus));
						pEvent = new MRVideoCmdAtmEvent(MRVideoCmdAutomate::enuEvtRspNOK); 

					}
						
				}
			}		
			else
			{
				TRACE_W(QString("MRVideoCmdThread::parseResponse: extractRspCode returned false!"));
				pEvent = new MRVideoCmdAtmEvent(MRVideoCmdAutomate::enuEvtRspNOK); 
			}
		}
		else if(sType.indexOf("image/jpeg", 0, Qt::CaseInsensitive) >= 0)	//check header if it is an image 
		{
			m_baImageCaptured = reply->readAll();
			
			if(m_baImageCaptured.size()>0)
			{

				m_bImageCaptured = true;
				QPixmap px;
				if(px.loadFromData(m_baImageCaptured, "JPG"))
				{	
					if(px.save(m_sTargetFileName, "JPG"))
					{
						pEvent = new MRVideoCmdAtmEvent(MRVideoCmdAutomate::enuEvtSavingImageOK); 
					}
					else
					{
						TRACE_W(QString("MRVideoCmdThread::parseResponse: Error saving to file %1!").arg(m_sTargetFileName));
					}
				}
				else
				{
					TRACE_W(QString("MRVideoCmdThread::parseResponse: Error creatinfg pixmap from received data!"));

				}
				//if not created by now it is an saving error
				if(pEvent==NULL)
					pEvent= new MRVideoCmdAtmEvent(MRVideoCmdAutomate::enuEvtSavingImageNOK);
			}
			else
			{
				TRACE_W(QString("MRVideoCmdThread::parseResponse: Image size == 0!"));
				pEvent = new MRVideoCmdAtmEvent(MRVideoCmdAutomate::enuEvtRspNOK); 
			}


		}
		else
		{
			TRACE_W(QString("MRVideoCmdThread::parseResponse: Unexpected contentType %1!").arg(sType));
			pEvent = new MRVideoCmdAtmEvent(MRVideoCmdAutomate::enuEvtRspNOK); 
			
		}
	
	}
	else
	{
		//process Error
		pEvent = new MRVideoCmdAtmEvent(MRVideoCmdAutomate::enuEvtErrOrCommTimeout); 
	}
	

	if(pEvent == NULL)
	{
		TRACE_W(QString("MRVideoCmdThread::parseResponse: ERROR: NO EVENT IDENTIFIED!"));
	}

	return pEvent;
}

bool MRVideoCmdThread::extractRspCode(QByteArray &baData, int *piCodeStatus)
{
	QByteArray baCodePrefix = "CODE:[";
	QByteArrayMatcher baMatcherCode(baCodePrefix);
	int iCodeIndex = baMatcherCode.indexIn(baData);

	if(iCodeIndex>=0)
	{
		QString sCode = baData.mid(iCodeIndex +  baCodePrefix.size()+4,2);
		
		if(!sCode.isEmpty())
		{
			bool bOK;
			int iCode = sCode.toInt(&bOK);

			if(bOK)
			{
				*piCodeStatus = iCode;
				return true;
			}
		}
	}

	return false;

}


//-------------------------------------------------------
// public functions used to enqueue internal messages
//-------------------------------------------------------
void MRVideoCmdThread::sendActivate(QString sHost, quint16 uiPort, QString sSourceId)
{	
	MRVideoMsgVideoActivateSrc* pMsg = new MRVideoMsgVideoActivateSrc();
	pMsg->m_uiNewPort = uiPort;
	pMsg->m_sNewHost = sHost;
	pMsg->m_sNewSourceId = sSourceId;
	putRVideoMessage(pMsg);
}

void MRVideoCmdThread::sendReactivate()
{
	MRVideoMsgReactivate * pMsg = new MRVideoMsgReactivate();
	putRVideoMessage(pMsg);
}


void MRVideoCmdThread::sendDeactivate()
{
	MRVideoMsgDeactivate * pMsg = new MRVideoMsgDeactivate();
	putRVideoMessage(pMsg);
}

void MRVideoCmdThread::sendGrabb()
{
	MRVideoMsgVideoFreeze * pMsg = new MRVideoMsgVideoFreeze();
	putRVideoMessage(pMsg);
}

void MRVideoCmdThread::sendResume()
{
	MRVideoMsgVideoUnfreeze * pMsg = new MRVideoMsgVideoUnfreeze();
	putRVideoMessage(pMsg);
}

void MRVideoCmdThread::sendGetImage(QString sTargeImagePath, bool bIsJpg)
{
	MRVideoMsgVideoSave * pMsg = new MRVideoMsgVideoSave();
	pMsg->m_sTargetFileName = sTargeImagePath;
	pMsg->m_bSaveJpg = bIsJpg; //if false will save BMP
	putRVideoMessage(pMsg);
}	

