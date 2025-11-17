#ifndef MKyb_MSG_H
#define MKyb_MSG_H

#include <QObject>


extern "C" {
	#include <virt_kyb.h>
}

class MKybMsg:public QObject
{
    Q_OBJECT
public:
	MKybMsg();
	~MKybMsg();
	
	enum enuMKybMsgObjType{
		enuMsgDefault = 0,  
		enuMsgKybConfig,  
		enuMsgKybShow,  
		enuMsgKybMove  
	};

	virtual enuMKybMsgObjType const getType() = 0;
private:

};

class MKybMsgKybConfig:public MKybMsg
{
public:
	virtual enuMKybMsgObjType const getType(){ return enuMsgKybConfig;};

	QString m_sKyboardTypeID;
};

class MKybMsgKybShow:public MKybMsg
{
public:
	virtual enuMKybMsgObjType const getType(){ return enuMsgKybShow;};
	
	bool m_bShow;
};


class MKybMsgKybMove:public MKybMsg
{
public:
	virtual enuMKybMsgObjType const getType(){ return enuMsgKybMove;};
	DWORD   dwPosX,dwPosY;
	DWORD   dwHeight,dwWidth;

};


#endif


