

#include "MRVideoCmdAutomate.h"
#include "MTracer.h"
#include "MRVideoCmdThread.h"
#include "MRVideoMsg.h"

//---------------------------------------------------------------------
//	 events desc initialization
//---------------------------------------------------------------------

MRVideoCmdAtmEvent::MRVideoCmdAtmEvent(int idEvent)
{
	m_iEventID = idEvent;

	switch(m_iEventID) {
//			enuEvtDefault,
		case MRVideoCmdAutomate::enuEvtErrOrCommTimeout:
			m_sDesc = "enuEvtErrOrCommTimeout";
			break;
		case MRVideoCmdAutomate::enuEvtPoolTimeout:
			m_sDesc = "enuEvtPoolTimeout";
			break;
		case MRVideoCmdAutomate::enuEvtACTIVATEReq:
			m_sDesc = "enuEvtACTIVATEReq";
			break;
		case MRVideoCmdAutomate::enuEvtDEACTIVATEReq:
			m_sDesc = "enuEvtDEACTIVATEReq";
			break;
		case MRVideoCmdAutomate::enuEvtGRABBReq:
			m_sDesc = "enuEvtGRABBReq";
			break;
		case MRVideoCmdAutomate::enuEvtSAVEReq:
			m_sDesc = "enuEvtSAVEReq";
			break;
		case MRVideoCmdAutomate::enuEvtUNFREEZEReq:
			m_sDesc = "enuEvtUNFREEZEReq";
			break;
		case MRVideoCmdAutomate::enuEvtRspOKStopped:
			m_sDesc = "enuEvtRspOKStopped";
			break;
		case MRVideoCmdAutomate::enuEvtRspOKRunning:
			m_sDesc = "enuEvtRspOKRunning";
			break;
		case MRVideoCmdAutomate::enuEvtRspOKGrabbing:
			m_sDesc = "enuEvtRspOKGrabbing";
			break;
		case MRVideoCmdAutomate::enuEvtRspOKGrabbed:
			m_sDesc = "enuEvtRspOKGrabbed";
			break;
		case MRVideoCmdAutomate::enuEvtRspOKImage:
			m_sDesc = "enuEvtRspOKImage";
			break;
		case MRVideoCmdAutomate::enuEvtRspOKInactive:
			m_sDesc = "enuEvtRspOKInactive";
			break;
		case MRVideoCmdAutomate::enuEvtRspNOK:
			m_sDesc = "enuEvtRspNOK";
			break;
		case MRVideoCmdAutomate::enuEvtSavingImageOK:
			m_sDesc = "enuEvtSavingImageOK";
			break;
		case MRVideoCmdAutomate::enuEvtSavingImageNOK:
			m_sDesc = "enuEvtSavingImageNOK";
			break;
			
				
		default:
			m_sDesc = "ERROR:Unknown event"; //this is error
					
	}


}

MRVideoCmdAtmEvent::~MRVideoCmdAtmEvent()
{

}




//---------------------------------------------------------------------
//	END events initialization
//---------------------------------------------------------------------




