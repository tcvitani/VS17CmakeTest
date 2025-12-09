#include "DmvAutomate.h"
#include "InstTracer.h"
#include "DmvWorker.h"
#include "OblakMsg.h"

DmvAutomateEvent::DmvAutomateEvent(int idEvent)
{
	m_iEventID = idEvent;

	switch (m_iEventID) {
	case DmvAutomate::enuEvtConnected:
		m_sDesc = "enuEvtConnected";
		break;
	case DmvAutomate::enuEvtDisconnected:
		m_sDesc = "enuEvtDisconnected";
		break;
	case DmvAutomate::enuEvtLinkError:
		m_sDesc = "enuEvtLinkError";
		break;
	case DmvAutomate::enuEvtRspOK:
		m_sDesc = "enuEvtRspOK";
		break;
	case DmvAutomate::enuEvtRspError:
		m_sDesc = "enuEvtRspError";
		break;
	case DmvAutomate::enuEvtRspTimeout:
		m_sDesc = "enuEvtRspTimeout";
		break;
	case DmvAutomate::enuEvtPollTimeout:
		m_sDesc = "enuEvtPollTimeout";
		break;
	case DmvAutomate::enuEvtActivationReq:
		m_sDesc = "enuEvtActivationReq";
		break;
	case DmvAutomate::enuEvtDisplayReq:
		m_sDesc = "enuEvtDisplayReq";
		break;
	case DmvAutomate::enuEvtTrafficLightReq:
		m_sDesc = "enuEvtTrafficLightReq";
		break;
	case DmvAutomate::enuEvtSetFlasherStateReq:
		m_sDesc = "enuEvtSetFlasherStateReq";
		break;
	case DmvAutomate::enuEvtConfirmFlasherStateReq:
		m_sDesc = "enuEvtConfirmFlasherStateReq";
		break;
	default:
		m_sDesc = "ERROR: Unknown event";
		break;
	}
}

DmvAutomateEvent::~DmvAutomateEvent()
{

}

