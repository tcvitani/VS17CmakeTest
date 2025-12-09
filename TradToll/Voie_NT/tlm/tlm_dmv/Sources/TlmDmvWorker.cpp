#include <TlmDmvWorker.h>
#include <TlmDmvAutomate.h>
#include <InstTracer.h>
#include <HelpFuncs.h>
#include <UdpClient.h>


extern "C"
{
	#include <mbx2slot.h>
}





EXPORT_C int InitTlmWorker(IN LPVOID pGlobalWorkerStructure)
{
	TWorkerStructure* pWork = (TWorkerStructure*)pGlobalWorkerStructure;
	TlmWorker * pClient = new  TlmWorker(pWork);

	pWork->pWorkerInstance = pClient;

	return 0;
}

EXPORT_C void DeinitTlmWorker(IN  LPVOID pGlobalWorkerStructure)
{
	TWorkerStructure* pWork = (TWorkerStructure*)pGlobalWorkerStructure;

	if (pWork->pWorkerInstance != nullptr)
	{
		delete pWork->pWorkerInstance;
		pWork->pWorkerInstance = nullptr;
	}
}

EXPORT_C void SlotNewMessage(void *pQObject, struct_neutre *psMsg)
{
	TlmWorker * pWorker = (TlmWorker *)pQObject;

	pWorker->emitNewMboxMsg(psMsg);
}

TlmWorker::TlmWorker(TWorkerStructure *pWork, QObject *parent) : QObject(parent), m_pDmv_Client(nullptr), m_pOblakDevice(nullptr)
{
	m_pWorkData = pWork;

	m_pTlmDmvInst = (struct_global *)m_pWorkData->pGlobalStructure;

	mapRegistryParametersToClassMembers();

	m_pOblakDevice = new OblakDevice();
	if (!getDeviceType())
	{
		ExitBad();
	}

	InitializeIosMbox();
	connect(this, SIGNAL(newMbxMsg(struct_neutre *)), this, SLOT(ReadAndProcessMailboxMsg(struct_neutre *)), Qt::QueuedConnection);

	createClientSocket();
	connect(this, SIGNAL(signal_connected()), this, SLOT(onConnected()));
	connect(this, SIGNAL(signal_disconnected()), this, SLOT(onDisconnected()));

	initializeDevice();

	initTimers();

	memset(&m_sOldStatus, 0, sizeof(struct_tlm_status));

	m_pAutomate = new TlmAutomate(m_siInstId, this);
	m_pAutomate->initialize();
}

TlmWorker::~TlmWorker()
{
	if (m_pDmv_Client != nullptr)
	{
		m_pDmv_Client->deleteLater();
	}

	if (m_pDmv_EchoClient != nullptr)
	{
		m_pDmv_EchoClient->deleteLater();
	}

	if (m_pOblakDevice != nullptr)
	{
		delete m_pOblakDevice;
	}

	if (this != nullptr)
	{
		this->deleteLater();
	}
}

bool TlmWorker::InitializeIosMbox()
{
	//Used in message box to slot
	m_pTlmDmvInst->sMbx2Slot.pQObject = this;
	PublieBALToQueue(m_pTlmDmvInst->nom_bal_ios, 0, &(m_pTlmDmvInst->sMbx2Slot));

	if (m_pTlmDmvInst->sMbx2Slot.hBal == NOYAU_ID_NOK)
	{
		MInstTrtacer_FichierError(m_siInstId, (char*)"Initialization impossible : error creating public mailbox");
		ExitBad();
	}

	m_dwDrvMboxId = m_pTlmDmvInst->sMbx2Slot.hBal;

	// Waiting for the external mailbox publication (by ANI thread)
	m_dwAniMboxId = AttendBAL(m_pTlmDmvInst->nom_bal_ani);
	if (m_dwAniMboxId <= 0)
		ExitBad();

	return true;
}

