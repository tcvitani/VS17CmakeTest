// SimuSendReciveThread.cpp: implementation of the SimuSendReciveThread class.
//
//////////////////////////////////////////////////////////////////////

#include <QFile>
#include <QDomDocument>
#include <QTime>
#include <QTextStream>
#include <QCoreApplication>


extern "C"
{
	#include <noyau.h>
	#include <virt_kyb.h>
}


#include "SimuSendReciveThread.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
quint64 SimuSendReciveThread::m_iInstanceCounter = 0;


SimuSendReciveThread::SimuSendReciveThread()
{
	m_iInstanceCounter = QCoreApplication::applicationPid();

	m_iCountMessagesEvents = 0;
}

SimuSendReciveThread::~SimuSendReciveThread()
{

}

void SimuSendReciveThread::initaialize(QString sTargetMboxName)
{
	m_sTargetMboxName = sTargetMboxName;
}

void SimuSendReciveThread::run()
{
    SetTextToMainDlg(QString("SIMU: Creation/Publication of response BAL..."));
	
	m_sSimuBalName = QString("SIMU_%1%2").arg(m_sTargetMboxName).arg(m_iInstanceCounter);

	// create
    if((m_stSimuData.m_piBalSimu = PublieBAL(m_sSimuBalName.toAscii().data(), NOYAU_BAL_ILLIMITEE)) < 0)
    {
		SetTextToMainDlg(QString("SIMU ERROR: PublieBAL() return error for BAL %1").arg(m_sSimuBalName));
		return;
    }

	SetTextToMainDlg(QString("SIMU: %1 BAL created").arg(m_sSimuBalName));
    SetTextToMainDlg(QString("SIMU: Connection to Virt Keyboard  ..."));
		
    SetTextToMainDlg(QString("SIMU: Waiting BAL=%1 ...").arg(m_sTargetMboxName));
		
	m_stSimuData.m_piBalModule = AttendBAL(m_sTargetMboxName.toAscii().data()) ;
    SetTextToMainDlg(QString("OK : No. of BAL = %1 ").arg(m_stSimuData.m_piBalModule));
		
	m_stSimuData.m_bAlive = true;
    SetTextToMainDlg(QString("SIMU: instances connected."));

	while(m_stSimuData.m_bAlive)
	{
		readRecivedMailslot();

		msleep(3);
	}
	
	
    SetTextToMainDlg(QString("SIMU: start ended ..."));
}




void SimuSendReciveThread::readRecivedMailslot()
{
    noyau_enum_retour				eNoyRet;
    struct_VIRT_KYB_message *		psMessage = NULL;
	
    // Watch for new message
    eNoyRet = RecoitMs(m_stSimuData.m_piBalSimu, (struct_neutre **)&psMessage, 10);
	
    if(eNoyRet == NOYAU_BAL_MESS)
    {

		QString sMsg = generateReadableMessage(psMessage);
        SetTextToMainDlg(QString("IN: %1:").arg(sMsg));
		
        switch(psMessage->entete.service)
        {
			case M_SRV_ARRET:
				switch (psMessage->entete.type_message)
				{
					// IHM is dead!
					// simulator can be swiched off.
				case SRV_TYP_ARRET_ACQ :
					m_stSimuData.m_bAlive = false;
					break;
				}
				break;

		}
		
        ExitLibere((struct_neutre **)&psMessage);
    }
}

