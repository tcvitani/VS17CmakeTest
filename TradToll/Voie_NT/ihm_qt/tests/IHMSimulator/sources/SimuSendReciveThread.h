// SimuSendReciveThread.h: interface for the SimuSendReciveThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMUSENDRECIVETHREAD_H__571B0066_A0FF_4044_9CF3_9EF488075376__INCLUDED_)
#define AFX_SIMUSENDRECIVETHREAD_H__571B0066_A0FF_4044_9CF3_9EF488075376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <QThread>
#include <QAction>
#include <QMutex>

extern "C"
{
#include <ihm.h>
}


#define SCRIPT_XML_ATTRIBUTE_NO_CHANGE "#NO_CHANGE#" 


class SimuSendReciveThread : public QThread  
{
	Q_OBJECT
		
public:
	SimuSendReciveThread(QString sIHMMailbox);
	virtual ~SimuSendReciveThread();

	void run();
	QAction * getSetTextAction();
	QAction * getConfigAction();
	QAction * getAffichageSetAction();
	QAction * getVideoAction();
	QAction * getToucheAction();
	QAction * getSaisieAction();
	QAction * getSaisieExAction();
	QAction * getPoliceAction();
	QAction * getDetectionAction();
	QAction * getArretAction();
	QAction * getScriptAction();
	QAction * getAuthAction();
	QAction * getAsyncInputAction();
	
	QString getLanguage();
		
	bool sendMessage(enum_ihm_service service, int type_message, void *data);

	static int getObjectIdFromString(QString sObjectName);
	static QString getStringFromObjectId(int iObjectId);
	static int getCommandIdFromString(QString sObjectName);
	static QString getNameForAuthReasonId(int iReasonId);
	static QString getVideoCmdString(int iCmdId);
	
	static QString getTableViewCmdString(int iCmdId);
	static int getTableViewCmdFromString(QString sCommand);
	
	void setLoopScript(bool bLoop);

	bool					m_bAuthAuto;
	bool					m_bTakeControlAuto;
	bool					m_bReturnControlAuto;
	int decMsgsEvents();
	
	static char * verify_if_no_change_string(char *);
	
	void setUserDialogConfirmed(bool bCancelScript);

public slots:
	void onConfig();
	void onAffichageSet();
	void onVideo();
	void onTouche();
	void onSaisie();
	void onSaisieEx();
	void onPolice();
	void onDetection();
	void onArret();
	void onAppendScript(QString);
	void onPrependScript(QString);
	
	void onAuth();
	void onAsyncInput();

signals:
	void message(QString);
	void authData(struct_ihm_authorization*);
	void openWaitDialog();

protected:
	void readRecivedMailslot();
	QString SimuGetMsgSrv(enum_ihm_service eSrv);
	QString SimuGetMsgType(enum_ihm_type eType);
	void SetTextToMainDlg(QString &szText);
	void OpenXMLScript(QString szScript);
	QString generateReadableMessage(struct_ihm_message  * p_msg);
		
	QAction				* m_pActionSetText;
	QAction				* m_pActionConfig;
	QAction				* m_pActionAffichageSet;
	QAction				* m_pActionVideo;
	QAction				* m_pActionTouche;
	QAction				* m_pActionSaisie;
	QAction				* m_pActionSaisieEx;
	QAction				* m_pActionPolice;
	QAction				* m_pActionDetection;
	QAction				* m_pActionArret;
	QAction				* m_pActionScript;
	QAction				* m_pActionAuth;
	QAction				* m_pActionAsyncInput;
	
	void incMsgsEvents();

private:
	struct SimuThreadData
	{
		bool			m_bAlive;
		unsigned int	m_iDelai;
		noyau_bal_id	m_piBalSimu;
		noyau_bal_id	m_piBalModule;
	};

	SimuThreadData		m_stSimuData;
	QString				m_szLanguage;

	QStringList			m_slScript;
	bool				m_bScript;

	struct_ihm_srv_detection m_oldDetection;
	static quint64 m_iInstanceCounter;

	QMutex m_mutex;
	bool m_bLoopScript;

	int m_iCountMessagesEvents;
	QMutex m_mutexMsgs;
	QString m_sIHMMailbox;

	void setWaitUserDialog();
	bool ifWait(bool& bShouldCancel);
	bool m_bWaitShouldContinue;
	bool m_bCancelScript;
};

#endif // !defined(AFX_SIMUSENDRECIVETHREAD_H__571B0066_A0FF_4044_9CF3_9EF488075376__INCLUDED_)
