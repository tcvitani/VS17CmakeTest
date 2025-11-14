

#include "MIhmAuthAutomate.h"
#include "MIhmSessionUserData.h"
#include "MIhmTCLInterfaces.h"
#include "MIhmMainLogic.h"

#include "MIhmMsg.h"
#include "MTracer.h"


//---------------------------------------------------------------------
//	 events desc initialization
//---------------------------------------------------------------------

MAuthAtmEvent::MAuthAtmEvent(int idEvent)
{
	m_pData = NULL;
	m_bHasData = false;
	m_iEventID = idEvent;

	switch(m_iEventID) {
// 		case MIHMAuthAutomate::enuEvtDefault:
// 			m_sDesc = "enuEvtDefault";
// 			break;
		case MIHMAuthAutomate::enuEvtInitTakeOver:
			m_sDesc = "enuEvtInitTakeOver";
			break;
		case MIHMAuthAutomate::enuEvtCommTimeout:
			m_sDesc = "enuEvtCommTimeout";
			break;
		case MIHMAuthAutomate::enuEvtReturnCtrlReq:
			m_sDesc = "enuEvtReturnCtrlReq";
			break;
		case MIHMAuthAutomate::enuEvtTakeCtrlRspOK:
			m_sDesc = "enuEvtTakeCtrlRspOK";
			break;
		case MIHMAuthAutomate::enuEvtTakeCtrlRspNOK:
			m_sDesc = "enuEvtTakeCtrlRspNOK";
			break;
		case MIHMAuthAutomate::enuEvtNewClientTakeOverCancel:
			m_sDesc = "enuEvtNewClientTakeOverCancel";
			break;
		case MIHMAuthAutomate::enuEvtTakeOverReject:
			m_sDesc = "enuEvtTakeOverReject";
			break;
		case MIHMAuthAutomate::enuEvtTakeOverTimeout:
			m_sDesc = "enuEvtTakeOverTimeout";
			break;
		case MIHMAuthAutomate::enuEvtTakeOverAccept:
			m_sDesc = "enuEvtTakeOverAccept";
			break;
		case MIHMAuthAutomate::enuEvtForceReturnCtrl:
			m_sDesc = "enuEvtForceReturnCtrl";
			break;
		case MIHMAuthAutomate::evtEvtInvalidUserTakingCtrlStatus:
			m_sDesc = "evtEvtInvalidUserTakingCtrlStatus";
			break;
			
		default:
			m_sDesc = "ERROR:Unknown event"; //this is error
					
	}


}

MAuthAtmEvent::~MAuthAtmEvent()
{

}

MIhmSessionUserData * MAuthAtmEvent::takeData()
{
	MIhmSessionUserData * pData;
	
	if(m_bHasData)
	{
		pData = m_pData;
		m_pData = NULL;	
		return pData;
	}
	else
		return NULL;
}

void MAuthAtmEvent::setData(MIhmSessionUserData *pData)
{
	m_bHasData = true;
	m_pData = pData;
}




//---------------------------------------------------------------------
//	END events initialization
//---------------------------------------------------------------------




