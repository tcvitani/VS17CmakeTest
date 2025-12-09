#ifndef TLM_DMV_AUTOMAT_H
#define TLM_DMV_AUTOMAT_H

#include "Automat.h"

class TlmWorker;

#define AUT_TRANS_ACTION(X) (bool (Automat::*)(class AutEvent *))&TlmAutomate::X

class TlmAutomate :public Automat
{

public:
	TlmAutomate(short int siInstId, TlmWorker * pWorker);
	virtual ~TlmAutomate();

	QString stateDesc();

	bool initialize();
	//---------------------------------------
	//  States
	enum {
		enuInitial = 0,
		enuConnectedIdle = 1,
		enuWaitingResponse = 2, 
		enuLinkError = 3,
	} enumAutStateType;
	//---------------------------------------

	//---------------------------------------
	//  Events
	enum {
		enuEvtSetDefaultState,
		enuEvtSetImage,
		enuEvtSetMemorizedImage,
		enuEvtSetImageOK,
		enuEvtSetImageNOK,
		enuEvtResponseTimeout,
		enuEvtReceivedAck,
		enuEvtPollTimeout,
		enuEvtLinkOK,
		enuEvtLinkError,
		enuEvtDisconnected,
		enuEvtVoid,
	} enuAutEventTypes;
	//---------------------------------------

	virtual bool reset();
	bool processEvent(AutEvent * event);

private:
	TlmWorker* m_pWorker;

	//----------------------------------------
	// The automate functions              
	//----------------------------------------
	bool doVoid(AutEvent *pEvent);
	bool doReinitSockets(AutEvent *pEvent);
	bool doInitOK(AutEvent *pEvent);
	bool doSendDefaultState(AutEvent *pEvent);
	bool doRetrySendDefaultState(AutEvent *pEvent);
	bool doSetImage(AutEvent *pEvent);
	bool doSetMemorizedImage(AutEvent *pEvent);
	bool doMemorizeSetImage(AutEvent *pEvent);
	bool doRetrySetImage(AutEvent *pEvent);
	bool doImageOK(AutEvent *pEvent);
	bool doSendPollRequest(AutEvent *pEvent);
	bool doLinkError(AutEvent *pEvent);
	bool doLinkOK(AutEvent *pEvent);
	bool doDisconnected(AutEvent *pEvent);
	bool doResetLinkError(AutEvent *pEvent);
	//----------------------------------------
	// END automate functions              
	//------------------------------------------

	//----------------------------------------
	// Help functions
	//------------------------------------------

};

//---------------------------------------
// Events
//---------------------------------------

class TlmAutomateEvent : public AutEvent
{
public:

	TlmAutomateEvent(){ ; };
	TlmAutomateEvent(int idEvent);
	~TlmAutomateEvent();

private:
};

class TlmAutomateEvent_SetImage : public TlmAutomateEvent
{
public:
	TlmAutomateEvent_SetImage(unsigned char ucImage) : TlmAutomateEvent((int)TlmAutomate::enuEvtSetImage), m_ucImage(ucImage){ ; };

	unsigned char m_ucImage;

};

class TlmAutomateEvent_SetImageNOK : public TlmAutomateEvent
{
public:
	TlmAutomateEvent_SetImageNOK(QByteArray &baFixedBytes) : TlmAutomateEvent((int)TlmAutomate::enuEvtSetImageNOK), m_baFixedBytes(baFixedBytes){ ; };

	QByteArray m_baFixedBytes;

};
//---------------------------------------
// END Events
//---------------------------------------






#endif