DmvAutomate::DmvAutomate(short int siInstId, DmvWorker * pWorker) : Automat(siInstId)
{
	//---------------------------------------------------------------------
	//	Private data initialization
	//---------------------------------------------------------------------
	m_pWorker = pWorker;
	m_iInitialStateID = enuInitial;
	//---------------------------------------------------------------------
	//	END Private data initialization
	//---------------------------------------------------------------------

	//---------------------------------------------------------------------
	// Automate initialization
	//---------------------------------------------------------------------
	//Define transitions for every defined event - state 
	AutState * pAutState;

	//enuInitial = 0,
	pAutState = new AutState(enuInitial);
	pAutState->addTransition(new AutTransition(enuEvtConnected,				enuActivation,		this, AUT_TRANS_ACTION(doResetLinkErrorInit)));
	pAutState->addTransition(new AutTransition(enuEvtLinkError,				enuLinkError,		this, AUT_TRANS_ACTION(doLinkError)));
	pAutState->addTransition(new AutTransition(enuEvtDisconnected,			enuInitial,			this, AUT_TRANS_ACTION(doDisconnected)));
	pAutState->addTransition(new AutTransition(enuEvtPollTimeout,			enuInitial,			this, AUT_TRANS_ACTION(doSendPollRequest)));
	pAutState->addTransition(new AutTransition(enuEvtRspTimeout,			enuInitial,			this, AUT_TRANS_ACTION(doSendPollRequest)));
	this->addAutState(pAutState);

	//enuActivation = 1,
	pAutState = new AutState(enuActivation);
	pAutState->addTransition(new AutTransition(enuEvtConnected,				enuActivation,		this, AUT_TRANS_ACTION(doVoid)));
	pAutState->addTransition(new AutTransition(enuEvtDisconnected,			enuInitial,			this, AUT_TRANS_ACTION(doDisconnected)));
	pAutState->addTransition(new AutTransition(enuEvtLinkError,				enuLinkError,		this, AUT_TRANS_ACTION(doLinkError)));
	pAutState->addTransition(new AutTransition(enuEvtActivationReq,			enuWaitActivateRsp, this, AUT_TRANS_ACTION(doActivationRequest)));
	pAutState->addTransition(new AutTransition(enuEvtRspTimeout,			enuActivation,		this, AUT_TRANS_ACTION(doResponseTimeout)));
	pAutState->addTransition(new AutTransition(enuEvtPollTimeout,			enuActivation,		this, AUT_TRANS_ACTION(doSendPollRequest)));
	this->addAutState(pAutState);

	//enuWaitActivateRsp = 2,
	pAutState = new AutState(enuWaitActivateRsp);
	pAutState->addTransition(new AutTransition(enuEvtConnected,				enuWaitActivateRsp, this, AUT_TRANS_ACTION(doActivationRequest)));
	pAutState->addTransition(new AutTransition(enuEvtDisconnected,			enuInitial,			this, AUT_TRANS_ACTION(doDisconnected)));
	pAutState->addTransition(new AutTransition(enuEvtLinkError,				enuLinkError,		this, AUT_TRANS_ACTION(doLinkError)));
	pAutState->addTransition(new AutTransition(enuEvtRspOK,					enuConnectedIdle,	this, AUT_TRANS_ACTION(doVoid)));
	pAutState->addTransition(new AutTransition(enuEvtRspError,				enuWaitActivateRsp,	this, AUT_TRANS_ACTION(doActivationRequest)));
	pAutState->addTransition(new AutTransition(enuEvtRspTimeout,			enuActivation,		this, AUT_TRANS_ACTION(doResponseTimeout))); 
	pAutState->addTransition(new AutTransition(enuEvtPollTimeout,			enuWaitActivateRsp, this, AUT_TRANS_ACTION(doVoid)));
	this->addAutState(pAutState);

	//enuConnectedIdle = 3,
	pAutState = new AutState(enuConnectedIdle);
	pAutState->addTransition(new AutTransition(enuEvtConnected,				enuConnectedIdle,	this, AUT_TRANS_ACTION(doVoid)));
	pAutState->addTransition(new AutTransition(enuEvtDisconnected,			enuInitial,			this, AUT_TRANS_ACTION(doDisconnected)));
	pAutState->addTransition(new AutTransition(enuEvtLinkError,				enuLinkError,		this, AUT_TRANS_ACTION(doLinkError)));
	pAutState->addTransition(new AutTransition(enuEvtDisplayReq,			enuWaitingResponse, this, AUT_TRANS_ACTION(doNewDisplayRequest)));
	pAutState->addTransition(new AutTransition(enuEvtTrafficLightReq,		enuWaitingResponse, this, AUT_TRANS_ACTION(doNewTrafficLightRequest)));
	pAutState->addTransition(new AutTransition(enuEvtSetFlasherStateReq,	enuWaitingResponse, this, AUT_TRANS_ACTION(doNewFlasherStateRequest)));
	pAutState->addTransition(new AutTransition(enuEvtConfirmFlasherStateReq,enuWaitingResponse, this, AUT_TRANS_ACTION(doConfirmFlasherStateRequest)));
	pAutState->addTransition(new AutTransition(enuEvtPollTimeout,			enuConnectedIdle,	this, AUT_TRANS_ACTION(doSendPollRequest)));
	this->addAutState(pAutState);

	//enuWaitingResponse = 4,
	pAutState = new AutState(enuWaitingResponse);
	pAutState->addTransition(new AutTransition(enuEvtConnected,				enuWaitingResponse, this, AUT_TRANS_ACTION(doVoid)));
	pAutState->addTransition(new AutTransition(enuEvtDisconnected,			enuInitial,			this, AUT_TRANS_ACTION(doDisconnected)));
	pAutState->addTransition(new AutTransition(enuEvtLinkError,				enuLinkError,		this, AUT_TRANS_ACTION(doLinkError)));
	pAutState->addTransition(new AutTransition(enuEvtDisplayReq,			enuWaitingResponse, this, AUT_TRANS_ACTION(doSaveForLater)));
	pAutState->addTransition(new AutTransition(enuEvtTrafficLightReq,		enuWaitingResponse, this, AUT_TRANS_ACTION(doSaveForLater)));
	pAutState->addTransition(new AutTransition(enuEvtSetFlasherStateReq,	enuWaitingResponse, this, AUT_TRANS_ACTION(doSaveForLater)));
	pAutState->addTransition(new AutTransition(enuEvtConfirmFlasherStateReq,enuWaitingResponse, this, AUT_TRANS_ACTION(doSaveForLater)));
	pAutState->addTransition(new AutTransition(enuEvtRspTimeout,			enuConnectedIdle,	this, AUT_TRANS_ACTION(doResponseTimeout))); 
	pAutState->addTransition(new AutTransition(enuEvtRspOK,					enuConnectedIdle,	this, AUT_TRANS_ACTION(doCommandOK)));
	pAutState->addTransition(new AutTransition(enuEvtRspError,				enuConnectedIdle,	this, AUT_TRANS_ACTION(doVoid))); // if rsp error is received, the protocol dll needs to be fixed
	pAutState->addTransition(new AutTransition(enuEvtPollTimeout,			enuWaitingResponse, this, AUT_TRANS_ACTION(doVoid)));
	this->addAutState(pAutState);

	//enuLinkError = 5,
	pAutState = new AutState(enuLinkError);
	pAutState->addTransition(new AutTransition(enuEvtLinkError,				enuLinkError,		this, AUT_TRANS_ACTION(doReinitSockets)));
	pAutState->addTransition(new AutTransition(enuEvtDisconnected,			enuLinkError,		this, AUT_TRANS_ACTION(doReinitSockets)));
	pAutState->addTransition(new AutTransition(enuEvtConnected,				enuConnectedIdle,	this, AUT_TRANS_ACTION(doResetLinkError)));
	pAutState->addTransition(new AutTransition(enuEvtPollTimeout,			enuLinkError,		this, AUT_TRANS_ACTION(doSendPollRequest)));
	pAutState->addTransition(new AutTransition(enuEvtRspTimeout,			enuLinkError,		this, AUT_TRANS_ACTION(doSendPollRequest)));
	pAutState->addTransition(new AutTransition(enuEvtDisplayReq,			enuLinkError,		this, AUT_TRANS_ACTION(doVoid)));
	pAutState->addTransition(new AutTransition(enuEvtTrafficLightReq,		enuLinkError,		this, AUT_TRANS_ACTION(doVoid)));
	pAutState->addTransition(new AutTransition(enuEvtSetFlasherStateReq,	enuLinkError,		this, AUT_TRANS_ACTION(doVoid)));
	pAutState->addTransition(new AutTransition(enuEvtConfirmFlasherStateReq,enuLinkError,		this, AUT_TRANS_ACTION(doVoid)));
	this->addAutState(pAutState);
}

