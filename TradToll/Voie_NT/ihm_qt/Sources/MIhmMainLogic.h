#ifndef MIHM_MAIN_LOGIC_H
#define MIHM_MAIN_LOGIC_H

#include <qobject.h>
#include <QTimer.h>

#include <MIhmConfigGeneral.h>

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
}


class MIhmAniThread;
class MIhmTCLInterfaces;
class MIhmVirtObjectsModel;
class QTimer;
class MIhmMsgStringDetection;
class MIhmMsgInputDialogRes;
class MIhmMsg;

/*!
	\class MIhmMainLogic
	\brief The core class of the IHM module

	The class reads the configuration from registry and creates an instance of the MIhmAniThread
	and MIhmTCLInterfaces. All the communication between the two sides is done trough the signal slot mechanism
	of this class. 
	
	\author Tihomir Cvitanic
	\version 1.0
	\date    2010/01/20
	\sa  
*/
class MIhmMainLogic:public QObject
{
    Q_OBJECT
public:
	MIhmMainLogic();
	~MIhmMainLogic();
	
	enum_instance_result Initialize(QString sBlName);
	bool InitializeTCLInterfaces();
	
	void sendMessageToAni(MIhmMsg * pMsg);

	static void initTrace(QString sModuleMbox);
	static void deinitTrace();
	
public slots:
	void exitProgram();
	void onAniStopRequested();
	void onStartTimerTimeout();
	
	void onAniVirtualObjUpdated();
	void onMessageFromAni();
	void onInitOK();

private:
	
	bool loadLanguages();

	MIhmConfigGeneral m_oConfGenData;	

	MIhmAniThread * m_pAniThread;
	MIhmTCLInterfaces * m_pTCLInterfaces;
	
	MIhmVirtObjectsModel *m_pTransVirtObjectModel;
	
	QTimer m_oTimerStart;
	bool m_bModuleInitOK;


};


#endif


