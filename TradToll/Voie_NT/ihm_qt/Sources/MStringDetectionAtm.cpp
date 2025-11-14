

#include "MStringDetection.h"
#include "MStringDetectionAtm.h"
#include "MTracer.h"

extern "C" {
	#include <run.H>
	#include <ihm.H>
};


//DISABLE TRACES
#undef TRACE_D
#define TRACE_D(X) {};

//---------------------------------------------------------------------
//	 events desc initialization
//---------------------------------------------------------------------

MStrDetAtmEvent::MStrDetAtmEvent(int idEvent)
{
	m_iEventID = idEvent;

	switch(m_iEventID) {
		case MStringDetectionAtm::enuEvtTimeout:
			m_sDesc = "enuEvtTimeout";
			break;
		case MStringDetectionAtm::enuEvtNewKey:
			m_sDesc = "enuEvtNewKey";
			break;
		case MStringDetectionAtm::enuEvtNoPreambleMatch:
			m_sDesc = "enuEvtNoPreambleMatch";
			break;
		case MStringDetectionAtm::enuEvtPreambleComplete:
			m_sDesc = "enuEvtPreambleComplete";
			break;
		case MStringDetectionAtm::enuEvtPreambleNotComplete:
			m_sDesc = "enuEvtPreambleNotComplete";
			break;
		case MStringDetectionAtm::enuEvtPostamble:
			m_sDesc = "enuEvtPostamble";
			break;
		case MStringDetectionAtm::enuEvtNotPostamble:
			m_sDesc = "enuEvtNotPostamble";
			break;
			
		default:
			m_sDesc = "ERROR:Unknown event"; //this is error
					
	}


}

MStrDetAtmEvent::~MStrDetAtmEvent()
{
	
}

//---------------------------------------------------------------------
//	END events initialization
//---------------------------------------------------------------------


MStringDetectionAtm::MStringDetectionAtm()
{
//---------------------------------------------------------------------
//	Private data initialization
//---------------------------------------------------------------------

	m_bInitialized = false;

	m_iInitialStateID = enuNoFrameReading;
	
//---------------------------------------------------------------------
//	END Private data initialization
//---------------------------------------------------------------------
	
	//---------------------------------------------------------------------
	// Automat initialization
	//---------------------------------------------------------------------
	//Define transitions for every defined event - state 
	MAutState * pAutState;
	

	//enuNoFrameReading = 1,
		pAutState = new MAutState(enuNoFrameReading);

		pAutState->addTransition(new MAutTransition(enuEvtNewKey,					enuNoFrameReading,		this,ATM_TRANS_STR_DET(doTrans1_1NewKey)));
		pAutState->addTransition(new MAutTransition(enuEvtNoPreambleMatch,			enuNoFrameReading,		this,ATM_TRANS_STR_DET(doTrans1_1NoPreambleMatch)));
		pAutState->addTransition(new MAutTransition(enuEvtPreambleNotComplete,		enuSelectingPreamble,	this,ATM_TRANS_STR_DET(doTrans1_2PreambleNotComplete)));
		pAutState->addTransition(new MAutTransition(enuEvtPreambleComplete,			enuPreambleSelected,	this,ATM_TRANS_STR_DET(doTrans1_3PreambleComplete)));
		this->addAutState(pAutState);

	//enuSelectingPreamble = 2,
		pAutState = new MAutState(enuSelectingPreamble);
		pAutState->addTransition(new MAutTransition(enuEvtTimeout,					enuNoFrameReading,		this,ATM_TRANS_STR_DET(doTrans2_1Timeout)));
		pAutState->addTransition(new MAutTransition(enuEvtNewKey,					enuSelectingPreamble,	this,ATM_TRANS_STR_DET(doTrans2_2NewKey)));
		pAutState->addTransition(new MAutTransition(enuEvtNoPreambleMatch,			enuNoFrameReading,		this,ATM_TRANS_STR_DET(doTrans2_1NoPreambleMatch)));
		pAutState->addTransition(new MAutTransition(enuEvtPreambleNotComplete,		enuSelectingPreamble,	this,ATM_TRANS_STR_DET(doTrans2_2PreambleNotComplete)));
		pAutState->addTransition(new MAutTransition(enuEvtPreambleComplete,			enuPreambleSelected,	this,ATM_TRANS_STR_DET(doTrans2_3PreambleComplete)));
		this->addAutState(pAutState);
		
	//enuPreambleSelected = 3,
		pAutState = new MAutState(enuPreambleSelected);
		pAutState->addTransition(new MAutTransition(enuEvtTimeout,					enuNoFrameReading,		this,ATM_TRANS_STR_DET(doTrans3_1Timeout)));
		pAutState->addTransition(new MAutTransition(enuEvtNewKey,					enuPreambleSelected,	this,ATM_TRANS_STR_DET(doTrans3_3NewKey)));
		pAutState->addTransition(new MAutTransition(enuEvtPostamble,				enuNoFrameReading,		this,ATM_TRANS_STR_DET(doTrans3_1PostambleMatch)));
		pAutState->addTransition(new MAutTransition(enuEvtNotPostamble,				enuPreambleSelected,	this,ATM_TRANS_STR_DET(doTrans3_3PostambleNotMatch)));
		this->addAutState(pAutState);
		
	//---------------------------------------------------------------------
	// END Automat initialization
	//---------------------------------------------------------------------
	
}