DmvAutomate::~DmvAutomate()
{

}

QString DmvAutomate::stateDesc()
{
	switch (m_iCurrentStateID)
	{
	case DmvAutomate::enuInitial: return "enuInitial";
	case DmvAutomate::enuActivation: return "enuActivation";
	case DmvAutomate::enuWaitActivateRsp: return "enuWaitActivateRsp";
	case DmvAutomate::enuConnectedIdle: return "enuConnectedIdle";
	case DmvAutomate::enuWaitingResponse: return "enuWaitingResponse";
	case DmvAutomate::enuLinkError: return "enuLinkError";
	default:
		return	"Unknown State!";
	}
}

QString DmvAutomate::getNextStateName(AutEvent * event)
{
	//Get the current state object
	AutState * pCurrentState = getCurrentState();

	//Test if the state is OK 
	if (pCurrentState == NULL)
	{
		return "Unknown State!";
	}

	m_iCurrentEventID = event->getEventId();

	//Find the transition in the current state that corresponds for the currently received event 
	AutTransition * pTransition = NULL;
	pTransition = pCurrentState->getTransition(m_iCurrentEventID);


	//Test if the state is OK 
	if (pTransition == NULL)
	{
		return "Unknown State!";
	}

	switch (pTransition->getNextStateId())
	{
	case DmvAutomate::enuInitial: return "enuInitial";
	case DmvAutomate::enuActivation: return "enuActivation";
	case DmvAutomate::enuWaitActivateRsp: return "enuWaitActivateRsp";
	case DmvAutomate::enuConnectedIdle: return "enuConnectedIdle";
	case DmvAutomate::enuWaitingResponse: return "enuWaitingResponse";
	case DmvAutomate::enuLinkError: return "enuLinkError";
	default:
		return	"Unknown State!";
	}
}

bool DmvAutomate::initialize()
{
	return this->test();
}

bool DmvAutomate::reset()
{
	m_iCurrentStateID = m_iInitialStateID;
	return true;
}

bool DmvAutomate::processEvent(AutEvent * event)
{
	QString sMsg;
	
#if !_DEBUG // Trace polling only in debug configuration
	int iCurrentEventId = getCurrentEventId();
	if (iCurrentEventId == DmvAutomate::enuEvtPollTimeout ||
		iCurrentEventId == DmvAutomate::enuEvtConnected)
	{
		return Automat::processEvent(event); 
	}
#endif

	TRACE_D(m_siInstId, "*******************************************************************************************************************************************************");

	sMsg = QString("DmvAutomate::processEvent()  Event = [%1]  State = [%2]  NextState = [%3]")
		.arg(event->getEventDesc())
		.arg(stateDesc())
		.arg(getNextStateName(event));

	TRACE_D(m_siInstId, sMsg);

	TRACE_D(m_siInstId, "*******************************************************************************************************************************************************");
	
	return Automat::processEvent(event);
}

