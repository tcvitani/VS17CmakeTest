


#include "MIhmAniThread.h"
#include "MIhmAniInitData.h"
#include <QApplication>
#include <MIhmSrv.h>
#include <MIhmMsg.h>
#include <MIhmKey.h>
#include <MTracer.h>


extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include <ihm.H>
};


MIhmAniCommData::MIhmAniCommData()
{
	m_bChangeSignalEmitted = false;
	m_iNumIsSignalEmittedRejects = 0;
}


void MIhmAniCommData::setSignalEmitted() 
{ 
	m_bChangeSignalEmitted = true; 
	m_iNumIsSignalEmittedRejects = 0;
}

bool MIhmAniCommData::isSignalEmitted() 
{
	if (m_bChangeSignalEmitted)
	{
		m_iNumIsSignalEmittedRejects++;

		if (m_iNumIsSignalEmittedRejects < MAX_SIGNALS_FILTER_STEP)
			return true;
		else
		{
			TRACE_D(QString("MIhmAniCommData::isSignalEmitted reset for %1 times. Forcing to FALSE and reset m_iNumIsSignalEmittedRejects ").arg(MAX_SIGNALS_FILTER_STEP));

			m_iNumIsSignalEmittedRejects = 0;
			return false;
		}
	}
	else
		return false;
}

void MIhmAniCommData::resetSignalEmitted() 
{ 
	m_bChangeSignalEmitted = false; 
}



MIhmAniCommData::~MIhmAniCommData()
{
	while(!m_lstMessagesFromAni.isEmpty())
		delete m_lstMessagesFromAni.takeLast();

}


//-----------------------------------------------------------


MIhmAniThread::MIhmAniThread()
{
	m_bKeepRunning = true;
	m_pInitData = new MIhmAniInitData();

	m_pIhmSrv = new MIhmSrv();
	QObject::connect(m_pIhmSrv,SIGNAL(requestStop()),this, SIGNAL(stopRequested()));

	InitializeCriticalSection(&m_AniCriticalSection);	

}


MIhmAniThread::~MIhmAniThread()
{
	TRACE_D("MIhmAniThread::~MIhmAniThread: Closing the ANI thread...");

	delete m_pIhmSrv;
	delete m_pInitData;

	this->quit();
    wait();

	DeleteCriticalSection(&m_AniCriticalSection);	


}


void MIhmAniThread::stop()
{
	EnterCriticalSection(&m_AniCriticalSection); 
		m_bKeepRunning = false;
	LeaveCriticalSection(&m_AniCriticalSection);
}

void MIhmAniThread::run()
{
    struct_neutre  * psNeutre;   // Message de service recu
	struct_ihm_message sIhmMessage;

	TRACE_D("IhmAniThread::run... started ..." );

    // Initialisation de la tache et recherche de son identificateur
    IhmAniInit();

	bool bModelChangeDetected;

	EnterCriticalSection(&m_AniCriticalSection); 

	while(m_bKeepRunning)
	{
		LeaveCriticalSection(&m_AniCriticalSection);
		
		if (Recoit( m_iAniBalId, (struct_neutre **)(&psNeutre),5) == NOYAU_BAL_MESS) 
		{
			EnterCriticalSection(&m_AniCriticalSection); 

				memcpy(&sIhmMessage, ((struct_ihm_message*)psNeutre),sizeof(struct_ihm_message));
				m_pIhmSrv->IhmTreatMessageService(&sIhmMessage);//(struct_ihm_message *)(psNeutre)
				ExitLibere( (struct_neutre **)(&psNeutre) );

				//update the shared objects if anything changed 
				bModelChangeDetected = IhmAniVerifyVirtObjChange();
				
				if(bModelChangeDetected)
				{
					if(!m_AniCommData.isSignalEmitted())
					{   
						m_AniCommData.setSignalEmitted();
						emit virtualObjUpdated();
					}
				}

			LeaveCriticalSection(&m_AniCriticalSection);

			if(bModelChangeDetected)
					msleep(1);
		}
		
		
		EnterCriticalSection(&m_AniCriticalSection); 

	}
	
	LeaveCriticalSection(&m_AniCriticalSection);
    
	TRACE_D("IhmAniThread::run... exit..." );
}