void TlmWorker::ReadAndProcessMailboxMsg(struct_neutre *pMsg)
{
	CHAR szMailboxSender[MAX_PATH + 1] = { 0 };

	// Get sender mailbox name (for trace)
	if (pMsg->bl_retour != NOYAU_BAL_HORLOGE_ID)
		DonneNomBAL(pMsg->bl_retour, szMailboxSender);
	else
		strcpy_s(szMailboxSender, sizeof(szMailboxSender), "NOYAU_BAL_HORLOGE_ID");

	if (pMsg->bl_retour == m_pTlmDmvInst->ani_bal)
	{
		// Message received from the ANI thread
		ReceiveFromAni(pMsg);
	}
	else
	{
		MInstTrtacer_FichierError(m_siInstId, (char*)"ReadAndProcessMailboxMsg():: Unknown!");
	}

	// Free the message in the mailbox
	ExitLibere((struct_neutre **)(&pMsg));
}

void TlmWorker::emitNewMboxMsg(struct_neutre *pMsg)
{
	emit newMbxMsg(pMsg);
}

OblakDevice::eOblakDeviceType TlmWorker::mapDeviceFromRegistry(int iDeviceType)
{
	switch (iDeviceType)
	{
		                        
	case 1: return OblakDevice::eOblakDevice_ExternalDisplay_RTNM_FC_P1V1_32x64_FL_12x12;
	case 2: return OblakDevice::eOblakDevice_Canopy_TRS_CA60;
	case 3: return OblakDevice::eOblakDevice_Canopy_TRS_PT80_CMC_ENP;

	default: return OblakDevice::eLastOblakDevice;
	}
}

void TlmWorker::mapRegistryParametersToClassMembers()
{
	m_siInstId = m_pWorkData->siInstId;
	m_bFullTrace = (bool)m_pWorkData->dwFullTrace;

	m_sLocalIpAddress = (char *)m_pWorkData->szLocalIPAddress;
	m_uiLocalIpPort = (quint16)m_pWorkData->dwLocalIPPort;
	m_uiLocalEchoPort = (quint16)m_pWorkData->dwLocalEchoPort;
	m_sDeviceIpAddress = (char *)m_pWorkData->szDeviceIPAddress;
	m_uiDeviceIpPort = (quint16)m_pWorkData->dwDeviceIPPort;
	m_uiDeviceEchoPort = (quint16)m_pWorkData->dwDeviceEchoPort;

	m_iWaitForResponseTimeoutMs = m_pWorkData->dwWaitForResponseTimeout;
	m_iWaitForStatusTimeoutMs = m_pWorkData->dwWaitForStatusTimeout;

	m_ucImageXCoordinate = m_pWorkData->dwImageXCoordinate;
	m_ucImageYCoordinate = m_pWorkData->dwImageYCoordinate;
	
	QString sX = QString::number(m_ucImageXCoordinate, 16);
	QString sY = QString::number(m_ucImageYCoordinate, 16);
	
	m_baImageXCoordinate = QByteArray::fromHex(sX.toUtf8());
	m_baImageYCoordinate = QByteArray::fromHex(sY.toUtf8());

	m_uiLinkErrCount = 0;
	m_uiMaxLinkError = (quint16)m_pWorkData->MaxLinkError;

	m_bImageMemorized = false;
}

bool TlmWorker::getDeviceType()
{
	int iDeviceType = m_pWorkData->dwDeviceType;

	OblakDevice::eOblakDeviceType eDevice = mapDeviceFromRegistry(iDeviceType);

	if (eDevice != OblakDevice::eLastOblakDevice)
	{
		TRACE_D(m_siInstId, QString("TlmWorker::getDeviceType() - Device set to  %1").arg(m_pOblakDevice->getDeviceTypeString(eDevice)));
		m_pOblakDevice->setDeviceType(eDevice);
		return true;
	}
	else
	{
		TRACE_W(m_siInstId, QString("TlmWorker::getDeviceType() - Invalid device type, check registry configuration"));
		return false;
	}
}