void DmvAutomate::StartSavedRequests()
{
	if (!m_lstReq.isEmpty())
	{
		AutEvent *pEvent = m_lstReq.takeFirst();

		if (pEvent != nullptr)
		{
			TRACE_D(m_siInstId, QString("DmvAutomate::StartSavedRequests() - Starting saved [%1]").arg(pEvent->getEventDesc()));

			switch (pEvent->getEventId())
			{
			case DmvAutomate::enuEvtDisplayReq:
			{
				NewDisplayEvent *pReqEvt = dynamic_cast<NewDisplayEvent*>(pEvent);
				if (pReqEvt != nullptr)
				{
					TRACE_D(m_siInstId, QString("DmvAutomate::StartSavedRequests() - AFF request"));
					processEvent(pReqEvt);
				}
			}
			break;
			case DmvAutomate::enuEvtTrafficLightReq:
			{
				NewTrafficLightEvent *pReqEvt = dynamic_cast<NewTrafficLightEvent*>(pEvent);
				if (pReqEvt != nullptr)
				{
					TRACE_D(m_siInstId, QString("DmvAutomate::StartSavedRequests() - TLM request"));
					processEvent(pReqEvt);
				}
			}
			break;
			case DmvAutomate::enuEvtSetFlasherStateReq:
			{
				NewFlasherStateEvent *pReqEvt = dynamic_cast<NewFlasherStateEvent*>(pEvent);
				if (pReqEvt != nullptr)
				{
					TRACE_D(m_siInstId, QString("DmvAutomate::StartSavedRequests() - Flasher State request"));
					processEvent(pReqEvt);
				}
			}
			case DmvAutomate::enuEvtConfirmFlasherStateReq:
			{
				DmvAutomateEvent *pReqEvt = dynamic_cast<DmvAutomateEvent *>(pEvent);
				if (pReqEvt != nullptr)
				{
					TRACE_D(m_siInstId, QString("DmvAutomate::StartSavedRequests() - Confirm Flasher State request"));
					processEvent(pEvent);
				}
			}
			break;
			default:
				TRACE_D(m_siInstId, QString("DmvAutomate::StartSavedRequests() - Invalid event saved"));
				break;
			}
		}
	}
}

//------------------------------------------------
//------------------------------------------------
//------------------------------------------------
//------------------------------------------------
//			Automate transitions functions
//------------------------------------------------
//------------------------------------------------
//------------------------------------------------
//------------------------------------------------

bool DmvAutomate::doVoid(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);

	return true;
}

bool DmvAutomate::doResetLinkError(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);

	m_pWorker->ResetLinkError();

	return true;
}

bool DmvAutomate::doResetLinkErrorInit(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);

	m_pWorker->m_uiLinkErrCount = 0;
	m_pWorker->SetLinkFailureStatus(false);

	processEvent(new DmvAutomateEvent(DmvAutomate::enuEvtActivationReq));

	return true;
}

bool DmvAutomate::doDisconnected(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);

	m_pWorker->IncLinkError();

	return true;
}

bool DmvAutomate::doLinkError(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);

	m_pWorker->SetLinkFailureStatus(true);

	return true;
}

bool DmvAutomate::doActivationRequest(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);

	m_pWorker->SendActivateScenario();
	m_pWorker->StartResponseTimer();

	return true;
}

bool DmvAutomate::doNewDisplayRequest(AutEvent *pEvent)
{
	NewDisplayEvent *pReqEvt = dynamic_cast<NewDisplayEvent*>(pEvent);

	if (pReqEvt == nullptr)
	{
		TRACE_D(m_siInstId, QString("DmvAutomate::doNewDisplayRequest() - Invalid AFF Request event"));
		return false;
	}

	m_pWorker->HandleAffRequest(pReqEvt->m_sScreen, pReqEvt->m_iPriority);
	m_pWorker->StartResponseTimer();

	return true;
}

