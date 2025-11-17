

#include "MKybSrv.h"
#include "MKybAniThread.h"
#include "MKybMsg.h"
#include "MTracer.h"
 

extern "C" {
	#include <virt_kyb.H>
	#include <csr_srv.h>
	#include <noyau.h>
};


MKybSrv * MKybSrv::m_pGlobalSrv = NULL;


MKybSrv::MKybSrv(MKybAniThread * pAniThread)
{
 	if((m_hService[M_VIRT_KYB_AFFICHAGE] = SrvLance(SRV_ILLIMITE)) == NULL)
		ExitBad();

	if(m_pGlobalSrv!=NULL)
	{
        TRACE_W("MKybSrv::MKybSrv: Duplicate instance of MKybSrv!");
		ExitBad();
	}

	m_pGlobalSrv = this; 
	m_pAniThread = pAniThread;

}


MKybSrv::~MKybSrv()
{
	if(SrvArret(&m_hService[M_VIRT_KYB_AFFICHAGE]) == FALSE)
		ExitBad();

}


void MKybSrv::KybTreatMessageService( IN struct_VIRT_KYB_message * psMsg )
{
    switch (psMsg->entete.service)
    {
        case M_VIRT_KYB_AFFICHAGE :
            KybSrvReceptionAffichage( psMsg );
            break ;
        case M_SRV_ARRET: 
            KybSrvReceptionArret( psMsg );
            break ;

        default :
            KybMessEnvoiAcquitement( psMsg->entete.neutre.bl_retour, psMsg->entete.service, SRV_TYP_DEMANDE_INCONNUE ) ;
            TRACE_W(QString("MKybSrv::KybTreatMessageService: Unknown message service %1").arg((int)psMsg->entete.service));
            break ;
    }
}




void MKybSrv::KybSrvReceptionArret( IN struct_VIRT_KYB_message * psMsg )
{
    TRACE_D("MKybSrv::KybSrvReceptionArret ..." );

    switch ( psMsg->entete.type_message )
    {
    // DEMANDE D'ARRÊT DE L'INTERFACE
    case SRV_TYP_DEMANDE :
        TRACE_D("MKybSrv::KybSrvReceptionArret: SRV_TYP_DEMANDE" );

		//STOP thread and exit program
		emit requestStop();

        // Acq de la demande
        KybMessEnvoiAcquitement(psMsg->entete.neutre.bl_retour, 
                                M_SRV_ARRET, 
                                SRV_TYP_DEMANDE_ACQ );
        break;
    
    // INCONNU
    default :
        TRACE_W(QString("MKybSrv::KybSrvReceptionArret: Unknown messages type %1").arg((int)psMsg->entete.type_message));
        KybMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
                                  M_SRV_ARRET, 
                                  SRV_TYP_DEMANDE_INCONNUE );
        break;
    }
}


void MKybSrv::KybSrvReceptionAffichage ( IN struct_VIRT_KYB_message *psMsg )
{
	TRACE_D(QString("MKybSrv::KybSrvReceptionAffichage: %1").arg(psMsg->entete.type_message));

    switch ( psMsg->entete.type_message )
    {
	case SRV_TYP_DEBUT :
        TRACE_D("MKybSrv::KybSrvReceptionAffichage: SRV_TYP_DEBUT" );
		KybDebutService((enum_srv_service)M_VIRT_KYB_AFFICHAGE, psMsg->entete.neutre.bl_retour);
		break;

	case SRV_TYP_FIN :
        TRACE_D("MKybSrv::KybSrvReceptionAffichage: SRV_TYP_FIN" );
		KybFinService((enum_srv_service)M_VIRT_KYB_AFFICHAGE, psMsg->entete.neutre.bl_retour);
		break;
        
	case VIRT_KYB_AFF_MOVE:
	case VIRT_KYB_AFF_SHOW:
    case VIRT_KYB_INIT_CONFIG :  
        // Routage de la demande vers l'interface VB (Msg evènementiel)
        if (KybSrvTraitementAffichage(psMsg))
        {
            // Pas d'erreur, Acq de la demande
            KybMessEnvoiAcquitement( psMsg->entete.neutre.bl_retour, 
                                      (enum_srv_service)M_VIRT_KYB_AFFICHAGE, 
                                      SRV_TYP_SET_ACQ);
        }
        else
        {
            TRACE_W("MKybSrv::KybSrvReceptionAffichage: ERREUR _IhmSrvTraitementAffichage retourne FALSE" );
            // Erreur, NAcq de la demande
            KybMessEnvoiAcquitement( psMsg->entete.neutre.bl_retour, 
                                      (enum_srv_service)M_VIRT_KYB_AFFICHAGE, 
                                      SRV_TYP_SET_NACQ);
        }
        break;

    // INCONNU
    default :
        TRACE_W(QString("MKybSrv::KybSrvReceptionAffichage: ERREUR type inconnu %1").arg((int)psMsg->entete.type_message));

        // NAcq de la demande
        KybMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
                                  (enum_srv_service)M_VIRT_KYB_AFFICHAGE, 
                                  SRV_TYP_DEMANDE_INCONNUE);
        break;
    }
}


noyau_pool_id MKybSrv::getPoolId() 
{
	return m_pAniThread->getPoolId();
}

noyau_bal_id MKybSrv::getAniBalId() 
{
	return m_pAniThread->getAniBalId();

};