/*

*/
MRVideoCmdAutomate::MRVideoCmdAutomate()
{
//---------------------------------------------------------------------
//	Private data initialization
//---------------------------------------------------------------------

	m_bInitialized = false;

	m_iInitialStateID = enuNotActivated;
	
//---------------------------------------------------------------------
//	END Private data initialization
//---------------------------------------------------------------------
	
	//---------------------------------------------------------------------
	// Automat initialization
	//---------------------------------------------------------------------
	//Define transitions for every defined event - state 
	MAutState * pAutState;
	
	//enuNotActivated = 0,
		pAutState = new MAutState(enuNotActivated);
		pAutState->addTransition(new MAutTransition(enuEvtPoolTimeout,		enuPoolingStatusRunning,	this,AUT_TRANS_ACTION_AUTH(doTrans0_2PoolTimeout)));
		pAutState->addTransition(new MAutTransition(enuEvtACTIVATEReq,		enuPoolingStatusRunning,	this,AUT_TRANS_ACTION_AUTH(doTrans0_2ReACTIVATE)));
		pAutState->addTransition(new MAutTransition(enuEvtGRABBReq,			enuNotActivated,			this,AUT_TRANS_ACTION_AUTH(doTrans0_0GRABBReqNOK)));
		pAutState->addTransition(new MAutTransition(enuEvtSAVEReq,			enuNotActivated,			this,AUT_TRANS_ACTION_AUTH(doTrans0_0SAVEReqNOK)));
		pAutState->addTransition(new MAutTransition(enuEvtUNFREEZEReq,		enuNotActivated,			this,AUT_TRANS_ACTION_AUTH(doTrans0_0UNFREEZEReq)));
		pAutState->addTransition(new MAutTransition(enuEvtDEACTIVATEReq,	enuNotActivated,			this,AUT_TRANS_ACTION_AUTH(doTrans0_0DEACTIVATEReq)));
		this->addAutState(pAutState);
		
	//enuStopped = 1,
		pAutState = new MAutState(enuStopped);
		pAutState->addTransition(new MAutTransition(enuEvtPoolTimeout,		enuPoolingStatusRunning,	this,AUT_TRANS_ACTION_AUTH(doTrans1_2PoolTimeout)));
		pAutState->addTransition(new MAutTransition(enuEvtACTIVATEReq,		enuPoolingStatusRunning,	this,AUT_TRANS_ACTION_AUTH(doTrans1_2ReACTIVATE)));
		pAutState->addTransition(new MAutTransition(enuEvtGRABBReq,			enuStopped,					this,AUT_TRANS_ACTION_AUTH(doTrans1_1GRABBReqNOK)));
		pAutState->addTransition(new MAutTransition(enuEvtSAVEReq,			enuStopped,					this,AUT_TRANS_ACTION_AUTH(doTrans1_1SAVEReqNOK)));
		pAutState->addTransition(new MAutTransition(enuEvtUNFREEZEReq,		enuStopped,					this,AUT_TRANS_ACTION_AUTH(doTrans1_1UNFREEZEReq)));
		pAutState->addTransition(new MAutTransition(enuEvtDEACTIVATEReq,	enuNotActivated,			this,AUT_TRANS_ACTION_AUTH(doTrans1_0DEACTIVATEReq)));
		this->addAutState(pAutState);

	//enuPoolingStatusRunning = 2,
		pAutState = new MAutState(enuPoolingStatusRunning);
		pAutState->addTransition(new MAutTransition(enuEvtErrOrCommTimeout,	enuStopped,					this,AUT_TRANS_ACTION_AUTH(doTrans2_1Timeout)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKRunning,		enuRunning,					this,AUT_TRANS_ACTION_AUTH(doTrans2_3RspOKRunning)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKStopped,		enuStopped,					this,AUT_TRANS_ACTION_AUTH(doTrans2_1RspOKStopped)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKGrabbing,	enuGrabbing,				this,AUT_TRANS_ACTION_AUTH(doTrans2_4RspOKGrabbing)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKGrabbed,		enuGrabbedOK,				this,AUT_TRANS_ACTION_AUTH(doTrans2_6RspOKGrabbed)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKImage,		enuRunning,					this,AUT_TRANS_ACTION_AUTH(doTrans2_1RspNOK)));
		pAutState->addTransition(new MAutTransition(enuEvtRspNOK,			enuStopped,					this,AUT_TRANS_ACTION_AUTH(doTrans2_1RspNOK)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKInactive,	enuStopped,					this,AUT_TRANS_ACTION_AUTH(doTrans2_1RspNOK)));
		pAutState->addTransition(new MAutTransition(enuEvtDEACTIVATEReq,	enuNotActivated,			this,AUT_TRANS_ACTION_AUTH(doTrans2_0DEACTIVATEReq)));
		this->addAutState(pAutState);
		
	//enuRunning = 3,
		pAutState = new MAutState(enuRunning);
		pAutState->addTransition(new MAutTransition(enuEvtPoolTimeout,		enuPoolingStatusRunning,	this,AUT_TRANS_ACTION_AUTH(doTrans3_2PoolTimeout)));
		pAutState->addTransition(new MAutTransition(enuEvtACTIVATEReq,		enuPoolingStatusRunning,	this,AUT_TRANS_ACTION_AUTH(doTrans3_2ReACTIVATE)));
		pAutState->addTransition(new MAutTransition(enuEvtGRABBReq,			enuPoolingStatusGrabbing,	this,AUT_TRANS_ACTION_AUTH(doTrans3_4GRABBReq)));
		pAutState->addTransition(new MAutTransition(enuEvtSAVEReq,			enuRunning,					this,AUT_TRANS_ACTION_AUTH(doTrans3_3SAVEReqNOK)));
		pAutState->addTransition(new MAutTransition(enuEvtUNFREEZEReq,		enuRunning,					this,AUT_TRANS_ACTION_AUTH(doTrans3_3UNFREEZEReq)));
		pAutState->addTransition(new MAutTransition(enuEvtDEACTIVATEReq,	enuNotActivated,			this,AUT_TRANS_ACTION_AUTH(doTrans3_0DEACTIVATEReq)));
		this->addAutState(pAutState);

	//enuGrabbing = 4,
		pAutState = new MAutState(enuGrabbing);
		pAutState->addTransition(new MAutTransition(enuEvtPoolTimeout,		enuPoolingStatusGrabbing,	this,AUT_TRANS_ACTION_AUTH(doTrans4_5PoolTimeout)));
		pAutState->addTransition(new MAutTransition(enuEvtACTIVATEReq,		enuPoolingStatusRunning,	this,AUT_TRANS_ACTION_AUTH(doTrans4_2ReACTIVATE)));
		pAutState->addTransition(new MAutTransition(enuEvtUNFREEZEReq,		enuPoolingStatusRunning,	this,AUT_TRANS_ACTION_AUTH(doTrans4_2UNFREEZEReq)));
		pAutState->addTransition(new MAutTransition(enuEvtGRABBReq,			enuPoolingStatusGrabbing,	this,AUT_TRANS_ACTION_AUTH(doTrans4_4GRABBReq)));
		pAutState->addTransition(new MAutTransition(enuEvtDEACTIVATEReq,	enuNotActivated,			this,AUT_TRANS_ACTION_AUTH(doTrans4_0DEACTIVATEReq)));
		this->addAutState(pAutState);

		
	//enuPoolingStatusGrabbing = 5,
		pAutState = new MAutState(enuPoolingStatusGrabbing);
		pAutState->addTransition(new MAutTransition(enuEvtErrOrCommTimeout,	enuNotActivated,			this,AUT_TRANS_ACTION_AUTH(doTrans5_1Timeout)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKStopped,		enuStopped,					this,AUT_TRANS_ACTION_AUTH(doTrans5_1RspOKStopped)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKRunning,		enuRunning,					this,AUT_TRANS_ACTION_AUTH(doTrans5_3RspOKRunning)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKGrabbing,	enuGrabbing,				this,AUT_TRANS_ACTION_AUTH(doTrans5_4RspOKGrabbing)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKGrabbed,		enuGrabbedOK,				this,AUT_TRANS_ACTION_AUTH(doTrans5_6RspOKGrabbed)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKImage,		enuGrabbedOK,				this,AUT_TRANS_ACTION_AUTH(doTrans5_6RspOKImage)));
		pAutState->addTransition(new MAutTransition(enuEvtRspNOK,			enuStopped,					this,AUT_TRANS_ACTION_AUTH(doTrans5_1RspNOK)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKInactive,	enuStopped,					this,AUT_TRANS_ACTION_AUTH(doTrans5_1RspNOK)));
		pAutState->addTransition(new MAutTransition(enuEvtDEACTIVATEReq,	enuNotActivated,			this,AUT_TRANS_ACTION_AUTH(doTrans5_0DEACTIVATEReq)));
		this->addAutState(pAutState);

	//enuGrabbedOK = 6,
		pAutState = new MAutState(enuGrabbedOK);
		pAutState->addTransition(new MAutTransition(enuEvtPoolTimeout,		enuPoolingStatusGrabbed,	this,AUT_TRANS_ACTION_AUTH(doTrans6_7PoolTimeout)));
		pAutState->addTransition(new MAutTransition(enuEvtACTIVATEReq,		enuPoolingStatusRunning,	this,AUT_TRANS_ACTION_AUTH(doTrans6_2ReACTIVATE)));
		pAutState->addTransition(new MAutTransition(enuEvtGRABBReq,			enuPoolingStatusGrabbing,	this,AUT_TRANS_ACTION_AUTH(doTrans6_5GRABBReq)));
		pAutState->addTransition(new MAutTransition(enuEvtSAVEReq,			enuGettingImage,			this,AUT_TRANS_ACTION_AUTH(doTrans6_8SAVEReq)));
		pAutState->addTransition(new MAutTransition(enuEvtUNFREEZEReq,		enuPoolingStatusRunning,	this,AUT_TRANS_ACTION_AUTH(doTrans6_2UNFREEZEReq)));
		pAutState->addTransition(new MAutTransition(enuEvtDEACTIVATEReq,	enuNotActivated,			this,AUT_TRANS_ACTION_AUTH(doTrans6_0DEACTIVATEReq)));
		this->addAutState(pAutState);

	//enuPoolingStatusGrabbed = 7,
		pAutState = new MAutState(enuPoolingStatusGrabbed);
		pAutState->addTransition(new MAutTransition(enuEvtErrOrCommTimeout,	enuNotActivated,			this,AUT_TRANS_ACTION_AUTH(doTrans7_2Timeout)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKRunning,		enuRunning,					this,AUT_TRANS_ACTION_AUTH(doTrans7_3RspOKRunning)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKGrabbing,	enuRunning,					this,AUT_TRANS_ACTION_AUTH(doTrans7_3RspOKGrabbing)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKGrabbed,		enuGrabbedOK,				this,AUT_TRANS_ACTION_AUTH(doTrans7_6RspOKGrabbed)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKStopped,		enuStopped,					this,AUT_TRANS_ACTION_AUTH(doTrans7_1RspOKStopped)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKImage,		enuGrabbedOK,				this,AUT_TRANS_ACTION_AUTH(doTrans7_6RspOKImage)));
		pAutState->addTransition(new MAutTransition(enuEvtRspNOK,			enuStopped,					this,AUT_TRANS_ACTION_AUTH(doTrans7_1RspNOK)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKInactive,	enuStopped,					this,AUT_TRANS_ACTION_AUTH(doTrans7_1RspNOK)));
		pAutState->addTransition(new MAutTransition(enuEvtDEACTIVATEReq,	enuNotActivated,			this,AUT_TRANS_ACTION_AUTH(doTrans7_0DEACTIVATEReq)));
		this->addAutState(pAutState);

	//	enuGettingImage = 8,
		pAutState = new MAutState(enuGettingImage);
		pAutState->addTransition(new MAutTransition(enuEvtErrOrCommTimeout,	enuStopped,					this,AUT_TRANS_ACTION_AUTH(doTrans8_1Timeout)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKRunning,		enuRunning,					this,AUT_TRANS_ACTION_AUTH(doTrans8_3RspOKRunning)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKGrabbing,	enuRunning,					this,AUT_TRANS_ACTION_AUTH(doTrans8_3RspOKGrabbing)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKGrabbed,		enuGrabbedOK,				this,AUT_TRANS_ACTION_AUTH(doTrans8_6RspOKGrabbed)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKStopped,		enuStopped,					this,AUT_TRANS_ACTION_AUTH(doTrans8_1RspOKStopped)));
		pAutState->addTransition(new MAutTransition(enuEvtSavingImageOK,	enuGrabbedOK,				this,AUT_TRANS_ACTION_AUTH(doTrans8_6RspOKSaveOK)));
		pAutState->addTransition(new MAutTransition(enuEvtSavingImageNOK,	enuGrabbedOK,				this,AUT_TRANS_ACTION_AUTH(doTrans8_6RspOKSaveNOK)));
		pAutState->addTransition(new MAutTransition(enuEvtRspNOK,			enuStopped,					this,AUT_TRANS_ACTION_AUTH(doTrans8_1RspNOK)));
		pAutState->addTransition(new MAutTransition(enuEvtRspOKInactive,	enuStopped,					this,AUT_TRANS_ACTION_AUTH(doTrans8_1RspNOK)));
		pAutState->addTransition(new MAutTransition(enuEvtDEACTIVATEReq,	enuNotActivated,			this,AUT_TRANS_ACTION_AUTH(doTrans8_0DEACTIVATEReq)));
		this->addAutState(pAutState);

	//---------------------------------------------------------------------
	// END Automat initialization
	//---------------------------------------------------------------------
	
}

