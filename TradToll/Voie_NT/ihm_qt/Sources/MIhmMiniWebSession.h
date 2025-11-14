#ifndef MIHM_MINI_WEB_SESSION_H
#define MIHM_MINI_WEB_SESSION_H

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


class MIhmMiniWebSession:public QObject
{
    Q_OBJECT
public:
	MIhmMiniWebSession(MIhmWebInterface *parent, MIhmWebCommonObjLogic * pCommonVisibleObjLogic);
	~MIhmMiniWebSession();

	QString getClientSessionID(){return m_sClientSessionID;}
	void initializeCompleteVisibleObjectLinks(MIhmWebCommonObjLogic *pCommonVisibleObjLogic);

	void setStatusInitializing();

	void setInitData(MClientSocket *pClient, bool bProxyUsed, QString sClientSessionID);
	void setAppRootUrl(QString sAppRootUrl) { m_sAppRootUrl = sAppRootUrl;};
	QString getAppRootUrl(){ return m_sAppRootUrl;};

	bool isProxyUsed(){return m_bProxyUsed;};
	MClientSocket * getLoginClient(){return m_pLoginClient;};

	static MIhmMiniWebSession * findSessionObj(QList <MIhmMiniWebSession *> * plstSessions, QString sSessionID);
	static MIhmMiniWebSession * findByClientSessionId(QList <MIhmMiniWebSession *> * plstSessions, QString sClientSessionID);
	static bool removeSessionObj(QList <MIhmMiniWebSession *> * plstSessions, QString sSessionID);
	static bool removeSessionObj(QList <MIhmMiniWebSession *> * plstSessions, MClientSocket *pClient);

	void getVisibleObjectsResponse(MIhmHttpRspRefresh * pRsp, bool bOnlyUpdated = false);
	void updateVisibleObjectTimestamp(QString sObjectName, QString sTimeStamp);

	bool isSessionDead(int iSessionTimeoutSecs);
	void updateLastResponseTime();

	void resetSessionIdleTimer();
	bool isIdleTimeout() { return m_bIdleTimeout;};

signals:
	void inputDlgsMsg(MIhmMsg*);
	void sessionIdleTimeout(QString);

private slots:
	void onSessionIdleTimeout();

private:
	MIhmWebInterface * m_pWebInterface;

	void generateNewSessionID();

	QString m_sAppRootUrl;

	QString m_sClientSessionID;	

	MClientSocket *m_pLoginClient;

	QString m_sLoginName;
	bool m_bLoginTakeControlFlag;
	bool m_bProxyUsed;

	QDateTime m_dtLastResponseTime;

 	QList <MWebVisibleObjSessionLink*> m_lstCompleteWebVisibleObjectsSessData;

	void stopSessionIdleTimer();
	void startSessionIdleTimer();
	QTimer m_tmInControlIdleCheck;

	bool m_bIdleTimeout;
	bool m_bLoggedOff;
};

#endif


