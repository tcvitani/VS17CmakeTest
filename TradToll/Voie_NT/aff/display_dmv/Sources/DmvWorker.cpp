#include <DmvWorker.h>

#include <InstTracer.h>
#include <HelpFuncs.h>
#include <UdpClient.h>

extern "C"
{
	#include <mbx2slot.h>
}

EXPORT_C int InitDmvWorker(IN LPVOID pGlobalWorkerStructure)
{
	TWorkerStructure* pWork = (TWorkerStructure*)pGlobalWorkerStructure;
	DmvWorker * pClient = new  DmvWorker(pWork);

	pWork->pWorkerInstance = pClient;

	return 0;
}

EXPORT_C void DeinitDmvWorker(IN  LPVOID pGlobalWorkerStructure)
{
	TWorkerStructure* pWork = (TWorkerStructure*)pGlobalWorkerStructure;

	if (pWork->pWorkerInstance != NULL)
	{
		delete pWork->pWorkerInstance;
		pWork->pWorkerInstance = NULL;
	}
}

//Used in message box to slot
EXPORT_C void SlotNewMessage(void *pQObject, struct_neutre *psMsg)
{
	DmvWorker * pWorker = (DmvWorker *)pQObject;

	pWorker->emitNewMboxMsg(psMsg);
}

DmvWorker::DmvWorker(TWorkerStructure *pWork, QObject *parent) : QObject(parent), m_pDmv_Client(nullptr), m_pOblakDevice(nullptr)
{
	m_pWorkData = pWork;
	
	m_pAffDmvInst = (struct_global *)m_pWorkData->pGlobalStructure;
	
	mapRegistryParametersToClassMembers();

	m_pOblakDevice = new OblakDevice();
	if (!getDeviceType())
	{
		ExitBad();
	}

	m_pTextProperties = new SetTextProperties();
	setTextProperties();

	InitializeIosMbox();
	connect(this, SIGNAL(newMbxMsg(struct_neutre *)), this, SLOT(ReadAndProcessMailboxMsg(struct_neutre *)), Qt::QueuedConnection);

	createClientSocket();
	connect(this, SIGNAL(signal_connected()), this, SLOT(onConnected()));
	connect(this, SIGNAL(signal_disconnected()), this, SLOT(onDisconnected()));

	initializeDevice();
	initTimers();

	m_eLastTlmImage = TLM_DMV_IMAGE_FULL_RED;
	m_eLastFlasherState = TLM_FLASHER_OFF;
	memset(&m_sLastAffScreen, 0, sizeof(struct_screen));

	memset(&m_sOldStatus, 0, sizeof(struct_b_etat_aff));

	m_pAutomate = new DmvAutomate(m_siInstId, this);
	m_pAutomate->initialize();
}

DmvWorker::~DmvWorker()
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

	OblakMsg::deleteOblakMsg(m_pTextProperties);

	if (this != nullptr)
	{
		this->deleteLater();
	}
}

bool DmvWorker::InitializeIosMbox()
{
	//Used in message box to slot
	m_pAffDmvInst->sMbx2Slot.pQObject = this;
	PublieBALToQueue(m_pAffDmvInst->ios_mbox_name, 0, &(m_pAffDmvInst->sMbx2Slot));

	if (m_pAffDmvInst->sMbx2Slot.hBal == NOYAU_ID_NOK)
	{
		MInstTrtacer_FichierError(m_siInstId, (char*)"Initialization impossible : error creating public mailbox");
		ExitBad();
	}

	m_dwDrvMboxId = m_pAffDmvInst->sMbx2Slot.hBal;

	// Waiting for the external mailbox publication (by ANI thread)
	m_dwAniMboxId = AttendBAL(m_pAffDmvInst->ani_mbox_name);
	if (m_dwAniMboxId <= 0)
		ExitBad();

	return true;
}