MRVideoCmdAutomate::~MRVideoCmdAutomate()
{
	
}


bool MRVideoCmdAutomate::initialize(MRVideoCmdThread * pRVideoCmdThread)
{
	m_pRVideoCmdThread = pRVideoCmdThread;

	return this->test();
}

bool MRVideoCmdAutomate::reset()
{
	m_iCurrentStateID = m_iInitialStateID;
	return true;
}

bool MRVideoCmdAutomate::ifCanProcessNewCommand()
{
	if(m_iCurrentStateID == MRVideoCmdAutomate::enuNotActivated || 
		m_iCurrentStateID == MRVideoCmdAutomate::enuStopped||
		m_iCurrentStateID == MRVideoCmdAutomate::enuRunning|| 
		m_iCurrentStateID == MRVideoCmdAutomate::enuGrabbing|| 
		m_iCurrentStateID == MRVideoCmdAutomate::enuGrabbedOK)
		return true;
	else
		return false;
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


//------------------------------------------------
//enuNotActivated = 0,
//------------------------------------------------


bool MRVideoCmdAutomate::doTrans0_2PoolTimeout(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans0_2PoolTimeout state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	m_pRVideoCmdThread->stopPoolTimer();
	return true;	
}

bool MRVideoCmdAutomate::doTrans0_2ReACTIVATE(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans0_2ReACTIVATE state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->stopPoolTimer();
	m_pRVideoCmdThread->sendHttpCommand(MRVideoCmdThread::enuHttpCmdGetStatus);

	return true;	
}

bool MRVideoCmdAutomate::doTrans0_0GRABBReqNOK(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans0_0GRABBReqNOK state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	sendFreezeRsp(false);
	return true;	
}

bool MRVideoCmdAutomate::doTrans0_0SAVEReqNOK(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans0_0SAVEReqNOK state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	sendSaveRsp(false);
	return true;	
}

bool MRVideoCmdAutomate::doTrans0_0UNFREEZEReq(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans0_0UNFREEZEReq state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	return true;	
}

bool MRVideoCmdAutomate::doTrans0_0DEACTIVATEReq(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans0_0DEACTIVATEReq state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	return true;	
}


//------------------------------------------------
//enuStopped = 1,
//------------------------------------------------

bool MRVideoCmdAutomate::doTrans1_2PoolTimeout(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans1_2PoolTimeout state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->stopPoolTimer();
	m_pRVideoCmdThread->sendHttpCommand(MRVideoCmdThread::enuHttpCmdGetStatus);

	return true;	
}

bool MRVideoCmdAutomate::doTrans1_2ReACTIVATE(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans1_2ReACTIVATE state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));


	m_pRVideoCmdThread->stopPoolTimer();
	m_pRVideoCmdThread->sendHttpCommand(MRVideoCmdThread::enuHttpCmdGetStatus);

	return true;	
}


