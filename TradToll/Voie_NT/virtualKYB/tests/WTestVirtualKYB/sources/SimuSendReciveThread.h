// SimuSendReciveThread.h: interface for the SimuSendReciveThread class.
//
//////////////////////////////////////////////////////////////////////

#ifndef AFX_SimuSendReciveThread
#define AFX_SimuSendReciveThread

#include <QThread>
#include <QAction>
#include <QMutex>

extern "C"
{
#include <virt_kyb.h>
}


class SimuSendReciveThread : public QThread  
{
	Q_OBJECT
		
public:
	SimuSendReciveThread();
	virtual ~SimuSendReciveThread();

	void initaialize(QString sTargetMboxName);
	void run();
	QAction * getAffichageSetAction();
		
	void sendAffMove(int x, int y);
	void sendAffHide();
	void sendAffShow();
	void sendInitConfig(QString sKeybID);
	void sendAffDEBUT();
	void sendAffFIN();
	void sendArret();
	
signals:
	void message(QString);
	
protected:
	void readRecivedMailslot();
	QString SimuGetMsgSrv(enum_srv_service eSrv);
	QString SimuGetMsgType(enum_srv_type eType);
	void SetTextToMainDlg(QString &szText);

private:
	QString generateReadableMessage(struct_VIRT_KYB_message  * psMessage);
	bool sendMessage(enum_srv_service service, enum_srv_type eTypeMessage, void *data);

	struct SimuThreadData
	{
		bool			m_bAlive;
		unsigned int	m_iDelai;
		noyau_bal_id	m_piBalSimu;
		noyau_bal_id	m_piBalModule;
	};

	SimuThreadData		m_stSimuData;
	QString				m_szLanguage;

	static quint64 m_iInstanceCounter;

	QMutex m_mutex;

	int m_iCountMessagesEvents;
	QMutex m_mutexMsgs;

	QString m_sTargetMboxName;
	QString m_sSimuBalName;
};

#endif 
