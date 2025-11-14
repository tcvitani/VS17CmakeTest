#ifndef MIHM_WEB_SESSION_H
#define MIHM_WEB_SESSION_H

#include <QObject>
#include <QDateTime>
#include <QTimer>
#include <QVector>

class MIhmWebVisibleObject;
class MClientSocket;
class MIhmVirtualObject;
class MIhmVirtContainer;
class MIhmVirtIco;
class MIhmVirtText;
class MIhmVirtList;
class MIhmVirtDynConfig;
class MIhmVirtStrDetectConfig;
class MIhmVirtInputDialogReq;
class MIhmVirtInputDlgExReq;
class MIhmVirtTableViewReq;
class MIhmVirtVideoViewReq;
class MIhmWebInterface;
class MIhmHttpRspRefresh;
class MIhmMsgInputDialogRes;
class MIhmMsgInputDialogInitErr;
class MIhmMsgInputDlgExRes;
class MIhmMsgInputDlgExInitErr;

class MIhmVirtButton;
class MIhmVirtTabView;
class MIhmVirtAsyncInput;
class MIhmVirtHMenuViewReq;
class MInputDialogReq;
class MInputDialogExReq;
class MIhmMsg;
class MIhmMsgVideo;
class MIhmWebRVideoView;
class MIhmWebVisibleImage;
class MIhmWebVisibleDynImage;
class MIhmWebCommonObjLogic;
class MWebVisibleObjSessionLink;


class MIhmWebSession:public QObject
{
    Q_OBJECT
public:
	MIhmWebSession(MIhmWebInterface *parent, MIhmWebCommonObjLogic * pCommonVisibleObjLogic);
	~MIhmWebSession();

	QString getSessionID(){return m_sSessionID;}
	QString getClientSessionID(){return m_sClientSessionID;}

	void processVirtualObject(MIhmVirtualObject* pVirtObj);

	enum enumIhmWebSessStatus{
		enuWebSessStatusUnknown = 0,  
		enuWebSessWaitingAuth,  
		enuWebSessInitializing,
		enuWebSessInitialized
	};

	enumIhmWebSessStatus getStatus() {return m_eStatus;}

	void setStatusInitializing();

	void setLoginData(QString sLoginName, bool bTakeControl, MClientSocket *pClient, bool bProxyUsed, QString sClientSessionID);
	void setAppRootUrl(QString sAppRootUrl) { m_sAppRootUrl = sAppRootUrl;};
	QString getAppRootUrl(){ return m_sAppRootUrl;};

	QString getLoginName();
	bool isProxyUsed(){return m_bProxyUsed;};
	MClientSocket * getLoginClient(){return m_pLoginClient;};

	static MIhmWebSession * findSessionObj(QList <MIhmWebSession *> * plstSessions, QString sSessionID);
	static MIhmWebSession * findByClientSessionId(QList <MIhmWebSession *> * plstSessions, QString sClientSessionID);
	static bool removeSessionObj(QList <MIhmWebSession *> * plstSessions, QString sSessionID);
	static bool removeSessionObj(QList <MIhmWebSession *> * plstSessions, MClientSocket *pClient);

	void getVisibleObjectsResponse(MIhmHttpRspRefresh * pRsp, bool bIsSessionReadOnly, bool bOnlyUpdated = false);
	void updateVisibleObjectTimestamp(QString sObjectName, QString sTimeStamp);
	bool getActionForCtrlId(QString sCtrlId, QString &sActionID, QString &sParams);

	void appendOpenLinkReq(QString sUrl, int iTargetWin, bool bOpen);

	bool isSessionDead(int iSessionTimeoutSecs);
	void updateLastResponseTime();
	
	bool getCurrentInputDialog(MInputDialogReq *pReq);
	void setDialogNotInProgress();
	bool getCurrentInputDialogEx(MInputDialogExReq *pReq);
	void setDialogExNotInProgress();
	

	bool isAnyInputDlgInProgress();

	bool setInControl(bool bInControl);

	void initClientLoginDlg(QString sLogin = QString(""));

	void initTakeOver(bool bQuest, QString sTakeOverUserName = QString(""));
	void closeTakeCtrlClientDlg();//used for web interface to stop the taking over procedure

	void initClientInfoDlg(QString sMsg, QString sMsgTitle);
	void initConnectingInfoDlg(QString sMsg, QString sMsgTitle);

	void closeHighPrioClientDlg();//used to close any high priority dialog
	int getHPrioDlgType();

	void resetSessionIdleTimer();
	bool isIdleTimeout() { return m_bIdleTimeout;};

	void logOff(){	m_bLoggedOff = true;};
	bool isLoggedOff(){return m_bLoggedOff;};

signals:
	void inputDlgsMsg(MIhmMsg*);
	void sessionIdleTimeout(QString);

private slots:
	void onSessionIdleTimeout();

private:
	MIhmWebInterface * m_pWebInterface;
	void initializeCompleteVisibleObjectLinks(MIhmWebCommonObjLogic *pCommonVisibleObjLogic);

	void updateSessionInControlStatuses(bool bInControl);
	void generateNewSessionID();

	void processVirtConfig(MIhmVirtDynConfig*pVirtObj);
	void processConfigPrjVersion(MIhmVirtDynConfig* pVirtObj);
	void processConfigPrjName(MIhmVirtDynConfig* pVirtObj);
	void processConfigLanguage(MIhmVirtDynConfig* pVirtObj);
	void processConfigLaneType(MIhmVirtDynConfig* pVirtObj);
	void processConfigShiftType(MIhmVirtDynConfig* pVirtObj);

	//Input dialog
	bool isInputDlgInProgress();
	void processInputDialogReq(MIhmVirtInputDialogReq* pVirtObj);
	void raiseNextInputDialog(MIhmVirtInputDialogReq* pVirtObj);
	void sendInputDialogInitErr();
	void onInputDialogCMDCancel();
	void sendInputDlgCanceled();

	//Input dialog ex
	bool isInputDlgExInProgress();
	void processInputDialogExReq(MIhmVirtInputDlgExReq* pVirtObj);
	void raiseNextInputDialogEx(MIhmVirtInputDlgExReq* pVirtObj);
	void sendInputDialogExInitErr(MInputDialogExReq *pReq);
	void onInputDlgExCMDCancel();
	void sendInputDlgExCanceled();

	bool isHPrioDlgInProgress();

	QString m_sSessionID;	
	QString m_sAppRootUrl;

	QString m_sClientSessionID;	
	QString m_sMainPage;	

	enumIhmWebSessStatus m_eStatus;

	MClientSocket *m_pLoginClient;

	QString m_sLoginName;
	bool m_bLoginTakeControlFlag;
	bool m_bProxyUsed;

	QDateTime m_dtLastResponseTime;

	bool m_bSessionIsInControl;
	void initializeSessionVisibleObjects();

	QVector <MIhmWebVisibleObject*> m_vectSessionWebVisibleObjects;
 	QVector <MWebVisibleObjSessionLink*> m_vectCompleteWebVisibleObjectsSessData;

	void stopSessionIdleTimer();
	void startSessionIdleTimer();
	QTimer m_tmInControlIdleCheck;

	bool m_bIdleTimeout;
	bool m_bLoggedOff;
};

#endif


