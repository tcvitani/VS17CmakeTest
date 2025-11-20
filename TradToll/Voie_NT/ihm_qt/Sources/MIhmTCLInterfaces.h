#ifndef MIHM_TCL_INTERFACES_H
#define MIHM_TCL_INTERFACES_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QTime>
#include <QMutex>
#include <QList>
#include "MIhmMsg.h"



class MIhmDesktopIntf;
class MIhmWebInterface;
class MIhmMainLogic;
class MIhmVirtualObject;
class MIhmSessionUserData;
class MIHMAuthAutomate;
class MIhmMsgVideo;
class MRVideoControlObject;
class MRVideoMsg;
class MIhmVirtObjectsModel;

class MIhmTCLInterfaces:public QObject
{
    Q_OBJECT
public:
	MIhmTCLInterfaces();
	~MIhmTCLInterfaces();

	bool initialize(MIhmMainLogic *pParent);
	void displayInitInfo(QString s);
	void updateVirtualObjects(MIhmVirtObjectsModel * pGUIModel);
	void processMessageFromAni(MIhmMsg* pMsg);

	//for the web to get updated objects
	void getUpdatedVirtualObjectsForWeb(MIhmVirtObjectsModel * pGUIModel);

public slots:
	void close();
	
private slots:
	void onDskKeyDetected(int);
	void onDskStringDetected(MIhmMsgStringDetection *);
	void onDskInputDialogResult(MIhmMsg * pMsg);
	void onDskAsyncInputRsp(MIhmMsg * pMsg);
	void onDskVideoCtlRsp(MIhmMsg * pMsg);
	void onDskMessageToAni(MIhmMsg * pMsg);
	

	void onDskAction(QString sAction, QString sParams);
	void onDskAboutDlgAction(QString sAction, QString sParams);

	void onDskIdentificationReq(QString sName, QString sSecCode, bool bLDAPOffline);
	void onDskReturnCtrlReq(QString sDskInternalSessionID, int iReason);
	void onDskEndIdentificationReq(QString sDskInternalSessionID, int iReason);
	void onDskAuthDlgCanceled(int reason);
	void onDskTakeCtrlReq(QString sDskInternalSessionID);

	void onDskTakeOverReject();
	void onDskTakeOverCancel();
	void onDskTakeOverAccept();

	void onWebAction(QString sWebSession, QString sAction, QString sParams);
	void onWebKeyDetected(QString sWebSession, int iKeyCode);

	//Function used to send messages from in control user inputs
	// from:ASYNC INPUT,INPUT DIALOG,INPUT DIALOG EX  to ANI thread. 
	void onWebInputDlgsMessage(QString sWebSession, MIhmMsg * pMsg);
	
	void onWebInitOK();
	void onWebAuthorisationReq(QString sUserId, QString sPwd, bool bTakeControl, QString sWebSessionId);
	void onWebReturnControlReq(QString sWebSessionId, int iReason);
	void onWebEndIdentificationReq(QString sWebSessionId, int iReason);
	void onWebTakeControlReq(QString sWebSessionId);

	void onWebTakeOverCancel(QString sWebSessionId);
	void onWebTakeOverAccept(QString sWebSessionId);
	void onWebTakeOverReject(QString sWebSessionId);
	
	//Slots to handle automate timers
	void onCommTimeout();
	void onTakeOverTimeout();

	void onRVideoResponseMessage(MRVideoMsg*pMsg, int eSourceVirtObjet);

signals:
	void exitProgram();

	void updateVirtObjDesktop(); //used for the desktop to update virtual objects
	void updateVirtObjWeb(); //used for the web to update virtual objects 

	void initOK();
	void authorizationRsp(QString sWebSessionId, bool bOK, QString sErr);
	void forceUnIdentification(QString);
	void webUserInControl(QString);

	void initWebTakeOver(bool bQuest, QString sWebSessionId, QString sName);
	void cancelWebTakeOver(QString);

	void showClientInfoMsg(QString sWebSessionId, QString sMsg, QString sMsgTitle);
	void hideClientInfoMsg(QString sWebSessionId);