void TlmWorker::createClientSocket()
{
	m_pDmv_Client = new UdpClient(m_siInstId, m_bFullTrace, m_sDeviceIpAddress, m_uiDeviceIpPort, m_sLocalIpAddress, m_uiLocalIpPort);
	connect(m_pDmv_Client, SIGNAL(receivedDatagram(const QByteArray &)), this, SLOT(onNewMsgFromDevice(const QByteArray &)));

	m_pDmv_EchoClient = new UdpClient(m_siInstId, m_bFullTrace, m_sDeviceIpAddress, m_uiDeviceEchoPort, m_sLocalIpAddress, m_uiLocalEchoPort);
	connect(m_pDmv_EchoClient, SIGNAL(receivedDatagram(const QByteArray &)), this, SLOT(onNewEchoMsgFromDevice(const QByteArray &)));
}

void TlmWorker::initializeDevice()
{
	connect(&m_InitTimer, SIGNAL(timeout()), this, SLOT(onInitializeConnectionTimeout()));
	m_InitTimer.setSingleShot(true);
	m_InitTimer.start(1000);
	SendPollRequest();
}

void TlmWorker::onConnected()
{
	m_pAutomate->processEvent(new TlmAutomateEvent(TlmAutomate::enuEvtLinkOK));
}

void TlmWorker::onDisconnected()
{
	m_pAutomate->processEvent(new TlmAutomateEvent(TlmAutomate::enuEvtDisconnected));
}

void TlmWorker::initTimers()
{
	connect(&m_WaitForResponseTimer, SIGNAL(timeout()), this, SLOT(onWaitForResponseTimeout()));
	m_bWaitForResponseTimerStared = false;

	connect(&m_CheckStatusTimer, SIGNAL(timeout()), this, SLOT(onCheckStatusTimeout()));
	m_CheckStatusTimer.start(5000);

	connect(&m_WaitForStatusTimer, SIGNAL(timeout()), this, SLOT(onWaitForStatusTimeout()));
}

void TlmWorker::StopResponseTimer(void)
{
	m_bWaitForResponseTimerStared = false;
	m_WaitForResponseTimer.stop();
}

void TlmWorker::StartResponseTimer(void)
{
	m_WaitForResponseTimer.setSingleShot(true);
	m_WaitForResponseTimer.start(m_iWaitForResponseTimeoutMs);
	m_bWaitForResponseTimerStared = true;
}

void TlmWorker::StopStatusTimer(void)
{
	m_WaitForStatusTimer.stop();
}

void TlmWorker::StartStatusTimer(void)
{
	m_WaitForStatusTimer.setSingleShot(true);
	m_WaitForStatusTimer.start(m_iWaitForStatusTimeoutMs);
}

void TlmWorker::onWaitForResponseTimeout()
{
	TRACE_D(m_siInstId, QString("TlmWorker::onWaitForResponseTimeout.."));

	if (m_bWaitForResponseTimerStared)
	{
		m_bWaitForResponseTimerStared = false;
		m_bImageMemorized = m_iLastImageSet;
		emit signal_disconnected();
	}
}

void TlmWorker::onInitializeConnectionTimeout()
{
	emit signal_disconnected();
}

void TlmWorker::onCheckStatusTimeout()
{
	m_pAutomate->processEvent(new TlmAutomateEvent(TlmAutomate::enuEvtPollTimeout));
}

void TlmWorker::onWaitForStatusTimeout()
{
	emit signal_disconnected();
}

void TlmWorker::IncLinkError()
{
	m_uiLinkErrCount++;

	if (m_uiLinkErrCount > m_uiMaxLinkError)
	{
		m_pAutomate->processEvent(new TlmAutomateEvent(TlmAutomate::enuEvtLinkError));
		SetLinkFailureStatus(true);
	}
}

void TlmWorker::ResetLinkError()
{
	m_uiLinkErrCount = 0;
	SetLinkFailureStatus(false);
}

void TlmWorker::SetLinkFailureStatus(bool bLinkFailure)
{
	struct_tlm_status sNewStatus = { 0 };

	if (bLinkFailure)
		sNewStatus.link_failure = 1;
	else
		sNewStatus.link_failure = 0;

	if (m_sOldStatus.link_failure != sNewStatus.link_failure)
	{
		m_sOldStatus.link_failure = sNewStatus.link_failure;
		SendStatus();

		gsTLM[m_siInstId].status.link_failure = m_sOldStatus.link_failure;
	}
}