bool MRVideoCmdAutomate::doTrans1_1GRABBReqNOK(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans1_1GRABBReqNOK state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	sendFreezeRsp(false);

	return true;	
}


bool MRVideoCmdAutomate::doTrans1_1SAVEReqNOK(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans1_1SAVEReqNOK state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	sendSaveRsp(false);
	return true;	
}

bool MRVideoCmdAutomate::doTrans1_1UNFREEZEReq(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans1_1UNFREEZEReq state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	return true;	
}

bool MRVideoCmdAutomate::doTrans1_0DEACTIVATEReq(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans1_0DEACTIVATEReq state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->stopPoolTimer();

	return true;	
}

//------------------------------------------------
//enuPoolingStatusRunning = 2,
//------------------------------------------------

bool MRVideoCmdAutomate::doTrans2_1Timeout(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans2_1Timeout state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->startPoolTimer();
	return true;	
}

bool MRVideoCmdAutomate::doTrans2_3RspOKRunning(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans2_3RspOKRunning state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->startPoolTimer();
	return true;	
}


bool MRVideoCmdAutomate::doTrans2_1RspOKStopped(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans2_1RspOKStopped state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->startPoolTimer();
	return true;	
}

bool MRVideoCmdAutomate::doTrans2_4RspOKGrabbing(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans2_4RspOKGrabbing state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->startPoolTimer();
	m_pRVideoCmdThread->sendResume(); //to resume the stream if it was reactivated

	return true;	
}

