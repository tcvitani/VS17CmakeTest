#ifndef MRVIDEO_MSG_H
#define MRVIDEO_MSG_H

#include <QObject>


class MRVideoMsg:public QObject
{
    Q_OBJECT
public:
	MRVideoMsg();
	~MRVideoMsg();
	
	enum enuMRVideoMsgObjType{
		enuRVideoMsgDefault = 0,  
		enuRVideoActivateSourceReq,
		enuRVideoReactivateReq,
		enuRVideoDeactivateReq,
		enuRVideoUnfreezeReq,
		enuRVideoFreezeReq,
		enuRVideoFreezeRsp,
		enuRVideoSaveReq,
		enuRVideoSaveRsp
	};

	virtual enuMRVideoMsgObjType const getType() = 0;
	
	QString getMessageDescription();
private:

};

//----------------------------------------------------------
//------ RVIDEO MESSAGES					-------------------
//----------------------------------------------------------

class MRVideoMsgVideoActivateSrc:public MRVideoMsg
{
public:
	virtual enuMRVideoMsgObjType const getType(){ return enuRVideoActivateSourceReq;};
	QString m_sNewHost;
	quint16	m_uiNewPort;
	QString m_sNewSourceId;
};

class MRVideoMsgReactivate:public MRVideoMsg
{
public:
	virtual enuMRVideoMsgObjType const getType(){ return enuRVideoReactivateReq;};
};


class MRVideoMsgDeactivate:public MRVideoMsg
{
public:
	virtual enuMRVideoMsgObjType const getType(){ return enuRVideoDeactivateReq;};
};



class MRVideoMsgVideoFreeze:public MRVideoMsg
{
public:
	virtual enuMRVideoMsgObjType const getType(){ return enuRVideoFreezeReq;};

};

class MRVideoMsgVideoFreezeRsp:public MRVideoMsg
{
public:
	virtual enuMRVideoMsgObjType const getType(){ return enuRVideoFreezeRsp;};
	bool m_bOK;	
};		

class MRVideoMsgVideoUnfreeze:public MRVideoMsg
{
public:
	virtual enuMRVideoMsgObjType const getType(){ return enuRVideoUnfreezeReq;};

};

class MRVideoMsgVideoSave:public MRVideoMsg
{
public:
	virtual enuMRVideoMsgObjType const getType(){ return enuRVideoSaveReq;};

	QString m_sTargetFileName;
	bool m_bSaveJpg; //if false will save BMP
	
};

class MRVideoMsgVideoSaveRsp:public MRVideoMsg
{
public:
	virtual enuMRVideoMsgObjType const getType(){ return enuRVideoSaveRsp;};
	bool m_bOK;	
};
		


#endif


