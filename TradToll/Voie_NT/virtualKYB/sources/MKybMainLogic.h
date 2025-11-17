#ifndef MKyb_MAIN_LOGIC_H
#define MKyb_MAIN_LOGIC_H

#include <QObject.h>
#include <QPoint>
#include <MKybConfigGeneral.h>

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
}


class MKybAniThread;
class MKybMsg;
class MKybMsgKybConfig;
class MKybMsgKybShow;
class MKybMsgKybMove;
class MKybFormMain;

/*!
	\class MKybMainLogic
	\brief The core class of the VIRT_KYB module

	The class reads the configuration from registry and creates an instance of the MKybAniThread.
	All the communication between the two sides is done trough the signal slot mechanism
	of this class. 
	
	\author Tihomir Cvitanic
	\version 1.0
	\date    2014/01/20
	\sa  
*/
class MKybMainLogic:public QObject
{
    Q_OBJECT
public:
	MKybMainLogic();
	~MKybMainLogic();
	
	enum_instance_result Initialize(QString sBlName, QString sMode = QString(""));
	bool InitializeTCLInterfaces();
	
	void sendMessageToAni(MKybMsg * pMsg);

	static void initTrace(QString sModuleMbox);
	static void deinitTrace();
	
	void showAlwaysOnTopMenu();
public slots:
	void exitProgram();
	void onAniStopRequested();
	void onKybFormStopRequested();
	
	void onMessageFromAni();
private:
	void processMessageFromAni(MKybMsg* pMsg);
	void processConfigKeyboardTypeMsg(MKybMsgKybConfig* pConfigMsg);
	bool processConfigKeyboardType(QString sNewKeybType);

	void processConfigShowMsg(MKybMsgKybShow* pMsgShow);
	void processConfigShow(bool bShow);

	void processConfigMove(MKybMsgKybMove* pMsgMove);

	MKybConfigGeneral m_oConfGenData;	
	MKybAniThread * m_pAniThread;

	MKybFormMain * m_pMainDialog;

	QString m_sCurrentKyboardTypeID;
	QPoint m_ptCurrentPosition;

	bool m_bModuleInitOK;
	bool m_bStandaloneMode;

};


#endif