bool MRVideoCmdAutomate::doTrans2_6RspOKGrabbed(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans2_6RspOKGrabbed state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));

	m_pRVideoCmdThread->startPoolTimer();
	m_pRVideoCmdThread->sendResume(); //to resume the stream if it was reactivated

	return true;	
}




bool MRVideoCmdAutomate::doTrans2_1RspNOK(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans2_1RspNOK state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->startPoolTimer();
	return true;	
}

bool MRVideoCmdAutomate::doTrans2_0DEACTIVATEReq(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans2_0DEACTIVATEReq state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	// cancel http request
	m_pRVideoCmdThread->stopPoolTimer();
	m_pRVideoCmdThread->abortHttp();
	return true;	
}


//------------------------------------------------
//enuRunning = 3,
//------------------------------------------------
bool MRVideoCmdAutomate::doTrans3_2PoolTimeout(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans3_2PoolTimeout state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->stopPoolTimer();
	m_pRVideoCmdThread->sendHttpCommand(MRVideoCmdThread::enuHttpCmdGetStatus);
	return true;	
}

bool MRVideoCmdAutomate::doTrans3_2ReACTIVATE(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans3_2ReACTIVATE state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));

	m_pRVideoCmdThread->stopPoolTimer();
	m_pRVideoCmdThread->sendHttpCommand(MRVideoCmdThread::enuHttpCmdGetStatus);
	return true;	
}