bool DmvAutomate::doNewTrafficLightRequest(AutEvent *pEvent)
{
	NewTrafficLightEvent *pReqEvt = dynamic_cast<NewTrafficLightEvent*>(pEvent);

	if (pReqEvt == nullptr)
	{
		TRACE_D(m_siInstId, QString("DmvAutomate::doNewTrafficLightRequest() - Invalid TLM Request event"));
		return false;
	}

	m_pWorker->HandleTlmRequest(pReqEvt->m_eTlmImage);
	m_pWorker->StartResponseTimer();

	return true;
}

bool DmvAutomate::doNewFlasherStateRequest(AutEvent *pEvent)
{
	NewFlasherStateEvent *pReqEvt = dynamic_cast<NewFlasherStateEvent*>(pEvent);

	if (pReqEvt == nullptr)
	{
		TRACE_D(m_siInstId, QString("DmvAutomate::doNewFlasherStateRequest() - Invalid Flasher Request event"));
		return false;
	}

	m_pWorker->HandleNewFlasherStateRequest(pReqEvt->m_eFlasherState);
	m_pWorker->StartResponseTimer();

	return true;
}

bool DmvAutomate::doConfirmFlasherStateRequest(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);

	m_pWorker->HandleConfirmFlasherStateRequest();
	m_pWorker->StartResponseTimer();

	return true;
}

bool DmvAutomate::doSaveForLater(AutEvent *pEvent)
{
	TRACE_D(m_siInstId, QString("DmvAutomate::doSaveForLater() - Saving [%1] event for later").arg(pEvent->getEventDesc()));

	switch (pEvent->getEventId())
	{
	case DmvAutomate::enuEvtDisplayReq:
	{
		NewDisplayEvent *pDisplayEvent = dynamic_cast<NewDisplayEvent*>(pEvent);
		NewDisplayEvent *pEventToSave = new NewDisplayEvent(pDisplayEvent->m_sScreen, pDisplayEvent->m_iPriority);
		m_lstReq.append(pEventToSave);
	}
		break;
	case DmvAutomate::enuEvtTrafficLightReq:
	{
		NewTrafficLightEvent *pTlmEvent = dynamic_cast<NewTrafficLightEvent*>(pEvent);
		NewTrafficLightEvent *pEventToSave = new NewTrafficLightEvent(pTlmEvent->m_eTlmImage);
		m_lstReq.append(pEventToSave);
	}
		break;
	case DmvAutomate::enuEvtSetFlasherStateReq:
	{
		NewFlasherStateEvent *pFlasherEvent = dynamic_cast<NewFlasherStateEvent*>(pEvent);
		NewFlasherStateEvent *pEventToSave = new NewFlasherStateEvent(pFlasherEvent->m_eFlasherState);
		m_lstReq.append(pEventToSave);
	}
		break;
	case DmvAutomate::enuEvtConfirmFlasherStateReq:
	{
		DmvAutomateEvent *pEventToSave = new DmvAutomateEvent(DmvAutomate::enuEvtConfirmFlasherStateReq);
		m_lstReq.append(pEventToSave);
	}
		break;
	default:
		break;
	}

	return true;
}

bool DmvAutomate::doCommandOK(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);

	TRACE_D(m_siInstId, QString("DmvAutomate::doCommandOK() - Number of events saved for later [%1]").arg(m_lstReq.size()));

	if (m_lstReq.size() == 0)
	{
		return true;
	}
	else
	{
		StartSavedRequests();
	}

	return true;
}

bool DmvAutomate::doReinitSockets(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);

	if (m_pWorker->m_pDmv_Client != nullptr)
		delete m_pWorker->m_pDmv_Client;

	if (m_pWorker->m_pDmv_EchoClient != nullptr)
		delete m_pWorker->m_pDmv_EchoClient;

	m_pWorker->createClientSocket();

	return true;
}

bool DmvAutomate::doConnectionOK(AutEvent *pEvent)
{
	m_pWorker->ResetLinkError();

	return true;
}

bool DmvAutomate::doResponseTimeout(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);

	m_pWorker->m_uiLinkErrCount++;

	if (m_pWorker->m_uiLinkErrCount > m_pWorker->m_uiMaxLinkError)
	{
		m_pWorker->SetLinkFailureStatus(true);
		m_lstReq.clear();
		processEvent(new DmvAutomateEvent(DmvAutomate::enuEvtLinkError));
	}
	else
	{
		m_pWorker->RetryLastRequest();
	}

	return true;
}

bool DmvAutomate::doSendPollRequest(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);

	if (!m_pWorker->m_bWaitForResponseTimerStared)
	{
		m_pWorker->SendPollRequest();
	}
	return true;
}