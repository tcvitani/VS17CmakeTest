
#ifndef MFORM_MAIN_H
#define MFORM_MAIN_H

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QVector>

#include "MInputDialogReq.h"
#include "MInputDialogExReq.h"
#include "MFormTakeOverDlg.h"
#include "MIhmDskVisibleObject.h"


class QLabel;
class QHBoxLayout;
class QUiLoader;
class QFrame;
class MFormAbout;
class MIhmDesktopIntf;
class MIhmVirtIco;
class MIhmVirtText;
class MIhmVirtList;
class MDigitalClock;
class QListWidget;
class MStringDetection;
class MIhmMsgStringDetection;
class MIhmVirtContainer;
class MIhmVirtButton;
class MIhmVirtStrDetectConfig;
class MIhmVirtTableViewReq;
class MIhmVirtHMenuViewReq;
class MIhmVirtVideoViewReq;
class MIhmVirtTabView;
class MIhmVirtInputDialogReq;
class MIhmVirtInputDlgExReq;
class MFormInput;
class MFormInputEx;
class MFormLogin;
class MFormTakeOverDlg;
class MIhmVirtAsyncInput;
class MDskAsyncInputs;
class QListWidgetItem;
class MIhmVideoView;
class MFormFileOpen;

class MFormMain: public QWidget
{
    Q_OBJECT

public:
    MFormMain(QWidget *parent = 0);
	~MFormMain();

	bool initialize(MIhmDesktopIntf *pParent, QString sLaneType);

	void processPrjNameAndVersion(QString sName, QString sVersion);
	void processContainer(MIhmVirtContainer* pVirtObj);
	void processIcon(MIhmVirtIco* pVirtObj);
	void processText(MIhmVirtText* pVirtObj);
	void processButton(MIhmVirtButton* pVirtObj);
	void processNewShiftType(QString sNewShiftType);
	void processList(MIhmVirtList* pVirtObj);
	void processStrDetConfig(MIhmVirtStrDetectConfig* pVirtObj);
	void processFileViewReq(MIhmVirtTableViewReq* pVirtObj);
	void processHMenuViewReq(MIhmVirtHMenuViewReq* pVirtObj);
	void processVideoViewReq(MIhmVirtVideoViewReq* pVirtObj);
	void processTabView(MIhmVirtTabView* pVirtObj);
	void processOpenLink(int iTargetDlg, QString sUrl, bool bVisible);
	void processInputDialogReq(MIhmVirtInputDialogReq* pVirtObj);
	void processInputDlgExReq(MIhmVirtInputDlgExReq* pVirtObj);
	void processAsyncInput(MIhmVirtAsyncInput* pVirtObj);

	//CAM video desktop implementation
	void processCAMVideoSelectSrc(int eVirtObjet, int iNewSource);
	void processCAMVideoOn(int eVirtObjet);
	void processCAMVideoOff(int eVirtObjet);
	void processCAMVideoZoom(int eVirtObjet, int iNewZoom);
	void processCAMVideoFreeze(int eVirtObjet);
	void processCAMVideoUnfreeze(int eVirtObjet);
	void processCAMVideoSave(int eVirtObjet, QString sTargetFileName, bool bSaveJpg);

	//Restream video desktop implementation
	void processRVideoSelectSrc(int eVirtObjet, QString sNewStreamSourceUrl);
	void processRVideoOn(int eVirtObjet);
	void processRVideoOff(int eVirtObjet);

	void updateAuthStatus();
	
	void showClientInfoMsg(QString sMsg, QString sMsgTitle);
	void hideClientInfoMsg();

	void showConnectingMsg(QString sMsg, QString sMsgTitle);
	void hideConnectingMsg();

	void initTakeOver(bool bQuest, QString sTakeOverUserName = QString(""));
	void cancelTakeOver();

	void openLoginDlg(QString sLoginName, bool bLoginFieldEnabled, bool bLDAPOnline, bool bShow);

	void center();
	bool keyPressEventHandler(QObject *target, QKeyEvent * event);
	void resetIdleTimer();

