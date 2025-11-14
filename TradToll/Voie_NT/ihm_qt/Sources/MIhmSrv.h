#ifndef MIHM_SRV_H
#define MIHM_SRV_H

#include <QObject>


extern "C" {
	#include <noyau.h>
}


#include "MIhmVirtualObject.h"
#include "MIhmKey.h"
#include "MIhmStatusSetCfg.h"

class MIhmAniVirtObjects;
class MIhmAniThread;
class MIhmMsg;
class MIhmMsgKeyAndCmdReq;
class MIhmMsgStringDetection;
class MIhmMsgInputDialogRes;
class MIhmMsgInputDialogInitErr;
class MIhmMsgInputDlgExRes;
class MIhmMsgInputDlgExInitErr;
class MIhmMsgAuthReq;
class MIhmMsgTakeCtrlReq;
class MIhmMsgAsyncInputRsp;
class MIhmMsgReturnCtrlReq;
class MIhmMsgAuthLogOffReq;
class MIhmMsgInputDlgCanceled;
class MIhmMsgInputDlgExCanceled;
class MIhmMsgInputDlgExOpened;
class MIhmMsgLoginDlgOpened;
class MIhmMsgLoginDlgClosed;
class MIhmMsgVideoFreezeRsp;
class MIhmMsgVideoSaveRsp;
class MIhmMsgRVideoUpdateSrc;
class MIhmMsgRVideoUpdateOnOff;
class MIhmMsgAuthDlgCanceled;

class BalIdDetectionPair  
{
public:
	noyau_bal_id blId;
	QString sActiveDetectPair;

	static bool existsBlPair(QList <BalIdDetectionPair*> *pLst, BalIdDetectionPair* pBlPair);
	static bool removeBlPair(QList <BalIdDetectionPair*> *pLst, BalIdDetectionPair* pBlPair);
	static bool addBlPair(QList <BalIdDetectionPair*> *pLst, BalIdDetectionPair* pBlPair);
	static bool existsPair(QList <BalIdDetectionPair*> *pLst,QString sPair);

};


class MIhmSrv:public QObject
{
    Q_OBJECT
public:
	MIhmSrv();
	~MIhmSrv();

	void Initialize(MIhmAniThread * pAniThread, MIhmAniVirtObjects * pAniVirtObjects);
	void IhmTreatMessageService( IN struct_ihm_message * psNeutre );

	void IhmTreatMessageTCL(MIhmMsg * pMsg);

	static MIhmSrv * getGlobalMSrv(){return m_pGlobalSrv;}
	noyau_pool_id getPoolId();
	noyau_bal_id getAniBalId();

signals:
	void requestStop();

public slots:
	
private:

	void IhmSrvReceptionArret( IN struct_ihm_message * psMsg );
	
	void IhmSrvReceptionConfig( IN struct_ihm_message * psMsg );
	void IhmMessEnvoiDonneesConfig(IN noyau_bal_id iBalDest); 
	//---------------------------------------------------------------------------	
	
	void IhmSrvReceptionAffichageStatus(IN struct_ihm_message *psMsg);
	BOOL IhmSrvTraitementAffichageStatus(IN struct_ihm_message * psMsg);


	//---------------------------------------------------------------------------	
	void IhmSrvReceptionAffichage ( IN struct_ihm_message *psMsg );
	bool AssureAffichageStructStrings(struct_ihm_srv_affichage * psAff);
	BOOL IhmSrvTraitementAffichage ( IN struct_ihm_message * psMsg );
	void updateTextObjectLabel(MIhmVirtualObject::enumVirtualObjectId eId, 
															const char* szNewValue, 
															const char* szToolTip,
															bool bVisible);
	void updateIconObjectImage(MIhmVirtualObject::enumVirtualObjectId eId, 
															const char* szNewValue, 
															const char* szToolTip,
															bool bVisible);
	void updateListObject(MIhmVirtualObject::enumVirtualObjectId eId, 
														const char * sValue, 
														int iProperty, 
														bool bVisible);

	void updateTableViewReq(MIhmVirtualObject::enumVirtualObjectId eId, char * sDataToShow,bool bExternalFile, enum_aff_table_command eCmd, bool bVisible);

	void updateHMenuViewReq(MIhmVirtualObject::enumVirtualObjectId eId, struct_ihm_menu_affichage * menu, bool bVisible);

	//processing video
	void updateVideoViewReq(MIhmVirtualObject::enumVirtualObjectId eId, struct_ihm_video_affichage stVideo, bool bVisible);
	void IhmProcessMessVideoFreezeRsp(MIhmMsgVideoFreezeRsp *pMsg);
	void IhmProcessMessVideoSaveRsp(MIhmMsgVideoSaveRsp *pMsg);
	void IhmProcessMessRVideoUpdateSrc(MIhmMsgRVideoUpdateSrc *pMsg);
	void IhmProcessMessRVideoUpdateOnOff(MIhmMsgRVideoUpdateOnOff *pMsg);

	void updateContainerReq(MIhmVirtualObject::enumVirtualObjectId eId, bool bVisible);

	void updateButtonObject(MIhmVirtualObject::enumVirtualObjectId eId, 
														const char* szImage, 
														const char* szText,
														const char* szToolTip,
														enum_ihm_object_enabled isEnabled, 
														bool bVisible);

	void updateTabControl(MIhmVirtualObject::enumVirtualObjectId eId, 
														const char * sValue, 
														int iProperty, 
														bool bVisible);

