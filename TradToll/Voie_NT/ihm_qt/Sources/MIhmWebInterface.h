#ifndef MIHM_WEB_INTERFACE_H
#define MIHM_WEB_INTERFACE_H

#include "MIhmConfigWeb.h"
#include "MIhmConfigImages.h"
#include "MIhmLanguages.h"
#include "MClock.h"

#include <QTimer>
#include <QString>
#include <QThread>
#include <QTcpSocket>
#include <QByteArray>
#include <QMutex>


// The class MIhmWebInterface provides web interface for the IHM module
// It starts http server and keep the session object for each client
// There are several types of requests possible:
//   1. first "Login" request  - 
//      - the new session ID key is generated. 
//		  Each following request must contain the received session ID key 
//        or is rejected (the client script should redirect it than to login form)
//   2. "Refresh" request - the client script is pooling the http server 
//		for the visible object updates. For each session object there is a 
//      timestamp  of the last update for each virtual object,
//		which is generated for the moment when the virtual object is changed
//		Each session object must have the complete list of visible objects.      
//      Each visible object for the session has the timestamp for the moment 
//		it was updated by the core application. 
//      The visible object is append to the response if its time stamp value is 
//		greater than the ack timestamp value received by enuReqObjectAck
//   3. enuReqKeyDown - the request must have correct session ID.
//		 It should also contain the key stroke entered by the user
//   TO DO .. other types of request ...
//        string detection,
//       or input dialog result

class MIhmHttpServer;
class MClientSocket;
class MIhmVirtualObject;
class MIhmHttpRequest;
class MIhmHttpResponse;
class MIhmWebSession;
class MIhmVirtDynConfig;
class MIhmTCLInterfaces;
class MIhmMsg;
class MIhmMsgVideo;
class MIhmMsgOpenLinkReq;
class MIhmVirtObjectsModel;
class MIhmWebVisibleObject;
class MIhmWebCommonObjLogic;
class MIhmMiniWebSession;

class MIhmWebInterface:public QThread
{
    Q_OBJECT
public:
    MIhmWebInterface(QObject * parent = 0);
    ~MIhmWebInterface();

	enum enumWebStatus {
		enuNotInitialized,
		enuInitializing,
		enuInitialized
	};

	enumWebStatus getStatus(){return m_eStatus;};

	//send config key to read the web general parameters
	//initialize the server
    bool initialize(MIhmTCLInterfaces *pTCLIntf, QString sModuleConfigKey); 
	void run();	

	QString getMainPage();
	QString getMiniWebPage();
	
	MIhmWebSession * getSessionInControl(){return m_pSessionInContol;};

	MIhmLaneTypeSettings * getCurrentLaneTypeSettings();
	QString getVisObjHtmlTemplate(QString sObjID);
	int getNumInitializedSessions();
	

signals:
	void initOK();
	void authorisationReq(QString sUserId, QString sPwd, bool bTakeControl, QString sSessionID);
	void endIdentificationReq(QString sWebSessionID, int iReason);
	void takeControlReq(QString sWebSessionID);
	void returnControlReq(QString sWebSessionID, int iReason);
	
	void takeOverCancel(QString sWebSessionID);
	void takeOverAccept(QString sWebSessionID);
	void takeOverReject(QString sWebSessionID);

	void keyDetected(QString sWebSessionID, int iKeyCode);
	void action(QString sWebSession, QString sAction, QString sParams);

	void webInputDlgsRsp(QString sWebSessionID, MIhmMsg *pMsg);

	void privateUpdateObjectsNeeded();

public slots:
	void onAuthorisationResult(QString sWebSessionID, bool bOK, QString sError);
	void onForcedUnIdentification(QString sWebSessionID);
	void onWebUserInControl(QString sWebSessionID);
				
	void onVirtualObjectsUpdated();
	void onInitTakeOver(bool bQuest, QString sWebSessionId, QString sName);
	void onCancelTakeOver(QString);

	void onShowClientInfoMsg(QString sWebSessionId, QString sMsg, QString sMsgTitle);
	void onHideClientInfoMsg(QString sWebSessionId);