	void openAboutDlg(QString sCfgFileName);
	void closeAboutDlg();
	
	void showMainDlg(bool bShow);

signals:
	void exitProgram();
	void keyDetected(int);
	void stringDetected(MIhmMsgStringDetection *);
	void action(QString sAction, QString sParams);
	void aboutDlgAction(QString sAction, QString sParams);
	
	void refreshReq(); //requested by refresh button
	
	void idleTimeout();

	void identificationReq(QString sName, QString sSecCode, bool bLDAPOffline);
	void authDlgCanceled(int);

	void takeOverReject();
	void takeOverAccept();
	void takeOverCancel();

public slots:
	void dateChanged();


protected:
	void changeEvent(QEvent * event);

	
	bool eventFilter(QObject *obj, QEvent *event);
	bool processKeyInObjectInFocus(int iAskedKey);
	bool processSetFocusKey(int iAskedKey);
	void closeEvent(QCloseEvent * event );

protected slots:
	void onInputDialogOK();
	void onInputDialogUSRCancel();

	void onInputDlgExOK();
	void onInputDlgExUSRCancel();
	
	void onLoginDlgOK();
	void onLoginDlgCancel();
	void onTakeOverOK();
	void onTakeOverReject();
	void onNumLockTimer();
	void onAction(QString sActionID, QString sActionParams);

	void onAboutDlgAction(QString sActionID, QString sActionParams);
	void onAboutDlgClosed();

	void onTabClickedAction(QString sAction, int iTabIndex);

	void onAsyncInputDataChanged();
	void onVideoGrabbed(int, bool);
	void onVideoSaved(int, bool);

	//To return control if no activity
	void onIdleTimer();

private:
	void updateInCtrlVisibleObjects(bool bInControl);
	void updateCtrlStatusLabel(bool bInControl);
	void initTakeControlButtons();
	void updateTakeControlButtons();

	MIhmDskVisibleObject * getVisibleVideoObject(int eVirtObjet);

	bool loadTemplate(QString sUIFilePath);
	void initializeVisibleObjects();
	bool connectVisibleObjects();
	void setAllVisibleObjIniSettings();
	bool setVisibleObjIniSettings(MIhmDskVisibleObject * pObject);
	void updateAllVisibleObjStaticProp();
	void checkResolution();
	
	void updateStaticImage(MIhmDskVisibleObject::enumVisibleObjectId eId, MIhmVirtIco* pVirtObj);
	void updateTextLabel(MIhmDskVisibleObject::enumVisibleObjectId eId, MIhmVirtText* pVirtObj);
	void updateDynImage(const QString &sImagePath, bool bShow, 
							MIhmDskVisibleObject::enumVisibleObjectId eVisContainerId, MIhmDskVisibleObject::enumVisibleObjectId eVisLabelName);
	void createClock();
	void createNumlockTimer();	

	void updateListObject(MIhmDskVisibleObject::enumVisibleObjectId eId, MIhmVirtList* pVirtObj);
	void updateFileView(MIhmDskVisibleObject::enumVisibleObjectId eId, MIhmVirtTableViewReq* pVirtObj);
	void updateHMenuView(MIhmDskVisibleObject::enumVisibleObjectId eId, MIhmVirtHMenuViewReq* pVirtObj);

	//video virtual objects processing
	void updateVideoView(MIhmDskVisibleObject::enumVisibleObjectId eId, MIhmVirtVideoViewReq* pVirtObj);
	void updateRVideoView(MIhmDskVisibleObject::enumVisibleObjectId eId, MIhmVirtVideoViewReq* pVirtObj);
	
	void updateContainer(MIhmDskVisibleObject::enumVisibleObjectId eId, bool bVisible, MIhmVirtContainer* pVirtObj);
	void updateButton(MIhmDskVisibleObject::enumVisibleObjectId eId, MIhmVirtButton* pVirtObj);
	void updateTabView(MIhmDskVisibleObject::enumVisibleObjectId eId, QStringList& lstTabText, int iCurrentIndex, bool bVisible);
	