bool MRVideoCmdAutomate::doTrans3_4GRABBReq(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans3_4GRABBReq state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->stopPoolTimer();
	m_pRVideoCmdThread->sendHttpCommand(MRVideoCmdThread::enuHttpCmdGrabb);
	return true;	
}


bool MRVideoCmdAutomate::doTrans3_3SAVEReqNOK(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans3_3SAVEReqNOK state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	sendSaveRsp(false);
	return true;	
}


bool MRVideoCmdAutomate::doTrans3_3UNFREEZEReq(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans3_3UNFREEZEReq state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	return true;	
}

bool MRVideoCmdAutomate::doTrans3_0DEACTIVATEReq(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans3_0DEACTIVATEReq state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));

	m_pRVideoCmdThread->stopPoolTimer();

	return true;	
}

//------------------------------------------------
//enuGrabbing = 4,
//------------------------------------------------
bool MRVideoCmdAutomate::doTrans4_5PoolTimeout(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans4_5PoolTimeout state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->stopPoolTimer();
	m_pRVideoCmdThread->sendHttpCommand(MRVideoCmdThread::enuHttpCmdGetStatus);
	return true;	
}

bool MRVideoCmdAutomate::doTrans4_2ReACTIVATE(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans4_2ReACTIVATE state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));

	m_pRVideoCmdThread->stopPoolTimer();
	m_pRVideoCmdThread->sendHttpCommand(MRVideoCmdThread::enuHttpCmdGetStatus);
	return true;	
}

bool MRVideoCmdAutomate::doTrans4_0DEACTIVATEReq(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans4_0DEACTIVATEReq state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));

	m_pRVideoCmdThread->stopPoolTimer();
	m_pRVideoCmdThread->abortHttp();
	sendFreezeRsp(false);
	return true;	
}

bool MRVideoCmdAutomate::doTrans4_2UNFREEZEReq(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans4_2UNFREEZEReq state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));

	m_pRVideoCmdThread->stopPoolTimer();
	sendFreezeRsp(false);
	m_pRVideoCmdThread->sendHttpCommand(MRVideoCmdThread::enuHttpCmdResume);

	return true;	
}


bool MRVideoCmdAutomate::doTrans4_4GRABBReq(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans4_4GRABBReq state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	//do not send another grab if currently is grabbing
	return true;	
}
//------------------------------------------------
//enuPoolingStatusGrabbing = 5,
//------------------------------------------------
bool MRVideoCmdAutomate::doTrans5_1Timeout(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans5_1Timeout state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->startPoolTimer();
	sendFreezeRsp(false);
	return true;	
}


bool MRVideoCmdAutomate::doTrans5_3RspOKRunning(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans5_3RspOKRunning state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->startPoolTimer();
	sendFreezeRsp(false);
	return true;	
}


bool MRVideoCmdAutomate::doTrans5_4RspOKGrabbing(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans5_4RspOKGrabbing state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	m_pRVideoCmdThread->startPoolTimer();
	
	return true;	
}


bool MRVideoCmdAutomate::doTrans5_1RspOKStopped(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans5_1RspOKStopped state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));

	m_pRVideoCmdThread->startPoolTimer();
	
	return true;	
}