void MKybSrv::KybMessEnvoiAcquitement(IN noyau_bal_id iBalDest, 
                                       IN enum_srv_service eService,
                                       IN enum_srv_type eTypeMessage)
{
	m_pAniThread->KybMessEnvoiAcquitement(iBalDest, eService, eTypeMessage);
}


bool MKybSrv::KybSrvTraitementAffichage ( IN struct_VIRT_KYB_message * psMsg )
{
    struct_VIRT_KYB_srv_affichage * psAff = &( psMsg->u.srv_affichage );
    BOOL bRes = TRUE;

    TRACE_D(QString( "MKybSrv::KybSrvTraitementAffichage: type_message:%1...  ").arg(psMsg->entete.type_message));
	
    switch ( psMsg->entete.type_message )
    {
	case VIRT_KYB_INIT_CONFIG:
		{
			MKybMsgKybConfig * pNewMsg = new MKybMsgKybConfig();
			pNewMsg->m_sKyboardTypeID = QString((char*)psAff->keyboard_id);
			
			m_pAniThread->emitMessageFromAni(pNewMsg);
		}

		break;
	case VIRT_KYB_AFF_MOVE:
		{
			MKybMsgKybMove * pNewMsg = new MKybMsgKybMove();
			pNewMsg->dwPosX = psAff->dwPosX;
			pNewMsg->dwPosY = psAff->dwPosY;

			m_pAniThread->emitMessageFromAni(pNewMsg);
		}
		break;
	case VIRT_KYB_AFF_SHOW:
		{
			MKybMsgKybShow * pNewMsg = new MKybMsgKybShow();
			pNewMsg->m_bShow = psAff->bShow?true:false;
			
			m_pAniThread->emitMessageFromAni(pNewMsg);
		}
		break;
	
	}

	return bRes;
}
//------------------------------------------------------------------------------------------------------


enum_srv_type MKybSrv::KybDebutService(IN enum_srv_service eService,
                                        IN noyau_bal_id dwBalDest)
{
	enum_srv_type  eResult;

	eResult = SRV_TYP_DEBUT_NACQ;

	// Verifing if the user already exists in the list of users 
	if((SrvEstDemandeur(m_hService[eService],
						dwBalDest) == FALSE))
		// Adding user to the list	
		if(SrvAjouteDemandeur(m_hService[eService],
							  dwBalDest, 0L) != NULL)
		{
			eResult = SRV_TYP_DEBUT_ACQ;
		}

	KybMessEnvoiAcquitement(dwBalDest, (enum_srv_service)eService, eResult);

	if(eResult == SRV_TYP_DEBUT_NACQ)
	{ 
		TRACE_W(QString("MKybSrv::KybDebutService ==> Debut Service: Pb Ajout Demandeur: %1")
												.arg(SRV_TYP_DEBUT_NACQ));
	}

	return eResult;
}

enum_srv_type MKybSrv::KybFinService(IN enum_srv_service eService,
                                      IN noyau_bal_id dwBalDest)
{
	PVOID pvSrvData;

    // Searching the list of users
	pvSrvData = SrvRechercheDemandeur(m_hService[eService], 
									  dwBalDest,
									  NULL,
									  NULL);
	if(pvSrvData == NULL)
	{
		KybMessEnvoiAcquitement(dwBalDest,
								(enum_srv_service)eService,
								SRV_TYP_FIN_NACQ);

		return SRV_TYP_FIN_NACQ;
	}

    // Removing a user from the list of users
	SrvEnleveDemandeur(m_hService[eService], &pvSrvData);
	KybMessEnvoiAcquitement(dwBalDest, (enum_srv_service)eService, SRV_TYP_FIN_ACQ);

	return SRV_TYP_FIN_ACQ;
}



//-----------------------------------------------------------------


void MKybSrv::KybTreatMessageTCL(MKybMsg * pMsg)
{
	if(pMsg!=NULL)	
	{
// 		switch(pMsg->getType())
// 		{
// 		case MKybMsg::enuIhmMsgVideo:
// 			{
// 				MKybMsgVideo *pMsgVideo = (MKybMsgVideo *)pMsg;
// 					
// 				switch(pMsgVideo->getVideoMsgType())
// 				{
// 					case MKybMsgVideo::enuIhmMsgVideoFreezeRsp:
// 							IhmProcessMessVideoFreezeRsp((MKybMsgVideoFreezeRsp *)pMsgVideo);
// 						break;
// 
// 					case MKybMsgVideo::enuIhmMsgVideoSaveRsp:
// 							IhmProcessMessVideoSaveRsp((MKybMsgVideoSaveRsp *)pMsgVideo);
// 						break;
// 					
// 					case MKybMsgVideo::enuIhmMsgRVideoUpdateSrc:
// 							IhmProcessMessRVideoUpdateSrc((MKybMsgRVideoUpdateSrc *)pMsgVideo);
// 						break;
// 					case MKybMsgVideo::enuIhmMsgRVideoUpdateOnOff:
// 							IhmProcessMessRVideoUpdateOnOff((MKybMsgRVideoUpdateOnOff *)pMsgVideo);
// 						break;
// 
// 
// 					default:
// 						TRACE_W(QString("MKybSrv::KybTreatMessageTCL: Unknown video message type: %1").
// 								arg((int)pMsgVideo->getVideoMsgType()));
// 						break;
// 				}
// 
// 			}
// 			break;
// 			
// 		default:
// 			TRACE_W(QString("MKybSrv::KybTreatMessageTCL: Unknown message type: %1").
// 											arg((int)pMsg->getType()));
// 		    break;
// 		}
	}
}