void TlmWorker::SendStatus()
{
	struct_tlm_message p_msg;

	p_msg.entete.neutre.bl_retour = gsTLM[m_siInstId].ani_bal;
	p_msg.entete.service = M_SRV_ETAT;
	p_msg.entete.type_message = (enum_srv_type)TLM_MESSAGE_ETAT_CHANGED;

	memcpy(&p_msg.srv.srv_etat, &m_sOldStatus, sizeof(struct_srv_etat));

	SendToAni(&p_msg);
}

void TlmWorker::ReceiveFromAni(struct_neutre *pNeutre)
{
	struct_tlm_message *pMsg = (struct_tlm_message *)pNeutre;

	// Message type
	switch (pMsg->entete.type_message)
	{
	case TLM_MESSAGE_SET_REQUESTED:
		{
			TRACE_D(m_siInstId, QString("TlmWorker::ReceiveFromAni() - TLM_MESSAGE_SET_REQUESTED: disp_srv.state = %1 [%2]")
					.arg(pMsg->srv.srv_tlm.u.display_srv.state)
					.arg(TlmEnumToString(pMsg->srv.srv_tlm.u.display_srv.state)));

			m_iImageRequestedByUser = (int)pMsg->srv.srv_tlm.u.display_srv.state;

			m_pAutomate->processEvent(new TlmAutomateEvent_SetImage(m_iImageRequestedByUser));
		}
		break;

	case TLM_MESSAGE_GET_REQUESTED:
		break;

	case TLM_MESSAGE_STOP_REQUESTED:
		// Stopping IOS thread
		SendRequest(OblakMsg::eImage_Red);
		//StopWorker(m_siInstId, M_ARRET_EFFECTUE);
		break;

	default:
		TRACE_W(m_siInstId, QString("*** TlmWorker::ReceiveFromAni() - undefined message %1 ***").arg(pMsg->entete.type_message));
		break;
	}
}

void TlmWorker::StopWorker(tlm_inst_id siInstId, enum_tlm_ani_ios eTypeMessage)
{
	struct_tlm_message *pMsg = nullptr;

	pMsg->entete.service = M_SRV_ARRET;
	pMsg->entete.type_message = (enum_srv_type)eTypeMessage;

	SendToAni(pMsg);
}

void TlmWorker::SendToAni(struct_tlm_message *psAniIos)
{
	struct_tlm_message *pAni = nullptr;

	// Allocating the memory
	ExitAlloue((struct_neutre **)(&pAni),
			   sizeof(struct_tlm_message),
			   m_pTlmDmvInst->pool);

	*pAni = *psAniIos;

	// Sending the message
	if (Envoie(m_dwAniMboxId,
		m_dwDrvMboxId,
		(struct_neutre *)pAni) != NOYAU_OK)
	{
		ExitLibere((struct_neutre **)&pAni);
	}
}

void TlmWorker::SendRequest(int iImage)
{
	QByteArray baReq;

	SetImage *pReq = new SetImage();

	pReq->setImage(ConvertImageToProtocol(iImage));
	pReq->setXCoordinate(m_baImageXCoordinate);
	pReq->setYCoordinate(m_baImageYCoordinate);
	pReq->encode(baReq);

	TRACE_D(m_siInstId, QString("TlmWorker::SendRequest() - REQUEST [%1]").arg(HelpFuncs::ByteArrayToHexStr(baReq)));

	m_pDmv_Client->sendDatagram(baReq);

	OblakMsg::deleteOblakMsg(pReq);
}


OblakMsg::eOblakImage TlmWorker::ConvertImageToProtocol(int iImageRequestedByUser)
{
	switch (iImageRequestedByUser)
	{
	case TLM_DMV_IMAGE_FULL_GREEN:				return OblakMsg::eImage_Green;
	case TLM_DMV_IMAGE_FULL_RED:				return OblakMsg::eImage_Red;
	case TLM_DMV_IMAGE_RED_CROSS:				return OblakMsg::eImage_RedCross;
	case TLM_DMV_IMAGE_GREEN_ARROW:				return OblakMsg::eImage_GreenArrow;
	default:
		return OblakMsg::eImage_Red;
		break;
	}
}