bool MRVideoCmdAutomate::doTrans5_6RspOKGrabbed(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans5_6RspOKGrabbed state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	sendFreezeRsp(true);
	m_pRVideoCmdThread->startPoolTimer();
	
	return true;	
}


bool MRVideoCmdAutomate::doTrans5_6RspOKImage(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans5_6RspOKImage state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	sendFreezeRsp(false);
	m_pRVideoCmdThread->startPoolTimer();
	return true;	
}


bool MRVideoCmdAutomate::doTrans5_1RspNOK(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans5_1RspNOK state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	sendFreezeRsp(false);
	m_pRVideoCmdThread->startPoolTimer();
	return true;	
}

bool MRVideoCmdAutomate::doTrans5_0DEACTIVATEReq(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans5_0DEACTIVATEReq state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	

	//todo cancel http request
	m_pRVideoCmdThread->stopPoolTimer();
	m_pRVideoCmdThread->abortHttp();
	sendFreezeRsp(false);
	return true;	
}

//------------------------------------------------
//enuGrabbedOK = 6,
//------------------------------------------------
bool MRVideoCmdAutomate::doTrans6_7PoolTimeout(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans6_7PoolTimeout state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->stopPoolTimer();
	m_pRVideoCmdThread->sendHttpCommand(MRVideoCmdThread::enuHttpCmdGetStatus);
	return true;	
}

bool MRVideoCmdAutomate::doTrans6_2ReACTIVATE(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans6_2ReACTIVATE state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));

	m_pRVideoCmdThread->stopPoolTimer();
	m_pRVideoCmdThread->sendHttpCommand(MRVideoCmdThread::enuHttpCmdGetStatus);
	return true;	
}

bool MRVideoCmdAutomate::doTrans6_5GRABBReq(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans6_5GRABBReq state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	

	m_pRVideoCmdThread->stopPoolTimer();
	m_pRVideoCmdThread->sendHttpCommand(MRVideoCmdThread::enuHttpCmdGrabb);
	return true;	
}


bool MRVideoCmdAutomate::doTrans6_8SAVEReq(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans6_8SAVEReq state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->stopPoolTimer();
	m_pRVideoCmdThread->sendHttpCommand(MRVideoCmdThread::enuHttpCmdGetImage);
	return true;	
}


bool MRVideoCmdAutomate::doTrans6_2UNFREEZEReq(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans6_2UNFREEZEReq state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->stopPoolTimer();
	m_pRVideoCmdThread->sendHttpCommand(MRVideoCmdThread::enuHttpCmdResume);
	return true;	
}


bool MRVideoCmdAutomate::doTrans6_0DEACTIVATEReq(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans6_0DEACTIVATEReq state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->stopPoolTimer();
	return true;	
}

//------------------------------------------------
//enuPoolingStatusGrabbed = 7,
//------------------------------------------------
bool MRVideoCmdAutomate::doTrans7_2Timeout(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans7_2Timeout state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));

	m_pRVideoCmdThread->stopPoolTimer();
	m_pRVideoCmdThread->sendHttpCommand(MRVideoCmdThread::enuHttpCmdGetStatus);
	return true;	
}


