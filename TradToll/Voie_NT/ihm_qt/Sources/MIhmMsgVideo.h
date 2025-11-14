#ifndef MIHM_MSG_VIDEO_H
#define MIHM_MSG_VIDEO_H

#include "MIhmMsg.h"


//----------------------------------------------------------
//------ VIDEO MESSAGES					-------------------
//----------------------------------------------------------
// typedef enum
// {
// 	IHM_VIDEO_ON=1,
// 	IHM_VIDEO_OFF,
// 	IHM_VIDEO_FREEZE,
// 	IHM_VIDEO_UNFREEZE,
// 	IHM_VIDEO_ZOOM_IN,
// 	IHM_VIDEO_ZOOM_OUT,
// 	IHM_VIDEO_ZOOM,
// 	IHM_VIDEO_SELECT_SRC,
// 	IHM_VIDEO_SAVE_BMP,
// 	IHM_VIDEO_SAVE_JPG,
// 
// 	IHM_VIDEO_SAVE_OK,
// 	IHM_VIDEO_SAVE_NOK,
// 	IHM_VIDEO_FREEZE_OK,
// 	IHM_VIDEO_FREEZE_NOK
// }
// enum_ihm_video_cmd;
// 
// typedef struct
// {
//     enum_ihm_video_cmd      eCmd;
// 	union
// 	{
// 		char                szValeur[IHM_LG_CHAINES_SRV];
// 		DWORD               dwValeur;
// 	} cmdval;
// }
// struct_ihm_video_affichage;

class MIhmMsgVideo:public MIhmMsg
{
public:
	enum enumIhmVideoMsgType{
		enuIhmMsgVideoUnknown = 0,
		enuIhmMsgVideoSelectSourceReq,
		enuIhmMsgVideoOnReq,
		enuIhmMsgVideoOffReq,
		enuIhmMsgVideoZoomReq,
		enuIhmMsgVideoFreezeReq,
		enuIhmMsgVideoUnfreezeReq,
		enuIhmMsgVideoSaveReq,
		enuIhmMsgVideoSaveRsp,
		enuIhmMsgVideoFreezeRsp,
		enuIhmMsgRVideoUpdateSrc,
		enuIhmMsgRVideoUpdateOnOff
	};

	MIhmMsgVideo();
	MIhmMsgVideo(int eTargetVirtObj);

	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgVideo;};
	virtual enumIhmVideoMsgType const getVideoMsgType() = 0;
	
	int getTargetVirtObject(){return m_eTargetVirtObject;};

private:
	int m_eTargetVirtObject;
};

class MIhmMsgVideoSelectSrc:public MIhmMsgVideo
{
public:
	MIhmMsgVideoSelectSrc(int eTargetVirtObj): 
				MIhmMsgVideo(eTargetVirtObj){};

	virtual enumIhmVideoMsgType const getVideoMsgType(){ return enuIhmMsgVideoSelectSourceReq;};
	int m_iNewSource;
};

class MIhmMsgVideoOn:public MIhmMsgVideo
{
public:
	MIhmMsgVideoOn(int eTargetVirtObj): 
				MIhmMsgVideo(eTargetVirtObj){};

	virtual enumIhmVideoMsgType const getVideoMsgType(){ return enuIhmMsgVideoOnReq;};
};

class MIhmMsgVideoOff:public MIhmMsgVideo
{
public:
	MIhmMsgVideoOff(int eTargetVirtObj): 
				MIhmMsgVideo(eTargetVirtObj){};

	virtual enumIhmVideoMsgType const getVideoMsgType(){ return enuIhmMsgVideoOffReq;};
};



class MIhmMsgVideoZoom:public MIhmMsgVideo
{
public:
	MIhmMsgVideoZoom(int eTargetVirtObj): 
				MIhmMsgVideo(eTargetVirtObj){};

	virtual enumIhmVideoMsgType const getVideoMsgType(){ return enuIhmMsgVideoZoomReq;};
	int m_iNewZoom;
};


class MIhmMsgVideoFreeze:public MIhmMsgVideo
{
public:
	MIhmMsgVideoFreeze(int eTargetVirtObj): 
				MIhmMsgVideo(eTargetVirtObj){};

	virtual enumIhmVideoMsgType const getVideoMsgType(){ return enuIhmMsgVideoFreezeReq;};
};

class MIhmMsgVideoFreezeRsp:public MIhmMsgVideo
{
public:
	MIhmMsgVideoFreezeRsp(int eTargetVirtObj): 
				MIhmMsgVideo(eTargetVirtObj){};

	virtual enumIhmVideoMsgType const getVideoMsgType(){ return enuIhmMsgVideoFreezeRsp;};
	int m_iResult;	
};		

class MIhmMsgVideoUnfreeze:public MIhmMsgVideo
{
public:
	MIhmMsgVideoUnfreeze(int eTargetVirtObj): 
				MIhmMsgVideo(eTargetVirtObj){};

	virtual enumIhmVideoMsgType const getVideoMsgType(){ return enuIhmMsgVideoUnfreezeReq;};
};

class MIhmMsgVideoSave:public MIhmMsgVideo
{
public:
	MIhmMsgVideoSave(int eTargetVirtObj): 
				MIhmMsgVideo(eTargetVirtObj){};

	virtual enumIhmVideoMsgType const getVideoMsgType(){ return enuIhmMsgVideoSaveReq;};

	QString m_sTargetFileName;
	bool m_bSaveJpg; //if false will save BMP
	
};

class MIhmMsgVideoSaveRsp:public MIhmMsgVideo
{
public:
	MIhmMsgVideoSaveRsp(int eTargetVirtObj): 
				MIhmMsgVideo(eTargetVirtObj){};

	virtual enumIhmVideoMsgType const getVideoMsgType(){ return enuIhmMsgVideoSaveRsp;};

	int m_iResult;	
};
		


class MIhmMsgRVideoUpdateSrc:public MIhmMsgVideo
{
public:
	MIhmMsgRVideoUpdateSrc(int eTargetVirtObj): 
				MIhmMsgVideo(eTargetVirtObj){};

	virtual enumIhmVideoMsgType const getVideoMsgType(){ return enuIhmMsgRVideoUpdateSrc;};

	QString m_sStreamSourceUrlLocal;
	QString m_sStreamSourceUrlRemote;

	QString m_sBaseUrl;
};

class MIhmMsgRVideoUpdateOnOff:public MIhmMsgVideo
{
public:
	MIhmMsgRVideoUpdateOnOff(int eTargetVirtObj): 
				MIhmMsgVideo(eTargetVirtObj){};

	virtual enumIhmVideoMsgType const getVideoMsgType(){ return enuIhmMsgRVideoUpdateOnOff;};

	bool m_bVideoOn;
};
#endif


