#ifndef MRVIDEO_CMD_THREAD_H
#define MRVIDEO_CMD_THREAD_H

#include <QThread>
#include <QHttp>
#include <QTimer>
#include <QMutex>


#include "MRVideoCmdAutomate.h"

class MRVideoMsg;

class MRVideoCmdThread:public QThread
{
    Q_OBJECT
public:
	MRVideoCmdThread(QString sTargetRVideoObjId);
	~MRVideoCmdThread();

	void setPoolTimerTimeout(int iPoolTimeout);

	void sendActivate(QString sHost, quint16 uiPort, QString sSourceId);
	void sendReactivate();
	void sendDeactivate();
	void sendGrabb();
	void sendResume();
	void sendGetImage(QString sTargeImagePath, bool bIsJpg);

	void setTraceAll() { m_bTraceAll = true;};
signals:
	void outputRVideoMessage(MRVideoMsg* pMsg);
	void checkMessages();

public slots:

private slots:
	void onCheckInputMessages();

	//Slots to handle automate timers
	void onPoolTimeout();

	//Slots to handle HTTP
	void onHttpRequestFinished(int idHttp, bool error);

private:
	bool initialize();
	void run();	

	void putRVideoMessage(MRVideoMsg* pMsg);
	void processMessage(MRVideoMsg* pMsg);
	void sendMyselfACheckMessagesSignal();
	void sendOutputRVideoMessage(MRVideoMsg* pMsg);
	
//-----------------------------------------
    enum StreamStatus
    { 
        Unknown     = 0,
        Stopped     = 11,
        Running     = 12,
        Grabbing    = 13,
        Paused      = 14,
		WaitingForImage = 15,
		ErrorRecovery   = 16
    };

	
	enum enuRVStreamHttpCommand{
		enuHttpCmdGetStatus,
// 		enuHttpCmdActivate,
// 		enuHttpCmdDeactivate,
		enuHttpCmdGrabb,
		enuHttpCmdResume,
		enuHttpCmdGetImage
	};

	QString generateCommandPath(enuRVStreamHttpCommand eCmdId, QString sRVideoStreamId);
	void sendHttpCommand(enuRVStreamHttpCommand eCmdId);
	void abortHttp();
	
	MAutEvent * parseResponse(bool bHttpErr);
	bool extractRspCode(QByteArray &baData, int *piCodeStatus);
	QHttp *m_pHttp;
	int m_iLastHttpRspId;
	QByteArray m_baImageCaptured;
	bool m_bImageCaptured;

	QString m_sTargetFileName;
	bool m_bSaveJpg;

//-----------------------------------------------

	MRVideoCmdAutomate m_RVideoAut;
	QString	m_sCurrentHost;
	quint16 m_uiPort;
	QString	m_sCurrentSourceId;
	QString m_sTargetRVideoObjId;

	QMutex m_Mutex;
	QList <MRVideoMsg*> m_lstInputMessages;

	void startPoolTimer();
	void stopPoolTimer();
	
	QTimer m_tmPoolTimer;
	bool m_bPoolStarted;
	int m_iPoolTimeout;
	
	friend class MRVideoCmdAutomate;
	
	bool getTraceAll(){return m_bTraceAll;};
	bool m_bTraceAll;
};


#endif


