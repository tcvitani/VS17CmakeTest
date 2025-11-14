
#ifndef MSTRING_DETECTION_ATM_H
#define MSTRING_DETECTION_ATM_H

#include "mautomat.h"


#define ATM_TRANS_STR_DET(X) (bool (MAutomat::*)(class MAutEvent *))&MStringDetectionAtm::X

class MStringDetection;

class MStringDetectionAtm:public MAutomat
{

public:
	MStringDetectionAtm();
	virtual ~MStringDetectionAtm();

	bool initialize(MStringDetection *pParent);

	void timeout();
	//---------------------------------------
	//  States
	enum {
		enuNoFrameReading = 1,
		enuSelectingPreamble = 2,
		enuPreambleSelected = 3
	} enumStrDetAtmStateTypes;
	//---------------------------------------
	
	//---------------------------------------
	//  Events
	enum {
		enuEvtTimeout,
		enuEvtNewKey,
		enuEvtNoPreambleMatch,
		enuEvtPreambleComplete,
		enuEvtPreambleNotComplete,
		enuEvtPostamble,
		enuEvtNotPostamble
	} enumStrDetAtmEventTypes;
	//---------------------------------------
	
	virtual bool reset();
	
private: 
	
	//internal helper variables
	bool m_bInitialized;
	MStringDetection *m_pParent;

	//----------------------------------------
	// The automate transitions           
	//------------------------------------------
	bool doTrans1_1NewKey(MAutEvent * pEvent); 
	bool doTrans1_1NoPreambleMatch(MAutEvent * pEvent); 
	bool doTrans1_2PreambleNotComplete(MAutEvent * pEvent); 
	bool doTrans1_3PreambleComplete(MAutEvent * pEvent); 

	bool doTrans2_1Timeout(MAutEvent * pEvent); 
	bool doTrans2_2NewKey(MAutEvent * pEvent); 
	bool doTrans2_1NoPreambleMatch(MAutEvent * pEvent); 
	bool doTrans2_2PreambleNotComplete(MAutEvent * pEvent); 
	bool doTrans2_3PreambleComplete(MAutEvent * pEvent); 

	bool doTrans3_1Timeout(MAutEvent * pEvent); 
	bool doTrans3_3NewKey(MAutEvent * pEvent); 
	bool doTrans3_1PostambleMatch(MAutEvent * pEvent); 
	bool doTrans3_3PostambleNotMatch(MAutEvent * pEvent); 

	//----------------------------------------
	// END automate transitions              
	//------------------------------------------

	
	
};



//---------------------------------------
// Events
//---------------------------------------


class MStrDetAtmEvent: public MAutEvent
{
public:

	MStrDetAtmEvent(){};
	MStrDetAtmEvent(int idEvent);
	~MStrDetAtmEvent();

private:

};



//---------------------------------------
// END Events
//---------------------------------------






#endif