void TlmWorker::onNewMsgFromDevice(const QByteArray &baResponse)
{
	OblakResponse *pRsp = new OblakResponse();
	OblakResponse::eRspCode eErrCode;
	QByteArray baResponseCopy = baResponse;

	int iCurrentState = m_pAutomate->getCurrentStateId();

	StopResponseTimer();

	if (iCurrentState == TlmAutomate::enuInitial)
	{
		m_pAutomate->processEvent(new TlmAutomateEvent(TlmAutomate::enuEvtPollTimeout));
	}
	else if (iCurrentState == TlmAutomate::enuLinkError)
	{
		m_pAutomate->processEvent(new TlmAutomateEvent(TlmAutomate::enuEvtLinkOK));
	}
	else if (iCurrentState == TlmAutomate::enuWaitingResponse)
	{
		if (!baResponse.isEmpty())
		{
			TRACE_D(m_siInstId, QString("TlmWorker::onNewMsgFromServer(): RECEIVED DATA HEX = [%1]").arg(HelpFuncs::ByteArrayToHexStr(baResponse)));

			if (pRsp->decode(baResponse))
			{
				eErrCode = pRsp->AnalyzeOblakResponse(baResponse);

				if (eErrCode != OblakResponse::eRspCode_NoError)
				{
					TRACE_W(m_siInstId, QString("TlmWorker::onNewMsgFromServer(): [NOK] -> %1").arg(pRsp->getErrorString()));
					m_pAutomate->processEvent(new TlmAutomateEvent_SetImageNOK(pRsp->getFixedBytes()));
				}
				else
				{
					TRACE_D(m_siInstId, QString("TlmWorker::onNewMsgFromServer() - Last command successfully processed"));
					m_pAutomate->processEvent(new TlmAutomateEvent(TlmAutomate::enuEvtSetImageOK));
				}
			}
		}
	}
	else
	{
		TRACE_D(m_siInstId, QString("TlmWorker::onNewMsgFromDevice() [UNCAUGHT CASE]: \nState: [%1]\nEvent: [%2] Id: \nMessage from device - [%3], ")
				.arg(m_pAutomate->getCurrentStateId())
				.arg(m_pAutomate->getCurrentEventId())
				.arg(printAutMsg(baResponseCopy)));

		m_pAutomate->processEvent(new TlmAutomateEvent(TlmAutomate::enuEvtLinkOK)); // TODO - Placeholder for new event if needed
	}

	if (pRsp != nullptr)
		delete pRsp;
}

void TlmWorker::SendPollRequest()
{
	StopStatusTimer();
	QByteArray baDummyPacket("\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39", 10);

	m_pDmv_EchoClient->sendDatagram(baDummyPacket);
	StartStatusTimer();
}

void TlmWorker::onNewEchoMsgFromDevice(const QByteArray &baEchoResponse)
{
	StopStatusTimer();
	ResetLinkError();
	
	emit signal_connected();
}

QString TlmWorker::printAutMsg(QByteArray &arr)
{
	if (!arr.isEmpty())
	{
		return QString(HelpFuncs::ByteArrayToHexStr(arr));
	}
	else
	{
		return QString("Empty response");
	}
}

QString TlmWorker::TlmEnumToString(int state)
{
	switch (state)
	{
	case TLM_DMV_IMAGE_FULL_GREEN:				return "TLM_DMV_IMAGE_FULL_GREEN";
	case TLM_DMV_IMAGE_FULL_RED:				return "TLM_DMV_IMAGE_FULL_RED";
	case TLM_DMV_IMAGE_RED_CROSS:				return "TLM_DMV_IMAGE_RED_CROSS";
	case TLM_DMV_IMAGE_GREEN_ARROW:				return "TLM_DMV_IMAGE_GREEN_ARROW";
	default:
		return "TLM_DMV_IMAGE_FULL_RED";
		break;
	}
}