bool SimuSendReciveThread::sendMessage(enum_srv_service service, enum_srv_type eTypeMessage, void *data)
{
	bool bRet = true;
	
    noyau_enum_retour   eNoyRet;
    struct_VIRT_KYB_message  * p_msg_emis = NULL;
    
    eNoyRet = Alloue((struct_neutre **)(&p_msg_emis), sizeof(struct_VIRT_KYB_message), 0);
	
    if(eNoyRet == NOYAU_OK)
    {
        p_msg_emis->entete.service = (enum_srv_service)service;
        p_msg_emis->entete.type_message = (enum_srv_type)eTypeMessage;

		switch(service)
		{
		case M_SRV_ARRET:
			// type SRV_TYP_DEMANDE dont need anything aditional
			break;
		case M_VIRT_KYB_AFFICHAGE:
			switch(eTypeMessage)
			{
			// types SRV_TYP_DEBUT and SRV_TYP_FIN dont need anything aditional
			//case SRV_TYP_DEBUT:
			//	break;
			//case SRV_TYP_FIN:
			//	break;
			case VIRT_KYB_INIT_CONFIG:
			case VIRT_KYB_AFF_MOVE:
			case VIRT_KYB_AFF_SHOW:
				memcpy(&p_msg_emis->u.srv_affichage, (struct_VIRT_KYB_srv_affichage*)data, sizeof(struct_VIRT_KYB_srv_affichage));
				break;
			}
			break;


		default:
			break;
		}
        
		eNoyRet = Envoie(m_stSimuData.m_piBalModule, m_stSimuData.m_piBalSimu, (struct_neutre *)(p_msg_emis));
        
        if(eNoyRet != NOYAU_OK)
		{
            SetTextToMainDlg(QString("SIMU ERREUR: sendMessage() retourne %1").arg(eNoyRet));
			ExitLibere((struct_neutre **)&p_msg_emis);
			bRet = false;
		}
        
    }
    else
    {
        SetTextToMainDlg(QString("SIMU ERREUR: Alloue() retourne %1").arg(eNoyRet));
		bRet = false;
    }
	
	return bRet;
}


void SimuSendReciveThread::sendInitConfig(QString sKeybID)
{
	struct_VIRT_KYB_srv_affichage sAff={0};
	
	strncpy((char*)sAff.keyboard_id,(char*)sKeybID.toAscii().data(), sizeof(sAff.keyboard_id));
	sendMessage((enum_srv_service)M_VIRT_KYB_AFFICHAGE, (enum_srv_type)VIRT_KYB_INIT_CONFIG, &sAff);
}

void SimuSendReciveThread::sendAffMove(int x, int y)
{
	struct_VIRT_KYB_srv_affichage sAff={0};

	sAff.dwPosX = x;
	sAff.dwPosY = y;
	sendMessage((enum_srv_service)M_VIRT_KYB_AFFICHAGE, (enum_srv_type)VIRT_KYB_AFF_MOVE, &sAff);

}

void SimuSendReciveThread::sendAffHide()
{
	struct_VIRT_KYB_srv_affichage sAff={0};
	
	sAff.bShow = FALSE;
	sendMessage((enum_srv_service)M_VIRT_KYB_AFFICHAGE, (enum_srv_type)VIRT_KYB_AFF_SHOW, &sAff);

}

void SimuSendReciveThread::sendAffShow()
{
	struct_VIRT_KYB_srv_affichage sAff={0};
	
	sAff.bShow = TRUE;
	sendMessage((enum_srv_service)M_VIRT_KYB_AFFICHAGE, (enum_srv_type)VIRT_KYB_AFF_SHOW, &sAff);
	
}

void SimuSendReciveThread::sendAffDEBUT()
{
	sendMessage((enum_srv_service)M_VIRT_KYB_AFFICHAGE, SRV_TYP_DEBUT, NULL);

}

void SimuSendReciveThread::sendAffFIN()
{
	sendMessage((enum_srv_service)M_VIRT_KYB_AFFICHAGE, SRV_TYP_FIN, NULL);
}

void SimuSendReciveThread::sendArret()
{
	sendMessage(M_SRV_ARRET, SRV_TYP_DEMANDE, NULL);
	
}




