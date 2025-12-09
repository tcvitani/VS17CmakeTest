#include "TlmDmvAutomate.h"
#include "InstTracer.h"
#include "TlmDmvWorker.h"
#include "OblakMsg.h"

TlmAutomateEvent::TlmAutomateEvent(int idEvent)
{
	m_iEventID = idEvent;

	switch (m_iEventID) {
	case TlmAutomate::enuEvtSetDefaultState:
		m_sDesc = "enuEvtSetDefaultState";
		break;
	case TlmAutomate::enuEvtLinkError:
		m_sDesc = "enuEvtLinkError";
		break;
	case TlmAutomate::enuEvtDisconnected:
		m_sDesc = "enuEvtDisconnected";
		break;
	case TlmAutomate::enuEvtLinkOK:
		m_sDesc = "enuEvtLinkOK";
		break;
	case TlmAutomate::enuEvtPollTimeout:
		m_sDesc = "enuEvtPollTimeout";
		break;
	case TlmAutomate::enuEvtReceivedAck:
		m_sDesc = "enuEvtReceivedAck";
		break;
	case TlmAutomate::enuEvtResponseTimeout:
		m_sDesc = "enuEvtResponseTimeout";
		break;
	case TlmAutomate::enuEvtSetImage:
		m_sDesc = "enuEvtSetImage";
		break;
	case TlmAutomate::enuEvtSetMemorizedImage:
		m_sDesc = "enuEvtSetMemorizedImage";
		break;
	case TlmAutomate::enuEvtSetImageNOK:
		m_sDesc = "enuEvtSetImageNOK";
		break;
	case TlmAutomate::enuEvtSetImageOK:
		m_sDesc = "enuEvtSetImageOK";
		break;
	case TlmAutomate::enuEvtVoid:
		m_sDesc = "enuEvtVoid";
		break;
	default:
		m_sDesc = "ERROR:Unknown event"; //this is error

	}
}

TlmAutomateEvent::~TlmAutomateEvent()
{

}

TlmAutomate::TlmAutomate(short int siInstId, TlmWorker * pWorker) : Automat(siInstId)
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
	pAutState->addTransition(new AutTransition(enuEvtLinkOK, enuConnectedIdle, this, AUT_TRANS_ACTION(doInitOK)));
	pAutState->addTransition(new AutTransition(enuEvtLinkError, enuLinkError, this, AUT_TRANS_ACTION(doLinkError)));
	pAutState->addTransition(new AutTransition(enuEvtDisconnected, enuInitial, this, AUT_TRANS_ACTION(doDisconnected)));
	pAutState->addTransition(new AutTransition(enuEvtPollTimeout, enuInitial, this, AUT_TRANS_ACTION(doSendPollRequest)));
	pAutState->addTransition(new AutTransition(enuEvtSetImage, enuInitial, this, AUT_TRANS_ACTION(doMemorizeSetImage)));
	pAutState->addTransition(new AutTransition(enuEvtResponseTimeout, enuInitial, this, AUT_TRANS_ACTION(doSendPollRequest)));
	this->addAutState(pAutState);

	//enuConnectedIdle = 1,
	pAutState = new AutState(enuConnectedIdle);
	pAutState->addTransition(new AutTransition(enuEvtSetDefaultState, enuWaitingResponse, this, AUT_TRANS_ACTION(doSendDefaultState)));
	pAutState->addTransition(new AutTransition(enuEvtLinkOK, enuConnectedIdle, this, AUT_TRANS_ACTION(doVoid)));
	pAutState->addTransition(new AutTransition(enuEvtLinkError, enuLinkError, this, AUT_TRANS_ACTION(doLinkError)));
	pAutState->addTransition(new AutTransition(enuEvtDisconnected, enuInitial, this, AUT_TRANS_ACTION(doDisconnected)));
	pAutState->addTransition(new AutTransition(enuEvtSetImage, enuWaitingResponse, this, AUT_TRANS_ACTION(doSetImage)));
	pAutState->addTransition(new AutTransition(enuEvtPollTimeout, enuConnectedIdle, this, AUT_TRANS_ACTION(doSendPollRequest)));
	pAutState->addTransition(new AutTransition(enuEvtResponseTimeout, enuWaitingResponse, this, AUT_TRANS_ACTION(doRetrySetImage)));
	this->addAutState(pAutState);

	//enuWaitingResponse = 2,
	pAutState = new AutState(enuWaitingResponse);
	pAutState->addTransition(new AutTransition(enuEvtLinkOK, enuWaitingResponse, this, AUT_TRANS_ACTION(doVoid)));
	pAutState->addTransition(new AutTransition(enuEvtLinkError, enuLinkError, this, AUT_TRANS_ACTION(doLinkError)));
	pAutState->addTransition(new AutTransition(enuEvtDisconnected, enuInitial, this, AUT_TRANS_ACTION(doDisconnected)));
	pAutState->addTransition(new AutTransition(enuEvtPollTimeout, enuWaitingResponse, this, AUT_TRANS_ACTION(doVoid)));
	pAutState->addTransition(new AutTransition(enuEvtResponseTimeout, enuConnectedIdle, this, AUT_TRANS_ACTION(doRetrySetImage)));
	pAutState->addTransition(new AutTransition(enuEvtSetImage, enuWaitingResponse, this, AUT_TRANS_ACTION(doMemorizeSetImage)));
	pAutState->addTransition(new AutTransition(enuEvtSetImageOK, enuConnectedIdle, this, AUT_TRANS_ACTION(doImageOK)));
	pAutState->addTransition(new AutTransition(enuEvtSetImageNOK, enuWaitingResponse, this, AUT_TRANS_ACTION(doRetrySetImage)));
	this->addAutState(pAutState);

	//enuLinkError = 3,
	pAutState = new AutState(enuLinkError);
	pAutState->addTransition(new AutTransition(enuEvtLinkError, enuLinkError, this, AUT_TRANS_ACTION(doReinitSockets)));
	pAutState->addTransition(new AutTransition(enuEvtDisconnected, enuLinkError, this, AUT_TRANS_ACTION(doReinitSockets)));
	pAutState->addTransition(new AutTransition(enuEvtLinkOK, enuConnectedIdle, this, AUT_TRANS_ACTION(doResetLinkError)));
	pAutState->addTransition(new AutTransition(enuEvtPollTimeout, enuLinkError, this, AUT_TRANS_ACTION(doSendPollRequest)));
	pAutState->addTransition(new AutTransition(enuEvtResponseTimeout, enuLinkError, this, AUT_TRANS_ACTION(doSendPollRequest)));
	this->addAutState(pAutState);
}

