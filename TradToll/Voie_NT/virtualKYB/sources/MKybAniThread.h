#ifndef MKyb_ANI_THREAD_H
#define MKyb_ANI_THREAD_H

#include <qthread.h>
#include <MKybAniInitData.h>
#include <QList>


extern "C" {
	#include <noyau.h>
	#include <module.h>

}

class MKybSrv;
class MKybMsg;

class MKybAniCommData
{
public:
	MKybAniCommData();
	~MKybAniCommData();

	void setSignalEmitted() {m_bChangeSignalEmitted = true;};
	bool isSignalEmitted() const { return m_bChangeSignalEmitted;};
	void resetSignalEmitted() {m_bChangeSignalEmitted = false;};

	QList <MKybMsg*> m_lstMessagesFromAni; //messages from ANI to TCL interface

private:
	bool m_bChangeSignalEmitted;
};

/*!
	\class MKybAniThread
	\brief Ani thread class publish the module mailbox and runs the message loop.

	\author Tihomir Cvitanic
	\version 1.0
	\date    2014/01/20
*/

class MKybAniThread:public QThread
{
    Q_OBJECT
public:
	MKybAniThread();
	~MKybAniThread();
	
	void run();	
	bool Initialize(QString szMboxName, QString sConfigKey);
	void KybAniInit();

	noyau_bal_id getAniBalId() {return m_iAniBalId;};
	noyau_pool_id getPoolId() {return m_pInitData->getPoolId();};
	QString& getModuleConfigKey() {return m_pInitData->getModuleConfigKey();};
	
	//lock the mutex, copies the updated objects and deletes
	// from the list of updated objects
	bool getMessagesFromAni(QList <MKybMsg*> *pList);

	void sendMessageToAni(MKybMsg* pMsg);
	void emitMessageFromAni(MKybMsg* pMsg);	
	
	MKybAniInitData * getAniInitData(){return m_pInitData;};

	void KybMessEnvoiAcquitement(IN noyau_bal_id iBalDest, 
								IN enum_srv_service eService,
								IN enum_srv_type eTypeMessage);

	void MyExitEnvoie(QString sCallStack, noyau_bal_id num_bal_destinataire,
											noyau_bal_id bal_retour,
											struct_neutre * p_neutre);
	void MyExitAlloue(QString sCallStack, struct_neutre ** pp_neutre,
											noyau_taille_bloc longueur,
											noyau_pool_id numero_pool);

signals:
	void stopRequested();
	void messageFromAni();

public slots:
	void stop();

private:

	bool m_bKeepRunning;
	noyau_bal_id m_iAniBalId;


	MKybAniInitData *	m_pInitData;
	MKybSrv *			m_pKybSrv;
	MKybAniCommData		m_AniCommData;

	CRITICAL_SECTION  m_AniCriticalSection;
};


#endif


