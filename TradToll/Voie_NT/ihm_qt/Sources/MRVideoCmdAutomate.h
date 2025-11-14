
#ifndef MAUTH_AUTOMAT_H
#define MAUTH_AUTOMAT_H

#include "mautomat.h"

class MRVideoCmdThread;

#define AUT_TRANS_ACTION_AUTH(X) (bool (MAutomat::*)(class MAutEvent *))&MRVideoCmdAutomate::X

class MRVideoCmdAutomate:public MAutomat
{

public:
	MRVideoCmdAutomate();
	virtual ~MRVideoCmdAutomate();

	bool initialize(MRVideoCmdThread * pRVideoCmdThread);
	bool ifCanProcessNewCommand();
	//---------------------------------------
	//  States
	enum {
		enuNotActivated = 0,
		enuStopped = 1,
		enuPoolingStatusRunning = 2,
		enuRunning = 3, 
		enuGrabbing = 4,
		enuPoolingStatusGrabbing = 5,
		enuGrabbedOK = 6,
		enuPoolingStatusGrabbed = 7,
		enuGettingImage = 8
	} enumVideoCmdAutStateType;
	//---------------------------------------
	
	//---------------------------------------
	//  Events
	enum {
			enuEvtDefault,
			enuEvtErrOrCommTimeout,
			enuEvtPoolTimeout,
			enuEvtACTIVATEReq,
			enuEvtDEACTIVATEReq,
			enuEvtGRABBReq,
			enuEvtSAVEReq,
			enuEvtUNFREEZEReq,
			enuEvtRspOKStopped,
			enuEvtRspOKRunning,
			enuEvtRspOKGrabbing,
			enuEvtRspOKGrabbed,			
			enuEvtRspOKImage,
			enuEvtRspOKInactive,
			enuEvtRspNOK,
			enuEvtSavingImageOK,
			enuEvtSavingImageNOK,
	} enumAuthAutEventTypes;
	//---------------------------------------
	
	virtual bool reset();
	
private: 
	
	//internal helper variables
	bool m_bInitialized;
	MRVideoCmdThread *m_pRVideoCmdThread;
	
	//----------------------------------------
	// The automat fuctions              
	//------------------------------------------
	//------------------------------------------------
	//enuNotActivated = 0,
	//------------------------------------------------
	bool doTrans0_2PoolTimeout(MAutEvent * pEvent);
	bool doTrans0_2ReACTIVATE(MAutEvent * pEvent);
	bool doTrans0_0GRABBReqNOK(MAutEvent * pEvent);
	bool doTrans0_0SAVEReqNOK(MAutEvent * pEvent);
	bool doTrans0_0UNFREEZEReq(MAutEvent * pEvent);
	bool doTrans0_0DEACTIVATEReq(MAutEvent * pEvent);

	//------------------------------------------------
	//enuStopped = 1,
	//------------------------------------------------
	bool doTrans1_2PoolTimeout(MAutEvent * pEvent);
	bool doTrans1_2ReACTIVATE(MAutEvent * pEvent);
	bool doTrans1_1GRABBReqNOK(MAutEvent * pEvent);
	bool doTrans1_1SAVEReqNOK(MAutEvent * pEvent);
	bool doTrans1_1UNFREEZEReq(MAutEvent * pEvent);
	bool doTrans1_0DEACTIVATEReq(MAutEvent * pEvent);
	//------------------------------------------------
	//enuPoolingStatusRunning = 2,
	//------------------------------------------------
	bool doTrans2_1Timeout(MAutEvent * pEvent);
	bool doTrans2_3RspOKRunning(MAutEvent * pEvent);
	bool doTrans2_4RspOKGrabbing(MAutEvent * pEvent);
	bool doTrans2_6RspOKGrabbed(MAutEvent * pEvent);
	bool doTrans2_1RspOKStopped(MAutEvent * pEvent);
	bool doTrans2_1RspNOK(MAutEvent * pEvent);
	bool doTrans2_0DEACTIVATEReq(MAutEvent * pEvent);
	