	void onShowConnectingMsg(QString sWebSessionId, QString sMsg, QString sMsgTitle);
	void onHideConnectingMsg(QString sWebSessionId);

	void onOpenLinkReq(MIhmMsgOpenLinkReq * pReq);
	void onReloadWebDynConfig();
	
private slots:
	void onClientDisconnected();
	void onReceivedData(const QByteArray &);
	void onSocketError(QAbstractSocket::SocketError);
	void onIncommingConnection(int);

	void onDeadSessionsCheckTimer();
	void onDateChanged();

	void onInputDlgsMsg(MIhmMsg *pMsg);
	void onPrivateUpdateObjects();

private:
	int rowOfClient(MClientSocket *client) const;
	bool isClientValid(MClientSocket *pClient) const;

	bool sendResponse(MClientSocket *pClient, MIhmHttpResponse *pResponse);

	void processLoginReq(MIhmHttpRequest *pHttpRequest,MClientSocket *pClient);
	void processRefreshReq(MIhmHttpRequest *pHttpRequest,
												MClientSocket *pClient,
												bool bOnlyUpdated = false);

	void processReqMiniWebRefresh(MIhmHttpRequest *pHttpRequest,
												MClientSocket *pClient,
												bool bOnlyUpdated = false);

	void processRefreshAckReq(MIhmHttpRequest *pHttpRequest, MClientSocket *pClient);
	void processKeyDownReq(MIhmHttpRequest *pHttpRequest, MClientSocket *pClient);
	void processDialogResponseReq(MIhmHttpRequest *pHttpRequest, MClientSocket *pClient);
	void processClickCommandReq(MIhmHttpRequest *pHttpRequest, MClientSocket *pClient);

	void processCommentsUpdate(MIhmHttpRequest *pHttpRequest, MClientSocket *pClient);
	bool verifyAndPrepareCommentsUpdate(QString &sData);

	void processTakeControlReq(MIhmHttpRequest *pHttpRequest, MClientSocket *pClient);
	void processReturnControlReq(MIhmHttpRequest *pHttpRequest, MClientSocket *pClient);
	void processGetDynImagePathReq(MIhmHttpRequest *pHttpRequest, MClientSocket *pClient);

	void processVirtConfigLanguage(MIhmVirtDynConfig* pVirtObj);
	void processVirtualObject(MIhmVirtualObject *pVirtObj);
	void refreshAllVirtualObjects();
	void refreshSessionObjects(MIhmWebSession * pSession);

	void initSessionVisibleObjects(MIhmWebSession *pSession);

	void setSessionInControl(MIhmWebSession * pNewCtrlSession);


	
	//to be able to read updated virtual objects...
	MIhmTCLInterfaces *m_pTCLInterface;


	enumWebStatus m_eStatus;
	bool m_bRefreshAllObjects;
	QString m_sModuleConfigKey;

	MIhmConfigWeb m_oWebConfig;

	MIhmHttpServer *m_pHttpServer;

	QList <MClientSocket *> m_lstClientSockets;

	//Web session handling ...
	void verifySessionTimeouts();
	void deleteSession(MIhmWebSession * pTargetSession, int iReason, bool bSendEndIdentif = true);

	QList <MIhmWebSession *> m_lstSessions;
	MIhmWebSession * m_pSessionInContol;


	void verifyMiniWebSessionTimeouts();
	QList <MIhmMiniWebSession *> m_lstMiniWebSessions;

	void signalUpdateNeeded(); 
	int m_iPrivateUpdateEmmited; //number of privateUpdateObjectsNeeded in the queue

	bool m_bThreadStarted;

	//used to periodically check for dead sessions
	QTimer m_tmDeadSessionsCheck;


	MClock m_dateCheck;
	
	MIhmVirtObjectsModel *m_pWEBVirtObjectsModel;

	bool ifShouldProcessInCommonObjects(MIhmVirtualObject * pVirtObj);
	bool ifShouldProcessInSessionObjects(MIhmVirtualObject * pVirtObj);
	MIhmWebCommonObjLogic * m_pCommonVisibleObjLogic;

};


#endif