TlmAutomate::~TlmAutomate()
{

}

QString TlmAutomate::stateDesc()
{
	switch (m_iCurrentStateID)
	{
	case TlmAutomate::enuInitial: return "enuInitial";
	case TlmAutomate::enuConnectedIdle: return "enuConnectedIdle";
	case TlmAutomate::enuWaitingResponse: return "enuWaitingResponse";
	case TlmAutomate::enuLinkError: return "enuLinkError";
	default:
		return	"Unknown State!";
	}
}

bool TlmAutomate::initialize()
{
	return this->test();
}

bool TlmAutomate::reset()
{
	m_iCurrentStateID = m_iInitialStateID;
	return true;
}

bool TlmAutomate::processEvent(AutEvent * event)
{
	QString sMsg;
	
#if !_DEBUG // Trace polling only in debug configuration
	int iCurrentEventId = getCurrentEventId();
	if (iCurrentEventId == TlmAutomate::enuEvtPollTimeout ||
		iCurrentEventId == TlmAutomate::enuEvtLinkOK)
	{
		return Automat::processEvent(event); 
	}
#endif

	TRACE_D(m_siInstId, "***************************************************************************************************");

	sMsg = QString("TlmAutomate::processEvent event = [%1]   m_iCurrentStateID = [%2] ")
		.arg(event->getEventDesc())
		.arg(stateDesc());

	TRACE_D(m_siInstId, sMsg);

	TRACE_D(m_siInstId, "***************************************************************************************************");
	
	return Automat::processEvent(event);
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

bool TlmAutomate::doVoid(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);

	return true;
}

bool TlmAutomate::doReinitSockets(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);

	if (m_pWorker->m_pDmv_Client != nullptr)
		delete m_pWorker->m_pDmv_Client;

	if (m_pWorker->m_pDmv_EchoClient != nullptr)
		delete m_pWorker->m_pDmv_EchoClient;

	m_pWorker->createClientSocket();
	
	return true;
}

bool TlmAutomate::doInitOK(AutEvent *pEvent)
{
	m_pWorker->m_InitTimer.stop();
	m_pWorker->ResetLinkError();

	processEvent(new TlmAutomateEvent(TlmAutomate::enuEvtSetDefaultState));

	return true;
}

