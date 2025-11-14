#ifndef MIHM_ANI_THREAD_H
#define MIHM_ANI_THREAD_H

#include <qthread.h>
#include <MIhmAniInitData.h>
#include <MIhmAniVirtObjects.h>
#include <MIhmVirtKyb.h>


extern "C" {
	#include <noyau.h>
}

class MIhmSrv;
class MIhmMsg;

#define MAX_SIGNALS_FILTER_STEP 100

class MIhmAniCommData
{
public:
	MIhmAniCommData();
	~MIhmAniCommData();

	void setSignalEmitted();
	
	bool isSignalEmitted();

	void resetSignalEmitted();

	QList <MIhmMsg*> m_lstMessagesFromAni; //messages from ANI to TCL interface

private:
	bool m_bChangeSignalEmitted;
	int m_iNumIsSignalEmittedRejects;
};

/*!
	\class MIhmAniThread
	\brief Ani thread class publish the module mailbox and runs the message loop.

	The class reads the configuration from registry and creates an instance of the MIhmAniThread
	and MIhmTCLInterfaces. All the communication between the two sides is done trough the signal slot mechanism
	of the MIhmMainLogic class.
	
	\author Tihomir Cvitanic
	\version 1.0
	\date    2010/01/20
	\sa  
*/

class MIhmAniThread:public QThread
{
    Q_OBJECT
public:
	MIhmAniThread();
	~MIhmAniThread();
	
	void run();	
	bool Initialize(QString szMboxName, QString sConfigKey);

	noyau_bal_id getAniBalId() {return m_iAniBalId;};
	noyau_bal_id getVirtKybBalId() {return m_iVirtKybBalId;};
		
	noyau_pool_id getPoolId() {return m_pInitData->getPoolId();};
	QString& getModuleConfigKey() {return m_pInitData->getModuleConfigKey();};
	DWORD getMaxVideoZoom() {return m_pInitData->getMaxVideoZoom();};
	
	//lock the mutex, copies the updated objects 
	bool copyUpdatedVirtObjects(MIhmVirtObjectsModel * pGUIModel);
	
	//lock the mutex, copies the updated objects and deletes
	// from the list of updated objects
	bool getMessagesFromAni(QList <MIhmMsg*> *pList);

	void sendMessageToAni(MIhmMsg* pMsg);
	void emitMessageFromAni(MIhmMsg* pMsg);	
	
	MIhmAniInitData * getAniInitData(){return m_pInitData;};

	void IhmMessEnvoiAcquitement(IN noyau_bal_id iBalDest, 
                                   IN enum_srv_service eService,
                                   IN enum_srv_type eTypeMessage);
	void MyExitEnvoie(QString sCallStack, noyau_bal_id num_bal_destinataire,
											noyau_bal_id bal_retour,
											struct_neutre * p_neutre);
	void MyExitAlloue(QString sCallStack, struct_neutre ** pp_neutre,
											noyau_taille_bloc longueur,
											noyau_pool_id numero_pool);

	void VirtualKyb_Hide();
	void VirtualKyb_Show(QString sCfg, int x, int y);
	void VirtualKyb_Show();

signals:
	void virtualObjUpdated();
	void stopRequested();
	void messageFromAni();

public slots:
	void stop();

private:
	void IhmAniInit();
	bool IhmAniVerifyVirtObjChange();

	bool m_bKeepRunning;
	noyau_bal_id m_iAniBalId;
	noyau_bal_id m_iVirtKybBalId;

	MIhmAniInitData *	m_pInitData;
	MIhmSrv *			m_pIhmSrv;
	MIhmAniVirtObjects	m_VirtObjects;
	MIhmAniCommData		m_AniCommData;
	MIhmVirtKyb			m_VirtualKeyboard;

	CRITICAL_SECTION  m_AniCriticalSection;
};


#endif