	//------------------------------------------------
	//enuRunning = 3,
	//------------------------------------------------
	bool doTrans3_2PoolTimeout(MAutEvent * pEvent);
	bool doTrans3_2ReACTIVATE(MAutEvent * pEvent);
	bool doTrans3_4GRABBReq(MAutEvent * pEvent);
	bool doTrans3_3SAVEReqNOK(MAutEvent * pEvent);
	bool doTrans3_3UNFREEZEReq(MAutEvent * pEvent);
	bool doTrans3_0DEACTIVATEReq(MAutEvent * pEvent);

	//------------------------------------------------
	//enuGrabbing = 4,
	//------------------------------------------------
	bool doTrans4_2ReACTIVATE(MAutEvent * pEvent);
	bool doTrans4_5PoolTimeout(MAutEvent * pEvent);
	bool doTrans4_0DEACTIVATEReq(MAutEvent * pEvent);
	bool doTrans4_2UNFREEZEReq(MAutEvent * pEvent);
	bool doTrans4_4GRABBReq(MAutEvent * pEvent);
	//------------------------------------------------
	//enuPoolingStatusGrabbing = 5,
	//------------------------------------------------
	bool doTrans5_1Timeout(MAutEvent * pEvent);
	bool doTrans5_3RspOKRunning(MAutEvent * pEvent);
	bool doTrans5_4RspOKGrabbing(MAutEvent * pEvent);
	bool doTrans5_6RspOKGrabbed(MAutEvent * pEvent);
	bool doTrans5_1RspOKStopped(MAutEvent * pEvent);
	bool doTrans5_6RspOKImage(MAutEvent * pEvent);
	bool doTrans5_1RspNOK(MAutEvent * pEvent);
	bool doTrans5_0DEACTIVATEReq(MAutEvent * pEvent);
	//------------------------------------------------
	//enuGrabbedOK = 6,
	//------------------------------------------------
	bool doTrans6_7PoolTimeout(MAutEvent * pEvent);
	bool doTrans6_2ReACTIVATE(MAutEvent * pEvent);
	bool doTrans6_5GRABBReq(MAutEvent * pEvent);
	bool doTrans6_8SAVEReq(MAutEvent * pEvent);
	bool doTrans6_2UNFREEZEReq(MAutEvent * pEvent);
	bool doTrans6_0DEACTIVATEReq(MAutEvent * pEvent);
	//------------------------------------------------
	//enuPoolingStatusGrabbed = 7,
	//------------------------------------------------
	bool doTrans7_2Timeout(MAutEvent * pEvent);
	bool doTrans7_3RspOKRunning(MAutEvent * pEvent);
	bool doTrans7_3RspOKGrabbing(MAutEvent * pEvent);
	bool doTrans7_6RspOKGrabbed(MAutEvent * pEvent);
	bool doTrans7_6RspOKImage(MAutEvent * pEvent);
	bool doTrans7_1RspOKStopped(MAutEvent * pEvent);
	bool doTrans7_1RspNOK(MAutEvent * pEvent);
	bool doTrans7_0DEACTIVATEReq(MAutEvent * pEvent);
	//------------------------------------------------
	//	enuGettingImage = 8,
	//------------------------------------------------
	bool doTrans8_1Timeout(MAutEvent * pEvent);
	bool doTrans8_3RspOKRunning(MAutEvent * pEvent);
	bool doTrans8_3RspOKGrabbing(MAutEvent * pEvent);
	bool doTrans8_6RspOKGrabbed(MAutEvent * pEvent);
	bool doTrans8_1RspOKStopped(MAutEvent * pEvent);

	bool doTrans8_6RspOKSaveOK(MAutEvent * pEvent);
	bool doTrans8_6RspOKSaveNOK(MAutEvent * pEvent);
	
	bool doTrans8_1RspNOK(MAutEvent * pEvent);
	bool doTrans8_0DEACTIVATEReq(MAutEvent * pEvent);
	//----------------------------------------
	// END automat fuctions              
	//------------------------------------------
	
	void sendFreezeRsp(bool bOK);
	void sendSaveRsp(bool bOK);
	
};



//---------------------------------------
// Events
//---------------------------------------

class MRVideoCmdAtmEvent: public MAutEvent
{
public:

	MRVideoCmdAtmEvent(){;};
	MRVideoCmdAtmEvent(int idEvent);
	~MRVideoCmdAtmEvent();

private:
};



//---------------------------------------
// END Events
//---------------------------------------






#endif