	void removeItemsFromListView(QListWidget *pListWidget, QStringList &lstToRemove);
	void addItemsToListView(QListWidget *pListWidget, QStringList &lstToAdd, QFont &fnt, QString sForeColor);

	void handleMouseClick(QObject *obj);

	bool isAnyInputDlgInProgress();

	//for MIhmVirtInputDialogReq
	void sendInputDialogRsp(bool bCanceled, QString sInputRes);
	void sendInputDialogInitErr();
	void hideInputDialog();
	void raiseNextInputDialog(MIhmVirtInputDialogReq* pVirtObj);
	bool isInputDialogInProgress();
	void onInputDialogCMDCancel();
	void sendInputDlgCanceled();
	

	//	for MIhmVirtInputDlgExReq
	void hideInputDlgEx();
	void raiseNextInputDlgEx(MIhmVirtInputDlgExReq* pVirtObj);
	void sendInputDlgExRsp(bool bCanceled, QString sInputRes);
	void sendInputDlgExInitErr();
	void sendInputDlgExOppened();
	void sendLoginDlgOppened();
	void sendLoginDlgClosed();


	bool isInputDlgExInProgress();
	void onInputDlgExCMDCancel();
	void sendInputDlgExCanceled();

	void openFileOpenDlg();

	void setModalWindow(QWidget * p);
	QWidget * getModalWindow();
	void deleteModalWindow(QWidget ** p);

	QVector <MIhmDskVisibleObject*> m_vectDskVisibleObjects;

	bool m_bTemplateLoaded;
	
	QWidget *m_dynFormWidget;
	QString m_sDynaFormStyleSheet;

	QHBoxLayout *m_layout;
	QFrame   *m_mainFrame;

	QString m_sLaneType;
	MIhmDesktopIntf * m_pDskIntf;

	MDigitalClock *m_DigitalClock;
	
	QTimer *m_ptimNumLock;

	//to return control if no activity
	void createIdleTimer();
	void startIdleTimer();
	void stopIdleTimer();

	QTimer *m_pIdleTimer;



	MDskAsyncInputs *m_pDskAsyncInputs;

//Modal dialogs that are modal for real
	MFormAbout * m_pAboutDlg;
	MFormLogin * m_pLoginDialog;
	MFormTakeOverDlg * m_pCurrentClientDlg;
	
	//Taking control ...
	void showClientDialog(MFormTakeOverDlg::enumDialogType eType, QString sMsgTitle, QString sMsg);
	void hideClientDialog();
	//preloaded client dialogs to be used on desktop
	void prepareClientInfoDialogs();

	MFormTakeOverDlg * m_pTakeOverMsg; //enuTakeOverMsg
	MFormTakeOverDlg * m_pTakeOverQuestion; //enuTakeOverQuestion
	MFormTakeOverDlg * m_pClientInfoMsg; //enuClientInfoMsg
	MFormTakeOverDlg * m_pClientConnectingMsg; //enuClientConnectingMsg


//Modal dialogs that have to be simulated to be modal
	//Input dialog 
	MFormInput * m_pInputDialog;
	MInputDialogReq m_oCurrentInputReq;

	//Input dialog ex (from xml files and ui)
	MFormInputEx * m_pInputDlgEx;
	MInputDialogExReq m_oCurrentInputExReq;

	QWidget * m_pModalWin;

//Keyboard handling
	void saveEvent(QObject *target, QKeyEvent * event);
	void reemitSavedEvent();
	void doNotFilterNextEvent(QObject *target, QKeyEvent * pEvent);
	
//Keyboard testing
	bool isKeyboardTestingInProgress();
	void processKeyTest(bool bIsTollKey, bool bIsStringDetection, int iAskedKey, int iAsciiCode, QString sDetectedStr, QString sCoupleId);
	void setKeyboardTestingInProgress();
	void unsetKeyboardTestingInProgress();

	MStringDetection *m_pStringDetection;
	bool m_bFilterStringDetectedKeys;

	bool m_bDoNotFilter;
	QKeyEvent * m_pSavedEvent;
	QObject * m_pSavedEventTarget;
};

#endif