bool TlmAutomate::doSendDefaultState(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);

	if (m_pWorker->m_bImageMemorized)
	{
		TRACE_D(m_siInstId, QString("TlmAutomate::doSendDefaultState() - Traffic light image set to last memorized: %1").arg(m_pWorker->TlmEnumToString(m_pWorker->m_iMemorizedImage)));

		m_pWorker->m_iLastImageSet = m_pWorker->m_iMemorizedImage;
		m_pWorker->SendRequest(m_pWorker->m_iMemorizedImage);
		m_pWorker->StartResponseTimer();
		m_pWorker->m_bImageMemorized = false;
	}
	else
	{
		m_pWorker->m_iImageRequestedByUser = OblakMsg::eImage_Red;

		TRACE_D(m_siInstId, QString("TlmAutomate::doSendDefaultState() - Traffic light image set to: eImage_Red"));

		m_pWorker->SendRequest(m_pWorker->m_iImageRequestedByUser);
		m_pWorker->StartResponseTimer();
	}

	return true;
}

bool TlmAutomate::doRetrySendDefaultState(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);

	m_pWorker->m_uiLinkErrCount++;
	if (m_pWorker->m_uiLinkErrCount > m_pWorker->m_uiMaxLinkError)
	{
		processEvent(new TlmAutomateEvent(TlmAutomate::enuEvtLinkError));
	}
	else
	{
		m_pWorker->m_iImageRequestedByUser = OblakMsg::eImage_Red;
		m_pWorker->m_iLastImageSet = m_pWorker->m_iImageRequestedByUser;
		m_pWorker->StartResponseTimer();
		m_pWorker->SendRequest(m_pWorker->m_iLastImageSet);
	}

	return true;
}

bool TlmAutomate::doSetImage(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);
	
	m_pWorker->m_iLastImageSet = m_pWorker->m_iImageRequestedByUser;

	m_pWorker->SendRequest(m_pWorker->m_iLastImageSet);
	m_pWorker->StartResponseTimer();
	
	return true;
}

bool TlmAutomate::doSetMemorizedImage(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);
	
	m_pWorker->m_bImageMemorized = false;
	m_pWorker->m_iLastImageSet = m_pWorker->m_iMemorizedImage;

	m_pWorker->SendRequest(m_pWorker->m_iLastImageSet);
	m_pWorker->StartResponseTimer();
	
	return true;
}

bool TlmAutomate::doRetrySetImage(AutEvent *pEvent)
{
	TlmAutomateEvent_SetImage *pSetImageEvent = dynamic_cast<TlmAutomateEvent_SetImage *>(pEvent);

	if (pSetImageEvent != nullptr)
	{
		m_pWorker->m_uiLinkErrCount++;
		if (m_pWorker->m_uiLinkErrCount > m_pWorker->m_uiMaxLinkError)
		{
			processEvent(new TlmAutomateEvent(TlmAutomate::enuEvtLinkError));
		}
		else
		{
			m_pWorker->m_iLastImageSet = m_pWorker->m_iImageRequestedByUser;
			m_pWorker->SendRequest(m_pWorker->m_iLastImageSet);
			m_pWorker->StartResponseTimer();
		}
	}
	else
	{
		processEvent(new TlmAutomateEvent(TlmAutomate::enuEvtSetImageOK));
	}

	return true;
}

bool TlmAutomate::doImageOK(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);

	m_pWorker->ResetLinkError();

	if (m_pWorker->m_bImageMemorized)
	{
		processEvent(new TlmAutomateEvent_SetImage(m_pWorker->m_iMemorizedImage));
		m_pWorker->m_bImageMemorized = false;
	}

	return true;
}

bool TlmAutomate::doSendPollRequest(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);

	if (!m_pWorker->m_bWaitForResponseTimerStared)
	{
		m_pWorker->SendPollRequest();
	}
	return true;
}

bool TlmAutomate::doMemorizeSetImage(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);

	m_pWorker->m_bImageMemorized = true;
	m_pWorker->m_iMemorizedImage = m_pWorker->m_iImageRequestedByUser;

	return true;
}

bool TlmAutomate::doLinkError(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);

	m_pWorker->SetLinkFailureStatus(true);

	return true;
}

bool TlmAutomate::doDisconnected(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);

	m_pWorker->IncLinkError();

	return true;
}

bool TlmAutomate::doResetLinkError(AutEvent *pEvent)
{
	Q_UNUSED(pEvent);

	m_pWorker->ResetLinkError();

	if (m_pWorker->m_bImageMemorized)
	{
		processEvent(new TlmAutomateEvent(TlmAutomate::enuEvtSetDefaultState));
	}

	return true;
}
