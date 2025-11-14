
#ifndef MIHM_MSG_H
#define MIHM_MSG_H

#include <QObject>
#include "MInputDialogReq.h"
#include "MInputDialogExReq.h"
#include "MIhmSessionUserData.h"



extern "C" {
	#include <ihm.h>
}

class MIhmMsg:public QObject
{
    Q_OBJECT
public:
	MIhmMsg();
	~MIhmMsg();
	
	enum enumIhmMsgObjType{
		enuIhmMsgDefault = 0,  
		enuIhmMsgKeyAndCmdReq,  
		enuIhmMsgStringDetection,
		enuIhmMsgInputDialogRes,
		enuIhmMsgInputDialogInitErr,
		enuIhmMsgInputDlgCanceled,
		enuIhmMsgInputDlgExRes,
		enuIhmMsgInputDlgExInitErr,
		enuIhmMsgInputDlgExCanceled,
		enuIhmMsgInputDlgExOpened,		
		enuIhmMsgLoginDlgOpened,
		enuIhmMsgLoginDlgClosed,
		enuIhmMsgAuthReq,
		enuIhmMsgAuthRsp,
		enuIhmMsgAuthLogoffReq,
		enuIhmMsgAuthDlgCanceled,
		enuIhmMsgTakeCtrlReq,
		enuIhmMsgTakeCtrlRsp,
		enuIhmMsgReturnCtrlReq,
		enuIhmMsgReturnCtrlRsp,
		enuIhmMsgOpenLinkReq,
		enuIhmMsgOpenLoginDlgReq,
		enuIhmMsgAsyncInputRsp,
		enuIhmMsgReloadDynConfigReq,
		enuIhmMsgVideo,
		enuIhmMsgOpenAboutDlgReq,
		enuIhmMsgCloseAboutDlgReq
	};

	virtual enumIhmMsgObjType const getType() = 0;
private:

};


class MIhmMsgReloadDynConfigReq:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgReloadDynConfigReq;};

};


// typedef enum
// {
//     enuIHM_TOUCHE_Key = 0,
//     enuIHM_TOUCHE_Command = 1
// }
// enum_ihm_touche_type;
// 
// 
// typedef struct
// {
//     char    nomgroupe[IHM_LG_CHAINES_SRV];
//     int     touche;
//     BYTE    etat;
// 	enum_ihm_touche_type type;	
// 	char    command[IHM_LG_CHAINES_SRV];
//	char    command_param[IHM_LG_CHAINES_SRV];
// }
// struct_ihm_srv_touches;

class MIhmMsgKeyAndCmdReq:public MIhmMsg
{
public:
	MIhmMsgKeyAndCmdReq():m_iKeyPressed(0){};

	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgKeyAndCmdReq;};

	int m_iKeyPressed;
	QString m_sCommand;
	QString m_sCommandParam;
	enum_ihm_touche_type m_type;

};


// Service Detection de chaine
// typedef struct
// {   
// 	BYTE	ChaineComplete;		// indique si la chaine transmise est complete ou coupee par timeout
//     BYTE	EnvoiePrePostambule;	// Dword Indiquant si les pre et post ambuyle sont renvoyes a l'application.
// 	BYTE    CoupleId[IHM_LG_CHAINES_SRV];	 // identifiant du couple pre/postambul (a compare avec la registerie
// 	BYTE	ChaineSaisie[IHM_LG_SAISIE_MAX];	// chaine saisie en cas de retour
// }
// struct_ihm_srv_detection;

class MIhmMsgStringDetection:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgStringDetection;};

	bool bStringComplete;
	bool bEnvoiePrePostambule;
	QString sCoupleId;
	QString sDetectedString;
};



// typedef struct 
// { 
//     BYTE    boutons;
//     DWORD   b_saisie;		// tmo 27/11/2000 BOOL devient DWORD
//     BYTE    string[IHM_LG_SAISIE_MAX];
// } 
// struct_ihm_srv_saisie_visu;

class MIhmMsgInputDialogRes:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgInputDialogRes;};

	MInputDialogReq oInputReq;
	bool bCanceled;
	QString sInputResult;

};