//warning: this one assumes that mutex m_AniMutex is locked
void MIhmAniThread::IhmMessEnvoiAcquitement(IN noyau_bal_id iBalDest, 
                                       IN enum_srv_service eService,
                                       IN enum_srv_type eTypeMessage)
{
    struct_ihm_message * psMsgEmis = NULL;
	

	//Because of performance issues in NEP project that is using many objects
	//the sending of acknowledgments is reduced to speed up the processing of messages
	if(eService == M_SRV_ARRET || eTypeMessage == SRV_TYP_SET_NACQ || eTypeMessage == SRV_TYP_GET_NACQ)
	{
		LeaveCriticalSection(&m_AniCriticalSection);

			TRACE_D(QString("MIhmAniThread::IhmMessEnvoiAcquitement ==> Service:%1;MsgType:%2")
														.arg(eTypeMessage).arg(eTypeMessage));

			ExitAlloue(( struct_neutre ** )( &psMsgEmis ),
				sizeof( struct_ihm_message ),
				getPoolId() );

			psMsgEmis->entete.service = (enum_srv_service)eService;
			psMsgEmis->entete.type_message = (enum_srv_type)eTypeMessage;

			ExitEnvoie(iBalDest, getAniBalId(), (struct_neutre *)psMsgEmis );

		EnterCriticalSection(&m_AniCriticalSection); 
	}

}

//warning: this one assumes that mutex m_AniMutex is locked
void MIhmAniThread::MyExitEnvoie(QString sCallStack, noyau_bal_id num_bal_destinataire,
                                        noyau_bal_id bal_retour,
                                        struct_neutre * p_neutre) 
{
	noyau_enum_retour err;

	LeaveCriticalSection(&m_AniCriticalSection);

		err = Envoie(num_bal_destinataire, bal_retour, p_neutre);

		if(err!= NOYAU_OK)
		{
			TRACE_W(QString("FATAL ERROR: MIhmAniThread::MyExitEnvoie::called from %1! Calling ExitBAD...").arg(sCallStack));
			ExitBad();
		}

	EnterCriticalSection(&m_AniCriticalSection);

}	
		
void MIhmAniThread::MyExitAlloue(QString sCallStack, struct_neutre ** pp_neutre,
                                        noyau_taille_bloc longueur,
                                        noyau_pool_id numero_pool)
{
	noyau_enum_retour err;

	err = Alloue(pp_neutre, longueur, numero_pool);
	if(err!= NOYAU_OK)
	{
		TRACE_W(QString("FATAL ERROR: MIhmAniThread::MyExitAlloue::called from %1! Calling ExitBAD...").arg(sCallStack));
		ExitBad();
	}

}


bool MIhmAniThread::Initialize(QString szMboxName, QString sConfigKey)
{
	m_pInitData->initialize(szMboxName, sConfigKey);
	m_pIhmSrv->Initialize(this, &m_VirtObjects);

	return true;
}


void MIhmAniThread::IhmAniInit()
{
    TRACE_D("IhmAniThread::IhmAniInit" );

    // publication de la BAL ANI
	QString mbox = m_pInitData->getMboxName();
    
	m_iAniBalId = PublieBAL( mbox.toLatin1().data(), NOYAU_BAL_ILLIMITEE );
    if ( m_iAniBalId <= 0 )
    {
        TRACE_W(QString("MIhmAniThread::IhmAniInit: PublieBAL retourne %1").
										arg((int)m_iAniBalId));
        emit stopRequested();
		return;
    }

//if virtual keyboard enabled 
//	AttendBal
//  call debut service
//  else return...
	if(m_pInitData->isVirtualKeyboardEnabled())
	{
		QString mVirtKybBox = m_pInitData->getVirtualKeyboardMboxName();
		
		m_iVirtKybBalId = AttendBAL( mVirtKybBox.toLatin1().data());
    
		if ( m_iVirtKybBalId <= 0 )
		{
			TRACE_W(QString("MIhmAniThread::IhmAniInit: Error starting VirtKyb... "));
			return;
		}
		else
		{
			//start service ...
			m_VirtualKeyboard.Initialize(this);

			TRACE_D(QString("MIhmAniThread::IhmAniInit: VirtKyb initialized OK... "));
		}

	}

    TRACE_D("IhmAniThread::IhmAniInit: Initialized OK!" );
}