/*

*/
MIHMAuthAutomate::MIHMAuthAutomate()
{
//---------------------------------------------------------------------
//	Private data initialization
//---------------------------------------------------------------------

	m_bInitialized = false;

	m_iInitialStateID = enuNoControl;
	
//---------------------------------------------------------------------
//	END Private data initialization
//---------------------------------------------------------------------
	
	//---------------------------------------------------------------------
	// Automat initialization
	//---------------------------------------------------------------------
	//Define transitions for every defined event - state 
	MAutState * pAutState;
	

	//enuNoControl = 1,
		pAutState = new MAutState(enuNoControl);

		pAutState->addTransition(new MAutTransition(enuEvtInitTakeOver,		enuWaitTakeCtrlRsp,	this,AUT_TRANS_ACTION_AUTH(doTrans1_2InitTakeOver)));
		this->addAutState(pAutState);

		
	//enuWaitTakeCtrlRsp = 2
		pAutState = new MAutState(enuWaitTakeCtrlRsp);
		pAutState->addTransition(new MAutTransition(enuEvtInitTakeOver,		enuWaitTakeCtrlRsp,		this,AUT_TRANS_ACTION_AUTH(doTrans2_2InitTakeOver)));
		pAutState->addTransition(new MAutTransition(enuEvtNewClientTakeOverCancel,	enuNoControl,	this,AUT_TRANS_ACTION_AUTH(doTrans2_1NewClientTakeOverCancel)));
		pAutState->addTransition(new MAutTransition(enuEvtTakeCtrlRspNOK,	enuNoControl,			this,AUT_TRANS_ACTION_AUTH(doTrans2_1TakeCtrlRspNOK)));
		pAutState->addTransition(new MAutTransition(enuEvtTakeCtrlRspOK,	enuInControl,			this,AUT_TRANS_ACTION_AUTH(doTrans2_3TakeCtrlRspOK)));
		pAutState->addTransition(new MAutTransition(enuEvtCommTimeout,		enuNoControl,			this,AUT_TRANS_ACTION_AUTH(doTrans2_1CommTimeout)));
		pAutState->addTransition(new MAutTransition(evtEvtInvalidUserTakingCtrlStatus,enuNoControl,	this,AUT_TRANS_ACTION_AUTH(doTrans2_1InvalidUTCtrlStatus)));
		this->addAutState(pAutState);


	//enuInControl = 3,
		pAutState = new MAutState(enuInControl);
		pAutState->addTransition(new MAutTransition(enuEvtInitTakeOver,			enuTakeOver,		this,AUT_TRANS_ACTION_AUTH(doTrans3_4InitTakeOver)));
		pAutState->addTransition(new MAutTransition(enuEvtReturnCtrlReq,		enuNoControl,		this,AUT_TRANS_ACTION_AUTH(doTrans3_1ReturnCtrlReq)));
		pAutState->addTransition(new MAutTransition(enuEvtForceReturnCtrl,		enuNoControl,		this,AUT_TRANS_ACTION_AUTH(doTrans3_1ForceReturnCtrl)));
		pAutState->addTransition(new MAutTransition(evtEvtInvalidUserTakingCtrlStatus,enuNoControl,	this,AUT_TRANS_ACTION_AUTH(doTrans3_1InvalidUTCtrlStatus)));
		pAutState->addTransition(new MAutTransition(enuEvtTakeCtrlRspOK,		enuInControl,		this,AUT_TRANS_ACTION_AUTH(doTrans2_3TakeCtrlRspOK)));
		
		this->addAutState(pAutState);
		
	//enuTakeOver = 4,
		pAutState = new MAutState(enuTakeOver);
		pAutState->addTransition(new MAutTransition(enuEvtInitTakeOver,				enuTakeOver,		this,AUT_TRANS_ACTION_AUTH(doTrans4_4InitTakeOver)));
		pAutState->addTransition(new MAutTransition(enuEvtNewClientTakeOverCancel,	enuInControl,		this,AUT_TRANS_ACTION_AUTH(doTrans4_3NewClientTakeOverCancel)));
		pAutState->addTransition(new MAutTransition(enuEvtTakeOverReject,			enuInControl,		this,AUT_TRANS_ACTION_AUTH(doTrans4_3TakeOverReject)));
		pAutState->addTransition(new MAutTransition(enuEvtTakeOverTimeout,			enuWaitTakeCtrlRsp,	this,AUT_TRANS_ACTION_AUTH(doTrans4_2TakeOverTimeout)));
		pAutState->addTransition(new MAutTransition(enuEvtTakeOverAccept,			enuWaitTakeCtrlRsp,	this,AUT_TRANS_ACTION_AUTH(doTrans4_2TakeOverAccept)));
		pAutState->addTransition(new MAutTransition(enuEvtReturnCtrlReq,			enuWaitTakeCtrlRsp,	this,AUT_TRANS_ACTION_AUTH(doTrans4_2ReturnCtrlReq)));
		pAutState->addTransition(new MAutTransition(enuEvtForceReturnCtrl,			enuWaitTakeCtrlRsp,	this,AUT_TRANS_ACTION_AUTH(doTrans4_2ForceReturnCtrl)));
		pAutState->addTransition(new MAutTransition(evtEvtInvalidUserTakingCtrlStatus,enuInControl,		this,AUT_TRANS_ACTION_AUTH(doTrans4_3InvalidUTCtrlStatus)));
		this->addAutState(pAutState);

		

	//---------------------------------------------------------------------
	// END Automat initialization
	//---------------------------------------------------------------------
	
}

MIHMAuthAutomate::~MIHMAuthAutomate()
{
	TRACE_D("MIHMAuthAutomate::~MIHMAuthAutomate(): Deleting MIHMAuthAutomate automat!");
	
}


bool MIHMAuthAutomate::initialize(MIhmTCLInterfaces * pTCLInterfaces)
{
	m_pTCLInf = pTCLInterfaces;

	return this->test();
}

