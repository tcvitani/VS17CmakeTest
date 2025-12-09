#ifndef TLM_DMV_WORKER_H
#define TLM_DMV_WORKER_H

#ifdef __cplusplus

extern "C"
{
#endif
#include <noyau.h>
#include <debug.h>
#include <TlmDmvGlobal.h>
#include <tlm_dmv.h>

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#define EXPORT_C extern "C"
#else
#define EXPORT_C
#endif

EXPORT_C int InitTlmWorker(IN  LPVOID pGlobalWorkerStructure);
EXPORT_C void DeinitTlmWorker(IN  LPVOID pGlobalWorkerStructure);
EXPORT_C void SlotNewMessage(void *pQObject, struct_neutre *psMsg);

#ifdef __cplusplus


#include <QObject>
#include <QTimer>
#include <QLinkedList>
#include <MQEventLoop.h>
#include <UdpClient.h>
#include <TlmDmvAutomate.h>
#include "OblakMsg.h"

class TlmAutomate;
class OblakMsg;
class SetBrightness;
class SetImage;

class TlmWorker : public QObject
{
	Q_OBJECT
public:
	TlmWorker(TWorkerStructure *pWork, QObject *parent = 0);
	~TlmWorker();

	void emitNewMboxMsg(struct_neutre *pMsg);

signals:
	void signal_connected();
	void signal_disconnected();
	void newMbxMsg(struct_neutre *pMsg);

public slots:
	void ReadAndProcessMailboxMsg(struct_neutre *pMsg);

private slots:
	void onConnected();
	void onDisconnected();
	void onNewMsgFromDevice(const QByteArray &);
	void onNewEchoMsgFromDevice(const QByteArray &);
	void onWaitForResponseTimeout();
	void onInitializeConnectionTimeout();
	void onCheckStatusTimeout();
	void onWaitForStatusTimeout();

private:
	short int m_siInstId;
	bool m_bFullTrace;

	UdpClient * m_pDmv_Client;
	UdpClient * m_pDmv_EchoClient;

	struct _TWorkerStructure*	m_pWorkData;
	struct _struct_global*		m_pTlmDmvInst;

	noyau_bal_id				m_dwAniMboxId;
	noyau_bal_id				m_dwDrvMboxId;

	void ReceiveFromAni(struct_neutre *pNeutre);
	void SendToAni(struct_tlm_message *psAniIos);
	void StopWorker(tlm_inst_id siInstId, enum_tlm_ani_ios eTypeMessage);

	//Automaton
	TlmAutomate *m_pAutomate;

	struct_tlm_status m_sOldStatus;

	QString m_sLocalIpAddress;
	quint16 m_uiLocalIpPort;
	quint16 m_uiLocalEchoPort;

	QString m_sDeviceIpAddress;
	quint16 m_uiDeviceIpPort;
	quint16 m_uiDeviceEchoPort;

	int m_iWaitForResponseTimeoutMs;
	QTimer m_WaitForResponseTimer;
	bool m_bWaitForResponseTimerStared;

	int m_iWaitForStatusTimeoutMs;
	QTimer m_WaitForStatusTimer;

	QTimer m_CheckStatusTimer;
	QTimer m_InitTimer;

	OblakDevice *m_pOblakDevice;
	OblakDevice::eOblakDeviceType mapDeviceFromRegistry(int iDeviceType);
	void mapRegistryParametersToClassMembers();
	bool InitializeIosMbox();
	bool getDeviceType();
	void createClientSocket();
	void initializeDevice();
	void initTimers();


	quint16 m_uiMaxLinkError;
	quint16 m_uiLinkErrCount;

	void IncLinkError();
	void ResetLinkError();
	void SetLinkFailureStatus(bool bLinkFailure);
	void SendStatus();

	void StopResponseTimer();
	void StartResponseTimer();
	void StopStatusTimer();
	void StartStatusTimer();

	void SendPollRequest();
	void SendRequest(int iImage);

	int m_iBrightness;
	unsigned char m_ucImageXCoordinate;
	unsigned char m_ucImageYCoordinate;
	QByteArray m_baImageXCoordinate;
	QByteArray m_baImageYCoordinate;

	bool m_bImageMemorized;
	int m_iLastImageSet;
	int m_iMemorizedImage;
	int m_iImageRequestedByUser;
	OblakMsg::eOblakImage ConvertImageToProtocol(int iImageRequestedByUser);
	QString	TlmEnumToString(int state);
	QString printAutMsg(QByteArray &arr);

	friend class TlmAutomate;
};

#endif
#endif
