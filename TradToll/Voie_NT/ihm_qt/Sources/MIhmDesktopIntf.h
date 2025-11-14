#ifndef MIHM_DESKTOP_INTERFACE_H
#define MIHM_DESKTOP_INTERFACE_H

#include <QObject>
#include <QString>


#include "MIhmVirtualObject.h"


class MIhmDskVisibleObject;
class MFormSplash;
class MFormMain;
class MIhmConfigGeneral;
class MIhmVirtContainer;
class MIhmVirtIco;
class MIhmVirtText;
class MIhmVirtList;
class MIhmVirtDynConfig;
class MIhmLanguages;
class MIhmMsgStringDetection;
class MIhmVirtStrDetectConfig;
class MIhmVirtButton;
class MIhmVirtInputDialogReq;
class MIhmVirtTableViewReq;
class MIhmVirtHMenuViewReq;
class MIhmVirtVideoViewReq;
class MIhmVirtTabView;
class MIhmMsg;
class MIhmMsgVideo;
class MIhmVirtObjectsModel;

#define SPECIFIC_ACTION_BTN_END_IDENTIF "ACTION_BTN_END_IDENTIF"


//Handling desktop dialog 
//It process inputs from the dialog and updates the 
// visible objects view  
class MIhmDesktopIntf:public QObject
{
    Q_OBJECT
public:
	MIhmDesktopIntf();
	~MIhmDesktopIntf();

	bool initialize();

	void forceRefreshAll();

	void processRefreshAllObjects();
	void updateVirtualObjects(MIhmVirtObjectsModel *pTransportModel);

	//handling direct ani messages
	void processOpenLink(int iTargetDlg, QString sUrl, bool bVisible);
	void processOpenLoginDlgReq(QString sLoginId, bool bLoginFieldEnabled, bool bLDAPOffline, bool bShow);
	void processOpenAboutDlgReq(QString sAboutDialogCfg);
	void processCloseAboutDlgReq();

	enum enumIHMStatus {
		enuDskNotInitialized = 0,
		enuDskInitializing,
		enuDskInitialized
	};

	void initTakeOver(bool bQuest, QString sTakeOverUserName = QString(""));
	void cancelTakeOver();
	void showClientInfoMsg(QString sMsg, QString sMsgTitle);
	void hideClientInfoMsg();
	void showConnectingMsg(QString sMsg, QString sMsgTitle);
	void hideConnectingMsg();

	enum enumAuthStatus {
		enuNotIdentified,
		enuIdentifiedReadonly,
		enuInControl
	};
	
	void setAuthStatus(enumAuthStatus eAuthStatus);
	void setInControl(bool bInControl);
	bool isInControl(){return (m_eAuthStatus == enuInControl);};
	void setUserIdentified(bool bIdentified, QString sInternalSessionID);	
	bool isIdentified(){return (m_eAuthStatus == enuIdentifiedReadonly);};
	QString	getDskIdentSessionID(){return m_sIdentSessionID;};
	
	QString getCurrentLaneTypeParam(QString sParamName);
	QString getLaneTypeParam(QString sLaneType, QString sParamName);

	MIhmVirtualObject *findVirtualObjectByID(MIhmVirtualObject::enumVirtualObjectId eID);
	
	//to test if the main application disabled the take ctrl button
	// to be able to block action connected to it
	bool isTakeCtrlBtnEnabled();
	bool isReturnCtrlBtnEnabled();

	void sendMsgToAni(MIhmMsg *);
	void sendInputDialogResponse(MIhmMsg *);
	void sendAsyncInputResponse(MIhmMsg *);
	void sendVideoCtlResponse(MIhmMsg *pMsg);

	void resetIdleTimer();


signals:
	void keyDetected(int);
	void stringDetected(MIhmMsgStringDetection *);
	void exitProgram();
	void inputDialogRsp(MIhmMsg *);
	void asyncInputRsp(MIhmMsg *);
	void videoCtlRsp(MIhmMsg *);
	void msgToAni(MIhmMsg *);

	void action(QString sAction, QString sParams);
	void aboutDlgAction(QString sAction, QString sParams);
	
	void identificationReq(QString sName, QString sSecCode, bool bLDAPOffline);
	void endIdentificationReq(QString sSesionId, int iReason);
	void authDlgCanceled(int);
	void takeCtrlReq(QString);
	void returnCtrlReq(QString, int iReason);
	void takeOverReject();
	void takeOverAccept();
	void takeOverCancel();

	void privateUpdateVirtObj();

public slots:
	void displayInitInfo(QString s);
	void close();
	void onVirtualObjectsUpdated();
	
private slots:
	void onVirtualObjectsUpdatedPrivate();
	void onRefreshAllReq();
	void onAction(QString sAction, QString sParams);

	void onIdentificationReq(QString sName, QString sSecCode, bool bLDAPOffline);
	void onDskIdleTimeout();
	
	void onTakeOverReject();
	void onTakeOverAccept();
	void onTakeOverCancel();

	void onDesktopWidgetChanged(int iData);
private:
	bool initializeSplashDialog();
	bool verifyMainDialogTemplates();
	void setStatus(enumIHMStatus eStatus);
	enumIHMStatus getStatus(){return m_eStatus;};
	
	//processes the updated virtual objects
	void processUpdatedVirtualObjects();
	bool m_bRefreshAllObjects;

	void processVirtualObject(MIhmVirtualObject* pVirtObj);
			
	void processContainer(MIhmVirtContainer* pVirtObj);
	void processIcon(MIhmVirtIco* pVirtObj);
	void processText(MIhmVirtText* pVirtObj);
	void processButton(MIhmVirtButton* pVirtObj);
	void processList(MIhmVirtList* pVirtObj);
	void processConfig(MIhmVirtDynConfig* pVirtObj);
	void processStrDetConfig(MIhmVirtStrDetectConfig* pVirtObj);
	void processFileViewReq(MIhmVirtTableViewReq* pVirtObj);
	void processHMenuViewReq(MIhmVirtHMenuViewReq* pVirtObj);
	void processVideoViewReq(MIhmVirtVideoViewReq* pVirtObj);
	void processTabView(MIhmVirtTabView* pVirtObj);
	void processInputDialogReq(MIhmVirtInputDialogReq* pVirtObj);
	void processInputDlgExReq(MIhmVirtInputDlgExReq* pVirtObj);
	void processAsyncInput(MIhmVirtAsyncInput* pVirtObj);

	
	//more detailed process functions...
	void processConfigPrjNameAndVersion();
	void processConfigLanguage(MIhmVirtDynConfig* pVirtObj);
	//reads a file name of the form template from registry and 
	// creates the form using the template file
	//if the state is initializing it switch to initialized
	void processConfigLaneType(MIhmVirtDynConfig* pVirtObj);
	void processConfigShiftType(MIhmVirtDynConfig* pVirtObj);
	void processConfigMainDlgHidden(MIhmVirtDynConfig* pVirtObj);


	MIhmVirtObjectsModel * m_pGUIModel;
	
	enumIHMStatus m_eStatus;
	enumAuthStatus m_eAuthStatus; //authentification status

	MFormSplash * m_pSplashDialog;
	MFormMain * m_pMainDialog;

	QString m_sIdentSessionID;

};

#endif