MStringDetectionAtm::~MStringDetectionAtm()
{
	TRACE_D("MStringDetectionAtm::~MStringDetectionAtm(): Deleting MStringDetectionAtm automat!");
	
}


bool MStringDetectionAtm::initialize(MStringDetection *pParent)
{
	m_pParent = pParent;

	return this->test();
}


bool MStringDetectionAtm::reset()
{
	TRACE_D(QString("MStringDetectionAtm::reset ..."));
	m_iCurrentStateID = m_iInitialStateID;

	return true;
}


void MStringDetectionAtm::timeout()
{
	MStrDetAtmEvent *pEvent = new MStrDetAtmEvent(enuEvtTimeout);
	
	this->processEvent(pEvent);
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


bool MStringDetectionAtm::doTrans1_1NewKey(MAutEvent * pEvent)
{
	TRACE_D(QString("MStringDetectionAtm::doTrans1_1NewKey state = %1 event:%2").
							arg(m_iCurrentStateID).
							arg(pEvent->getEventDesc()));

	bool bCompleteMatch;
	int iCountMatch = m_pParent->getCountPreambleMatch(&bCompleteMatch);

	if(iCountMatch>1 || (iCountMatch==1 && !bCompleteMatch))
	{
		MStrDetAtmEvent *pEvent = new MStrDetAtmEvent(enuEvtPreambleNotComplete);
		processEvent(pEvent);
	}
	else if(iCountMatch==1 && bCompleteMatch)
	{
		MStrDetAtmEvent *pEvent = new MStrDetAtmEvent(enuEvtPreambleComplete);
		processEvent(pEvent);
	}
	else 
	{
		MStrDetAtmEvent *pEvent = new MStrDetAtmEvent(enuEvtNoPreambleMatch);
		processEvent(pEvent);
	}


	return true;
}
 
bool MStringDetectionAtm::doTrans1_1NoPreambleMatch(MAutEvent * pEvent)
{
	TRACE_D(QString("MStringDetectionAtm::doTrans1_1NoPreambleMatch state = %1 event:%2").
							arg(m_iCurrentStateID).
							arg(pEvent->getEventDesc()));

	m_pParent->clearInputBuffer();

	return true;
}

 
bool MStringDetectionAtm::doTrans1_2PreambleNotComplete(MAutEvent * pEvent)
{
	TRACE_D(QString("MStringDetectionAtm::doTrans1_2PreambleNotComplete state = %1 event:%2").
							arg(m_iCurrentStateID).
							arg(pEvent->getEventDesc()));

	m_pParent->startTimer();

	return true;
}

 
bool MStringDetectionAtm::doTrans1_3PreambleComplete(MAutEvent * pEvent)
{
	TRACE_D(QString("MStringDetectionAtm::doTrans1_3PreambleComplete state = %1 event:%2").
							arg(m_iCurrentStateID).
							arg(pEvent->getEventDesc()));

	m_pParent->startTimer();

	return true;
}



bool MStringDetectionAtm::doTrans2_1Timeout(MAutEvent * pEvent)
{
	TRACE_D(QString("MStringDetectionAtm::doTrans2_1Timeout state = %1 event:%2").
							arg(m_iCurrentStateID).
							arg(pEvent->getEventDesc()));

	m_pParent->stopTimer();
	m_pParent->clearInputBuffer();

	return true;
}

 
bool MStringDetectionAtm::doTrans2_2NewKey(MAutEvent * pEvent)
{
	TRACE_D(QString("MStringDetectionAtm::doTrans2_2NewKey state = %1 event:%2").
							arg(m_iCurrentStateID).
							arg(pEvent->getEventDesc()));

	doTrans1_1NewKey(pEvent);

	return true;
}

 
bool MStringDetectionAtm::doTrans2_1NoPreambleMatch(MAutEvent * pEvent)
{
	TRACE_D(QString("MStringDetectionAtm::doTrans2_1NoPreambleMatch state = %1 event:%2").
							arg(m_iCurrentStateID).
							arg(pEvent->getEventDesc()));

	m_pParent->stopTimer();
	m_pParent->clearInputBuffer();

	return true;
}

 
bool MStringDetectionAtm::doTrans2_2PreambleNotComplete(MAutEvent * pEvent)
{
	TRACE_D(QString("MStringDetectionAtm::doTrans2_2PreambleNotComplete state = %1 event:%2").
							arg(m_iCurrentStateID).
							arg(pEvent->getEventDesc()));

	m_pParent->stopTimer();
	m_pParent->startTimer();

	return true;
}


bool MStringDetectionAtm::doTrans2_3PreambleComplete(MAutEvent * pEvent)
{
	TRACE_D(QString("MStringDetectionAtm::doTrans2_3PreambleComplete state = %1 event:%2").
							arg(m_iCurrentStateID).
							arg(pEvent->getEventDesc()));

	m_pParent->stopTimer();
	m_pParent->startTimer();

	return true;
}

 

bool MStringDetectionAtm::doTrans3_1Timeout(MAutEvent * pEvent)
{
	TRACE_D(QString("MStringDetectionAtm::doTrans3_1Timeout state = %1 event:%2").
							arg(m_iCurrentStateID).
							arg(pEvent->getEventDesc()));

	m_pParent->stopTimer();
	m_pParent->clearInputBuffer();

	return true;
}


bool MStringDetectionAtm::doTrans3_3NewKey(MAutEvent * pEvent)
{
	TRACE_D(QString("MStringDetectionAtm::doTrans3_3NewKey state = %1 event:%2").
							arg(m_iCurrentStateID).
							arg(pEvent->getEventDesc()));

	bool bInputBufferOK; 
	bool bIsPostambleMatch = m_pParent->isPostambleMatch(&bInputBufferOK);

	if(bInputBufferOK)
	{
		if(bIsPostambleMatch)
		{
			MStrDetAtmEvent *pEvent = new MStrDetAtmEvent(enuEvtPostamble);
			processEvent(pEvent);
		}
		else 
		{
			MStrDetAtmEvent *pEvent = new MStrDetAtmEvent(enuEvtNotPostamble);
			processEvent(pEvent);
		}
	}
	else
	{
		m_pParent->stopTimer();
		m_pParent->clearInputBuffer();
		reset();	
	}

	return true;
}

 
bool MStringDetectionAtm::doTrans3_1PostambleMatch(MAutEvent * pEvent)
{
	TRACE_D(QString("MStringDetectionAtm::doTrans3_1PostambleMatch state = %1 event:%2").
							arg(m_iCurrentStateID).
							arg(pEvent->getEventDesc()));

		
	m_pParent->detectionComplete();
	m_pParent->stopTimer();

	return true;
}

 
bool MStringDetectionAtm::doTrans3_3PostambleNotMatch(MAutEvent * pEvent)
{
	TRACE_D(QString("MStringDetectionAtm::doTrans3_3PostambleNotMatch state = %1 event:%2").
							arg(m_iCurrentStateID).
							arg(pEvent->getEventDesc()));

	m_pParent->stopTimer();
	m_pParent->startTimer();

	return true;
}

 


//------------------------------------------------


