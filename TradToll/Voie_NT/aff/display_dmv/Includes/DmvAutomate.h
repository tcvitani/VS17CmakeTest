#ifndef DMV_AUTOMATE_H
#define DMV_AUTOMATE_H

#ifdef __cplusplus

extern "C"
{
#endif
#include <tlm_dmv.h>
#include <fic_gere.h>
#include <aff_ext.h>
#include <aff_util.h>
#include <csr_tlm.h>

#ifdef __cplusplus
}
#endif

#include "Automat.h"
#include <QList>

class DmvWorker;

#define AUT_TRANS_ACTION(X) (bool (Automat::*)(class AutEvent *))&DmvAutomate::X

class DmvAutomate :public Automat
{

public:
	DmvAutomate(short int siInstId, DmvWorker * pWorker);
	virtual ~DmvAutomate();

	QString stateDesc();
	QString getNextStateName(AutEvent * event);

	bool initialize();
	//---------------------------------------
	//  States
	enum _enumAutStateType {
		enuInitial = 0,
		enuActivation = 1,
		enuWaitActivateRsp = 2,
		enuConnectedIdle = 3,
		enuWaitingResponse = 4, 
		enuLinkError = 5,
	} ;
	//---------------------------------------

	//---------------------------------------
	//  Events
	enum _enuAutEventTypes {
		enuEvtConnected,
		enuEvtDisconnected,
		enuEvtLinkError,
		enuEvtRspOK,
		enuEvtRspError,
		enuEvtRspTimeout,
		enuEvtPollTimeout,
		enuEvtActivationReq,
		enuEvtDisplayReq,
		enuEvtTrafficLightReq,
		enuEvtSetFlasherStateReq,
		enuEvtConfirmFlasherStateReq,
	} ;
	//---------------------------------------

	virtual bool reset();
	bool processEvent(AutEvent * event);

private:
	DmvWorker* m_pWorker;
	void StartSavedRequests();
	QList<AutEvent *> m_lstReq;

	//----------------------------------------
	// The automate functions              
	//----------------------------------------
	bool doVoid(AutEvent *pEvent);
	bool doReinitSockets(AutEvent *pEvent);
	bool doConnectionOK(AutEvent *pEvent);
	bool doActivationRequest(AutEvent *pEvent);
	bool doNewDisplayRequest(AutEvent *pEvent);
	bool doNewTrafficLightRequest(AutEvent *pEvent);
	bool doNewFlasherStateRequest(AutEvent *pEvent);
	bool doConfirmFlasherStateRequest(AutEvent *pEvent);
	bool doSaveForLater(AutEvent *pEvent);
	bool doResponseTimeout(AutEvent *pEvent);
	bool doCommandOK(AutEvent *pEvent);
	bool doSendPollRequest(AutEvent *pEvent);
	bool doLinkError(AutEvent *pEvent);
	bool doLinkOK(AutEvent *pEvent);
	bool doDisconnected(AutEvent *pEvent);
	bool doResetLinkError(AutEvent *pEvent);
	bool doResetLinkErrorInit(AutEvent *pEvent);
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

class DmvAutomateEvent : public AutEvent
{
public:

	DmvAutomateEvent(){ ; };
	DmvAutomateEvent(int idEvent);
	~DmvAutomateEvent();

private:
};

class NewDisplayEvent : public DmvAutomateEvent
{
public:
	NewDisplayEvent(struct_screen sScreen, int iPriority) : DmvAutomateEvent((int)DmvAutomate::enuEvtDisplayReq),
																			 m_sScreen(sScreen),
																			 m_iPriority(iPriority){}
	
	struct_screen m_sScreen;
	int m_iPriority;
};

class NewTrafficLightEvent : public DmvAutomateEvent
{
public:
	NewTrafficLightEvent(enum_tlm_dmv_images eTlmImage) : DmvAutomateEvent((int)DmvAutomate::enuEvtTrafficLightReq),
																		   m_eTlmImage(eTlmImage){}
	
	enum_tlm_dmv_images m_eTlmImage;
};

class NewFlasherStateEvent : public DmvAutomateEvent
{
public:
	NewFlasherStateEvent(enum_flasher_state eFlasherState) : DmvAutomateEvent((int)DmvAutomate::enuEvtSetFlasherStateReq),
																			   m_eFlasherState(eFlasherState){}

	enum_flasher_state m_eFlasherState;
};
//---------------------------------------
// END Events
//---------------------------------------






#endif
