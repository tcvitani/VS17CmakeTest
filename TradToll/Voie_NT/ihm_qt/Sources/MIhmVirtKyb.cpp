

#include "MIhmAniThread.h"

 

extern "C" {
	#include <ihm.H>
	#include "ihm_loc.h"
	#include "virt_kyb.h"
	
};

MIhmVirtKyb::MIhmVirtKyb()
{
	m_pAniThread = NULL;
}

MIhmVirtKyb::~MIhmVirtKyb()
{

}

void MIhmVirtKyb::Stop()
{
	sendArret();
}

void MIhmVirtKyb::Initialize(MIhmAniThread * pAniThread)
{
	m_pAniThread = pAniThread;

	sendAffDEBUT();

	m_sDefaultCfg = m_pAniThread->getAniInitData()->getVirtKeyDefaultCfg();
	m_iDefaultX = m_pAniThread->getAniInitData()->getVirtKeyDefaultPosX(); 
	m_iDefaultY = m_pAniThread->getAniInitData()->getVirtKeyDefaultPosY();

}

void MIhmVirtKyb::Hide()
{
	sendAffHide();
}

void MIhmVirtKyb::Show(QString sCfg, int X, int Y)
{
	bool bDoMove = false;

	if(sCfg.isEmpty())
		sCfg = m_sDefaultCfg;

	if(X==-1||Y==-1)
	{
		X = m_iDefaultX;
		Y = m_iDefaultY;
	}

	if(m_X != X||m_Y != Y)
	{
		m_X = X;m_Y = Y;
		bDoMove = true;
	}

	if(m_sCfg != sCfg) //if configuration changed..
	{
		m_sCfg = sCfg;
		sendInitConfig(m_sCfg);
		sendAffMove(m_X,m_Y); //after init we have to assure that the position is correct
	}
	else if(bDoMove) //if position is different
		sendAffMove(m_X,m_Y);

	sendAffShow();
}



bool MIhmVirtKyb::sendMessage(enum_srv_service service, enum_srv_type eTypeMessage, void *data)
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
        
		eNoyRet = Envoie(m_pAniThread->getVirtKybBalId(), m_pAniThread->getAniBalId(), (struct_neutre *)(p_msg_emis));
        
        if(eNoyRet != NOYAU_OK)
		{
			ExitLibere((struct_neutre **)&p_msg_emis);
			bRet = false;
		}
        
    }
    else
    {
		bRet = false;
    }
	
	return bRet;
}


void MIhmVirtKyb::sendInitConfig(QString sKeybID)
{
	struct_VIRT_KYB_srv_affichage sAff={0};
	
	strncpy((char*)sAff.keyboard_id,(char*)sKeybID.toLatin1().data(), sizeof(sAff.keyboard_id));
	sendMessage((enum_srv_service)M_VIRT_KYB_AFFICHAGE, (enum_srv_type)VIRT_KYB_INIT_CONFIG, &sAff);
}

void MIhmVirtKyb::sendAffMove(int x, int y)
{
	struct_VIRT_KYB_srv_affichage sAff={0};

	sAff.dwPosX = x;
	sAff.dwPosY = y;
	sendMessage((enum_srv_service)M_VIRT_KYB_AFFICHAGE, (enum_srv_type)VIRT_KYB_AFF_MOVE, &sAff);

}

void MIhmVirtKyb::sendAffHide()
{
	struct_VIRT_KYB_srv_affichage sAff={0};
	
	sAff.bShow = FALSE;
	sendMessage((enum_srv_service)M_VIRT_KYB_AFFICHAGE, (enum_srv_type)VIRT_KYB_AFF_SHOW, &sAff);

}

void MIhmVirtKyb::sendAffShow()
{
	struct_VIRT_KYB_srv_affichage sAff={0};
	
	sAff.bShow = TRUE;
	sendMessage((enum_srv_service)M_VIRT_KYB_AFFICHAGE, (enum_srv_type)VIRT_KYB_AFF_SHOW, &sAff);
	
}

void MIhmVirtKyb::sendAffDEBUT()
{
	sendMessage((enum_srv_service)M_VIRT_KYB_AFFICHAGE, SRV_TYP_DEBUT, NULL);

}

void MIhmVirtKyb::sendAffFIN()
{
	sendMessage((enum_srv_service)M_VIRT_KYB_AFFICHAGE, SRV_TYP_FIN, NULL);
}

void MIhmVirtKyb::sendArret()
{
	sendMessage(M_SRV_ARRET, SRV_TYP_DEMANDE, NULL);
	
}

