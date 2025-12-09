#ifndef DMV_WORKER_H
#define DMV_WORKER_H

#ifdef __cplusplus

extern "C"
{
#endif
#include <noyau.h>
#include <debug.h>
#include <aff_ext.h>
#include <DmvGlobal.h>
#include <tlm_dmv.h>
	
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#define EXPORT_C extern "C"
#else
#define EXPORT_C
#endif

EXPORT_C int InitDmvWorker(IN  LPVOID pGlobalWorkerStructure);
EXPORT_C void DeinitDmvWorker(IN  LPVOID pGlobalWorkerStructure);
EXPORT_C void SlotNewMessage(void *pQObject, struct_neutre *psMsg);

#ifdef __cplusplus


#include <QObject>
#include <QTimer>
#include <QLinkedList>
#include <MQEventLoop.h>
#include <UdpClient.h>
#include <DmvAutomate.h>
#include "OblakMsg.h"

class DmvAutomate;
class OblakMsg;
class SetTextProperties;
class SetTextMessage;
class SetMultiLineTextMessage;
class SetImage;

class DmvWorker : public QObject
{
	Q_OBJECT
public:
	DmvWorker(TWorkerStructure *pWork, QObject *parent = 0);
	~DmvWorker();

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
	void onNewEchoMsgFromDevice(const QByteArray &);
	void onWaitForResponseTimeout();
	void onInitializeConnectionTimeout();
	void onCheckStatusTimeout();
	void onWaitForStatusTimeout();
	void onNewMsgFromDevice(const QByteArray &);

private:
	short int m_siInstId;
	bool m_bFullTrace;

	UdpClient * m_pDmv_Client;
	UdpClient * m_pDmv_EchoClient;

	struct _TWorkerStructure*	m_pWorkData;
	struct _struct_global*		m_pAffDmvInst;

	noyau_bal_id				m_dwAniMboxId;
	noyau_bal_id				m_dwDrvMboxId;

	void ReceiveFromAni(struct_neutre *pNeutre);
	void SendToAni(struct_ani_ios *psAniIos);

	DmvAutomate *m_pAutomate;
	
	struct_b_etat_aff m_sOldStatus;

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
	
	OblakDevice *m_pOblakDevice;
	OblakDevice::eOblakDeviceType mapDeviceFromRegistry(int iDeviceType);
	OblakMsg::eOblakFont m_eFont;
	OblakMsg::eOblakFont mapFontFromRegistry(int iFont);
	OblakMsg::eOblakFontColor m_eFontColor;
	OblakMsg::eOblakFontColor mapFontColorFromRegistry(int dwFontColor);
	OblakMsg::eOblakAlignment m_eAlignment;
	OblakMsg::eOblakAlignment mapAlignmentFromRegistry(int iAlignment);
	
	void setTextProperties();
	SetTextProperties *m_pTextProperties;

	void mapRegistryParametersToClassMembers();
	bool InitializeIosMbox();
	bool getDeviceType();
	void createClientSocket();
	void initializeDevice();
	void initTimers();

	OblakMsg::eFlasherState ConvertFlasherStateToProtocol(enum_flasher_state eState);
	OblakMsg::eOblakImage ConvertImageToProtocol(int iImageRequestedByUser);
	QString	TlmEnumToString(int state);

	void IncLinkError();
	void ResetLinkError();
	void SetLinkFailureStatus(bool bLinkFailure);
	void SendStatus();

	QByteArray m_baLastReq;
	void RetryLastRequest();
	void SendPollRequest();
	void HandleAffRequest(struct_screen sScreen, int iPriority);
	void HandleTlmRequest(enum_tlm_dmv_images eTlmImage);
	void HandleNewFlasherStateRequest(enum_flasher_state eFlasherState);
	void HandleConfirmFlasherStateRequest();
	void SendActivateScenario();
	void SendTextProperties();
	void ClearScreen();
	bool GetTextToDisplay(SetPropertiesAndText *pMsg, 
						  struct_screen sScreen, 
						  int iPriority, 
						  QByteArray &baPacket);
	int	 CalculateMaxCharacters(int iMaxSpacing);
	void NewView(dmv_inst_id inst_id,const char* line1, const char* line2, const char* line3, const char* line4);

	void StopResponseTimer();
	void StartResponseTimer();
	void StopStatusTimer();
	void StartStatusTimer();

	bool isZeroed(const void* data, size_t size);

	quint16 m_uiMaxLinkError;
	quint16 m_uiLinkErrCount;
	quint16 m_uiReqRetryCount;

	int m_iBrightness;

	// AFF Parameters
	int m_iMaxSpacing;
	unsigned char m_ucTextYCoordinate;
	unsigned char m_ucTextXRCoordinate;
	unsigned char m_ucTextXLCoordinate;
	QByteArray m_baTextYCoordinate;
	QByteArray m_baTextXRCoordinate;
	QByteArray m_baTextXLCoordinate;
	int m_iLastAffPriority;
	struct_screen m_sLastAffScreen;

	// TLM Parameters
	bool m_bIsTlmServiceIntegrated;
	unsigned char m_ucImageXCoordinate;
	unsigned char m_ucImageYCoordinate;
	QByteArray m_baImageXCoordinate;
	QByteArray m_baImageYCoordinate;
	enum_tlm_dmv_images m_eLastTlmImage;

	// Flasher Parameters
	enum_flasher_state m_eLastFlasherState;

	friend class DmvAutomate;
};

#endif
#endif