	void showConnectingMsg(QString sWebSessionId, QString sMsg, QString sMsgTitle);
	void hideConnectingMsg(QString sWebSessionId);
	
	void reloadWebDynConfig();

	void openLinkReq(MIhmMsgOpenLinkReq * pReq);
	
private:
	void executeWebAction(QString sAction, QString sParams);
	void executeDskAction(QString sAction, QString sParams);
	void execKeyPressed(int iAskedIhmKeyCode);
	void execCommandReq(QString sCommand, QString sCommandParam);
	
	void processAuthorisationRsp(MIhmMsgAuthRsp *);
	void processTakeControlRsp(MIhmMsgTakeCtrlRsp *);
	void processReturnControlRsp(MIhmMsgReturnCtrlRsp *);

	void processOpenLoginDlgReq(MIhmMsgOpenLoginDlgReq * pReq);

	void processOpenLinkReq(MIhmMsgOpenLinkReq * pReq);

	void processOpenAboutDlgReq(MIhmMsgOpenAboutDlgReq * pReq);
	void processCloseAboutDlgReq(MIhmMsgCloseAboutDlgReq * pReq);

	void processVideoMsgReq(MIhmMsgVideo* pReq);
	bool processRVideoMsg(MIhmMsgVideo* pReq); 

	void processReloadDynConfig();

	void sendMessageToAni(MIhmMsg * pMsg);

	bool isDskUserInControl();
	bool isWebUserInControl(QString sWebSessionId);

	void webSendWebAuthRsp(QString sWebSessionID, bool bAuthorized, QString sMsg);
	void dskDisplayClientInfoMsg(QString sMsg, QString sMsgTitle);	
	
	MIhmMainLogic * m_pMainLogic; 
	
	//desktop interface - must exist otherwise the process should end
	MIhmDesktopIntf * m_pDesktopIntf;
	
	//web interface - optionally can exist 
	MIhmWebInterface * m_pWebIntf;

	bool m_bDskInitOK;

	const MIhmSessionUserData * getUserInControl() const{return m_pUserInControl;};
	void setUserInControl(MIhmSessionUserData *);
	void updateInControlStatus();

	void setTakeOverInProgress();
	void cancelTakeOverInProgress();

	void showConnectingInProgress();
	void hideConnectingInProgress();

	void informRequestingUserRejected();
	void informRequestingUserFailure(QString sMsg);
	void removeIHMSession(MIhmSessionUserData * pReq);


	MIhmSessionUserData * m_pUserInControl;

	void addTakeCtrlReq(MIhmSessionUserData * pReq);
	void setUserTakingControl(MIhmSessionUserData * pUserTakingCtrl);
	MIhmSessionUserData * getUserTakingControl(){return m_pUserTakingCtrl;};
	MIhmSessionUserData * m_pUserTakingCtrl;
	
	QList <MIhmSessionUserData*> m_lstAuthRequestQueue;
	QList <MIhmSessionUserData*> m_lstIdentifiedUsers;
	
	void startCommTimer();
	void stopCommTimer();
	void startTakeOverTimer();
	void stopTakeOverTimer();
	
	QTimer m_tmCommTimer;
	bool m_bCommStarted;

	QTimer m_tmTakeOverTimer;
	bool m_bTakeOverStarted;

	MIHMAuthAutomate * m_pAuthAutomate; 

	friend class MIHMAuthAutomate;


	//------------------------------------------------------
	//list of objects used for restream video interface that is used 
	// to host threads that use HTTP commands to control the MJPEG stream 
	// that is to be received from restream_server and rendered to the clients using
	// visible objects :MRVideoView(using AxAxis) for desktop and AxAxis for WEB
	QList <MRVideoControlObject*> m_lstRVideoCtrl;
	
	void InitRVideoControlObjects();
	MRVideoControlObject* getRVideoControlObject(int eTargetVirtObjet);

	//------------------------------------
	//Input frequency limitation - should be adjusted depending on system performances
	bool IsInputBufferOverflow();
	QTime m_dtLastInput;
	int m_iNumMsgsInLastSecond;
	
	QMutex m_WebMutex;
	MIhmVirtObjectsModel *m_pWEBVirtObjectModel;

};




#endif


