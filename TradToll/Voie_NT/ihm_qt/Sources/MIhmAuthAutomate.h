
#ifndef MAUTH_AUTOMAT_H
#define MAUTH_AUTOMAT_H

#include "mautomat.h"


#define AUT_TRANS_ACTION_AUTH(X) (bool (MAutomat::*)(class MAutEvent *))&MIHMAuthAutomate::X

class MIhmTCLInterfaces;
class MIhmSessionUserData;

class MIHMAuthAutomate:public MAutomat
{

public:
	MIHMAuthAutomate();
	virtual ~MIHMAuthAutomate();

	bool initialize(MIhmTCLInterfaces * pTCLInterfaces);
	bool isTakeOverInProgress();
	//---------------------------------------
	//  States
	enum {
		enuNoControl = 1,
		enuWaitTakeCtrlRsp = 2,
		enuInControl = 3,
		enuTakeOver = 4
	} enumAuthAutStatesTypes;
	//---------------------------------------
	
	//---------------------------------------
	//  Events
	enum {
			enuEvtDefault,
			enuEvtCommTimeout,
			enuEvtInitTakeOver,
			enuEvtReturnCtrlReq, 
			enuEvtTakeCtrlRspOK,
			enuEvtTakeCtrlRspNOK,
			enuEvtTakeOverTimeout,
			enuEvtTakeOverReject,
			enuEvtNewClientTakeOverCancel,
			enuEvtTakeOverAccept,
			enuEvtForceReturnCtrl,
			evtEvtInvalidUserTakingCtrlStatus
	} enumAuthAutEventTypes;
	//---------------------------------------
	
	virtual bool reset();
	
private: 
	
	//internal helper variables
	bool m_bInitialized;
	
	MIhmTCLInterfaces * m_pTCLInf; 

	

	
	//----------------------------------------
	// The automat fuctions              
	//------------------------------------------
	bool doTrans1_2InitTakeOver(MAutEvent * pEvent); 

	bool doTrans2_2InitTakeOver(MAutEvent * pEvent); 
	bool doTrans2_1TakeCtrlRspNOK(MAutEvent * pEvent); 
	bool doTrans2_3TakeCtrlRspOK(MAutEvent * pEvent); 
	bool doTrans2_1CommTimeout(MAutEvent * pEvent); 
	bool doTrans2_1InvalidUTCtrlStatus(MAutEvent * pEvent); 
	bool doTrans2_1NewClientTakeOverCancel(MAutEvent * pEvent); 
	

	bool doTrans3_4InitTakeOver(MAutEvent * pEvent); 
	bool doTrans3_1ReturnCtrlReq(MAutEvent * pEvent); 
	bool doTrans3_1ForceReturnCtrl(MAutEvent * pEvent);
	bool doTrans3_1InvalidUTCtrlStatus(MAutEvent * pEvent);

	bool doTrans4_3NewClientTakeOverCancel(MAutEvent * pEvent); 
	bool doTrans4_3TakeOverReject(MAutEvent * pEvent); 
	bool doTrans4_2TakeOverTimeout(MAutEvent * pEvent); 
	bool doTrans4_2TakeOverAccept(MAutEvent * pEvent); 
	bool doTrans4_2ReturnCtrlReq(MAutEvent * pEvent);
	bool doTrans4_2ForceReturnCtrl(MAutEvent * pEvent);
	bool doTrans4_4InitTakeOver(MAutEvent * pEvent); 
	bool doTrans4_3InvalidUTCtrlStatus(MAutEvent * pEvent); 
	

	//----------------------------------------
	// END automat fuctions              
	//------------------------------------------
	bool processTakeOver();
	
	
};



//---------------------------------------
// Events
//---------------------------------------


//default event - initial event (do unconditionaly)

class MAuthAtmEvent: public MAutEvent
{
public:

	MAuthAtmEvent(){m_pData = NULL;	m_bHasData = false; m_iReason = 0;};
	MAuthAtmEvent(int idEvent);
	~MAuthAtmEvent();

	virtual MIhmSessionUserData * takeData();
	virtual void setData(MIhmSessionUserData *pData);
	int getReason(){return m_iReason;};
	void setReason(int iReason){m_iReason = iReason;};

	QString getError(){return m_sError;};
	void setError(QString sError){m_sError = sError;};
private:
	 MIhmSessionUserData * m_pData;
	 int m_iReason;
	 QString m_sError;
};



//---------------------------------------
// END Events
//---------------------------------------






#endif