bool MRVideoCmdAutomate::doTrans7_3RspOKRunning(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
	TRACE_D(QString("MRVideoCmdAutomate::doTrans7_3RspOKRunning state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->startPoolTimer();
	return true;	
}


bool MRVideoCmdAutomate::doTrans7_3RspOKGrabbing(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
	TRACE_D(QString("MRVideoCmdAutomate::doTrans7_3RspOKGrabbing state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->startPoolTimer();
	return true;	
}


bool MRVideoCmdAutomate::doTrans7_6RspOKGrabbed(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
	TRACE_D(QString("MRVideoCmdAutomate::doTrans7_6RspOKGrabbed state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->startPoolTimer();
	return true;	
}


bool MRVideoCmdAutomate::doTrans7_6RspOKImage(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
	TRACE_D(QString("MRVideoCmdAutomate::doTrans7_6RspOKImage state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->startPoolTimer();
	return true;	
}


bool MRVideoCmdAutomate::doTrans7_1RspOKStopped(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
	TRACE_D(QString("MRVideoCmdAutomate::doTrans7_1RspOKStopped state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->startPoolTimer();
	return true;	
}

bool MRVideoCmdAutomate::doTrans7_1RspNOK(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
	TRACE_D(QString("MRVideoCmdAutomate::doTrans7_1RspNOK state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->startPoolTimer();
	return true;	
}

bool MRVideoCmdAutomate::doTrans7_0DEACTIVATEReq(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
		TRACE_D(QString("MRVideoCmdAutomate::doTrans7_0DEACTIVATEReq state = %1 event:%2").
									arg(m_iCurrentStateID).
									arg(pEvent->getEventDesc()));
	m_pRVideoCmdThread->stopPoolTimer();
	
	return true;	
}

//------------------------------------------------
//	enuGettingImage = 8,
//------------------------------------------------
bool MRVideoCmdAutomate::doTrans8_1Timeout(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
	TRACE_D(QString("MRVideoCmdAutomate::doTrans8_1Timeout state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));
	
	sendSaveRsp(false);
	m_pRVideoCmdThread->startPoolTimer();
	return true;	
}


bool MRVideoCmdAutomate::doTrans8_3RspOKRunning(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
	TRACE_D(QString("MRVideoCmdAutomate::doTrans8_3RspOKRunning state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));
	
	sendSaveRsp(false);
	m_pRVideoCmdThread->startPoolTimer();
	return true;	
}


bool MRVideoCmdAutomate::doTrans8_3RspOKGrabbing(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
	TRACE_D(QString("MRVideoCmdAutomate::doTrans8_3RspOKGrabbing state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));
	
	sendSaveRsp(false);
	m_pRVideoCmdThread->startPoolTimer();
	return true;	
}


bool MRVideoCmdAutomate::doTrans8_6RspOKGrabbed(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
	TRACE_D(QString("MRVideoCmdAutomate::doTrans8_6RspOKGrabbed state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));
	
	sendSaveRsp(false);
	m_pRVideoCmdThread->startPoolTimer();
	return true;	
}

bool MRVideoCmdAutomate::doTrans8_1RspOKStopped(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
	TRACE_D(QString("MRVideoCmdAutomate::doTrans8_1RspOKStopped state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));
	
	m_pRVideoCmdThread->startPoolTimer();
	return true;	
}



bool MRVideoCmdAutomate::doTrans8_6RspOKSaveOK(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
	TRACE_D(QString("MRVideoCmdAutomate::doTrans8_6RspOKSaveOK state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));
	
	sendSaveRsp(true);
	m_pRVideoCmdThread->startPoolTimer();
		
	return true;	
}

bool MRVideoCmdAutomate::doTrans8_6RspOKSaveNOK(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
	TRACE_D(QString("MRVideoCmdAutomate::doTrans8_6RspOKSaveNOK state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));
	
	sendSaveRsp(false);
	m_pRVideoCmdThread->startPoolTimer();
	return true;	
}

bool MRVideoCmdAutomate::doTrans8_1RspNOK(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
	TRACE_D(QString("MRVideoCmdAutomate::doTrans8_1RspNOK state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));
	
	sendSaveRsp(false);
	m_pRVideoCmdThread->startPoolTimer();
	return true;	
}

bool MRVideoCmdAutomate::doTrans8_0DEACTIVATEReq(MAutEvent * pEvent)
{
	if(m_pRVideoCmdThread->getTraceAll())
	TRACE_D(QString("MRVideoCmdAutomate::doTrans8_0DEACTIVATEReq state = %1 event:%2").
								arg(m_iCurrentStateID).
								arg(pEvent->getEventDesc()));
	
	//cancel http request
	m_pRVideoCmdThread->stopPoolTimer();
	m_pRVideoCmdThread->abortHttp();
	sendSaveRsp(false);
	return true;	
}

//------------------------------------------------

void MRVideoCmdAutomate::sendFreezeRsp(bool bOK)
{
	MRVideoMsgVideoFreezeRsp *pMsg = new MRVideoMsgVideoFreezeRsp();
	pMsg->m_bOK = bOK;
	m_pRVideoCmdThread->sendOutputRVideoMessage(pMsg);
}

void MRVideoCmdAutomate::sendSaveRsp(bool bOK)
{
	MRVideoMsgVideoSaveRsp *pMsg = new MRVideoMsgVideoSaveRsp();
	pMsg->m_bOK = bOK;
	m_pRVideoCmdThread->sendOutputRVideoMessage(pMsg);
}