QString SimuSendReciveThread::generateReadableMessage(struct_VIRT_KYB_message  * psMessage)
{
	QString sMessage;

       switch(psMessage->entete.service)
        {
        case M_SRV_ARRET:
            switch (psMessage->entete.type_message)
            {
				// IHM is dead!
				// simulator can be swiched off.
				case SRV_TYP_ARRET_ACQ :
					sMessage += QString("ARRET done!");
					break;
 				default:
					sMessage = QString(" M_SRV_ARRET:  TYP: %1!!!").arg( psMessage->entete.type_message);
           }
           break;
            
		case M_VIRT_KYB_AFFICHAGE:
			switch (psMessage->entete.type_message)
			{
			case SRV_TYP_SET_ACQ:
				sMessage += QString(" M_VIRT_KYB_AFFICHAGE: SRV_TYP_SET_ACQ");
				break;
			case SRV_TYP_SET_NACQ:  
				sMessage += QString(" M_VIRT_KYB_AFFICHAGE: SRV_TYP_SET_NACQ");
				break;
			case VIRT_KYB_INIT_CONFIG:
				sMessage += QString(" M_VIRT_KYB_AFFICHAGE: VIRT_KYB_INIT_CONFIG");
				break;
			case VIRT_KYB_AFF_MOVE:
				sMessage += QString(" M_VIRT_KYB_AFFICHAGE: VIRT_KYB_AFF_MOVE");
				break;
			case VIRT_KYB_AFF_SHOW:
				sMessage += QString(" M_VIRT_KYB_AFFICHAGE: VIRT_KYB_AFF_SHOW");
				break;
			default:
				sMessage = QString(" M_VIRT_KYB_AFFICHAGE:  TYP: %1!!!").arg( psMessage->entete.type_message);

			}
			break;
            

		default:
			sMessage = QString("SRV UNKNOWN:%1  TYP: %1!!!").arg( psMessage->entete.type_message);
		}
		
	return sMessage;
}



void SimuSendReciveThread::SetTextToMainDlg(QString &szText)
{
//	incMsgsEvents();

	emit message(QString("%1").arg(szText));
//	QCoreApplication::processEvents();
}



QString SimuSendReciveThread::SimuGetMsgType(enum_srv_type eTypeMessage)
{
	QString szReturn;
	
    switch( eTypeMessage )
    {
    case SRV_TYP_DEBUT              : szReturn = "SRV_TYP_DEBUT";					break;
    case SRV_TYP_FIN                : szReturn = "SRV_TYP_FIN";						break;
    case SRV_TYP_DEMANDE            : szReturn = "SRV_TYP_DEMANDE";					break;
    case SRV_TYP_DEMANDE_ANNULATION : szReturn = "SRV_TYP_DEMANDE_ANNULATION";		break;
    case SRV_TYP_SET                : szReturn = "SRV_TYP_SET";						break;
    case SRV_TYP_SET_ACQ            : szReturn = "SRV_TYP_SET_ACQ";					break;
    case SRV_TYP_SET_NACQ           : szReturn = "SRV_TYP_SET_NACQ";				break;
    case SRV_TYP_GET                : szReturn = "SRV_TYP_GET";						break;
    case SRV_TYP_GET_ACQ            : szReturn = "SRV_TYP_GET_ACQ";					break;
    case SRV_TYP_GET_NACQ           : szReturn = "SRV_TYP_GET_NACQ";				break;
    case SRV_TYP_DEBUT_ACQ          : szReturn = "SRV_TYP_DEBUT_ACQ";				break;
    case SRV_TYP_DEBUT_NACQ         : szReturn = "SRV_TYP_DEBUT_NACQ";				break;
    case SRV_TYP_FIN_ACQ            : szReturn = "SRV_TYP_FIN_ACQ";					break;
    case SRV_TYP_FIN_NACQ           : szReturn = "SRV_TYP_FIN_NACQ";				break;
    case SRV_TYP_DEMANDE_ACQ        : szReturn = "SRV_TYP_DEMANDE_ACQ";				break;
    case SRV_TYP_DEMANDE_NACQ       : szReturn = "SRV_TYP_DEMANDE_NACQ";			break;
    case SRV_TYP_ARRET_ACQ          : szReturn = "SRV_TYP_ARRET_ACQ";				break;
    case SRV_TYP_ARRET_NACQ         : szReturn = "SRV_TYP_ARRET_NACQ";				break;
    case SRV_TYP_NOUVEL_ETAT        : szReturn = "SRV_TYP_NOUVEL_ETAT";				break;
    case VIRT_KYB_INIT_CONFIG       : szReturn = "VIRT_KYB_INIT_CONFIG";			break;
    case VIRT_KYB_AFF_MOVE			: szReturn = "VIRT_KYB_AFF_MOVE";				break;
    case VIRT_KYB_AFF_SHOW			: szReturn = "VIRT_KYB_AFF_SHOW";				break;
    default                         : szReturn = QString("Unknown: %1").arg(eTypeMessage);	break;
    }
	
	return szReturn;
}