	void processOpenLink(int iTarget, QString sUrl, bool bVisible);
	void processLoginDlg(QString sLoginId, bool bManualLogin, bool bLDAPOffline, bool bOpen);
	void processAboutDlg(QString szAboutDialogCfg, bool bVisible);

	//------------------------------------
	void processReloadDynConfig();

	void IhmSrvReceptionPolice ( IN struct_ihm_message *psMsg );
	bool IhmSrvTraitementPolice ( IN struct_ihm_message * psMsg );

	void updateContainerObjectFont(MIhmVirtualObject::enumVirtualObjectId eId, const char * szNewPolice);

	void updateAllObjectFont(const char * sNewFont);

	
	void IhmSrvReceptionDetectionChaine ( IN struct_ihm_message *psMsg );
	bool IhmSrvTraitementDetectionChaine ( IN struct_ihm_message * psMsg );
	bool IhmDebutServiceDetection(IN noyau_bal_id dwBalDest,
                                                 IN QString sCoupleId);
	bool IhmFinServiceDetection(IN noyau_bal_id dwBalDest,
												IN QString sCoupleId);
	bool EnvoiDemandeurDetection(IN PVOID Param,
									  IN noyau_bal_id BalDemandeur,
									  IN PVOID DataDuDemandeur);
	void IhmMessEnvoiDetectionChaine(MIhmMsgStringDetection *pMsg);


	void IhmSrvReceptionTouches ( IN struct_ihm_message *psMsg );
	void IhmMessEnvoiTouche (MIhmMsgKeyAndCmdReq *pMsg);


	void IhmSrvReceptionSaisie( IN struct_ihm_message *psMsg);
	bool IhmSrvDemandeSaisieVisu( IN struct_ihm_message * psMsg );
	bool IhmSrvDemandeAnnulationSaisieVisu();
	void IhmMessEnvoiSasieVisu(MIhmMsgInputDialogRes *pMsg);
	void IhmSasieVisuInitErr(MIhmMsgInputDialogInitErr *pMsg);
	void IhmSasieVisuCanceled(MIhmMsgInputDlgCanceled *pMsg);
	void checkInputDialog();
	
	void IhmSrvReceptionSaisieEx(IN struct_ihm_message *psMsg);
	bool IhmSrvDemandeSaisieVisuEx( IN struct_ihm_message * psMsg );
	bool IhmSrvDemandeAnnulSaisieVisuEx(IN struct_ihm_message * psMsg );
	void IhmMessEnvoiSasieVisuEx(MIhmMsgInputDlgExRes *pMsg);
	void IhmMessEnvoiSasieVisuExAnnule(MInputDialogExReq* pCurrent);

	void IhmSasieVisuExInitErr(MIhmMsgInputDlgExInitErr *pMsg);
	void IhmSasieVisuExCanceled(MIhmMsgInputDlgExCanceled *pMsg);
	void IhmSasieVisuExOpened(MIhmMsgInputDlgExOpened *pMsg);
	void IhmLoginDlgOpened(MIhmMsgLoginDlgOpened *pMsg);
	void IhmLoginDlgClosed(MIhmMsgLoginDlgClosed *pMsg);

	void checkInputDialogEx();

	void IHMSaisieExShowVirtualKeyboard(QString sVirtKybCfg, int iVirtKybPosX, int iVirtKybPosY);
	void IHMSaisieExHideVirtualKeyboard();

	void IhmSrvReceptionIndicateurs (struct_ihm_message *psMsg);

	
	void IhmMessEnvoiAcquitement(IN noyau_bal_id iBalDest, 
                                       IN enum_srv_service eService,
                                       IN enum_srv_type eTypeMessage);


	enum_srv_type IhmDebutService(IN enum_ihm_service eService,
                                        IN noyau_bal_id dwBalDest);
	enum_srv_type IhmFinService(IN enum_ihm_service eService,
                                      IN noyau_bal_id dwBalDest);

	//Authorization handling
	void IhmSrvReceptionAuthorisation(struct_ihm_message *psMsg);
	bool IhmSrvTraitementAuthorization(IN struct_ihm_message * psMsg);

	int getAuthBalId();
	void IhmMessEnvoiAuthReq(MIhmMsgAuthReq *pMsg);
	void IhmMessEnvoiAuthLogoffReq(MIhmMsgAuthLogOffReq *pMsg); //end identification
	void IhmMessEnvoiAuthDlgCanceled(MIhmMsgAuthDlgCanceled *pMsg); //identification dlg canceled 

	void IhmMessEnvoiTakeCtrlReq(MIhmMsgTakeCtrlReq *pMsg);
	void IhmMessEnvoiReturnCtrlReq(MIhmMsgReturnCtrlReq *pMsg);

	// Async input functions...
	void IhmSrvReceptionAsyncInput(struct_ihm_message *psMsg);
	bool IhmSrvTraitementAsyncInputSet( IN struct_ihm_message * psMsg );
	bool IhmSrvTraitementAsyncInputGet( IN struct_ihm_message * psMsg );
	void IhmProcessMessAsyncInputRsp(MIhmMsgAsyncInputRsp *pMsg);


	static MIhmSrv * m_pGlobalSrv;

	HANDLE				m_hService[NB_IHM_SERVICE];

	MIhmAniVirtObjects *		m_pAniVirtObjects;
	MIhmAniThread *				m_pAniThread;
	
	MIhmKey						m_oIhmKey;
	MIhmStatusSetCfg			m_oIhmStatusSetConfig;

	QList <BalIdDetectionPair*> m_lstSrvDetectionPairs;
};


#endif