//in cases when the input dialog request has wrong parameters
// this message is used to remove the request from the virtual  
// object MIhmVirtInputDialogReq request list
class MIhmMsgInputDialogInitErr:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgInputDialogInitErr;};

	MInputDialogReq oInputReq;
};


class MIhmMsgInputDlgCanceled:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgInputDlgCanceled;};

};



class MIhmMsgInputDlgExRes:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgInputDlgExRes;};

	MInputDialogExReq oInputReq;
	bool bCanceled;
	QString sInputResult;

};

//in cases when the input dialog ex request has wrong parameters
// this message is used to remove the request from the virtual  
// object MIhmVirtInputDialogExReq request list
class MIhmMsgInputDlgExInitErr:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgInputDlgExInitErr;};

	MInputDialogExReq oInputReq;
};


class MIhmMsgInputDlgExCanceled:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgInputDlgExCanceled;};

};

class MIhmMsgInputDlgExOpened:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgInputDlgExOpened;};
	MInputDialogExReq oInputReq;
	
};


class MIhmMsgLoginDlgOpened:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgLoginDlgOpened;};
	
};


class MIhmMsgLoginDlgClosed:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgLoginDlgClosed;};
	
};


//---------------------------------------------
//Authorization and take control messages ...
//---------------------------------------------

class MIhmMsgAuthReq:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgAuthReq;};

	MIhmSessionUserData m_usrData;
};

class MIhmMsgAuthRsp:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgAuthRsp;};

	enum_ihm_auth_ret_val	m_iAuthRsp;
	QString m_sError;
	MIhmSessionUserData m_usrData;

};

//End identification request
class MIhmMsgAuthLogOffReq:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgAuthLogoffReq;};

	MIhmSessionUserData m_usrData;
	enum_ihm_auth_ret_ctrl_reason	reason;
};

//Identification dialog canceled
class MIhmMsgAuthDlgCanceled:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgAuthDlgCanceled;};

	MIhmSessionUserData m_usrData;
	enum_ihm_auth_ret_ctrl_reason	reason;
};



class MIhmMsgTakeCtrlReq:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgTakeCtrlReq;};

	MIhmSessionUserData m_usrData;
};

class MIhmMsgTakeCtrlRsp:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgTakeCtrlRsp;};

	enum_ihm_auth_ret_val	m_iAuthRsp;
	QString m_sError;

	MIhmSessionUserData m_usrData;
};

class MIhmMsgReturnCtrlReq:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgReturnCtrlReq;};

	MIhmSessionUserData m_usrData;
	int m_iReason;

};

class MIhmMsgReturnCtrlRsp:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgReturnCtrlRsp;};

	enum_ihm_auth_ret_val	m_iAuthRsp;
	MIhmSessionUserData m_usrData;
};


class MIhmMsgOpenAboutDlgReq:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgOpenAboutDlgReq;};
	
	MIhmMsgOpenAboutDlgReq& operator=(const MIhmMsgOpenAboutDlgReq& right);
	
	QString m_sAboutDlgCnf;
};

class MIhmMsgCloseAboutDlgReq:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgCloseAboutDlgReq;};
	
	MIhmMsgCloseAboutDlgReq& operator=(const MIhmMsgCloseAboutDlgReq& right);
	
};


class MIhmMsgOpenLinkReq:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgOpenLinkReq;};

	MIhmMsgOpenLinkReq& operator=(const MIhmMsgOpenLinkReq& right);

	int m_iTargetWin;
	QString m_sUrl;
	bool m_bVisible;
};

class MIhmMsgOpenLoginDlgReq:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgOpenLoginDlgReq;};

	QString m_sLoginId;
	bool m_bManualLogin;
	bool m_bLDAPOffline;
	bool m_bShow; 
};

class MIhmMsgAsyncInputRsp:public MIhmMsg
{
public:
	virtual enumIhmMsgObjType const getType(){ return enuIhmMsgAsyncInputRsp;};

	QString m_sEncStrValues;
};


#endif