void MIhmAniThread::VirtualKyb_Hide()
{
	if(m_pInitData->isVirtualKeyboardEnabled())
		m_VirtualKeyboard.Hide();

}

void MIhmAniThread::VirtualKyb_Show(QString sCfg, int x, int y)
{
	if(m_pInitData->isVirtualKeyboardEnabled())
		m_VirtualKeyboard.Show(sCfg, x, y);
}

void MIhmAniThread::VirtualKyb_Show() //show default virt keyboard
{
	if(m_pInitData->isVirtualKeyboardEnabled())
		m_VirtualKeyboard.Show("", -1, -1);
}


bool MIhmAniThread::IhmAniVerifyVirtObjChange()
{
	return m_VirtObjects.getIfObjectsChanged();
}


//The function is used to read the virtual objects from the ANI thread
// The virtual objects are used to transfer data issued by the Lane application commands
// and send it to the TCL interface for processing (to show hide change visible objects)
//Working: It lock the mutex, gets the updated objects and removes the read objects from the shared list
bool MIhmAniThread::copyUpdatedVirtObjects(MIhmVirtObjectsModel * pGUIModel)
{
	bool bAnyNew = false;
	
	EnterCriticalSection(&m_AniCriticalSection); 
		bAnyNew = m_VirtObjects.copyUpdatedVirtObjects(pGUIModel, true);
		m_AniCommData.resetSignalEmitted();
	LeaveCriticalSection(&m_AniCriticalSection);
	
	return bAnyNew;
}


void MIhmAniThread::emitMessageFromAni(MIhmMsg* pMsg)
{
    TRACE_D(QString( "MIhmAniThread::emitMessageFromAni: Message %1 from ANI sent!").
										arg((int)pMsg->getType()));
	
	m_AniCommData.m_lstMessagesFromAni.append(pMsg);

	emit messageFromAni();
}	



bool MIhmAniThread::getMessagesFromAni(QList <MIhmMsg*> *pList)
{
	MIhmMsg * p;	
	bool bAnyNew = false;
	
	EnterCriticalSection(&m_AniCriticalSection); 

		//get only one message at a time
		 if(!m_AniCommData.m_lstMessagesFromAni.isEmpty())
		 {	 
			p = m_AniCommData.m_lstMessagesFromAni.takeFirst();

			if (p!=NULL)
			{
				pList->append(p);//copy the object 
				bAnyNew = true;
			}
		 }

	LeaveCriticalSection(&m_AniCriticalSection);
	
	return bAnyNew;
}


void MIhmAniThread::sendMessageToAni(MIhmMsg* pMsg)
{
	bool bChangeDetected = false;

	EnterCriticalSection(&m_AniCriticalSection); 

		if(pMsg!=NULL)
		{
			m_pIhmSrv->IhmTreatMessageTCL(pMsg);
			delete pMsg;
		}

		//if the message processing updated the virtual objects emit the change so it can be rendered at GUI
		bChangeDetected = IhmAniVerifyVirtObjChange(); 

		if(bChangeDetected)
		{
			if(!m_AniCommData.isSignalEmitted())
			{   
				m_AniCommData.setSignalEmitted();
				emit virtualObjUpdated();
			}
		}

	LeaveCriticalSection(&m_AniCriticalSection);

}