//This way the TCL interface knows if the take over is in progress
// to decide if the issued commands and keys should be sent to the main app
bool MIHMAuthAutomate::isTakeOverInProgress()
{
	if(getCurrentStateId()==enuTakeOver)
		return true;
	else
		return false;

}


bool MIHMAuthAutomate::reset()
{
	m_iCurrentStateID = m_iInitialStateID;


	return true;
}


//------------------------------------------------
//------------------------------------------------
//------------------------------------------------
//------------------------------------------------
//			Automat transitions functions
//------------------------------------------------
//------------------------------------------------
//------------------------------------------------
//------------------------------------------------

bool MIHMAuthAutomate::doTrans1_2InitTakeOver(MAutEvent * pEvent)
{
	TRACE_D(QString("MIHMAuthAutomate::doTrans1_2InitTakeOver state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));

	MIhmSessionUserData *pData = NULL;
	pData =  ((MAuthAtmEvent*)pEvent)->takeData();

	if(pData!=NULL)
	{
		m_pTCLInf->setUserTakingControl(pData);
		MIhmMsgTakeCtrlReq *pMsg = new MIhmMsgTakeCtrlReq();
		pMsg->m_usrData = *pData;	
		m_pTCLInf->sendMessageToAni(pMsg);
		m_pTCLInf->startCommTimer();
		m_pTCLInf->showConnectingInProgress();

	}
	else
	{
		TRACE_W(QString("MIHMAuthAutomate::doTrans1_2InitTakeOver: Error invalid event data!"));
		MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::evtEvtInvalidUserTakingCtrlStatus); 
		processEvent(pEvent);
	}
	
	return true;	
}


//--------------------------------------------------------------------------

bool MIHMAuthAutomate::doTrans2_2InitTakeOver(MAutEvent * pEvent)
{
	TRACE_D(QString("MIHMAuthAutomate::doTrans2_2InitTakeOver state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));

	//To be used to inform a new requesting user that another user is in process of getting control
	return true;	

}


bool MIHMAuthAutomate::doTrans2_1TakeCtrlRspNOK(MAutEvent * pEvent) 
{
	TRACE_D(QString("MIHMAuthAutomate::doTrans2_1TakeCtrlRspNOK state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));

	MIhmSessionUserData *pData = NULL;
	m_pTCLInf->stopCommTimer();
	pData = m_pTCLInf->getUserTakingControl();

	if(pData!=NULL)
	{
		m_pTCLInf->hideConnectingInProgress();

		QString sErr = 	((MAuthAtmEvent*)pEvent)->getError();
		if(sErr.isEmpty())
			sErr = "LABEL_ERR_TAKE_CTRL_REFUSED_BY_APP";

		m_pTCLInf->informRequestingUserFailure(sErr);
	}
	
	m_pTCLInf->setUserTakingControl(NULL);

	return true;	
}


bool MIHMAuthAutomate::doTrans2_3TakeCtrlRspOK(MAutEvent * pEvent) 
{
	TRACE_D(QString("MIHMAuthAutomate::doTrans2_3TakeCtrlRspOK state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));

	MIhmSessionUserData *pData = NULL;
	m_pTCLInf->stopCommTimer();
	pData = m_pTCLInf->getUserTakingControl();
	m_pTCLInf->hideConnectingInProgress();

	// verify who is to be in control and update the application
	if(pData==NULL)
	{
		TRACE_W("MIHMAuthAutomate::doTrans2_3TakeCtrlRspOK : User taking control is NULL! Automat in invalid state!");
		
		MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::evtEvtInvalidUserTakingCtrlStatus); 
		processEvent(pEvent);
	}
	else
	{	
		m_pTCLInf->setUserInControl(pData);
	}

	m_pTCLInf->setUserTakingControl(NULL);

	return true;	
}

bool MIHMAuthAutomate::doTrans2_1CommTimeout(MAutEvent * pEvent)
{
	TRACE_D(QString("MIHMAuthAutomate::doTrans2_1CommTimeout state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));

	m_pTCLInf->stopCommTimer();
	
	MIhmSessionUserData *pData = NULL;
	pData = m_pTCLInf->getUserTakingControl();

	if(pData!=NULL)
	{
		m_pTCLInf->hideConnectingInProgress();
		m_pTCLInf->informRequestingUserFailure("LABEL_ERR_TAKE_CTRL_COMM_TIMEOUT");
	}

	m_pTCLInf->setUserTakingControl(NULL);

	return true;	
}




bool MIHMAuthAutomate::doTrans2_1InvalidUTCtrlStatus(MAutEvent * pEvent)
{
	TRACE_D(QString("MIHMAuthAutomate::doTrans2_1InvalidUTCtrlStatus state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));

	m_pTCLInf->stopCommTimer();
	m_pTCLInf->hideConnectingInProgress();
	m_pTCLInf->setUserTakingControl(NULL);

	return true;	
}

bool MIHMAuthAutomate::doTrans2_1NewClientTakeOverCancel(MAutEvent * pEvent)
{
	TRACE_D(QString("MIHMAuthAutomate::doTrans2_1NewClientTakeOverCancel state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));

	m_pTCLInf->stopCommTimer();
	m_pTCLInf->hideConnectingInProgress();
	m_pTCLInf->setUserTakingControl(NULL);

	return true;	
}



//-------------------------------------------------------------------------

bool MIHMAuthAutomate::doTrans3_4InitTakeOver(MAutEvent * pEvent) 
{
	TRACE_D(QString("MIHMAuthAutomate::doTrans3_4TakeOver state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));

	MIhmSessionUserData *pData = NULL;
	pData =  ((MAuthAtmEvent*)pEvent)->takeData();

	if(pData!=NULL)
	{
		m_pTCLInf->setUserTakingControl(pData);
		m_pTCLInf->setTakeOverInProgress();
		m_pTCLInf->startTakeOverTimer();
	}
	else
	{
		TRACE_W(QString("MIHMAuthAutomate::doTrans3_4InitTakeOver: Error invalid event data!"));
		MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::evtEvtInvalidUserTakingCtrlStatus); 
		processEvent(pEvent);
	}
	
	return true;	
}



bool MIHMAuthAutomate::doTrans3_1ReturnCtrlReq(MAutEvent * pEvent) 
{
	TRACE_D(QString("MIHMAuthAutomate::doTrans3_1ReturnCtrlReq state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));
	


	if(m_pTCLInf->getUserInControl() != NULL)
	{
		MIhmMsgReturnCtrlReq *pMsg = new MIhmMsgReturnCtrlReq();
		pMsg->m_usrData = *(m_pTCLInf->getUserInControl());	
		pMsg->m_iReason = ((MAuthAtmEvent*)pEvent)->getReason();
		m_pTCLInf->sendMessageToAni(pMsg);
	}

	m_pTCLInf->setUserInControl(NULL);

	return true;	
}



bool MIHMAuthAutomate::doTrans3_1ForceReturnCtrl(MAutEvent * pEvent) 
{
	TRACE_D(QString("MIHMAuthAutomate::doTrans3_1ForceReturnCtrl state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));
	
	if(m_pTCLInf->getUserInControl() != NULL)
	{
		MIhmMsgReturnCtrlReq *pMsg = new MIhmMsgReturnCtrlReq();
		pMsg->m_usrData = *(m_pTCLInf->getUserInControl());	
		pMsg->m_iReason = (int)enuRETURN_CTRL_FORCED;
		m_pTCLInf->sendMessageToAni(pMsg);
	}
	
	m_pTCLInf->setUserInControl(NULL);

	return true;	
}


bool MIHMAuthAutomate::doTrans3_1InvalidUTCtrlStatus(MAutEvent * pEvent)
{
	TRACE_D(QString("MIHMAuthAutomate::doTrans3_1InvalidUTCtrlStatus state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));

	return true;	
}




//------------------------------------------------

bool MIHMAuthAutomate::doTrans4_4InitTakeOver(MAutEvent * pEvent)
{
	TRACE_D(QString("MIHMAuthAutomate::doTrans4_4InitTakeOver state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));

	//To be used to inform a new requesting user that another user is in process of getting control
	return true;	

}



bool MIHMAuthAutomate::doTrans4_3NewClientTakeOverCancel(MAutEvent * pEvent)
{
	TRACE_D(QString("MIHMAuthAutomate::doTrans4_3NewClientTakeOverCancel state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));

	m_pTCLInf->stopTakeOverTimer();
	m_pTCLInf->cancelTakeOverInProgress();
	m_pTCLInf->setUserTakingControl(NULL);

	return true;	
}


bool MIHMAuthAutomate::doTrans4_3TakeOverReject(MAutEvent * pEvent) 
{
	TRACE_D(QString("MIHMAuthAutomate::doTrans4_3TakeOverReject state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));
	

	m_pTCLInf->stopTakeOverTimer();
	m_pTCLInf->cancelTakeOverInProgress();
	m_pTCLInf->informRequestingUserRejected();
	
	m_pTCLInf->setUserTakingControl(NULL);

	return true;	
}	

bool MIHMAuthAutomate::doTrans4_2TakeOverTimeout(MAutEvent * pEvent) 
{
	TRACE_D(QString("MIHMAuthAutomate::doTrans4_2TakeOverTimeout state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));
	

	m_pTCLInf->stopTakeOverTimer();

	if(m_pTCLInf->getUserInControl() != NULL)
	{
		MIhmMsgReturnCtrlReq *pMsg = new MIhmMsgReturnCtrlReq();
		pMsg->m_usrData = *(m_pTCLInf->getUserInControl());	
		pMsg->m_iReason = (int)enuRETURN_CTRL_TAKEOVER_TIMEOUT;
		m_pTCLInf->sendMessageToAni(pMsg);
	}

	return processTakeOver();	
}


bool MIHMAuthAutomate::doTrans4_2TakeOverAccept(MAutEvent * pEvent) 
{
	TRACE_D(QString("MIHMAuthAutomate::doTrans4_2TakeOverAccept state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));

	m_pTCLInf->stopTakeOverTimer();

	if(m_pTCLInf->getUserInControl() != NULL)
	{
		MIhmMsgReturnCtrlReq *pMsg = new MIhmMsgReturnCtrlReq();
		pMsg->m_usrData = *(m_pTCLInf->getUserInControl());	
		pMsg->m_iReason = (int)enuRETURN_CTRL_VOLUNTARY;
		m_pTCLInf->sendMessageToAni(pMsg);
	}
	
	
	return processTakeOver();	
}

bool MIHMAuthAutomate::doTrans4_2ReturnCtrlReq(MAutEvent * pEvent) 
{
	TRACE_D(QString("MIHMAuthAutomate::doTrans3_1ReturnCtrlReq state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));
	

	m_pTCLInf->stopTakeOverTimer();

	if(m_pTCLInf->getUserInControl() != NULL)
	{
		MIhmMsgReturnCtrlReq *pMsg = new MIhmMsgReturnCtrlReq();
		pMsg->m_usrData = *(m_pTCLInf->getUserInControl());	
		pMsg->m_iReason = ((MAuthAtmEvent*)pEvent)->getReason();
		m_pTCLInf->sendMessageToAni(pMsg);
	}

	return processTakeOver();	
}


bool MIHMAuthAutomate::doTrans4_2ForceReturnCtrl(MAutEvent * pEvent) 
{
	TRACE_D(QString("MIHMAuthAutomate::doTrans4_2ForceReturnCtrl state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));
	

	m_pTCLInf->stopTakeOverTimer();
	
	if(m_pTCLInf->getUserInControl() != NULL)
	{
		MIhmMsgReturnCtrlReq *pMsg = new MIhmMsgReturnCtrlReq();
		pMsg->m_usrData = *(m_pTCLInf->getUserInControl());	
		pMsg->m_iReason = (int)enuRETURN_CTRL_FORCED;
		m_pTCLInf->sendMessageToAni(pMsg);
	}
	

	return processTakeOver();	
}

bool MIHMAuthAutomate::doTrans4_3InvalidUTCtrlStatus(MAutEvent * pEvent)
{
	TRACE_D(QString("MIHMAuthAutomate::doTrans4_3InvalidUTCtrlStatus state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));
	
	return doTrans4_3TakeOverReject(pEvent);
}


bool MIHMAuthAutomate::processTakeOver()
{
	m_pTCLInf->stopTakeOverTimer();
	m_pTCLInf->cancelTakeOverInProgress();
	m_pTCLInf->setUserInControl(NULL);

	MIhmSessionUserData *pData = NULL;
	pData = m_pTCLInf->getUserTakingControl();

	if(pData!=NULL)
	{
		m_pTCLInf->showConnectingInProgress();
		MIhmMsgTakeCtrlReq *pMsg = new MIhmMsgTakeCtrlReq();
		pMsg->m_usrData = *pData;	
		m_pTCLInf->sendMessageToAni(pMsg);
		m_pTCLInf->startCommTimer();
	}
	else
	{
		MAuthAtmEvent * pEvent = new MAuthAtmEvent(MIHMAuthAutomate::evtEvtInvalidUserTakingCtrlStatus); 
		processEvent(pEvent);
	}

	return true;
}

//------------------------------------------------


