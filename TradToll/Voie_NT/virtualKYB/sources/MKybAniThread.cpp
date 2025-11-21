


#include "MKybAniThread.h"
#include "MKybAniInitData.h"
#include <QApplication>
#include <MKybSrv.h>
#include <MKybMsg.h>
#include <MTracer.h>

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include <virt_kyb.H>
};


MKybAniCommData::MKybAniCommData()
{
	m_bChangeSignalEmitted = false;
}

MKybAniCommData::~MKybAniCommData()
{
	while(!m_lstMessagesFromAni.isEmpty())
		delete m_lstMessagesFromAni.takeLast();

}



MKybAniThread::MKybAniThread()
{
	m_bKeepRunning = true;
	m_pInitData = new MKybAniInitData();
	
	m_pKybSrv = new MKybSrv(this);

	QObject::connect(m_pKybSrv,SIGNAL(requestStop()),this, SIGNAL(stopRequested()));
	
	InitializeCriticalSection(&m_AniCriticalSection);	

}


MKybAniThread::~MKybAniThread()
{
	TRACE_D("MKybAniThread::~MKybAniThread: Closing the ANI thread...");

	delete m_pKybSrv;
	delete m_pInitData;

	this->quit();
    wait();

	DeleteCriticalSection(&m_AniCriticalSection);	


}


void MKybAniThread::stop()
{
	EnterCriticalSection(&m_AniCriticalSection); 
		m_bKeepRunning = false;
	LeaveCriticalSection(&m_AniCriticalSection);
}

void MKybAniThread::run()
{
    struct_neutre  * psNeutre;   // Message de service reçu
	struct_VIRT_KYB_message sKybMessage;

	TRACE_D("KybAniThread::run... started ..." );

    // Initialisation de la tache et recherche de son identificateur
    KybAniInit();

	EnterCriticalSection(&m_AniCriticalSection); 

	while(m_bKeepRunning)
	{
		LeaveCriticalSection(&m_AniCriticalSection);
		
		if (Recoit( m_iAniBalId, (struct_neutre **)(&psNeutre),5) == NOYAU_BAL_MESS) 
		{
			EnterCriticalSection(&m_AniCriticalSection); 

				memcpy(&sKybMessage, ((struct_VIRT_KYB_message*)psNeutre),sizeof(struct_VIRT_KYB_message));
				m_pKybSrv->KybTreatMessageService(&sKybMessage);//(struct_Kyb_message *)(psNeutre)
				ExitLibere( (struct_neutre **)(&psNeutre) );

			LeaveCriticalSection(&m_AniCriticalSection);

		}
		
		
		EnterCriticalSection(&m_AniCriticalSection); 

	}
	
	LeaveCriticalSection(&m_AniCriticalSection);
    
	TRACE_D("KybAniThread::run... exit..." );
}


//warning: this one assumes that mutex m_AniMutex is locked
void MKybAniThread::KybMessEnvoiAcquitement(IN noyau_bal_id iBalDest, 
                                       IN enum_srv_service eService,
                                       IN enum_srv_type eTypeMessage)
{
    struct_VIRT_KYB_message * psMsgEmis = NULL;
	

	//Because of performance issues in NEP project that is using many objects
	//the sending of acknowledgments is reduced to speed up the processing of messages
	if(eService == M_SRV_ARRET || eTypeMessage == SRV_TYP_SET_NACQ || eTypeMessage == SRV_TYP_GET_NACQ)
	{
		LeaveCriticalSection(&m_AniCriticalSection);

			TRACE_D(QString("MKybAniThread::KybMessEnvoiAcquitement ==> Service:%1;MsgType:%2")
														.arg(eTypeMessage).arg(eTypeMessage));

			ExitAlloue(( struct_neutre ** )( &psMsgEmis ),
				sizeof( struct_VIRT_KYB_message ),
				getPoolId() );

			psMsgEmis->entete.service = (enum_srv_service)eService;
			psMsgEmis->entete.type_message = (enum_srv_type)eTypeMessage;

			ExitEnvoie(iBalDest, getAniBalId(), (struct_neutre *)psMsgEmis );

		EnterCriticalSection(&m_AniCriticalSection); 
	}

}

//warning: this one assumes that mutex m_AniMutex is locked
void MKybAniThread::MyExitEnvoie(QString sCallStack, noyau_bal_id num_bal_destinataire,
                                        noyau_bal_id bal_retour,
                                        struct_neutre * p_neutre) 
{
	noyau_enum_retour err;

	LeaveCriticalSection(&m_AniCriticalSection);

		err = Envoie(num_bal_destinataire, bal_retour, p_neutre);

		if(err!= NOYAU_OK)
		{
			TRACE_W(QString("FATAL ERROR: MKybAniThread::MyExitEnvoie::called from %1! Calling ExitBAD...").arg(sCallStack));
			ExitBad();
		}

	EnterCriticalSection(&m_AniCriticalSection);

}	
		
void MKybAniThread::MyExitAlloue(QString sCallStack, struct_neutre ** pp_neutre,
                                        noyau_taille_bloc longueur,
                                        noyau_pool_id numero_pool)
{
	noyau_enum_retour err;

	err = Alloue(pp_neutre, longueur, numero_pool);
	if(err!= NOYAU_OK)
	{
		TRACE_W(QString("FATAL ERROR: MKybAniThread::MyExitAlloue::called from %1! Calling ExitBAD...").arg(sCallStack));
		ExitBad();
	}

}


bool MKybAniThread::Initialize(QString szMboxName, QString sConfigKey)
{
	m_pInitData->initialize(szMboxName, sConfigKey);

	return true;
}


void MKybAniThread::KybAniInit()
{
    TRACE_D("KybAniThread::KybAniInit" );

    // publication de la BAL ANI
	QString mbox = m_pInitData->getMboxName();
    
	m_iAniBalId = PublieBAL( mbox.toLatin1().data(), NOYAU_BAL_ILLIMITEE );
    if ( m_iAniBalId <= 0 )
    {
        TRACE_W(QString("MKybAniThread::KybAniInit: PublieBAL retourne %1").
										arg((int)m_iAniBalId));
        emit stopRequested();
		return;
    }

    TRACE_D("KybAniThread::KybAniInit: Initialized OK!" );
}


void MKybAniThread::emitMessageFromAni(MKybMsg* pMsg)
{
//     TRACE_D(QString( "MKybAniThread::emitMessageFromAni: Message %1 from ANI sent!").
// 										arg((int)pMsg->getType()));

	m_AniCommData.m_lstMessagesFromAni.append(pMsg);

	emit messageFromAni();
}	



bool MKybAniThread::getMessagesFromAni(QList <MKybMsg*> *pList)
{
	MKybMsg * p;	
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


void MKybAniThread::sendMessageToAni(MKybMsg* pMsg)
{
	bool bChangeDetected = false;

	EnterCriticalSection(&m_AniCriticalSection); 

		if(pMsg!=NULL)
		{
			m_pKybSrv->KybTreatMessageTCL(pMsg);
			delete pMsg;
		}

	LeaveCriticalSection(&m_AniCriticalSection);

}