void DmvWorker::ReadAndProcessMailboxMsg(struct_neutre *pMsg)
{
	CHAR szMailboxSender[MAX_PATH + 1] = { 0 };

	// Get sender mailbox name (for trace)
	if (pMsg->bl_retour != NOYAU_BAL_HORLOGE_ID)
		DonneNomBAL(pMsg->bl_retour, szMailboxSender);
	else
		strcpy_s(szMailboxSender, sizeof(szMailboxSender), "NOYAU_BAL_HORLOGE_ID");

	if (pMsg->bl_retour == m_pAffDmvInst->ani_mbox)
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

void DmvWorker::emitNewMboxMsg(struct_neutre *pMsg)
{
	emit newMbxMsg(pMsg);
}

void DmvWorker::mapRegistryParametersToClassMembers()
{
	m_siInstId = m_pWorkData->siInstId;
	m_bFullTrace = (bool)m_pWorkData->dwFullTrace;

	m_sLocalIpAddress = (char *)m_pWorkData->szLocalIPAddress;
	m_uiLocalIpPort = (quint16)m_pWorkData->dwLocalIPPort;
	m_uiLocalEchoPort = (quint16)m_pWorkData->dwLocalEchoPort;
	m_sDeviceIpAddress = (char *)m_pWorkData->szDeviceIPAddress;
	m_uiDeviceIpPort = (quint16)m_pWorkData->dwDeviceIPPort;
	m_uiDeviceEchoPort = (quint16)m_pWorkData->dwDeviceEchoPort;

	m_bIsTlmServiceIntegrated = (bool)m_pWorkData->dwIsTlmServiceIntegrated;
	m_ucImageXCoordinate = m_pWorkData->dwImageXCoordinate;
	m_ucImageYCoordinate = m_pWorkData->dwImageYCoordinate;

	QString sX = QString::number(m_ucImageXCoordinate, 16);
	QString sY = QString::number(m_ucImageYCoordinate, 16);

	m_baImageXCoordinate = QByteArray::fromHex(sX.toUtf8());
	m_baImageYCoordinate = QByteArray::fromHex(sY.toUtf8());

	m_iWaitForResponseTimeoutMs = m_pWorkData->dwWaitForResponseTimeout;
	m_iWaitForStatusTimeoutMs = m_pWorkData->dwWaitForStatusTimeout;

	m_uiLinkErrCount = 0;
	m_uiMaxLinkError = (quint16)m_pWorkData->MaxLinkError;
}

bool DmvWorker::getDeviceType()
{
	int iDeviceType = m_pWorkData->dwDeviceType;

	OblakDevice::eOblakDeviceType eDevice = mapDeviceFromRegistry(iDeviceType);

	if (eDevice != OblakDevice::eLastOblakDevice)
	{
		TRACE_D(m_siInstId, QString("DmvWorker::DmvWorker() - Device set to  %1").arg(m_pOblakDevice->getDeviceTypeString(eDevice)));
		m_pOblakDevice->setDeviceType(eDevice);
		return true;
	}
	else
	{
		TRACE_W(m_siInstId, QString("DmvWorker::DmvWorker() - Invalid device type, check registry configuration"));
		return false;
	}
}

void DmvWorker::setTextProperties()
{
	m_eFont = mapFontFromRegistry((int)m_pWorkData->dwFont);
	m_eFontColor = mapFontColorFromRegistry((int)m_pWorkData->dwFontColor);
	m_eAlignment = mapAlignmentFromRegistry((int)m_pWorkData->dwAlignment);
	m_iMaxSpacing = (int)m_pWorkData->dwMaxSpacing;

	m_pTextProperties->setFont(m_eFont);
	m_pTextProperties->setFontColor(m_eFontColor);
	m_pTextProperties->setAlignment(m_eAlignment);
	m_pTextProperties->setMaxSpacing(m_iMaxSpacing);

	m_ucTextYCoordinate = m_pWorkData->dwTextYCoordinate;
	m_ucTextXRCoordinate = m_pWorkData->dwTextXRCoordinate;
	m_ucTextXLCoordinate = m_pWorkData->dwTextXLCoordinate;

	QString sY = QString::number(m_ucTextYCoordinate, 16);
	QString sXR = QString::number(m_ucTextXRCoordinate, 16);
	QString sXL = QString::number(m_ucTextXLCoordinate, 16);

	m_baTextYCoordinate = QByteArray::fromHex(sY.toUtf8());
	m_baTextXRCoordinate = QByteArray::fromHex(sXR.toUtf8());
	m_baTextXLCoordinate = QByteArray::fromHex(sXL.toUtf8());

	m_pTextProperties->setYCoordinate(m_baTextYCoordinate);
	m_pTextProperties->setXCoordinateRighttSide(m_baTextXRCoordinate);
	m_pTextProperties->setXCoordinateLeftSide(m_baTextXLCoordinate);
}

void DmvWorker::createClientSocket()
{
	m_pDmv_Client = new UdpClient(m_siInstId, m_bFullTrace, m_sDeviceIpAddress, m_uiDeviceIpPort, m_sLocalIpAddress, m_uiLocalIpPort);
	connect(m_pDmv_Client, SIGNAL(receivedDatagram(const QByteArray &)), this, SLOT(onNewMsgFromDevice(const QByteArray &)));

	m_pDmv_EchoClient = new UdpClient(m_siInstId, m_bFullTrace, m_sDeviceIpAddress, m_uiDeviceEchoPort, m_sLocalIpAddress, m_uiLocalEchoPort);
	connect(m_pDmv_EchoClient, SIGNAL(receivedDatagram(const QByteArray &)), this, SLOT(onNewEchoMsgFromDevice(const QByteArray &)));
}

void DmvWorker::initTimers()
{
	connect(&m_WaitForResponseTimer, SIGNAL(timeout()), this, SLOT(onWaitForResponseTimeout()));
	m_bWaitForResponseTimerStared = false;

	connect(&m_CheckStatusTimer, SIGNAL(timeout()), this, SLOT(onCheckStatusTimeout()));
	m_CheckStatusTimer.start(5000);

	connect(&m_WaitForStatusTimer, SIGNAL(timeout()), this, SLOT(onWaitForStatusTimeout()));
}

void DmvWorker::initializeDevice()
{
	SendPollRequest();
}

void DmvWorker::onConnected()
{
	m_pAutomate->processEvent(new DmvAutomateEvent(DmvAutomate::enuEvtConnected));
}

void DmvWorker::onDisconnected()
{
	m_pAutomate->processEvent(new DmvAutomateEvent(DmvAutomate::enuEvtDisconnected));
}

OblakDevice::eOblakDeviceType DmvWorker::mapDeviceFromRegistry(int iDeviceType)
{
	switch (iDeviceType)
	{
	case 1: return OblakDevice::eOblakDevice_ExternalDisplay_RTNM_FC_P1V1_32x64_FL_12x12;
	case 2: return OblakDevice::eOblakDevice_Canopy_TRS_CA60;
	case 3: return OblakDevice::eOblakDevice_Canopy_TRS_PT80_CMC_ENP;
	case 4: return OblakDevice::eOblakDevice_ExternalDisplay_RTNM_FC_P1V1_32x64_FL_Circle;

	default: return OblakDevice::eLastOblakDevice;
	}
}

OblakMsg::eOblakFont DmvWorker::mapFontFromRegistry(int iFont)
{
	switch (iFont)
	{
	case 1: return OblakMsg::eFont_IsoLatin2;
	case 2: return OblakMsg::eFont_IsoCyrillic;

	default: return OblakMsg::eFont_IsoLatin2;
	}
}

OblakMsg::eOblakFontColor DmvWorker::mapFontColorFromRegistry(int dwFontColor)
{
	switch (dwFontColor)
	{
	case 0: return OblakMsg::eColor_Off;
	case 1: return OblakMsg::eColor_Red;
	case 2: return OblakMsg::eColor_Green;
	case 3: return OblakMsg::eColor_Blue;
	case 4: return OblakMsg::eColor_White;
	case 5: return OblakMsg::eColor_Yellow;
	case 6: return OblakMsg::eColor_Orange;

	default: return OblakMsg::eColor_Red;
	}
}

OblakMsg::eOblakAlignment DmvWorker::mapAlignmentFromRegistry(int iAlignment)
{
	switch (iAlignment)
	{
	case 1: return OblakMsg::eAlignLeft;
	case 2: return OblakMsg::eAlignCenter;
	case 3: return OblakMsg::eAlignRight;

	default: return OblakMsg::eAlignLeft;
	}
}

void DmvWorker::StopResponseTimer(void)
{
	m_bWaitForResponseTimerStared = false;
	m_WaitForResponseTimer.stop();
}

void DmvWorker::StartResponseTimer(void)
{
	m_WaitForResponseTimer.setSingleShot(true);
	m_WaitForResponseTimer.start(m_iWaitForResponseTimeoutMs);
	m_bWaitForResponseTimerStared = true;
}

void DmvWorker::StopStatusTimer(void)
{
	m_WaitForStatusTimer.stop();
}

void DmvWorker::StartStatusTimer(void)
{
	m_WaitForStatusTimer.setSingleShot(true);
	m_WaitForStatusTimer.start(m_iWaitForStatusTimeoutMs);
}

void DmvWorker::onWaitForResponseTimeout()
{
	TRACE_D(m_siInstId, QString("DmvWorker::onWaitForResponseTimeout.."));

	if (m_bWaitForResponseTimerStared)
	{
		m_bWaitForResponseTimerStared = false;
		m_pAutomate->processEvent(new DmvAutomateEvent(DmvAutomate::enuEvtRspTimeout));
	}
}

void DmvWorker::onInitializeConnectionTimeout()
{
	emit signal_disconnected();
}

void DmvWorker::onCheckStatusTimeout()
{
	m_pAutomate->processEvent(new DmvAutomateEvent(DmvAutomate::enuEvtPollTimeout));
}

void DmvWorker::onWaitForStatusTimeout()
{
	emit signal_disconnected();
}

void DmvWorker::IncLinkError()
{
	m_uiLinkErrCount++;

	if (m_uiLinkErrCount > m_uiMaxLinkError)
	{
		m_pAutomate->processEvent(new DmvAutomateEvent(DmvAutomate::enuEvtLinkError));
		SetLinkFailureStatus(true);
	}
}

void DmvWorker::ResetLinkError()
{
	m_uiLinkErrCount = 0;
	SetLinkFailureStatus(false);

	m_pAutomate->processEvent(new NewTrafficLightEvent(m_eLastTlmImage));

	if (isZeroed(&m_sLastAffScreen, sizeof(struct_screen)))
	{
		ClearScreen();
	}
	else
	{
		m_pAutomate->processEvent(new NewDisplayEvent(m_sLastAffScreen, m_iLastAffPriority));
	}
}

void DmvWorker::SetLinkFailureStatus(bool bLinkFailure)
{
	struct_b_etat_aff sNewStatus = { 0 };

	if (bLinkFailure)
		sNewStatus.aff_hs = 1;
	else
		sNewStatus.aff_hs = 0;

	if (m_sOldStatus.aff_hs != sNewStatus.aff_hs)
	{
		m_sOldStatus.aff_hs = sNewStatus.aff_hs;

		SendStatus();
	}
}

void DmvWorker::SendStatus()
{
	struct_ani_ios sAniIos = { 0 };
	sAniIos.message_id = M_CHANGEMENT_ETAT;
	memcpy(&sAniIos.u.ios_ani.sStatus, &m_sOldStatus, sizeof(sAniIos.u.ios_ani.sStatus));
	SendToAni(&sAniIos);
}

void DmvWorker::ReceiveFromAni(struct_neutre *pNeutre)
{
	struct_ani_ios *pMessage = (struct_ani_ios*)pNeutre;

	switch (pMessage->message_id)
	{
	case M_AFFICHAGE:
		{
			// Backups for link error resetting
			memcpy(&m_sLastAffScreen, &gsDMV[m_siInstId].sScreen, sizeof(struct_screen));
			m_iLastAffPriority = pMessage->u.ani_ios.u.contenu.modif_priorite;

			TRACE_D(m_siInstId, QString("DmvWorker::ReceiveFromAni() => request DISPLAY [%1]").arg(m_sLastAffScreen.tab_request[m_iLastAffPriority].p_label->label));

			m_pAutomate->processEvent(new NewDisplayEvent(m_sLastAffScreen, m_iLastAffPriority));
		}	
		break;

	case M_ALLUME:
		TRACE_D(m_siInstId, QString("DmvWorker::ReceiveFromAni() => request ALLUME"));
		break;

	case M_ARRET:
		TRACE_D(m_siInstId, QString("DmvWorker::ReceiveFromAni() => request STOP "));
		break;

	case M_TLM_MESSAGE_SET:
		{
			if (m_bIsTlmServiceIntegrated)
			{
				TRACE_D(m_siInstId, QString("DmvWorker::ReceiveFromAni() - TLM_MESSAGE_SET_REQUESTED: disp_srv.state = %1 [%2]")
						.arg(pMessage->u.ani_ios.u.tlm.state)
						.arg(TlmEnumToString(pMessage->u.ani_ios.u.tlm.state)));

				m_eLastTlmImage = (enum_tlm_dmv_images)pMessage->u.ani_ios.u.tlm.state;

				m_pAutomate->processEvent(new NewTrafficLightEvent(m_eLastTlmImage));
			}
			else
			{
				TRACE_D(m_siInstId, QString("DmvWorker::ReceiveFromAni() - TLM service is disabled in registry"));
			}
		}
		break;

	case M_TLM_FLASHER_SET:
	{
		if (m_bIsTlmServiceIntegrated)
		{
			TRACE_D(m_siInstId, QString("DmvWorker::ReceiveFromAni() - M_TLM_FLASHER_SET: flasher.state = %1")
					.arg(pMessage->u.ani_ios.u.flasher.state));

			m_eLastFlasherState = pMessage->u.ani_ios.u.flasher.state;

			m_pAutomate->processEvent(new NewFlasherStateEvent(m_eLastFlasherState));
			m_pAutomate->processEvent(new DmvAutomateEvent(DmvAutomate::enuEvtConfirmFlasherStateReq));
		}
		else
		{
			TRACE_D(m_siInstId, QString("DmvWorker::ReceiveFromAni() - TLM service is disabled in registry"));
		}
	}
	break;

	case TLM_MESSAGE_GET_REQUESTED:
		break;

	case TLM_MESSAGE_STOP_REQUESTED:
		// Stopping IOS thread
		// SendSetImage(OblakMsg::eImage_Red);
		// StopWorker(m_siInstId, M_ARRET_EFFECTUE);
		break;

	default:
		TRACE_W(m_siInstId, QString("DmvWorker::ReceiveFromAni() => request '%1' unknown *****").arg(pMessage->message_id));
		break;
	}
}

void DmvWorker::SendToAni(struct_ani_ios *psAniIos)
{
	struct_ani_ios *pAni = NULL;

	// Allocating the memory
	ExitAlloue((struct_neutre **)(&pAni),
			   sizeof(struct_ani_ios),
			   m_pAffDmvInst->pool);

	*pAni = *psAniIos;

	// Sending the message
	if (Envoie(m_dwAniMboxId,
		m_dwDrvMboxId,
		(struct_neutre *)pAni) != NOYAU_OK)
	{
		ExitLibere((struct_neutre **)&pAni);
	}
}

void DmvWorker::SendActivateScenario()
{
	QByteArray baReq;
	
	ActivateScenario *pReq = new ActivateScenario();

	QByteArray baScenario;
	baScenario.append(QByteArray(2, '\x00'));

	pReq->setScenarioIndex(baScenario);
	pReq->encode(baReq);

	TRACE_D(m_siInstId, QString("DmvWorker::SendActivateScenario() - REQUEST [%1]").arg(HelpFuncs::ByteArrayToHexStr(baReq)));
	
	m_baLastReq = baReq;
	m_pDmv_Client->sendDatagram(baReq);

	OblakMsg::deleteOblakMsg(pReq);
}

void DmvWorker::HandleAffRequest(struct_screen sScreen, int iPriority)
{
	QByteArray baReq;
	bool bShouldSend;

	SetPropertiesAndText *pReq = new SetPropertiesAndText();

	TRACE_D(m_siInstId, QString("DmvWorker::HandleAffRequest() - Requested label: [%1]").arg(sScreen.tab_request[iPriority].p_label->label));

	pReq->setPropertiesObject(m_pTextProperties);
	pReq->setPropertiesArray();

	bShouldSend = GetTextToDisplay(pReq, sScreen, iPriority, baReq);
	
	if (bShouldSend)
	{
		pReq->encode(baReq);
		TRACE_D(m_siInstId, QString("DmvWorker::HandleAffRequest() - REQUEST: [%1]").arg(HelpFuncs::ByteArrayToAnsi(baReq)));
		m_baLastReq = baReq;
		m_pDmv_Client->sendDatagram(baReq);
	}
	else
	{
		TRACE_D(m_siInstId, QString("DmvWorker::HandleAffRequest() - Label saved but not sent due to priority"));
	}

	OblakMsg::deleteOblakMsg(pReq);
}

void DmvWorker::HandleTlmRequest(enum_tlm_dmv_images eTlmImage)
{
	QByteArray baReq;

	SetImage *pReq = new SetImage();

	TRACE_D(m_siInstId, QString("DmvWorker::HandleTlmRequest() - Image to set [%1]").arg(TlmEnumToString(eTlmImage)));

	pReq->setImage(ConvertImageToProtocol(eTlmImage));
	pReq->setXCoordinate(m_baImageXCoordinate);
	pReq->setYCoordinate(m_baImageYCoordinate);
	pReq->encode(baReq);

	TRACE_D(m_siInstId, QString("DmvWorker::HandleTlmRequest() - REQUEST [%1]").arg(HelpFuncs::ByteArrayToHexStr(baReq)));

	m_baLastReq = baReq;
	m_pDmv_Client->sendDatagram(baReq);

	OblakMsg::deleteOblakMsg(pReq);
}

void DmvWorker::HandleNewFlasherStateRequest(enum_flasher_state eFlasherState)
{
	QByteArray baReq;

	SetFlasher *pReq = new SetFlasher();

	pReq->setDisplayType(m_pOblakDevice->getDeviceType());
	pReq->setFlasherState(ConvertFlasherStateToProtocol(eFlasherState));
	pReq->encode(baReq);

	TRACE_D(m_siInstId, QString("DmvWorker::HandleNewFlasherStateRequest() - REQUEST [%1]").arg(HelpFuncs::ByteArrayToHexStr(baReq)));

	m_baLastReq = baReq;
	m_pDmv_Client->sendDatagram(baReq);

	OblakMsg::deleteOblakMsg(pReq);
}

void DmvWorker::HandleConfirmFlasherStateRequest()
{
	QByteArray baReq;

	ChangeFlasherParameter *pReq = new ChangeFlasherParameter();

	pReq->setDisplayType(m_pOblakDevice->getDeviceType());
	pReq->encode(baReq);

	TRACE_D(m_siInstId, QString("DmvWorker::HandleConfirmFlasherStateRequest() - REQUEST [%1]").arg(HelpFuncs::ByteArrayToHexStr(baReq)));

	m_baLastReq = baReq;
	m_pDmv_Client->sendDatagram(baReq);

	OblakMsg::deleteOblakMsg(pReq);
}

void DmvWorker::SendTextProperties()
{
	QByteArray baReq;

	m_pTextProperties->encode(baReq);

	TRACE_D(m_siInstId, QString("DmvWorker::SendTextProperties() - REQUEST [%1]").arg(HelpFuncs::ByteArrayToHexStr(baReq)));

	m_baLastReq = baReq;
	m_pDmv_Client->sendDatagram(baReq);
}

void DmvWorker::ClearScreen()
{
	SetTextMessage *pReq = new SetTextMessage();
	QByteArray baReq;

	pReq->setLine(QByteArray("          ", CalculateMaxCharacters(m_iMaxSpacing)), SetTextMessage::eOblak_Line1);
	pReq->setLine(QByteArray("          ", CalculateMaxCharacters(m_iMaxSpacing)), SetTextMessage::eOblak_Line2);
	pReq->setLine(QByteArray("          ", CalculateMaxCharacters(m_iMaxSpacing)), SetTextMessage::eOblak_Line3);
	pReq->setLine(QByteArray("          ", CalculateMaxCharacters(m_iMaxSpacing)), SetTextMessage::eOblak_Line4);
	
	NewView(m_siInstId, "          ", 
						"          ",
						"          ", 
						"          ");

	pReq->encode(baReq);

	TRACE_D(m_siInstId, QString("DmvWorker::ClearScreen() - REQUEST [%1]").arg(HelpFuncs::ByteArrayToHexStr(baReq)));

	m_baLastReq = baReq;
	m_pDmv_Client->sendDatagram(baReq);

	OblakMsg::deleteOblakMsg(pReq);
}

bool DmvWorker::GetTextToDisplay(SetPropertiesAndText *pMsg, struct_screen sScreen, int iPriority, QByteArray &baPacket)
{
	short						iPriorityInProgress1 = 0, 
								iPriorityInProgress2 = 0, 
								iPriorityInProgress3 = 0,
								iPriorityInProgress4 = 0;

	struct_list_text			*psText1 = nullptr,
								*psText2 = nullptr,
								*psText3 = nullptr,
								*psText4 = nullptr;


	struct_list_text			sEmptyText = { 'A', "          ", FALSE, NULL };

	struct_request_screen		*p_line1,
								*p_line2,
								*p_line3,
								*p_line4;

	bool bShouldBeSent = false;

	iPriorityInProgress1 = AFF_MsgRequestLinePriority(1, &sScreen);
	iPriorityInProgress2 = AFF_MsgRequestLinePriority(2, &sScreen);
	iPriorityInProgress3 = AFF_MsgRequestLinePriority(3, &sScreen);
	iPriorityInProgress4 = AFF_MsgRequestLinePriority(4, &sScreen);

	/* Update the display if it is the priority to display */
	if (iPriorityInProgress1 <= iPriority || iPriorityInProgress2 <= iPriority || iPriorityInProgress3 <= iPriority || iPriorityInProgress4 <= iPriority)
	{
		/* Choice of line 1 */
		if (iPriorityInProgress1 != -1)
		{
			p_line1 = &(gsDMV[m_siInstId].sScreen.tab_request[iPriorityInProgress1]);
			psText1 = AFF_MsgResearchLineNumber(p_line1, 1);
		}
		else
		{
			psText1 = &sEmptyText;
		}

		/* Choice of line 2 */
		if (iPriorityInProgress2 != -1)
		{
			p_line2 = &(gsDMV[m_siInstId].sScreen.tab_request[iPriorityInProgress2]);
			psText2 = AFF_MsgResearchLineNumber(p_line2, 2);
		}
		else
		{
			psText2 = &sEmptyText;
		}

		/* Choice of line 3 */
		if (iPriorityInProgress3 != -1)
		{
			p_line3 = &(gsDMV[m_siInstId].sScreen.tab_request[iPriorityInProgress3]);
			psText3 = AFF_MsgResearchLineNumber(p_line3, 3);
		}
		else
		{
			psText3 = &sEmptyText;
		}

		/* Choice of line 4 */
		if (iPriorityInProgress4 != -1)
		{
			p_line4 = &(gsDMV[m_siInstId].sScreen.tab_request[iPriorityInProgress4]);
			psText4 = AFF_MsgResearchLineNumber(p_line4, 4);
		}
		else
		{
			psText4 = &sEmptyText;
		}

		if (psText1 != 0 && psText1->police == 'A')
		{
			if (psText1->texte[0] == 0)
				psText1->texte[0] = '\x20';

			psText1->texte[strlen((char*)psText1->texte)] = '\x0';

			pMsg->setLine(QByteArray(reinterpret_cast<char*>(psText1->texte)), OblakMsg::eOblak_Line1);
		}
		if (psText2 != 0 && psText2->police == 'A')
		{
			if (psText2->texte[0] == 0)
				psText2->texte[0] = '\x20';

			psText2->texte[strlen((char*)psText2->texte)] = '\x0';

			pMsg->setLine(QByteArray(reinterpret_cast<char*>(psText2->texte)), OblakMsg::eOblak_Line2);
		}
		if (psText3 != 0 && psText3->police == 'A')
		{
			if (psText3->texte[0] == 0)
				psText3->texte[0] = '\x20';

			psText3->texte[strlen((char*)psText3->texte)] = '\x0';
			
			pMsg->setLine(QByteArray(reinterpret_cast<char*>(psText3->texte)), OblakMsg::eOblak_Line3);
		}
		if (psText4 != 0 && psText4->police == 'A')
		{
			if (psText4->texte[0] == 0)
				psText4->texte[0] = '\x20';

			psText4->texte[strlen((char*)psText4->texte)] = '\x0';

			pMsg->setLine(QByteArray(reinterpret_cast<char*>(psText4->texte)), OblakMsg::eOblak_Line4);
		}

		NewView(m_siInstId, (char*)(&psText1->texte), (char*)(&psText2->texte), (char*)(&psText3->texte), (char*)(&psText4->texte));

		bShouldBeSent = true;
	}

	return bShouldBeSent;
}

int DmvWorker::CalculateMaxCharacters(int iMaxSpacing)
{
	if (iMaxSpacing == 3)
	{
		return 9;
	}
	else if (iMaxSpacing == 2)
	{
		return 10;
	}
	else if (iMaxSpacing == 1)
	{
		return 10;
	}
	else
	{
		m_iMaxSpacing = 1; //3,2,1 should be the only accepted ones
		return 9;
	}
}

void DmvWorker::NewView(dmv_inst_id inst_id, const char* line1, const char* line2, const char* line3, const char* line4)
{
	DWORD dwSize;
	struct_ani_ios * p_ios = (struct_ani_ios *)(0);

	TRACE_D(inst_id, QString("DmvWorker::NewView()"));

	ExitAlloue((struct_neutre **)(&p_ios),
			   sizeof(*p_ios),
			   gsDMV[inst_id].pool);

	p_ios->message_id = M_CHANGEMENT_VISU;

	dwSize = sizeof(p_ios->u.ios_ani.current_view[0]);
	strncpy_s(p_ios->u.ios_ani.current_view[0], sizeof(p_ios->u.ios_ani.current_view[0]), line1, dwSize);
	strncpy_s(p_ios->u.ios_ani.current_view[1], sizeof(p_ios->u.ios_ani.current_view[1]), line2, dwSize);
	strncpy_s(p_ios->u.ios_ani.current_view[2], sizeof(p_ios->u.ios_ani.current_view[2]), line3, dwSize);
	strncpy_s(p_ios->u.ios_ani.current_view[3], sizeof(p_ios->u.ios_ani.current_view[3]), line4, dwSize);

	p_ios->u.ios_ani.current_view[0][dwSize - 1] = '\0';
	p_ios->u.ios_ani.current_view[1][dwSize - 1] = '\0';
	p_ios->u.ios_ani.current_view[2][dwSize - 1] = '\0';
	p_ios->u.ios_ani.current_view[3][dwSize - 1] = '\0';


	/* postage dans la boite aux lettres */
	if (Envoie(gsDMV[inst_id].ani_mbox, 
			   gsDMV[inst_id].ios_mbox, 
			   (struct_neutre *)p_ios) != NOYAU_OK)
	{
		ExitLibere((struct_neutre **)&p_ios);
	}

}

void DmvWorker::onNewMsgFromDevice(const QByteArray &baResponse)
{
	OblakResponse *pRsp = new OblakResponse();
	OblakResponse::eRspCode eErrCode;

	StopResponseTimer();

	if (!baResponse.isEmpty())
	{
		TRACE_D(m_siInstId, QString("DmvWorker::onNewMsgFromDevice(): RECEIVED DATA HEX = [%1]").arg(HelpFuncs::ByteArrayToHexStr(baResponse)));

		if (pRsp->decode(baResponse))
		{
			eErrCode = pRsp->AnalyzeOblakResponse(baResponse);

			if (eErrCode != OblakResponse::eRspCode_NoError)
			{
				TRACE_W(m_siInstId, QString("DmvWorker::onNewMsgFromServer(): [NOK] -> %1").arg(pRsp->getErrorString()));
				m_pAutomate->processEvent(new DmvAutomateEvent(DmvAutomate::enuEvtRspError));
			}
			else
			{
				TRACE_D(m_siInstId, QString("DmvWorker::onNewMsgFromServer() - Last command successfully processed"));
				m_pAutomate->processEvent(new DmvAutomateEvent(DmvAutomate::enuEvtRspOK));
			}
		}
		else
		{
			TRACE_W(m_siInstId, QString("DmvWorker::onNewMsgFromServer(): [NOK] -> Decode Error -> Check if right device is configured or protocol DLL needs fixing"));
			m_pAutomate->processEvent(new DmvAutomateEvent(DmvAutomate::enuEvtRspError));
		}
	}

	OblakMsg::deleteOblakMsg(pRsp);
}

void DmvWorker::RetryLastRequest()
{
	if (!m_baLastReq.isEmpty())
	{
		m_pDmv_Client->sendDatagram(m_baLastReq);
	}
}

void DmvWorker::SendPollRequest()
{
	StopStatusTimer();
	QByteArray baDummyPacket("\x20", 1);

	m_pDmv_EchoClient->sendDatagram(baDummyPacket);
	StartStatusTimer();
}

void DmvWorker::onNewEchoMsgFromDevice(const QByteArray &baEchoResponse)
{
	StopStatusTimer();

	emit signal_connected();
}

OblakMsg::eFlasherState DmvWorker::ConvertFlasherStateToProtocol(enum_flasher_state eState)
{
	switch (eState)
	{
	case TLM_FLASHER_OFF:	return OblakMsg::eFlasher_OFF;
	case TLM_FLASHER_ON:	return OblakMsg::eFlasher_ON;
	default:
		return OblakMsg::eFlasher_OFF;
		break;
	}
}

OblakMsg::eOblakImage DmvWorker::ConvertImageToProtocol(int iImageRequestedByUser)
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

QString DmvWorker::TlmEnumToString(int state)
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

bool DmvWorker::isZeroed(const void* data, size_t size)
{
	const unsigned char* bytes = static_cast<const unsigned char*>(data);
	for (size_t i = 0; i < size; ++i) 
	{
		if (bytes[i] != 0) 
		{  
			return false;  
		}
	}
	return true;  
}