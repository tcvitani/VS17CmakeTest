

#include "MIhmSrv.h"
#include "MIhmAniVirtObjects.h"
#include "MIhmVirtualObject.h"
#include "MIhmAniThread.h"
#include "MHelpFuncs.h"
#include "MIhmKey.h"
#include "MIhmMsg.h"
#include "MIhmMsgVideo.h"
#include "MStrDetectionPair.h"
#include "MInputDialogReq.h"
#include "MInputDialogExReq.h"
#include "MTracer.h"
#include "MInputAsyncExReq.h"
 

extern "C" {
	#include <ihm.H>
	#include "ihm_loc.h"
	#include <csr_srv.h>
	#include <noyau.h>
};


bool BalIdDetectionPair::existsBlPair(QList <BalIdDetectionPair*> *pLst, BalIdDetectionPair* pBlPair)
{
	for (int i=0;i<pLst->size();++i)
	{
		if(pLst->at(i)->blId == pBlPair->blId &&
			pLst->at(i)->sActiveDetectPair == pBlPair->sActiveDetectPair)
				return true;
	}
	
	return false;
}

bool BalIdDetectionPair::existsPair(QList <BalIdDetectionPair*> *pLst, QString sPair)
{
	for (int i=0;i<pLst->size();++i)
	{
		if(pLst->at(i)->sActiveDetectPair == sPair)
				return true;
	}
	
	return false;
}

bool BalIdDetectionPair::removeBlPair(QList <BalIdDetectionPair*> *pLst, BalIdDetectionPair* pBlPair)
{
	bool bRetVal = false;

	for (int i=0;i<pLst->size();++i)
	{
		if(pLst->at(i)->blId == pBlPair->blId &&
			pLst->at(i)->sActiveDetectPair == pBlPair->sActiveDetectPair)
		{
			BalIdDetectionPair *p = pLst->takeAt(i);
			delete p;
			bRetVal = true;
			break;
		}
	}
	
	return bRetVal;
}

bool BalIdDetectionPair::addBlPair(QList <BalIdDetectionPair*> *pLst, BalIdDetectionPair* pBlPair)
{
	
	if(!existsBlPair(pLst,pBlPair))
	{
		BalIdDetectionPair * p = new BalIdDetectionPair();
		*p = *pBlPair;
		pLst->append(p);
		return true;	
	}

	return false;
}



MIhmSrv * MIhmSrv::m_pGlobalSrv = NULL;


MIhmSrv::MIhmSrv()
{
 	if((m_hService[M_IHM_AFFICHAGE] = SrvLance(SRV_ILLIMITE)) == NULL)
		ExitBad();

	//only a single user can be used for authorization
 	if((m_hService[M_IHM_AUTHORIZATION] = SrvLance(1)) == NULL)
		ExitBad();
	
 	if((m_hService[M_IHM_ASYNC_INPUT] = SrvLance(SRV_ILLIMITE)) == NULL)
		ExitBad();

 	if((m_hService[M_IHM_SAISIE_VISU_EX] = SrvLance(SRV_ILLIMITE)) == NULL)
		ExitBad();


	if(m_pGlobalSrv!=NULL)
	{
        TRACE_W("MIhmSrv::MIhmSrv: Duplicate instance of MIhmSrv!");
		ExitBad();
	}

	m_pGlobalSrv = this; 
}


MIhmSrv::~MIhmSrv()
{
	if(SrvArret(&m_hService[M_IHM_AFFICHAGE]) == FALSE)
		ExitBad();

	if(SrvArret(&m_hService[M_IHM_AUTHORIZATION]) == FALSE)
		ExitBad();

	if(SrvArret(&m_hService[M_IHM_ASYNC_INPUT]) == FALSE)
		ExitBad();

	while(!m_lstSrvDetectionPairs.isEmpty())
		delete m_lstSrvDetectionPairs.takeLast();

}

void MIhmSrv::Initialize(MIhmAniThread * pAniThread, MIhmAniVirtObjects * pAniVirtObjects)
{
	m_pAniVirtObjects = pAniVirtObjects;
	m_pAniThread = pAniThread;

	m_oIhmKey.IhmKeyInit(m_pAniThread->getModuleConfigKey());
	m_oIhmStatusSetConfig.Init(m_pAniThread->getModuleConfigKey());
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE VOID IhmTreatMessageService ( IN struct_neutre * psNeutre )
 * PARAMETERS: IN struct_neutre * psNeutre : Message recu dans la bal
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Oriente le message vers la routine de traitement appropiee
 *             en fonction de son type
 * --------------------------------------------------------------------
 */
void MIhmSrv::IhmTreatMessageService( IN struct_ihm_message * psMsg )
{
    switch (psMsg->entete.service)
    {
        case M_SRV_ARRET :
            IhmSrvReceptionArret( psMsg );
            break;

        case M_IHM_CONFIG :
            IhmSrvReceptionConfig( psMsg );
            break ;

        case M_IHM_AFFICHAGE :
            IhmSrvReceptionAffichage( psMsg );
            break ;
        case M_IHM_AFFICHAGE_STATUS :
            IhmSrvReceptionAffichageStatus( psMsg );
            break ;

        case M_IHM_TOUCHES :
            IhmSrvReceptionTouches( psMsg );
            break ;

        case M_IHM_SAISIE_VISU :
            IhmSrvReceptionSaisie( psMsg );
            break;

        case M_IHM_SAISIE_VISU_EX :
            IhmSrvReceptionSaisieEx( psMsg );
            break;


		case M_IHM_POLICE :
            IhmSrvReceptionPolice( psMsg );
            break ;
	
		case M_IHM_DETECTION_CHAINE :
            IhmSrvReceptionDetectionChaine( psMsg );
            break ;

		case M_IHM_AUTHORIZATION :
            IhmSrvReceptionAuthorisation( psMsg );
            break ;

		case M_IHM_ASYNC_INPUT :
            IhmSrvReceptionAsyncInput( psMsg );
            break ;

		
	
        default :
            IhmMessEnvoiAcquitement( psMsg->entete.neutre.bl_retour, psMsg->entete.service, SRV_TYP_DEMANDE_INCONNUE ) ;
            TRACE_W(QString("MIhmSrv::IhmTreatMessageService: Unknown message service %1").arg((int)psMsg->entete.service));
            break ;
    }
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : void IhmSrvReceptionArret ( IN struct_ihm_message * psMsg )
 * PARAMETERS: IN struct_ihm_message * psMsg : Message recu de l'application
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Traite un message de type arret provenant de l'application
 * --------------------------------------------------------------------
 */
void MIhmSrv::IhmSrvReceptionArret( IN struct_ihm_message * psMsg )
{
    TRACE_D("MIhmSrv::IhmSrvReceptionArret ..." );

    switch ( psMsg->entete.type_message )
    {
    // DEMANDE D'ARRET DE L'INTERFACE
    case SRV_TYP_DEMANDE :
        TRACE_D("MIhmSrv::IhmSrvReceptionArret: SRV_TYP_DEMANDE" );

		//STOP thread and exit program
		emit requestStop();

        // Acq de la demande
        IhmMessEnvoiAcquitement(psMsg->entete.neutre.bl_retour, 
                                M_SRV_ARRET, 
                                SRV_TYP_DEMANDE_ACQ );
        break;
    
    // INCONNU
    default :
        TRACE_W(QString("MIhmSrv::IhmSrvReceptionArret: Unknown messages type %1").arg((int)psMsg->entete.type_message));
        IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
                                  M_SRV_ARRET, 
                                  SRV_TYP_DEMANDE_INCONNUE );
        break;
    }
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void IhmSrvReceptionConfig ( IN struct_ihm_message * psMsg )
 * PARAMETERS: IN struct_ihm_message * psMsg : Message recu de l'application
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Traite un message de type config provenant de l'application
 * --------------------------------------------------------------------
 */
void MIhmSrv::IhmSrvReceptionConfig( IN struct_ihm_message * psMsg )
{
	//assure that used strings are zero terminated 
	psMsg->u.srv_config.projet[IHM_LG_CHAINES_SRV-1] = '\0';
    psMsg->u.srv_config.no_version[IHM_LG_CHAINES_SRV-1] = '\0';
    psMsg->u.srv_config.langue[IHM_LG_CHAINES_SRV-1] = '\0';
    psMsg->u.srv_config.type_voie[IHM_LG_CHAINES_SRV-1] = '\0';
	psMsg->u.srv_config.type_poste[IHM_LG_CHAINES_SRV - 1] = '\0';
	psMsg->u.srv_config.main_dlg_hidden[IHM_LG_CHAINES_SRV - 1] = '\0';


    switch ( psMsg->entete.type_message )
    {
    // DEMANDE DE CHANGEMENT DE CONFIG
    case SRV_TYP_SET :  
        TRACE_D("MIhmSrv::IhmSrvReceptionConfig: SRV_TYP_SET" );
			
        // Nom projet if modification required (avec d'@ de la chaine a recuperer)
		if(!IHM_STR_IS_DO_NOT_CHANGE((char*)psMsg->u.srv_config.projet))
        {
			MIhmVirtDynConfig * pVirtObj = 
					(MIhmVirtDynConfig *)m_pAniVirtObjects->
						getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtConfProjectName);

			QString sNewValue = QString::fromLocal8Bit((char*)psMsg->u.srv_config.projet);
			TRACE_D(QString( "MIhmSrv::IhmSrvReceptionConfig: projet:[%1]").arg(sNewValue));
			
			if(!pVirtObj->isInitialized()) pVirtObj->setInitialized();

           if(pVirtObj->setDynConfigSetting(sNewValue))
				pVirtObj->setUpdated(); //if changed
        }

        // No version if modification required (avec d'@ de la chaine a recuperer)
		if(!IHM_STR_IS_DO_NOT_CHANGE((char*)psMsg->u.srv_config.no_version))
        {
			MIhmVirtDynConfig * pVirtObj = 
					(MIhmVirtDynConfig *)m_pAniVirtObjects->
						getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtConfProjectVersion);

			QString sNewValue = QString::fromLocal8Bit((char*)psMsg->u.srv_config.no_version);
			TRACE_D(QString( "MIhmSrv::IhmSrvReceptionConfig: no_version:[%1] ").arg(sNewValue));
			
			if(!pVirtObj->isInitialized()) pVirtObj->setInitialized();

            if(pVirtObj->setDynConfigSetting(sNewValue))
				pVirtObj->setUpdated(); //if changed
        }

        // Type de Voie if modification required
		if (!IHM_STR_IS_DO_NOT_CHANGE((char*)psMsg->u.srv_config.type_voie) && psMsg->u.srv_config.type_voie[0]!='\0')
        {
			MIhmVirtDynConfig * pVirtObj = 
					(MIhmVirtDynConfig *)m_pAniVirtObjects->
						getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtConfLaneType);

			QString sNewValue = QString::fromLocal8Bit((char*)psMsg->u.srv_config.type_voie);
			TRACE_D(QString("MIhmSrv::IhmSrvReceptionConfig: type_voie:[%1]").arg(sNewValue));
			
			if(!pVirtObj->isInitialized()) pVirtObj->setInitialized();

			if(pVirtObj->setDynConfigSetting(sNewValue))
			{
				pVirtObj->setUpdated(); //if changed

				//Also update the InputAsync if any
				//load the DEF file for async input
				MIhmVirtAsyncInput * pVirtObjAsync = 
					(MIhmVirtAsyncInput *)m_pAniVirtObjects->getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtAsyncInputsID);
				
				if(pVirtObjAsync!=NULL)
					pVirtObjAsync->initialize(sNewValue);

			}
        }

		//psMsg->u.srv_config.main_dlg_hidden[IHM_LG_CHAINES_SRV - 1] = '\0';
		if (!IHM_STR_IS_DO_NOT_CHANGE((char*)psMsg->u.srv_config.main_dlg_hidden))
		{
			MIhmVirtDynConfig * pVirtObj =
				(MIhmVirtDynConfig *)m_pAniVirtObjects->
				getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtConfMainDlgHidden);

			QString sNewValue = QString::fromLocal8Bit((char*)psMsg->u.srv_config.main_dlg_hidden);
			TRACE_D(QString("MIhmSrv::IhmSrvReceptionConfig: main_dlg_hidden:[%1]").arg(sNewValue));

			if (!pVirtObj->isInitialized()) pVirtObj->setInitialized();

			pVirtObj->setDynConfigSetting(sNewValue);
			pVirtObj->setUpdated(); //execute this one allways...
			
		}	



        // Type de Poste if modification required
		if(!IHM_STR_IS_DO_NOT_CHANGE((char*)psMsg->u.srv_config.type_poste))
        {
			MIhmVirtDynConfig * pVirtObj = 
					(MIhmVirtDynConfig *)m_pAniVirtObjects->
						getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtConfShiftType);

			QString sNewValue = QString::fromLocal8Bit((char*)psMsg->u.srv_config.type_poste);
			TRACE_D(QString( "MIhmSrv::IhmSrvReceptionConfig: type_poste:[%1]").arg(sNewValue));
			
 			if(!pVirtObj->isInitialized()) pVirtObj->setInitialized();

			if(pVirtObj->setDynConfigSetting(sNewValue))
				pVirtObj->setUpdated(); //if changed
        }

        // Langue if modification required (avec d'@ de la chaine a recuperer)
		if (!IHM_STR_IS_DO_NOT_CHANGE((char*)psMsg->u.srv_config.langue) && psMsg->u.srv_config.langue[0] != '\0')
        {
			MIhmVirtDynConfig * pVirtObj = 
					(MIhmVirtDynConfig *)m_pAniVirtObjects->
						getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtConfLanguage);

			QString sNewValue = QString::fromLocal8Bit((char*)psMsg->u.srv_config.langue);
			TRACE_D(QString("MIhmSrv::IhmSrvReceptionConfig: langue:[%1]").arg(sNewValue));
			
 			if(!pVirtObj->isInitialized()) pVirtObj->setInitialized();

			if(pVirtObj->setDynConfigSetting(sNewValue))
				pVirtObj->setUpdated(); //if changed
        }

		//Whatever is received send message to the TCL interface so it can reload DynamicConfig
		// This is used to dynamically update the ihm configuration parameters
		processReloadDynConfig();
		
        // Acq de la demande
        IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, (enum_srv_service)M_IHM_CONFIG, SRV_TYP_SET_ACQ);
        break;

    // DEMANDE DE LA CONFIG
    case SRV_TYP_GET :
        TRACE_D("MIhmSrv::IhmSrvReceptionConfig: IHM_GET" );

        // Acq de la demande
        IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, (enum_srv_service)M_IHM_CONFIG, SRV_TYP_GET_ACQ);
        
        // Envoi de la config actuelle
        IhmMessEnvoiDonneesConfig( psMsg->entete.neutre.bl_retour );
        break;

    // INCONNU
    default :
		TRACE_W(QString("MIhmSrv::IhmSrvReceptionConfig: ERREUR type inconnu %1 from mbox:%2")
			.arg((int)psMsg->entete.type_message).arg((int)psMsg->entete.neutre.bl_retour));

        // NAcq de la demande
        IhmMessEnvoiAcquitement( psMsg->entete.neutre.bl_retour, (enum_srv_service)M_IHM_CONFIG, SRV_TYP_DEMANDE_INCONNUE );
        break;
    }
}



void MIhmSrv::IhmMessEnvoiDonneesConfig(IN noyau_bal_id iBalDest)
{
    struct_ihm_message * psMsgEmis = NULL;
    struct_ihm_srv_config * psSrv;

    TRACE_D("MIhmSrv::IhmMessEnvoiDonneesConfig ..." );

    m_pAniThread->MyExitAlloue("MIhmSrv::IhmMessEnvoiDonneesConfig",
        ( struct_neutre ** )( &psMsgEmis ),
        sizeof( struct_ihm_message ),
        m_pAniThread->getPoolId());

    psMsgEmis->entete.service = (enum_srv_service)M_IHM_CONFIG;
    psMsgEmis->entete.type_message = (enum_srv_type)IHM_CONFIG_ACTUELLE;

    psSrv = &psMsgEmis->u.srv_config;

	MIhmVirtDynConfig * pVirtObj;
	
	 
	//extract language
	pVirtObj = (MIhmVirtDynConfig *)m_pAniVirtObjects->
			getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtConfLanguage);
	
	if(pVirtObj!=NULL)
		IHM_STRNCPY( (char*)psSrv->langue, pVirtObj->m_sDynConfigSetting.toLatin1().data(),    sizeof(psSrv->langue) );

	//extract project name
	pVirtObj = (MIhmVirtDynConfig *)m_pAniVirtObjects->
			getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtConfProjectName);
	
	if(pVirtObj!=NULL)
		IHM_STRNCPY( (char*)psSrv->projet, pVirtObj->m_sDynConfigSetting.toLatin1().data(), sizeof(psSrv->projet) );
	
	//extract version number
	pVirtObj = (MIhmVirtDynConfig *)m_pAniVirtObjects->
			getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtConfProjectVersion);
	
	if(pVirtObj!=NULL)
		IHM_STRNCPY( (char*)psSrv->no_version, pVirtObj->m_sDynConfigSetting.toLatin1().data(), sizeof(psSrv->no_version) );

	//extract lane type
	pVirtObj = (MIhmVirtDynConfig *)m_pAniVirtObjects->
			getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtConfLaneType);
	
	if(pVirtObj!=NULL)
		IHM_STRNCPY( (char*)psSrv->type_voie, pVirtObj->m_sDynConfigSetting.toLatin1().data(), sizeof(psSrv->type_voie) );

	//extract shift type
	pVirtObj = (MIhmVirtDynConfig *)m_pAniVirtObjects->
			getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtConfShiftType);
	
	if(pVirtObj!=NULL)
		IHM_STRNCPY( (char*)psSrv->type_poste, pVirtObj->m_sDynConfigSetting.toLatin1().data(), sizeof(psSrv->type_poste) );

    m_pAniThread->MyExitEnvoie("MIhmSrv::IhmMessEnvoiDonneesConfig", iBalDest, m_pAniThread->getAniBalId(), (struct_neutre *)psMsgEmis );

}

//----------------------------------------------------------------------------------------------------
// M_IHM_AFFICHAGE_STATUS
//----------------------------------------------------------------------------------------------------

void MIhmSrv::IhmSrvReceptionAffichageStatus(IN struct_ihm_message *psMsg)
{
	
    switch ( psMsg->entete.type_message )
    {
	case SRV_TYP_DEBUT :
        TRACE_D("MIhmSrv::IhmSrvReceptionAffichageStatus: SRV_TYP_DEBUT" );
		IhmDebutService(M_IHM_AFFICHAGE_STATUS, psMsg->entete.neutre.bl_retour);
		break;
		
	case SRV_TYP_FIN :
        TRACE_D("MIhmSrv::IhmSrvReceptionAffichageStatus: SRV_TYP_FIN" );
		IhmFinService(M_IHM_AFFICHAGE_STATUS, psMsg->entete.neutre.bl_retour);
		break;
        
    case SRV_TYP_SET :  
        TRACE_D("MIhmSrv::IhmSrvReceptionAffichageStatus: SRV_TYP_SET" );
		
        // Routage de la demande vers l'interface VB (Msg evenementiel)
        if (IhmSrvTraitementAffichageStatus(psMsg))
        {
            // Pas d'erreur, Acq de la demande
            IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
				(enum_srv_service)M_IHM_AFFICHAGE_STATUS, 
				SRV_TYP_SET_ACQ);
        }
        else
        {
            TRACE_W("MIhmSrv::IhmSrvReceptionAffichageStatus: ERREUR _IhmSrvTraitementAffichage retourne FALSE" );
            // Erreur, NAcq de la demande
            IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
				(enum_srv_service)M_IHM_AFFICHAGE_STATUS, 
				SRV_TYP_SET_NACQ);
        }
        break;
		
		// INCONNU
    default :
        TRACE_W(QString("MIhmSrv::IhmSrvReceptionAffichageStatus: ERREUR type inconnu %1").arg((int)psMsg->entete.type_message));
		
        // NAcq de la demande
        IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
			(enum_srv_service)M_IHM_AFFICHAGE_STATUS, 
			SRV_TYP_DEMANDE_INCONNUE);
        break;
    }
}



BOOL MIhmSrv::IhmSrvTraitementAffichageStatus ( IN struct_ihm_message * psMsg )
{
    struct_ihm_srv_affichage * psAff = &( psMsg->u.srv_affichage );
    BOOL bRes = FALSE;

	//assure that strings are zero terminated
	psAff->valeur.szValeur[IHM_MAX_XML_BUFFER_SIZE-1] = '\0';
	bool bEnabled;

	if(psAff->is_enabled==enuIHM_OBJECT_ENABLED)
		bEnabled = true; 
	else if(psAff->is_enabled==enuIHM_OBJECT_DISABLED)
		bEnabled = false;
	else
	{
        TRACE_W(QString("MIhmSrv::IhmSrvTraitementAffichageStatus: ERREUR Unknown value StatusSet group:[%1], is_enabled:%2")
												.arg(psAff->valeur.szValeur)
												.arg(psAff->is_enabled));
		return false;
	}


    TRACE_D(QString( "MIhmSrv::IhmSrvTraitementAffichageStatus ... valeur.szValeur:[%1], enabled:%2")
												.arg(psAff->valeur.szValeur)
													.arg((int)bEnabled));
	

	MStatusSetGroup * pStatusSetGroup = m_oIhmStatusSetConfig.getStatusSetGroup(psAff->valeur.szValeur);

	if(pStatusSetGroup!=NULL)
	{
		QList <SStatusSetGroupObject *> * pLstObjects = pStatusSetGroup->getIdList();

		SStatusSetGroupObject * pCurrent;

		for(int i=0;i<pLstObjects->size();i++)
		{
			pCurrent = pLstObjects->at(i);
			MIhmVirtualObject::enumVirtualObjectId  eVirtObjID = MIhmVirtualObject::mapIhmObjIdToVirtObjId(pCurrent->objet);
			MIhmVirtualObject * pVirtObj = m_pAniVirtObjects->getVirtualObjectByID(eVirtObjID);

			if(pVirtObj!=NULL)
			{
				if(pVirtObj->setEnabled(bEnabled))
					pVirtObj->setUpdated(); 
			}
		}
		
	}

    return TRUE;
}


//----------------------------------------------------------------------------------------------------


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void IhmSrvReceptionAffichage ( IN struct_ihm_message *psMsg )
 * PARAMETERS: IN struct_ihm_message * psMsg : Message recu de l'application
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Traite un message de type affichange provenant de l'application
 * --------------------------------------------------------------------
 */
void MIhmSrv::IhmSrvReceptionAffichage ( IN struct_ihm_message *psMsg )
{

    switch ( psMsg->entete.type_message )
    {
	case SRV_TYP_DEBUT :
        TRACE_D("MIhmSrv::IhmSrvReceptionAffichage: SRV_TYP_DEBUT" );
		IhmDebutService(M_IHM_AFFICHAGE, psMsg->entete.neutre.bl_retour);
		break;

	case SRV_TYP_FIN :
        TRACE_D("MIhmSrv::IhmSrvReceptionAffichage: SRV_TYP_FIN" );
		IhmFinService(M_IHM_AFFICHAGE, psMsg->entete.neutre.bl_retour);
		break;
        
    case SRV_TYP_SET :  
        TRACE_D("MIhmSrv::IhmSrvReceptionAffichage: SRV_TYP_SET" );

        // Routage de la demande vers l'interface VB (Msg evenementiel)
        if (IhmSrvTraitementAffichage(psMsg))
        {
            // Pas d'erreur, Acq de la demande
            IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
                                      (enum_srv_service)M_IHM_AFFICHAGE, 
                                      SRV_TYP_SET_ACQ);
        }
        else
        {
            TRACE_W("MIhmSrv::IhmSrvReceptionAffichage: ERREUR _IhmSrvTraitementAffichage retourne FALSE" );
            // Erreur, NAcq de la demande
            IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
                                      (enum_srv_service)M_IHM_AFFICHAGE, 
                                      SRV_TYP_SET_NACQ);
        }
        break;

    // INCONNU
    default :
        TRACE_W(QString("MIhmSrv::IhmSrvReceptionAffichage: ERREUR type inconnu %1").arg((int)psMsg->entete.type_message));

        // NAcq de la demande
        IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
                                  (enum_srv_service)M_IHM_AFFICHAGE, 
                                  SRV_TYP_DEMANDE_INCONNUE);
        break;
    }
}


////////////////////////////////////
// debut tmo 14/09/20000

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void IhmSrvReceptionPolice ( IN struct_ihm_message *psMsg )
 * PARAMETERS: IN struct_ihm_message * psMsg : Message recu de l'application
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Traite un message de type Police provenant de l'application
 * --------------------------------------------------------------------
 */
void MIhmSrv::IhmSrvReceptionPolice ( IN struct_ihm_message *psMsg )
{
    switch ( psMsg->entete.type_message )
    {
        
    case SRV_TYP_SET :  
        TRACE_D("MIhmSrv::IhmSrvReceptionPolice: SRV_TYP_SET" );

        // Routage de la demande vers l'interface VB (Msg evenementiel)
        if ( IhmSrvTraitementPolice( psMsg ) )
        {
            // Pas d'erreur, Acq de la demande
            IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
                                      (enum_srv_service)M_IHM_POLICE, 
                                      SRV_TYP_SET_ACQ);
        }
        else
        {
            TRACE_W("MIhmSrv::IhmSrvReceptionPolice: retourne FALSE" );
            // Erreur, NAcq de la demande
            IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
                                      (enum_srv_service)M_IHM_POLICE,			// 27/11/2000 TMO correction : envoi acq du srv Police au lieu du srv affichage
                                      SRV_TYP_SET_NACQ);
        }
        break;

    // INCONNU
    default :
        TRACE_W(QString("MIhmSrv::IhmSrvReceptionPolice: type inconnu %1").arg((int)psMsg->entete.type_message));

        // NAcq de la demande
        IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
                                  (enum_srv_service)M_IHM_POLICE, 
                                  SRV_TYP_DEMANDE_INCONNUE);
        break;
    }
}





void MIhmSrv::updateContainerObjectFont(MIhmVirtualObject::enumVirtualObjectId eId, const char * szNewPolice)
{
	MIhmVirtContainer * pVirtObj =
		(MIhmVirtContainer *)m_pAniVirtObjects->
		getVirtualObjectByID(eId);

	if (pVirtObj != NULL)
	{
		if (!pVirtObj->isInitialized()) pVirtObj->setInitialized();

		QString sNewPolice = QString::fromLocal8Bit(szNewPolice);
		pVirtObj->m_oPoliceValue.updateWithString(sNewPolice);
		pVirtObj->setUpdated(); //to be updated to the GUI
	}
}



void MIhmSrv::updateAllObjectFont(const char * sNewFont)
{

	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtPlazaName, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtPlazaName1, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtPlazaName2, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtPlazaName3, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtPlazaNameTitle, sNewFont);

	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtLaneName, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtLaneName1, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtLaneName2, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtLaneNameTitle, sNewFont);

	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtPlazaNbr, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtPlazaNbrTitle, sNewFont);

	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtLaneNbr, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtLaneNbrTitle, sNewFont);

	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTransactionNbr, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTransactionNbrTitle, sNewFont);

	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtReceiptNbr, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtReceiptNbrTitle, sNewFont);

	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtAxles, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtAxlesTitle, sNewFont);

	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtBalance, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtBalanceTitle, sNewFont);

	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtClass, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtClassTitle, sNewFont);
	
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtEntry, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtEntryTitle, sNewFont);

	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtFare, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtFareTitle, sNewFont);
	
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtSale, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtSaleTitle, sNewFont);

	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtSaleDue, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtSaleDueTitle, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtSaleDuePaid, sNewFont);
	
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTransactionType, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTransactionTypeTitle, sNewFont);

	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtFareType, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtFareTypeTitle, sNewFont);

	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtModeTitle, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtMode, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtCollectorTitle, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtInCtrlUserTitle, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtInCtrlUser, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtInCtrlUser1, sNewFont);

	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtTrsContainerTitle, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtAlarmsTitle, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtTrsHistoryTitle, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtAlertesTitle, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtCamNumPic, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtScanNumPic, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtNbCarSAS, sNewFont);

	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCollectorID, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCollectorName, sNewFont);

	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare1, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare2, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare3, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare4, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare5, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare6, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare7, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare8, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare9, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare10, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare11, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare12, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare13, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare14, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare15, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare16, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare17, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare18, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare19, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare20, sNewFont);

	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare21, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare22, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare23, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare24, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare25, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare26, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare27, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare28, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare29, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare30, sNewFont);

	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare31, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare32, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare33, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare34, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare35, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare36, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare37, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare38, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare39, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare40, sNewFont);

	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare41, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare42, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare43, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare44, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare45, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare46, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare47, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare48, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare49, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare50, sNewFont);

	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare51, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare52, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare53, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare54, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare55, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare56, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare57, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare58, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare59, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare60, sNewFont);


	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtNbCar, sNewFont);

	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtErrPayment, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtPayment, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtRemark, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtInstruction, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtInstruction2, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCurrentDate, sNewFont);

	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtLstAlarms, sNewFont);
	updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtLstWarnings, sNewFont);
	
}


bool MIhmSrv::IhmSrvTraitementPolice ( IN struct_ihm_message * psMsg )
{
    struct_ihm_srv_police * psPolice = &( psMsg->u.srv_police );
    BOOL bRes = TRUE;

	//assure that strings are zero terminated
	psPolice->valeur.szValeur[IHM_LG_CHAINES_SRV-1] = '\0';

    TRACE_D(QString("MIhmSrv::IhmSrvTraitementPolice ... object %1 Id: %2 ")
												.arg(MHelpFuncs::getStringFromObjectId((int)psPolice->objet))
												.arg((int)psPolice->objet));

    switch ( psPolice->objet )
    {
		case IHM_OBJ_All_Object:
			updateAllObjectFont(((char*)psPolice->valeur.szValeur));
		break;
		//Text labels
		case IHM_OBJ_PlazaName :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtPlazaName, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_PlazaName1 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtPlazaName1, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_PlazaName2 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtPlazaName2, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_PlazaName3 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtPlazaName3, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_PlazaNameTitle :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtPlazaNameTitle, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_LaneName :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtLaneName, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_LaneName1 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtLaneName1, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_LaneName2 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtLaneName2, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_LaneNameTitle :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtLaneNameTitle, ((char*)psPolice->valeur.szValeur));
 		break;

		case IHM_OBJ_PlazaNbr :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtPlazaNbr, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_PlazaNbrTitle :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtPlazaNbrTitle, ((char*)psPolice->valeur.szValeur));
		break;
 		
		case IHM_OBJ_LaneNbr :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtLaneNbr, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_LaneNbrTitle :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtLaneNbrTitle, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_CollectorID :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCollectorID, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_CollectorName :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCollectorName, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_TxtSpare1 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare1, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_TxtSpare2 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare2, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_TxtSpare3 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare3, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_TxtSpare4 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare4, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_TxtSpare5 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare5, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare6 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare6, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare7 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare7, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare8 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare8, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare9 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare9, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare10 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare10, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare11 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare11, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare12 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare12, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare13 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare13, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare14 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare14, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare15 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare15, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare16 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare16, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare17 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare17, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare18 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare18, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare19 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare19, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare20 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare20, ((char*)psPolice->valeur.szValeur));
		break;


		case IHM_OBJ_TxtSpare21 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare21, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare22 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare22, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare23 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare23, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare24 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare24, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare25 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare25, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare26 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare26, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare27 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare27, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare28 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare28, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare29 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare29, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare30 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare30, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_TxtSpare31 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare31, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare32 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare32, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare33 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare33, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare34 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare34, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare35 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare35, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare36 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare36, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare37 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare37, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare38 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare38, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare39 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare39, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare40 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare40, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_TxtSpare41 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare41, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare42 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare42, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare43 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare43, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare44 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare44, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare45 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare45, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare46 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare46, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare47 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare47, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare48 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare48, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare49 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare49, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare50 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare50, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_TxtSpare51 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare51, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare52 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare52, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare53 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare53, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare54 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare54, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare55 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare55, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare56 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare56, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare57 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare57, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare58 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare58, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare59 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare59, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtSpare60 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtSpare60, ((char*)psPolice->valeur.szValeur));
		break;


		case IHM_OBJ_TransactionNbr:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTransactionNbr, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TransactionNbrTitle:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTransactionNbrTitle, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_ReceiptNbr :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtReceiptNbr, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_ReceiptNbrTitle :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtReceiptNbrTitle, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_Axles :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtAxles, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_AxlesTitle :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtAxlesTitle, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_Balance :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtBalance, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_BalanceTitle :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtBalanceTitle, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_Class :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtClass, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_ClassTitle :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtClassTitle, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_AdditionalRevenue :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtAddRevenue, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_AdditionalRevenueTitle :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtAddRevenueTitle, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_NbCar :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtNbCar, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_Entry :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtEntry, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_EntryTitle :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtEntryTitle, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_Currency :
			//NOT USED
			//updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtCurrency, QString((char*)psPolice->valeur.szValeur));
			//updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtCurrencyTitle, "Z2_ENTRY");
		break;

		case IHM_OBJ_Fare :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtFare, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_FareTitle :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtFareTitle, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_Sale :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtSale, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_SaleTitle :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtSaleTitle, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_SaleDue :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtSaleDue, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_SaleDueTitle :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtSaleDueTitle, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_SaleDuePaid :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtSaleDuePaid, ((char*)psPolice->valeur.szValeur));
		break;


		case IHM_OBJ_TransactionType :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTransactionType, ((char*)psPolice->valeur.szValeur));
		break;
		
		case IHM_OBJ_TransactionTypeTitle :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTransactionTypeTitle, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_FareType :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtFareType, ((char*)psPolice->valeur.szValeur));
		break;
		
		case IHM_OBJ_FareTypeTitle :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtFareTypeTitle, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_TxtModeTitle :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtModeTitle, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtMode:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtMode, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtCollectorTitle :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtCollectorTitle, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtInCtrlUserTitle :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtInCtrlUserTitle, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtInCtrlUser :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtInCtrlUser, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtInCtrlUser1 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtInCtrlUser1, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtTrsContainerTitle :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtTrsContainerTitle, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtAlarmsTitle :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtAlarmsTitle, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtTrsHistoryTitle :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtTrsHistoryTitle, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtAlertesTitle :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtAlertesTitle, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_TxtCamNumPic :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtCamNumPic, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_TxtScanNumPic :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtScanNumPic, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_TxtNbCarSAS :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtTxtNbCarSAS, ((char*)psPolice->valeur.szValeur));
		break;

// 	case IHM_OBJ_Product1 :
//         IHM_STRNCPY( gsIhmInst.szProduct1, psPolice->valeur.szValeur, sizeof(gsIhmInst.szProduct1) );
//         bRes = _IhmSrvAffEnvoiLabel( WM_CSR_OBJ_Product1, psPolice->valeur.szValeur );
//         break;
// 
// 	case IHM_OBJ_Product2 :
//         IHM_STRNCPY( gsIhmInst.szProduct2, psPolice->valeur.szValeur, sizeof(gsIhmInst.szProduct2) );
//         bRes = _IhmSrvAffEnvoiLabel( WM_CSR_OBJ_Product2, psPolice->valeur.szValeur );
//         break;
// 
// 	case IHM_OBJ_Product3 :
//         IHM_STRNCPY( gsIhmInst.szProduct3, psPolice->valeur.szValeur, sizeof(gsIhmInst.szProduct3) );
//         bRes = _IhmSrvAffEnvoiLabel( WM_CSR_OBJ_Product3, psPolice->valeur.szValeur );
//         break;
// 
// 	case IHM_OBJ_Product4 :
//         IHM_STRNCPY( gsIhmInst.szProduct4, psPolice->valeur.szValeur, sizeof(gsIhmInst.szProduct4) );
//         bRes = _IhmSrvAffEnvoiLabel( WM_CSR_OBJ_Product4, psPolice->valeur.szValeur );
//         break;
// 	// END AHA

//     // ICONS  
		case IHM_OBJ_IcoLaneStatus :        
		case IHM_OBJ_IcoLaneMode :        
		case IHM_OBJ_IcoCollector :        
		case IHM_OBJ_IcoEntryGate :        
		case IHM_OBJ_IcoExitGate :        
		case IHM_OBJ_IcoCar :        
		case IHM_OBJ_IcoTrafficLight :        
		case IHM_OBJ_IcoViolation :        
		case IHM_OBJ_IcoEntryLoop :        
		case IHM_OBJ_IcoExitLoop :        
		case IHM_OBJ_IcoSpare1 :        
		case IHM_OBJ_IcoSpare2 :        
		case IHM_OBJ_IcoSpare3 :        
		case IHM_OBJ_IcoSpare4 :        
		case IHM_OBJ_IcoSpare5 :        
		case IHM_OBJ_IcoSpare6 :
		case IHM_OBJ_IcoSpare7 :
		case IHM_OBJ_IcoSpare8 :
		case IHM_OBJ_IcoSpare9 :
		case IHM_OBJ_IcoSpare10:
		case IHM_OBJ_IcoSpare11:
		case IHM_OBJ_IcoSpare12:
		case IHM_OBJ_IcoSpare13:
		case IHM_OBJ_IcoSpare14:
		case IHM_OBJ_IcoSpare15:
		case IHM_OBJ_IcoSpare16:
		case IHM_OBJ_IcoSpare17:
		case IHM_OBJ_IcoSpare18:
		case IHM_OBJ_IcoSpare19:
		case IHM_OBJ_IcoSpare20:
		case IHM_OBJ_IcoSpare21:
		case IHM_OBJ_IcoSpare22:
		case IHM_OBJ_IcoSpare23:
		case IHM_OBJ_IcoSpare24:
		case IHM_OBJ_IcoSpare25:
		case IHM_OBJ_IcoSpare26:
		case IHM_OBJ_IcoSpare27:
		case IHM_OBJ_IcoSpare28:
		case IHM_OBJ_IcoSpare29:
		case IHM_OBJ_IcoSpare30:
		case IHM_OBJ_IcoSpare31:
		case IHM_OBJ_IcoSpare32:
		case IHM_OBJ_IcoSpare33:
		case IHM_OBJ_IcoSpare34:
		case IHM_OBJ_IcoSpare35:
		case IHM_OBJ_IcoSpare36:
		case IHM_OBJ_IcoSpare37:
		case IHM_OBJ_IcoSpare38:
		case IHM_OBJ_IcoSpare39:
		case IHM_OBJ_IcoSpare40:
		case IHM_OBJ_IcoSpare41:
		case IHM_OBJ_IcoSpare42:
		case IHM_OBJ_IcoSpare43:
		case IHM_OBJ_IcoSpare44:
		case IHM_OBJ_IcoSpare45:
		case IHM_OBJ_IcoSpare46:
		case IHM_OBJ_IcoSpare47:
		case IHM_OBJ_IcoSpare48:
		case IHM_OBJ_IcoSpare49:
		case IHM_OBJ_IcoSpare50: 
		case IHM_OBJ_IcoCamera1:
		case IHM_OBJ_IcoCamera2:     
		case IHM_OBJ_IcoBeacon2:
		case IHM_OBJ_IcoSignalLight:
		case IHM_OBJ_IcoExtOpticalBarrier:

			break;

		// INSTRUCTIONS
		case IHM_OBJ_ErrPayment :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtErrPayment, ((char*)psPolice->valeur.szValeur));
		break;

		case IHM_OBJ_Payment :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtPayment, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_Remark :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtRemark, ((char*)psPolice->valeur.szValeur));
		break;
		case IHM_OBJ_Instruction :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtInstruction, ((char*)psPolice->valeur.szValeur));
			break;
		
		case IHM_OBJ_Instruction2 :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtInstruction2, ((char*)psPolice->valeur.szValeur));
			break;


		case IHM_OBJ_LstAlarms :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtLstAlarms, ((char*)psPolice->valeur.szValeur));
			break;

		case IHM_OBJ_LstWarnings :
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtLstWarnings, ((char*)psPolice->valeur.szValeur));
			break;

		case IHM_OBJ_CNT_HeaderGroup:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntHeaderGroup, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_InstructionGroup:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntInstructionGroup, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_ModeGroup:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntModeGroup, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_PaymentGroup:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntPaymentGroup, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_TrsGroup:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntTrsGroup, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Comments:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntComments, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Camera:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntCamera, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Scanner:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntScanner, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Spare1:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntSpare1, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Spare2:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntSpare2, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Spare3:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntSpare3, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Spare4:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntSpare4, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Spare5:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntSpare5, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Spare6:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntSpare6, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Spare7:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntSpare7, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Spare8:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntSpare8, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Spare9:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntSpare9, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Spare10:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntSpare10, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Spare11:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntSpare11, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Spare12:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntSpare12, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Spare13:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntSpare13, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Spare14:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntSpare14, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Spare15:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntSpare15, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Spare16:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntSpare16, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Spare17:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntSpare17, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Spare18:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntSpare18, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Spare19:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntSpare19, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_Spare20:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntSpare20, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_MiniWeb:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntMiniWeb, ((char*)psPolice->valeur.szValeur));
			break;
		case IHM_OBJ_CNT_AlarmsGroup:
			updateContainerObjectFont(MIhmVirtualObject::enuIhmVirtCntAlarmsGroup, ((char*)psPolice->valeur.szValeur));
			break;


		break;
    // INCONNU
    default :
        TRACE_W(QString("MIhmSrv::IhmSrvTraitementPolice: Unknown object Id: %1").arg((int)psPolice->objet));
        bRes = FALSE;
        break;
    }

    return bRes;	
}





/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void IhmSrvReceptionDetectionChaine ( IN struct_ihm_message *psMsg )
 * PARAMETERS: IN struct_ihm_message * psMsg : Message recu de l'application
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Traite un message de type DetectionChaine provenant de l'application
 * --------------------------------------------------------------------
 */
void MIhmSrv::IhmSrvReceptionDetectionChaine ( IN struct_ihm_message *psMsg )
{
    TRACE_D("MIhmSrv::IhmSrvReceptionDetectionChaine" );

    switch ( psMsg->entete.type_message )
    {
        
        case SRV_TYP_DEBUT :  
			TRACE_D("MIhmSrv::IhmSrvReceptionDetectionChaine: SRV_TYP_DEBUT" );

			if (IhmSrvTraitementDetectionChaine( psMsg ) )
			{
				// Pas d'erreur, Acq de la demande
				IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
										  (enum_srv_service)M_IHM_DETECTION_CHAINE, 
										  SRV_TYP_DEBUT_ACQ);
			}
			else
			{
				TRACE_W("(IhmSrvReceptionDetectionChaine) ERREUR _IhmSrvTraitementDetectionChaine retourne FALSE sur reception de SRV_TYP_DEBUT" );
				// Erreur, NAcq de la demande
				IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
										  (enum_srv_service)M_IHM_DETECTION_CHAINE, 
										  SRV_TYP_DEBUT_NACQ);
			}
		break;

        case SRV_TYP_FIN :  
			TRACE_D("MIhmSrv::IhmSrvReceptionDetectionChaine: SRV_TYP_FIN" );

			if (IhmSrvTraitementDetectionChaine( psMsg ) )
			{
				// Pas d'erreur, Acq de la demande
				IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
										  (enum_srv_service)M_IHM_DETECTION_CHAINE, 
										  SRV_TYP_FIN_ACQ);
			}
			else
			{
				TRACE_W("MIhmSrv::IhmSrvReceptionDetectionChaine: ERREUR retourne FALSE sur reception de SRV_TYP_FIN" );
				// Erreur, NAcq de la demande
				IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
										  (enum_srv_service)M_IHM_DETECTION_CHAINE, 
										  SRV_TYP_FIN_NACQ);
			}
		break;


    // INCONNU
    default :
        TRACE_W(QString("(IhmSrvReceptionDetectionChaine) ERREUR type inconnu %1").arg((int)psMsg->entete.type_message));

        // NAcq de la demande
        IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
                                  (enum_srv_service)M_IHM_DETECTION_CHAINE, 
                                  SRV_TYP_DEMANDE_INCONNUE);
        break;
    }
}

bool MIhmSrv::IhmSrvTraitementDetectionChaine ( IN struct_ihm_message * psMsg )
{
    struct_ihm_srv_detection * psDetect=&(psMsg->u.srv_detection);
    bool bRes = true;
	
	//assure that strings are zero terminated
	psDetect->CoupleId[IHM_LG_CHAINES_SRV-1] = '\0';
	psDetect->ChaineSaisie[IHM_LG_SAISIE_MAX-1] = '\0';
	
	QString sPairId = QString::fromLocal8Bit((char*)psDetect->CoupleId);


	switch(psMsg->entete.type_message)
	{
		case SRV_TYP_DEBUT :
			{
				TRACE_D(QString("MIhmSrv::IhmSrvTraitementDetectionChaine:SRV_TYP_DEBUT - BL_ID:%1, Chaine complete:%2,EnvoiePrePostambule:%3,couple ID:%4")
												.arg((int)psMsg->entete.neutre.bl_retour)
												.arg((int)psDetect->ChaineComplete)
												.arg((int)psDetect->EnvoiePrePostambule)
												.arg(sPairId));
				
				bool bPairExistInVirtual = BalIdDetectionPair::existsPair(&m_lstSrvDetectionPairs, sPairId);
				
				if(IhmDebutServiceDetection(psMsg->entete.neutre.bl_retour, sPairId))
				{
					
					MStrDetectionPair oNewPair(sPairId);
					oNewPair.m_bEnvoiePrePostambule = psDetect->EnvoiePrePostambule;
					oNewPair.m_bActive = true;

					MIhmVirtStrDetectConfig * pVirtObj = 
								(MIhmVirtStrDetectConfig *)m_pAniVirtObjects->
								getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtStrDetectConfigID);

					if(pVirtObj!=NULL)
					{
 						if(!pVirtObj->isInitialized()) pVirtObj->setInitialized();

						if(pVirtObj->updateDetectionPair(&oNewPair))
						{	
							pVirtObj->setUpdated(); //to be updated to the GUI
						}
					}	
					
					bRes = true;
				}
				
			}
			break;
		case SRV_TYP_FIN :
			{
				TRACE_D(QString("MIhmSrv::IhmSrvTraitementDetectionChaine:SRV_TYP_FIN - BL_ID:%1, Chaine complete:%2,EnvoiePrePostambule:%3,couple ID:%4")
												.arg((int)psMsg->entete.neutre.bl_retour)
												.arg((int)psDetect->ChaineComplete)
												.arg((int)psDetect->EnvoiePrePostambule)
												.arg((char*)psDetect->CoupleId));

				if(IhmFinServiceDetection(psMsg->entete.neutre.bl_retour, sPairId))
				{
					
					if(!BalIdDetectionPair::existsPair(&m_lstSrvDetectionPairs,sPairId))
					{
						MStrDetectionPair oNewPair(sPairId);
						oNewPair.m_bEnvoiePrePostambule = psDetect->EnvoiePrePostambule;
						oNewPair.m_bActive = false;

						MIhmVirtStrDetectConfig * pVirtObj = 
									(MIhmVirtStrDetectConfig *)m_pAniVirtObjects->
											getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtStrDetectConfigID);

						if(pVirtObj!=NULL)
						{
 							if(!pVirtObj->isInitialized()) pVirtObj->setInitialized();

							pVirtObj->updateDetectionPair(&oNewPair);
							{	
								pVirtObj->setUpdated(); //to be updated to the GUI
							}
						}
					}

					bRes = true;
				}
				
			}
			break;
	    default :
			TRACE_W(QString("MIhmSrv::IhmSrvTraitementDetectionChaine:ERREUR type_message inconnu %1")
												.arg((int)psMsg->entete.type_message));
			bRes = false;
			break;
    }

	return bRes;
}







bool MIhmSrv::IhmDebutServiceDetection(IN noyau_bal_id dwBalDest,
                                                 IN QString sCoupleId)
{
	BalIdDetectionPair oPair;

	oPair.blId = dwBalDest;
	oPair.sActiveDetectPair = sCoupleId;
	
	if(BalIdDetectionPair::addBlPair(&m_lstSrvDetectionPairs,&oPair))
		return true;

	return false;
}


bool MIhmSrv::IhmFinServiceDetection(IN noyau_bal_id dwBalDest,
												IN QString sCoupleId)
{
	BalIdDetectionPair oPair;

	oPair.blId = dwBalDest;
	oPair.sActiveDetectPair = sCoupleId;
	
	if(BalIdDetectionPair::removeBlPair(&m_lstSrvDetectionPairs,&oPair))
		return true;

	return false;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void IhmSrvReceptionTouches ( IN struct_ihm_message *psMsg )
 * PARAMETERS: IN struct_ihm_message * psMsg : Message recu de l'application
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Traite un message de type touche provenant de l'application
 * --------------------------------------------------------------------
 */
void MIhmSrv::IhmSrvReceptionTouches ( IN struct_ihm_message *psMsg )
{
	//assure that strings are zero terminated
	psMsg->u.srv_touches.nomgroupe[IHM_LG_CHAINES_SRV-1] = '\0';
	psMsg->u.srv_touches.command[IHM_LG_CHAINES_SRV-1] = '\0';
	psMsg->u.srv_touches.command_param[IHM_LG_CHAINES_SRV-1] = '\0';

    switch ( psMsg->entete.type_message )
    {
    // DEMANDE DE CONTRoLE DE TOUCHES
    case SRV_TYP_DEMANDE : 
		{
			TRACE_D("MIhmSrv::IhmSrvReceptionTouches: SRV_TYP_DEMANDE");
			
			noyau_bal_id   iBalId = psMsg->entete.neutre.bl_retour;
			char         * szGroup = psMsg->u.srv_touches.nomgroupe;
			int            iKey = psMsg->u.srv_touches.touche;
			BYTE           bState = psMsg->u.srv_touches.etat;

			// Mise a jour du tableau de controle de touches
			if ( m_oIhmKey.IhmKeyUpdateKeyTable(iBalId, szGroup, iKey, bState) == true)
			{
				// Pas d'erreur, acq de la demande
				IhmMessEnvoiAcquitement( psMsg->entete.neutre.bl_retour, (enum_srv_service)M_IHM_TOUCHES, SRV_TYP_DEMANDE_ACQ );
			}
			else
			{
				TRACE_W("MIhmSrv::IhmSrvReceptionTouches: ERREUR IhmKeyUpdateKeyTable retourne FALSE" );
				// NAcq de la demande
				IhmMessEnvoiAcquitement( psMsg->entete.neutre.bl_retour, (enum_srv_service)M_IHM_TOUCHES, SRV_TYP_DEMANDE_NACQ );
			}
			break;
		}
    // INCONNU
    default :
        TRACE_W(QString("MIhmSrv::IhmSrvReceptionTouches: ERREUR type inconnu %1")
												.arg((int)psMsg->entete.type_message));
        // NAcq de la demande
        IhmMessEnvoiAcquitement( psMsg->entete.neutre.bl_retour, (enum_srv_service)M_IHM_TOUCHES, SRV_TYP_DEMANDE_INCONNUE );
        break;
    }
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void IhmMessEnvoiDetectionChaine ( )
 * PARAMETERS: IN DWORD dwStringId      : Identifiant de la chaine resultant de la saisie et renvoyee
 *                                        par l'interface graphique grace a la zone d'echange
 *             IN noyau_bal_id iBalDest : Bal de destination du message
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Envoyer a l'applicatif le resultat d'une saisie par douchette
 * --------------------------------------------------------------------
 */
void MIhmSrv::IhmMessEnvoiDetectionChaine(MIhmMsgStringDetection *pMsg)
{
//    struct_ihm_message * psMsgEmis = NULL;
	struct_ihm_srv_detection oDetect;
    struct_ihm_message * psMsgEmis = NULL;


    TRACE_D("MIhmSrv::IhmMessEnvoiDetectionChaine...");

	oDetect.ChaineComplete = (pMsg->bStringComplete)?1:0;
	oDetect.EnvoiePrePostambule = (pMsg->bEnvoiePrePostambule)?1:0;
	strncpy((char*)oDetect.CoupleId, pMsg->sCoupleId.toLatin1().data(),IHM_LG_CHAINES_SRV);
	strncpy((char*)oDetect.ChaineSaisie, pMsg->sDetectedString.toLatin1().data(),IHM_LG_SAISIE_MAX);


	BalIdDetectionPair* pCurrentPair;

	for(int i=0; i<m_lstSrvDetectionPairs.size();++i)
	{	
		pCurrentPair = m_lstSrvDetectionPairs.at(i);

		if(pCurrentPair->sActiveDetectPair == (char*)oDetect.CoupleId)
		{
			m_pAniThread->MyExitAlloue("MIhmSrv::IhmMessEnvoiDetectionChaine",( struct_neutre ** )( &psMsgEmis ), sizeof( struct_ihm_message ), m_pAniThread->getPoolId());

			psMsgEmis->entete.service = (enum_srv_service)M_IHM_DETECTION_CHAINE;
			psMsgEmis->entete.type_message = (enum_srv_type)IHM_NOUVELLE_CHAINE;
    
			memcpy(&(psMsgEmis->u.srv_detection), &oDetect, sizeof(struct_ihm_srv_detection));
			
			m_pAniThread->MyExitEnvoie("MIhmSrv::IhmMessEnvoiDetectionChaine", pCurrentPair->blId, m_pAniThread->getAniBalId(), (struct_neutre *)psMsgEmis);
		
		}
	}
}





/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void IhmMessEnvoiTouche ( IN int iKey )
 * PARAMETERS: IN int iKey : code ascii de la touche a dispatcher
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Dispatcher une touche recue de l'ihm VB vers les taches la desirant 
 * --------------------------------------------------------------------
 */
void MIhmSrv::IhmMessEnvoiTouche (MIhmMsgKeyAndCmdReq *pMsg)
{
    int                  iBalId;
    struct_ihm_message * psMsgEmis = NULL;
    BOOL                 bKeySent  = FALSE;
	DWORD				 InvalidKeyDisplay=0;
	DWORD dwRes = ERROR_SUCCESS ;

    TRACE_D(QString("MIhmSrv::IhmMessEnvoiTouche: type:%1, key:%2, command:%3, cmd_param:%4")
												.arg((int)pMsg->m_type)
												.arg(pMsg->m_iKeyPressed)
												.arg(pMsg->m_sCommand)
												.arg(pMsg->m_sCommandParam));

	int iKey = pMsg->m_iKeyPressed;

	if(pMsg->m_type == enuIHM_TOUCHE_Key)
	{
		if (iKey > IHM_KB_NB_TOUCHES_MAX )
		{
			// touche non geree
			TRACE_W(QString("MIhmSrv::IhmMessEnvoiTouche: ERREUR invalid key %1")
												.arg(iKey));
			return;
		}
	}

    // Recherche des taches interessees par la touche
    for ( iBalId = 0 ; iBalId < IHM_KB_NB_TACHES_MAX ; iBalId++ )
    {
        
		if(pMsg->m_type == enuIHM_TOUCHE_Key)
		{
			if (m_oIhmKey.IhmKeyDontCare( iKey, iBalId ) ) // Si no d'une tache interessee, lui envoyer la touche
				continue;
        }
		else
		{
		
			if (m_oIhmKey.IhmCommandDontCare(iBalId ) ) // Si no d'une tache interessee, lui envoyer la command
				continue;

		}

        m_pAniThread->MyExitAlloue("MIhmSrv::IhmMessEnvoiTouche",
            ( struct_neutre ** )( &psMsgEmis ),
            sizeof( struct_ihm_message ),
			m_pAniThread->getPoolId());



        psMsgEmis->entete.service = (enum_srv_service)M_IHM_TOUCHES;
        psMsgEmis->entete.type_message = (enum_srv_type)IHM_NOUVELLE_TOUCHE;

        memset( &psMsgEmis->u, 0, sizeof( psMsgEmis->u ) );

        if(pMsg->m_type == enuIHM_TOUCHE_Key)
		{    // Renvoyer le code remappe de la touche
			psMsgEmis->u.srv_touches.type = enuIHM_TOUCHE_Key;
			int iMappedKey = m_oIhmKey.IhmKeyMap( iKey );
            psMsgEmis->u.srv_touches.touche = iMappedKey;

			if(iMappedKey==0)
			{
				TRACE_W(QString("MIhmSrv::IhmMessEnvoiTouche: Unknown key translation (reg key KeyTranslationAppli) for key %1!").arg(iKey))
			}
		}
		else
		{
			psMsgEmis->u.srv_touches.type = enuIHM_TOUCHE_Command;
            psMsgEmis->u.srv_touches.touche = pMsg->m_iKeyPressed;
			QString sCommand = pMsg->m_sCommand;
			int iMinSize = qMin((int)sizeof(psMsgEmis->u.srv_touches.command)-1, sCommand.size());
			strncpy((char*)psMsgEmis->u.srv_touches.command, sCommand.toLatin1().data(), iMinSize);
			QString sCommandParam = pMsg->m_sCommandParam;
			iMinSize = qMin((int)sizeof(psMsgEmis->u.srv_touches.command_param)-1, sCommandParam.size());
			strncpy((char*)psMsgEmis->u.srv_touches.command_param, sCommandParam.toLatin1().data(), iMinSize);
		}

		int iAniBalId = m_pAniThread->getAniBalId();
        
		m_pAniThread->MyExitEnvoie("MIhmSrv::IhmMessEnvoiTouche", iBalId, iAniBalId, (struct_neutre *)psMsgEmis );
		
		TRACE_D(QString("MIhmSrv::IhmMessEnvoiTouche: The message sent to BL_ID %1:")
												.arg(iBalId));

        bKeySent = TRUE;
    }

	if(!bKeySent)
	{
		TRACE_D(QString("MIhmSrv::IhmMessEnvoiTouche: The message was not sent to any thread!"));
	}

	if(m_oIhmKey.isConfigInvalidKeyDisplay()&&pMsg->m_type == enuIHM_TOUCHE_Key) 
	{
		if (!bKeySent)
		{  
			// Touche demandee par aucune tache, on envoie "Invalid Key" en Zone d'Erreur de l'Ihm
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtErrPayment, "ZMSG_KEY_NOT_VALID","", true);
				
		}
		else
		{
			// Clear "Invalid Key" if key expected 
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtErrPayment, "","", false);
		}
	}
}



//------------------------------------------------
//Service M_IHM_SAISIE_VISU handling functions
//------------------------------------------------

void MIhmSrv::IhmSasieVisuInitErr(MIhmMsgInputDialogInitErr *pMsg)
{

    TRACE_D(QString("MIhmSrv::IhmSasieVisuInitErr:Error in request:%1")
												.arg(pMsg->oInputReq.getDescription()));

	//remove the request from the list of requests
	MIhmVirtInputDialogReq * pVirtObj = 
			(MIhmVirtInputDialogReq *)m_pAniVirtObjects->
			getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtInputDialogID);

	if(pVirtObj!=NULL)
	{
 		if(!pVirtObj->isInitialized()) pVirtObj->setInitialized();

		pVirtObj->removeReq(&(pMsg->oInputReq));
		pVirtObj->setUpdated();

		//also update the DlgEx because it is possible that there are some dlg requests that needs to be processed
		if(pVirtObj->isEmpty() && !pVirtObj->isCancel())
				checkInputDialogEx();
	}	
		
}



void MIhmSrv::IhmSasieVisuCanceled(MIhmMsgInputDlgCanceled *pMsg)
{

    TRACE_D(QString("MIhmSrv::IhmSasieVisuCanceled:..."));

	//remove the request from the list of requests
	MIhmVirtInputDialogReq * pVirtObj = 
			(MIhmVirtInputDialogReq *)m_pAniVirtObjects->
			getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtInputDialogID);

	if(pVirtObj!=NULL)
	{
 		if(!pVirtObj->isInitialized()) pVirtObj->setInitialized();

		pVirtObj->confirmCanceled();	
		pVirtObj->setUpdated(); 
		
		//also update the DlgEx because it is possible that there are some dlg requests that needs to be processed
		if(pVirtObj->isEmpty() && !pVirtObj->isCancel())
				checkInputDialogEx();
	
	}	
		
}




void MIhmSrv::IhmMessEnvoiSasieVisu(MIhmMsgInputDialogRes *pMsg)
{
    struct_ihm_message * psMsgEmis = NULL;

    TRACE_D("MIhmSrv::IhmMessEnvoiSasieVisu:.." );

	//first verify the response
	QString sNewDesc;
	bool bResponseOK;

	if(pMsg->bCanceled)
		bResponseOK = true; //all canceled dialogs response is in good format
	else
		bResponseOK = pMsg->oInputReq.verifyResponseFormat(pMsg->sInputResult, sNewDesc);
	
	if(bResponseOK)
	{
		m_pAniThread->MyExitAlloue("MIhmSrv::IhmMessEnvoiSasieVisu",
			( struct_neutre ** )( &psMsgEmis ),
			sizeof( struct_ihm_message ),
			m_pAniThread->getPoolId());

		memset(psMsgEmis,0,sizeof( struct_ihm_message ));

		psMsgEmis->entete.service = (enum_srv_service)M_IHM_SAISIE_VISU;
		psMsgEmis->entete.type_message = (enum_srv_type)(pMsg->bCanceled ? IHM_SAISIE_ANNULEE : IHM_NOUVELLE_SAISIE);

// 		if ( pMsg->bCanceled )
// 		{
// 			strcpy( (char*)psMsgEmis->u.srv_saisie_visu.string, pMsg->sInputResult.toLatin1().data() );
// 			//psMsgEmis->u.srv_saisie_visu.b_saisie = IHMSAISIE_VISUSEUL;
// 		}
// 		else
// 		{
			strncpy((char*)psMsgEmis->u.srv_saisie_visu.string, pMsg->sInputResult.toLatin1().data(),
						qMin((int)sizeof(psMsgEmis->u.srv_saisie_visu.string)-1, pMsg->sInputResult.size()));
			
			TRACE_D(QString("MIhmSrv::IhmMessEnvoiSasieVisu: sInputResult:%1,sent string:%2")
												.arg(pMsg->sInputResult)
												.arg((char*)psMsgEmis->u.srv_saisie_visu.string));
// 		}

		psMsgEmis->u.srv_saisie_visu.b_saisie = pMsg->oInputReq.getDialogInputType();

		m_pAniThread->MyExitEnvoie("MIhmSrv::IhmMessEnvoiSasieVisu", pMsg->oInputReq.getRetBalId(), m_pAniThread->getAniBalId(), (struct_neutre *)psMsgEmis );	

	}
	else
	{
		TRACE_W(QString("MIhmSrv::IhmMessEnvoiSasieVisu:Invalid input result format: %1")
												.arg(pMsg->sInputResult));
	}

	//remove the request from the list of requests
	MIhmVirtInputDialogReq * pVirtObj = 
			(MIhmVirtInputDialogReq *)m_pAniVirtObjects->
			getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtInputDialogID);

	if(pVirtObj!=NULL)
	{
 		if(!pVirtObj->isInitialized()) pVirtObj->setInitialized();

		pVirtObj->removeReq(&(pMsg->oInputReq));

		pVirtObj->setUpdated(); //to be updated to the GUI
		
		//also update the DlgEx because it is possible that there are some dlg requests that needs to be processed
		if(pVirtObj->isEmpty() && !pVirtObj->isCancel())
				checkInputDialogEx();
	}
}





/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE BOOL _IhmSrvDemandeSaisieVisu ( IN struct_ihm_message * psMsg )
 * PARAMETERS: IN struct_ihm_message * psMsg : Message de demande de saisie a traiter
 * RETURN    : True si envoye, False sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Prepare et envoie a l'interface graphique le window message
 *             charge de provoquer une saisie ou une visu
 * --------------------------------------------------------------------
 */
bool MIhmSrv::IhmSrvDemandeSaisieVisu( IN struct_ihm_message * psMsg )
{
    bool bRes = false;
    struct_ihm_srv_saisie_visu * psSrv = &psMsg->u.srv_saisie_visu;
	MInputDialogReq oInputReq;
	
	//assure that strings are zero terminated
	psSrv->string[IHM_LG_CHAINES_SRV-1] = '\0';
	QString sDesc = QString::fromLocal8Bit((char*)psSrv->string);

    TRACE_D(QString("MIhmSrv::IhmSrvDemandeSaisieVisu: InputType:%1 Buttons:%2 Description:%3")
												.arg(psSrv->b_saisie)
												.arg(psSrv->boutons)
												.arg(sDesc));

	oInputReq.setRetBalId(psMsg->entete.neutre.bl_retour);
	oInputReq.setDialogInputType(psSrv->b_saisie);//VISU/SAISIE/S_EXT_ACTIF/S_EXT_TOUS
	oInputReq.setButtons(psSrv->boutons);
	oInputReq.setDescription(sDesc);

	if(oInputReq.isValid())
	{
		QString sDefinition = MInputDialogReq::readInputDialogDefinition(m_pAniThread->getModuleConfigKey(), oInputReq.getDialogId());
		
		if(oInputReq.setDefinition(sDefinition))
		{
			MIhmVirtInputDialogReq * pVirtObj = 
						(MIhmVirtInputDialogReq *)m_pAniVirtObjects->
						getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtInputDialogID);

			if(pVirtObj!=NULL)
			{
 				if(!pVirtObj->isInitialized()) pVirtObj->setInitialized();

				pVirtObj->addReq(&oInputReq);
				pVirtObj->setUpdated(); //to be updated to the GUI
				bRes = true;
			}	
		}
		else
		{
			TRACE_W(QString("MIhmSrv::IhmSrvDemandeSaisieVisu: Invalid definition for %1")
												.arg(oInputReq.getDialogId()));
		}
		
	}
	else
	{
			TRACE_W("MIhmSrv::IhmSrvDemandeSaisieVisu: Invalid input request!");
	}

	return bRes;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE BOOL _IhmSrvDemandeAnnulationSaisieVisu (  )
 * PARAMETERS: Aucun
 * RETURN    : True si envoye, False sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Prepare et envoie a l'interface graphique le window message
 *             charge de provoquer l'annulation d'un saisie ou d'une visu
 *             en cours.
 * --------------------------------------------------------------------
 */
bool MIhmSrv::IhmSrvDemandeAnnulationSaisieVisu()
{
    bool bRes = false;
    TRACE_D("MIhmSrv::IhmSrvDemandeAnnulationSaisieVisu: ..." );

	MIhmVirtInputDialogReq * pVirtObj = 
				(MIhmVirtInputDialogReq *)m_pAniVirtObjects->
				getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtInputDialogID);

	if(pVirtObj!=NULL)
	{
 		if(!pVirtObj->isInitialized()) pVirtObj->setInitialized();

		pVirtObj->clearAllReq();
		pVirtObj->setCancel();
		pVirtObj->setUpdated(); //to be updated to the GUI
		bRes = true;
	}
	else
	{
		TRACE_W("MIhmSrv::IhmSrvDemandeAnnulationSaisieVisu: Error not found object enuIhmVirtInputDialogID!");
	}



	return bRes;

}

void MIhmSrv::checkInputDialogEx()
{
		MIhmVirtInputDlgExReq * pVirtObjEx = 
				(MIhmVirtInputDlgExReq *)m_pAniVirtObjects->
				getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtInputDlgExID);

		if(pVirtObjEx!=NULL && 
			(!((MIhmVirtInputDlgExReq *)pVirtObjEx)->isEmpty()||((MIhmVirtInputDlgExReq *)pVirtObjEx)->isCancel()))
			pVirtObjEx->setUpdated();


}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void IhmSrvReceptionSaisie ( IN struct_ihm_message * psMsg )
 * PARAMETERS: IN struct_ihm_message * psMsg : Message recu de l'application
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Traite un message de type saisie provenant de l'application
 * --------------------------------------------------------------------
 */
void MIhmSrv::IhmSrvReceptionSaisie( IN struct_ihm_message *psMsg)
{
    switch (psMsg->entete.type_message)
    {
    // DEMANDE DE SAISIE
    case SRV_TYP_DEMANDE:
	case SRV_TYP_SET: 
        TRACE_D("MIhmSrv::IhmSrvReceptionSaisie: SRV_TYP_DEMANDE" );
		
        // Execution de la demande de saisie
        if ( IhmSrvDemandeSaisieVisu ( psMsg ) )
        { 	            
			// Pas d'erreur, Acq de la demande
            IhmMessEnvoiAcquitement( psMsg->entete.neutre.bl_retour, 
									(enum_srv_service)M_IHM_SAISIE_VISU, 
									(enum_srv_type)SRV_TYP_DEMANDE_ACQ );
        }
        else
        {
            TRACE_W("MIhmSrv::IhmSrvReceptionSaisie: ERREUR _IhmSrvDemandeSaisieVisu retourne FALSE" );
            // NAcq de la demande
            IhmMessEnvoiAcquitement( psMsg->entete.neutre.bl_retour,
									(enum_srv_service)M_IHM_SAISIE_VISU, 
									(enum_srv_type)SRV_TYP_DEMANDE_NACQ );
        }
        break;

    // DEMANDE D'ANNULATION DE SAISIE
    case SRV_TYP_DEMANDE_ANNULATION : 
        TRACE_D("MIhmSrv::IhmSrvReceptionSaisie: SRV_TYP_DEMANDE_ANNULATION" );

        // Annulation de la saisie
        if ( IhmSrvDemandeAnnulationSaisieVisu () )
        {
            // Pas d'erreur, acq de la demande
            IhmMessEnvoiAcquitement( psMsg->entete.neutre.bl_retour, 
									(enum_srv_service)M_IHM_SAISIE_VISU, 
									(enum_srv_type)SRV_TYP_DEMANDE_ACQ );
        }
        else
        {
            TRACE_W("MIhmSrv::IhmSrvReceptionSaisie: ERREUR _IhmSrvDemandeAnnulationSaisieVisu retourne FALSE" );
            // NAcq de la demande
            IhmMessEnvoiAcquitement( psMsg->entete.neutre.bl_retour, 
									(enum_srv_service)M_IHM_SAISIE_VISU, 
									(enum_srv_type)SRV_TYP_DEMANDE_NACQ );
        }
        break;


    // INCONNU
    default :
        TRACE_W(QString("MIhmSrv::IhmSrvReceptionSaisie: ERREUR type inconnu %1")
												.arg((int)psMsg->entete.type_message));

        // NAcq de la demande
        IhmMessEnvoiAcquitement( psMsg->entete.neutre.bl_retour, 
								(enum_srv_service)M_IHM_SAISIE_VISU, 
								SRV_TYP_DEMANDE_INCONNUE );
        break;
    }
}
//------------------------------------------------
// END Service M_IHM_SAISIE_VISU handling functions
//------------------------------------------------


//------------------------------------------------
//Service M_IHM_SAISIE_VISU_EX handling functions
//------------------------------------------------
void MIhmSrv::IhmSrvReceptionSaisieEx( IN struct_ihm_message *psMsg)
{
    switch (psMsg->entete.type_message)
    {
	case SRV_TYP_DEBUT :
        TRACE_D("MIhmSrv::IhmSrvReceptionSaisieEx: SRV_TYP_DEBUT" );
		IhmDebutService(M_IHM_SAISIE_VISU_EX, psMsg->entete.neutre.bl_retour);
		break;

	case SRV_TYP_FIN :
        TRACE_D("MIhmSrv::IhmSrvReceptionSaisieEx: SRV_TYP_FIN" );
		IhmFinService(M_IHM_SAISIE_VISU_EX, psMsg->entete.neutre.bl_retour);
		break;

		
		// DEMANDE DE SAISIE
    case SRV_TYP_DEMANDE :  
	case SRV_TYP_SET: 
		TRACE_D(QString("MIhmSrv::IhmSrvReceptionSaisieEx: SRV_TYP_DEMANDE:%1").arg((int)psMsg->entete.type_message));
		
        // Execution de la demande de saisie
        if ( IhmSrvDemandeSaisieVisuEx( psMsg ) )
        { 	            
			// Pas d'erreur, Acq de la demande
            IhmMessEnvoiAcquitement( psMsg->entete.neutre.bl_retour, 
									(enum_srv_service)M_IHM_SAISIE_VISU_EX, 
									(enum_srv_type)SRV_TYP_DEMANDE_ACQ ); 
        }
        else
        {
            TRACE_W("MIhmSrv::IhmSrvReceptionSaisieEx: ERREUR IhmSrvDemandeSaisieVisuEx retourne FALSE" );
            // NAcq de la demande
            IhmMessEnvoiAcquitement( psMsg->entete.neutre.bl_retour,
									(enum_srv_service)M_IHM_SAISIE_VISU_EX, 
									(enum_srv_type)SRV_TYP_DEMANDE_NACQ );
        }
        break;

    // DEMANDE D'ANNULATION DE SAISIE
    case SRV_TYP_DEMANDE_ANNULATION : 
        TRACE_D(QString("MIhmSrv::IhmSrvReceptionSaisieEx: SRV_TYP_DEMANDE_ANNULATION:%1").arg((int)psMsg->entete.type_message));

        // Annulation de la saisie
        if ( IhmSrvDemandeAnnulSaisieVisuEx(psMsg) )
        {
            // Pas d'erreur, acq de la demande
            IhmMessEnvoiAcquitement( psMsg->entete.neutre.bl_retour, 
									(enum_srv_service)M_IHM_SAISIE_VISU_EX, 
									(enum_srv_type)SRV_TYP_DEMANDE_ACQ );
        }
        else
        {
            TRACE_W("MIhmSrv::IhmSrvReceptionSaisieEx: ERREUR IhmSrvDemandeAnnulSaisieVisuEx retourne FALSE" );
            // NAcq de la demande
            IhmMessEnvoiAcquitement( psMsg->entete.neutre.bl_retour, 
									(enum_srv_service)M_IHM_SAISIE_VISU_EX, 
									(enum_srv_type)SRV_TYP_DEMANDE_NACQ );
        }
        break;


    // INCONNU
    default :
        TRACE_W(QString("MIhmSrv::IhmSrvReceptionSaisieEx: ERREUR type inconnu %1")
												.arg((int)psMsg->entete.type_message));

        // NAcq de la demande
        IhmMessEnvoiAcquitement( psMsg->entete.neutre.bl_retour, 
								(enum_srv_service)M_IHM_SAISIE_VISU_EX, 
								SRV_TYP_DEMANDE_INCONNUE );
        break;
    }
}


bool MIhmSrv::IhmSrvDemandeSaisieVisuEx( IN struct_ihm_message * psMsg )
{
    bool bRes = false;
    struct_ihm_srv_saisie_visu_ex * psSrv = &psMsg->u.srv_saisie_visu_ex;
	MInputDialogExReq oInputReq;

	//assure that strings are zero terminated
	psSrv->dialog_id[IHM_LG_CHAINES_SRV-1] = '\0';
	psSrv->data[IHM_MAX_XML_BUFFER_SIZE-1] = '\0';
	
	QString sDlgId = QString::fromLocal8Bit((char*)psSrv->dialog_id);
	QString sDlgData = QString::fromLocal8Bit((char*)psSrv->data);

    TRACE_D(QString( "MIhmSrv::IhmSrvDemandeSaisieVisuEx: Dialog_ID:%1 Description:%2")
												.arg(sDlgId)
												.arg(sDlgData));

	oInputReq.init(m_pAniThread->getModuleConfigKey(),sDlgId);
	
	if(!oInputReq.isValid())
	{
		TRACE_W("MIhmSrv::IhmSrvDemandeSaisieVisuEx: Invalid definition of input request!");
		return false;
	}

	oInputReq.setRetBalId(psMsg->entete.neutre.bl_retour);
	
	if (oInputReq.getDialogType() != MInputDialogExReq::enuSAISIE_EX_CUSTOM_PLUGIN_DLG)
		oInputReq.setDescription(sDlgData, psSrv->is_file);
	else
		oInputReq.setCustomDlg_Parameters(sDlgData, psSrv->is_file);

	if(oInputReq.isValid())
	{

		MIhmVirtInputDlgExReq * pVirtObj = 
						(MIhmVirtInputDlgExReq *)m_pAniVirtObjects->
						getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtInputDlgExID);

		if(pVirtObj!=NULL)
		{
 			if(!pVirtObj->isInitialized()) pVirtObj->setInitialized();

			pVirtObj->addReq(&oInputReq);
			pVirtObj->setUpdated(); //to be updated to the GUI
			bRes = true;
		}	
		
	}
	else
	{
		TRACE_W("MIhmSrv::IhmSrvDemandeSaisieVisuEx: Invalid description of input request!");
	}

	return bRes;
}



bool MIhmSrv::IhmSrvDemandeAnnulSaisieVisuEx(IN struct_ihm_message * psMsg )
{
    bool bRes = false;
    struct_ihm_srv_saisie_visu_ex * psSrv = &psMsg->u.srv_saisie_visu_ex;
	
	QString sDlgId = QString::fromLocal8Bit((char*)psSrv->dialog_id);
	
    TRACE_D(QString( "MIhmSrv::IhmSrvDemandeAnnulSaisieVisuEx: Dialog_ID:%1 ")
												.arg(sDlgId));

	MIhmVirtInputDlgExReq * pVirtObj = 
				(MIhmVirtInputDlgExReq *)m_pAniVirtObjects->
				getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtInputDlgExID);

	if(pVirtObj!=NULL)
	{	
 		if(!pVirtObj->isInitialized()) pVirtObj->setInitialized();

		bool bClearAll = (sDlgId.isEmpty())?true:false;
		
		if(bClearAll) //remove all - one by one and send message for each
		{	
			MInputDialogExReq* pCurrent = pVirtObj->takeFirstReq();
			
			while(pCurrent!=NULL)
			{
				//send annule response 
				IhmMessEnvoiSasieVisuExAnnule(pCurrent);
				delete pCurrent;

				pCurrent = pVirtObj->takeFirstReq();
			}

			pVirtObj->setCancel();

			pVirtObj->setUpdated(); //to be updated to the GUI
			bRes = true;
		}
		else //remove one if found
		{
			
			MInputDialogExReq* pFirst = pVirtObj->getFirstReq();
			MInputDialogExReq* pCurrent;
			
			pCurrent = pVirtObj->takeReq(sDlgId);

			if(pCurrent!=NULL)
			{
				//send annule response 
				IhmMessEnvoiSasieVisuExAnnule(pCurrent);
				
				if(pFirst == pCurrent)
					pVirtObj->setCancel();

				delete pCurrent;
				pVirtObj->setUpdated(); //to be updated to the GUI
				bRes = true;
			}
		}
	}
	else
	{
		TRACE_W("MIhmSrv::IhmSrvDemandeAnnulSaisieVisuEx: Error not found object enuIhmVirtInputDialogID!");
	}

	return bRes;
}



BOOL WINAPI EnvoiDemandeurSasieVisuExAnnule (PVOID psMsgOrig , noyau_bal_id BalDemandeur, PVOID Param)
{
    MIhmSrv *pIhmSrv = MIhmSrv::getGlobalMSrv();
	struct_ihm_message * psMsgToSend;
	struct_ihm_message * psMsgOriginal = (struct_ihm_message *)psMsgOrig;

    // test si demandeur interesse ou non
    if ( psMsgOriginal!=NULL)
    {
        if(Alloue((struct_neutre **)(&psMsgToSend),sizeof(struct_ihm_message),pIhmSrv->getPoolId())!= NOYAU_OK)
		{
			TRACE_W("EnvoiDemandeurSasieVisuExAnnule: Error while calling Alloue! Calling ExitBAD!");
			ExitBad();
		}
        else
		{
			// recopie du message
			*psMsgToSend = *psMsgOriginal;
        
			if(Envoie(BalDemandeur, pIhmSrv->getAniBalId(),(struct_neutre *)(psMsgToSend)) != NOYAU_OK)
			{
				TRACE_W("EnvoiDemandeurSasieVisuExAnnule: Error! Message not sent!");
				ExitLibere((struct_neutre **)&psMsgToSend);
			}
		}
    }

    return TRUE;
}


void MIhmSrv::IhmMessEnvoiSasieVisuExAnnule(MInputDialogExReq* pCurrent)
{
    struct_ihm_message * psMsgToSend = NULL;

	TRACE_D(QString("MIhmSrv::IhmMessEnvoiSasieVisuExAnnule: DialogId:%1!").arg(pCurrent->getDialogId()))

	m_pAniThread->MyExitAlloue("MIhmSrv::IhmMessEnvoiSasieVisuExAnnule", 
		( struct_neutre ** )( &psMsgToSend ),
		sizeof( struct_ihm_message ),
		m_pAniThread->getPoolId());

	memset(psMsgToSend,0,sizeof( struct_ihm_message ));

	psMsgToSend->entete.service = (enum_srv_service)M_IHM_SAISIE_VISU_EX;
	psMsgToSend->entete.type_message = (enum_srv_type)IHM_RESULT_DEMANDE_ANNULATION;

	QString sDialogId = pCurrent->getDialogId();
	strncpy((char*)psMsgToSend->u.srv_saisie_visu_ex.dialog_id, sDialogId.toLatin1().data(),
				qMin((int)sizeof(psMsgToSend->u.srv_saisie_visu_ex.dialog_id)-1, sDialogId.size()));

	strcpy( (char*)psMsgToSend->u.srv_saisie_visu_ex.data, "" );

	SrvPourChaqueDemandeur (m_hService[M_IHM_SAISIE_VISU_EX], EnvoiDemandeurSasieVisuExAnnule, (PVOID)psMsgToSend);

	ExitLibere((struct_neutre **)&psMsgToSend);
}



void MIhmSrv::IhmMessEnvoiSasieVisuEx(MIhmMsgInputDlgExRes *pMsg)
{
    struct_ihm_message * psMsgEmis = NULL;
	QString sDialogId = pMsg->oInputReq.getDialogId();

	TRACE_D(QString("MIhmSrv::IhmMessEnvoiSasieVisuEx: DialogId:%1!").arg(sDialogId));
  
	//remove the request from the list of requests
	MIhmVirtInputDlgExReq * pVirtObj = 
			(MIhmVirtInputDlgExReq *)m_pAniVirtObjects->
			getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtInputDlgExID);

	if(pVirtObj!=NULL)
	{
		m_pAniThread->VirtualKyb_Hide();

		if(pVirtObj->removeReq(&(pMsg->oInputReq)))
		{
			//----------------------------------------------------
			//  Send the result only if there was the request of this type in the list of requested 
			//  SasieVisuEx dialogs ...
			m_pAniThread->MyExitAlloue("MIhmSrv::IhmMessEnvoiSasieVisuEx",
				( struct_neutre ** )( &psMsgEmis ),
				sizeof( struct_ihm_message ),
				m_pAniThread->getPoolId());

			memset(psMsgEmis,0,sizeof( struct_ihm_message ));

			psMsgEmis->entete.service = (enum_srv_service)M_IHM_SAISIE_VISU_EX;
			psMsgEmis->entete.type_message = (enum_srv_type)(pMsg->bCanceled ? IHM_SAISIE_ANNULEE : IHM_NOUVELLE_SAISIE);

			psMsgEmis->u.srv_saisie_visu_ex.is_file = 0;
			strncpy((char*)psMsgEmis->u.srv_saisie_visu_ex.dialog_id, sDialogId.toLatin1().data(),
						qMin((int)sizeof(psMsgEmis->u.srv_saisie_visu_ex.dialog_id)-1, sDialogId.size()));

			if ( pMsg->bCanceled )
			{
				strcpy( (char*)psMsgEmis->u.srv_saisie_visu_ex.data, "" );
			}
			else
			{
				strncpy((char*)psMsgEmis->u.srv_saisie_visu_ex.data, pMsg->sInputResult.toLatin1().data(),
							qMin((int)sizeof(psMsgEmis->u.srv_saisie_visu_ex.data)-2, pMsg->sInputResult.size()));
				
				TRACE_D(QString("MIhmSrv::IhmMessEnvoiSasieVisuEx: sInputResult:%1")
													.arg(pMsg->sInputResult));
			}

			m_pAniThread->MyExitEnvoie("MIhmSrv::IhmMessEnvoiSasieVisuEx",  pMsg->oInputReq.getRetBalId(), m_pAniThread->getAniBalId(), (struct_neutre *)psMsgEmis );	
			//----------------------------------------------------

			pVirtObj->setUpdated(); 

			//also update the Dlgs because it is possible that there are some dlg requests that needs to be processed
			if(pVirtObj->isEmpty() && !pVirtObj->isCancel())
					checkInputDialog();
		}
		else
		{
			TRACE_W(QString("MIhmSrv::IhmMessEnvoiSasieVisuEx: The request for DialogId:%1 was canceled by the main app before the user interaction!").arg(sDialogId));
		}

	}

}


void MIhmSrv::IhmSasieVisuExInitErr(MIhmMsgInputDlgExInitErr *pMsg)
{
    TRACE_W(QString("MIhmSrv::IhmSasieVisuExInitErr:..Error detected in request: %1")
												.arg(pMsg->oInputReq.getDialogId()));

	//remove the request from the list of requests
	MIhmVirtInputDlgExReq * pVirtObj = 
			(MIhmVirtInputDlgExReq *)m_pAniVirtObjects->
			getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtInputDlgExID);

	if(pVirtObj!=NULL)
	{
		//send the IHM_RESULT_DEMANDE_ANNULATION like it was requested to cancel so the main application can 
		// be informed about the error 
		IhmMessEnvoiSasieVisuExAnnule(&(pMsg->oInputReq));
		m_pAniThread->VirtualKyb_Hide();

		pVirtObj->removeReq(&(pMsg->oInputReq));
		pVirtObj->setUpdated(); //to be updated to the GUI

		//also update the Dlgs because it is possible that there are some dlg requests that needs to be processed
		if(pVirtObj->isEmpty() && !pVirtObj->isCancel())
				checkInputDialog();
	}	
}


void MIhmSrv::IhmSasieVisuExCanceled(MIhmMsgInputDlgExCanceled *pMsg)
{
    TRACE_D(QString("MIhmSrv::IhmSasieVisuExCanceled:.."));

	//remove the request from the list of requests
	MIhmVirtInputDlgExReq * pVirtObj = 
			(MIhmVirtInputDlgExReq *)m_pAniVirtObjects->
			getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtInputDlgExID);

	if(pVirtObj!=NULL)
	{
		m_pAniThread->VirtualKyb_Hide();

		pVirtObj->confirmCanceled();
		pVirtObj->setUpdated();

		//also update the normal saisie dialogs(non saisie_ex dialogs) 
		//because it is possible that there are some dlg requests that needs to be processed
		if(pVirtObj->isEmpty() && !pVirtObj->isCancel())
				checkInputDialog();

	}	
}


void MIhmSrv::IhmSasieVisuExOpened(MIhmMsgInputDlgExOpened *pMsg)
{
    TRACE_D(QString("MIhmSrv::IhmSasieVisuExOpened:.."));

	//remove the request from the list of requests
	MIhmVirtInputDlgExReq * pVirtObj = 
			(MIhmVirtInputDlgExReq *)m_pAniVirtObjects->
			getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtInputDlgExID);

	if(pVirtObj!=NULL)
	{
		//show virtual keyboard	
		m_pAniThread->VirtualKyb_Show(pMsg->oInputReq.getVirKybConfig(),pMsg->oInputReq.getVirtPosX(),pMsg->oInputReq.getVirtPosY());

	}	
}


void MIhmSrv::IhmLoginDlgOpened(MIhmMsgLoginDlgOpened *pMsg)
{
    TRACE_D(QString("MIhmSrv::IhmLoginDlgOpened:.."));
	//show default virtual keyboard	
	m_pAniThread->VirtualKyb_Show();
		
}

void MIhmSrv::IhmLoginDlgClosed(MIhmMsgLoginDlgClosed *pMsg)
{
    TRACE_D(QString("MIhmSrv::IhmLoginDlgClosed:.."));
	//hide default virtual keyboard	
	m_pAniThread->VirtualKyb_Hide();
	
}

void MIhmSrv::checkInputDialog()
{
	//also update the Dlg because it is possible that there are some dlg requests
	//that needs to be processed
	MIhmVirtInputDialogReq * pVirtObjDlg = 
			(MIhmVirtInputDialogReq *)m_pAniVirtObjects->
			getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtInputDialogID);

	if(pVirtObjDlg!=NULL && 
		(!((MIhmVirtInputDialogReq *)pVirtObjDlg)->isEmpty()||((MIhmVirtInputDialogReq *)pVirtObjDlg)->isCancel()))
			pVirtObjDlg->setUpdated();

}

//------------------------------------------------
// ***** END Service M_IHM_SAISIE_VISU_EX handling functions
//------------------------------------------------




noyau_pool_id MIhmSrv::getPoolId() 
{
	return m_pAniThread->getPoolId();
}

noyau_bal_id MIhmSrv::getAniBalId() 
{
	return m_pAniThread->getAniBalId();

};


BOOL WINAPI EnvoiDemandeurAsyncInput (PVOID psMsgOrig , noyau_bal_id BalDemandeur, PVOID Param)
{
    MIhmSrv *pIhmSrv = MIhmSrv::getGlobalMSrv();
	struct_ihm_message * psMsgToSend;
	struct_ihm_message * psMsgOriginal = (struct_ihm_message *)psMsgOrig;

    // test si demandeur interesse ou non
    if ( psMsgOriginal!=NULL)
    {
        if(Alloue((struct_neutre **)(&psMsgToSend),sizeof(struct_ihm_message),pIhmSrv->getPoolId())!= NOYAU_OK)
		{
			TRACE_W("EnvoiDemandeurAsyncInput: Error while calling Alloue! Calling ExitBAD!");
			ExitBad();
		}
        else
		{
			// recopie du message
			*psMsgToSend = *psMsgOriginal;
        
			if(Envoie(BalDemandeur, pIhmSrv->getAniBalId(),(struct_neutre *)(psMsgToSend)) != NOYAU_OK)
			{
				TRACE_W("EnvoiDemandeurAsyncInput: Error! Message not sent!");
				ExitLibere((struct_neutre **)&psMsgToSend);
			}
		}
    }

    return TRUE;
}


void MIhmSrv::IhmProcessMessAsyncInputRsp(MIhmMsgAsyncInputRsp *pMsg)
{
    struct_ihm_message * psMsgToSend = NULL;

	//first verify the response
	QString sNewValues = pMsg->m_sEncStrValues;

	MIhmVirtAsyncInput * pVirtObj = 
			(MIhmVirtAsyncInput *)m_pAniVirtObjects->
			getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtAsyncInputsID);

	QString sCurrentUpdatedValues;

	if(pVirtObj!=NULL)
	{
		pVirtObj->updateInputValues(sNewValues);
		sCurrentUpdatedValues = pVirtObj->m_pAsyncReq->getEncStrValues();
		//do not update to widgets so it will not be refreshed to the current in control user
		//pVirtObj->setUpdated();
	}	

	if(sNewValues!="" && sCurrentUpdatedValues!="")
	{
		m_pAniThread->MyExitAlloue("MIhmSrv::IhmProcessMessAsyncInputRsp",
			( struct_neutre ** )( &psMsgToSend ),
			sizeof( struct_ihm_message ),
			m_pAniThread->getPoolId());

		memset(psMsgToSend,0,sizeof( struct_ihm_message ));

		psMsgToSend->entete.service = (enum_srv_service)M_IHM_ASYNC_INPUT;
		psMsgToSend->entete.type_message = (enum_srv_type)IHM_NOUVELLE_SAISIE;

		psMsgToSend->u.srv_async_input.is_file = 0;
		strncpy((char*)psMsgToSend->u.srv_async_input.data, sCurrentUpdatedValues.toLatin1().data(),
					qMin((int)sizeof(psMsgToSend->u.srv_async_input.data)-1, sCurrentUpdatedValues.size()));


		SrvPourChaqueDemandeur (m_hService[M_IHM_ASYNC_INPUT], EnvoiDemandeurAsyncInput, (PVOID)psMsgToSend);

		ExitLibere((struct_neutre **)&psMsgToSend);
	}


}



void MIhmSrv::updateTabControl(MIhmVirtualObject::enumVirtualObjectId eId, 
															const char * szValue, 
															int iProperty, 
															bool bVisible)
{
	MIhmVirtTabView * pVirtObj = 
				(MIhmVirtTabView *)m_pAniVirtObjects->
					getVirtualObjectByID(eId);

	if(pVirtObj!=NULL)
	{
		if(!pVirtObj->isInitialized())
		{
			pVirtObj->setInitialized();
			pVirtObj->setUpdated(); 
		}
		
		if(pVirtObj->setVisible(bVisible))
		{	
			pVirtObj->setUpdated();		
		}
		
		QString sValue = szValue;
		QStringList sLst = sValue.split("|");
		
		if(sLst.at(0)=="")
			sLst.removeAt(0);

		if(sLst.size()>0)
		{
			pVirtObj->m_slstTabText = sLst;
			
			if(iProperty<pVirtObj->m_slstTabText.count())
				pVirtObj->m_iCurrentIndex = iProperty;
			else
				pVirtObj->m_iCurrentIndex = 0;

			pVirtObj->setUpdated();	
		}
	}	
}




void MIhmSrv::updateListObject(MIhmVirtualObject::enumVirtualObjectId eId, 
															const char * szValue, 
															int iProperty, 
															bool bVisible)
{
	QString sValue = QString::fromLocal8Bit(szValue);

	TRACE_D(QString("MIhmSrv::updateListObject: Value:%1;Property:%2;").arg(sValue).arg(iProperty));

	
	MIhmVirtList * pVirtObj = 
				(MIhmVirtList *)m_pAniVirtObjects->
					getVirtualObjectByID(eId);

	if(pVirtObj!=NULL)
	{
		if(!pVirtObj->isInitialized())
		{
			pVirtObj->setInitialized();
			pVirtObj->setUpdated(); 
		}

		if(pVirtObj->setVisible(bVisible))
		{	
			pVirtObj->setUpdated();		
		}

		if(iProperty==0) //remove entry
		{
			for(int i=0; i<pVirtObj->m_slistIDs.size(); ++i)
			{
				if(pVirtObj->m_slistIDs.at(i)==sValue)
				{
					TRACE_D(QString("MIhmSrv::updateListObject: Value:%1 removed!").arg(sValue));
					pVirtObj->m_slistIDs.removeAt(i);
					pVirtObj->setUpdated();
					break;
				}
			}
		}
		else //add entry if it is not in the list
		{
			if(!pVirtObj->m_slistIDs.contains(sValue))
			{
				pVirtObj->m_slistIDs.append(sValue);
				pVirtObj->setUpdated();
			}
			else
			{
				TRACE_D(QString("MIhmSrv::updateListObject: Value:%1 already in list!").arg(sValue));
			}
		}

	}	
}



void MIhmSrv::updateTextObjectLabel(MIhmVirtualObject::enumVirtualObjectId eId, 
															const char* szNewValue, 
															const char* szToolTip,
															bool bVisible)
{
	TRACE_D(QString("MIhmSrv::updateTextObjectLabel: Value:%1;Visible:%2;").arg(szNewValue).arg(bVisible?1:0));

	MIhmVirtText * pVirtObj = 
				(MIhmVirtText *)m_pAniVirtObjects->
					getVirtualObjectByID(eId);

	if(pVirtObj!=NULL)
	{	
		if(!pVirtObj->isInitialized())
		{
			pVirtObj->setInitialized();
			pVirtObj->setUpdated(); 
		}

		bool bChanged,bTemp;
		bChanged = pVirtObj->setVisible(bVisible);
		if(!IHM_STR_IS_DO_NOT_CHANGE(szNewValue))
		{
			bTemp = pVirtObj->setTextID(QString::fromLocal8Bit(szNewValue));bChanged = bChanged || bTemp;
		}	
		
		if(!IHM_STR_IS_DO_NOT_CHANGE(szToolTip))
		{
			bTemp = pVirtObj->setToolTip(QString::fromLocal8Bit(szToolTip));bChanged = bChanged || bTemp;
		}	

		if(bChanged) 
			pVirtObj->setUpdated(); //if something was changed
	}
}

void MIhmSrv::updateIconObjectImage(MIhmVirtualObject::enumVirtualObjectId eId, 
															const char* szNewValue, 
															const char* szToolTip,
															bool bVisible)
{

	TRACE_D(QString("MIhmSrv::updateIconObjectImage: Value:%1;Visible:%2;").arg(szNewValue).arg(bVisible?1:0));

	MIhmVirtIco * pVirtObj = 
				(MIhmVirtIco *)m_pAniVirtObjects->
					getVirtualObjectByID(eId);


	if(pVirtObj!=NULL)
	{	
		if(!pVirtObj->isInitialized())
		{
			pVirtObj->setInitialized();
			pVirtObj->setUpdated(); 
		}

		bool bChanged,bTemp;
		bChanged = pVirtObj->setVisible(bVisible);

		if(!IHM_STR_IS_DO_NOT_CHANGE(szNewValue))
		{
			bTemp = pVirtObj->setImageID(QString::fromLocal8Bit(szNewValue));bChanged = bChanged || bTemp;
		}

		if(!IHM_STR_IS_DO_NOT_CHANGE(szToolTip))
		{
			bTemp = pVirtObj->setToolTip(QString::fromLocal8Bit(szToolTip));bChanged = bChanged || bTemp;
		}	
		
		if(bChanged) 
			pVirtObj->setUpdated(); //if something was changed
	}
}

void MIhmSrv::updateTableViewReq(MIhmVirtualObject::enumVirtualObjectId eId, char * szDataToShow,bool bExternalFile, enum_aff_table_command eCmd, bool bVisible)
{

	MIhmVirtTableViewReq * pVirtObj = 
				(MIhmVirtTableViewReq *)m_pAniVirtObjects->
					getVirtualObjectByID(eId);

	// add the new command with unique timestamp in a list of command structures in virtual object
	// if the content is to be read from the file it should be read to a buffer that is part of the command structure
	// if command is of type clear rows the commands list is cleared and basic timestamp is reset
	// copying of the virtual object command list is to be optimized using timestamps
	
	if(pVirtObj!=NULL)
	{	
		if(!pVirtObj->isInitialized())	pVirtObj->setInitialized();
		
		QString sDataToShow = QString::fromUtf8(szDataToShow);

		TRACE_D(QString("MIhmSrv::updateTableViewReq:eId:%1 eCmd:%2 Data:%3").arg(MIhmVirtualObject::getNameForId(eId)).arg(eCmd).arg(sDataToShow));

		if(bExternalFile)
		{
			if((eCmd==enuIHM_AFF_TABLE_CMD_PREPEND_ROWS||eCmd==enuIHM_AFF_TABLE_CMD_APPEND_ROWS||eCmd==enuIHM_AFF_TABLE_CMD_RESET_AND_APPEND_ROWS))
			{
				QString sFullPath = m_pAniThread->getAniInitData()->getCommFileFullPath(sDataToShow);
				sDataToShow = MHelpFuncs::loadFileContent(sFullPath);

				if(sDataToShow.isEmpty())
				{
					TRACE_W(QString("MIhmSrv::updateTableViewReq: Error loading file %1!").arg(sFullPath));
				}
			}
		}
		
		pVirtObj->appendNewCommand(sDataToShow,eCmd, m_pAniThread->getAniInitData()->getMaxTableRowsInMemory());
		pVirtObj->setVisible(bVisible); 
		pVirtObj->setUpdated(); 
	}
}

void MIhmSrv::updateHMenuViewReq(MIhmVirtualObject::enumVirtualObjectId eId,  struct_ihm_menu_affichage * menu, bool bVisible)
{

	MIhmVirtHMenuViewReq * pVirtObj = 
		(MIhmVirtHMenuViewReq *)m_pAniVirtObjects->
		getVirtualObjectByID(eId);
	
	if(pVirtObj!=NULL)
	{	
		if(!pVirtObj->isInitialized())
		{
			pVirtObj->setInitialized();
			pVirtObj->setUpdated(); 
		}
		
		bool bChanged,bTemp;
		bChanged = pVirtObj->setVisible(bVisible);
		bTemp = pVirtObj->setMenuContent(QString::fromLocal8Bit(menu->szMenuContent));bChanged = bChanged || bTemp;
		bTemp = pVirtObj->setUseExternalFile((menu->bUseExternalFile!=0)?true:false);bChanged = bChanged || bTemp;
		bTemp = pVirtObj->setDoNotReloadMenu((menu->bDoNotReloadMenu==0)?false:true);bChanged = bChanged || bTemp;

		if(bChanged) 
			pVirtObj->setUpdated(); //if something was changed
		
	}
}			


void MIhmSrv::updateContainerReq(MIhmVirtualObject::enumVirtualObjectId eId, bool bVisible)
{
	MIhmVirtContainer * pVirtObj = 
				(MIhmVirtContainer *)m_pAniVirtObjects->
					getVirtualObjectByID(eId);


	if(pVirtObj!=NULL)
	{	
		if(!pVirtObj->isInitialized())	pVirtObj->setInitialized();

		if(pVirtObj->setVisible(bVisible)) 
			pVirtObj->setUpdated(); 
	}
}			


//-----------------------------------------------------------------------
// Processing of video objects that are implemented re-stream video objects 
// that are implemented using restream_server process IHM_OBJ_RVideoView1 & IHM_OBJ_RVideoView2
// Not using CAM objects
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

void MIhmSrv::updateVideoViewReq(MIhmVirtualObject::enumVirtualObjectId eId, struct_ihm_video_affichage stVideo, bool bVisible)
{

	MIhmVirtVideoViewReq * pVirtObj = (MIhmVirtVideoViewReq *)m_pAniVirtObjects->
						getVirtualObjectByID(eId);
	
	
	if(pVirtObj!=NULL)
	{	
		TRACE_D(QString("MIhmSrv::updateVideoViewReq: Object:%1 : Command:%2")
						.arg(MIhmVirtualObject::getNameForId(eId))
						.arg(stVideo.eCmd));



		if(!pVirtObj->isInitialized())	pVirtObj->setInitialized();

		if(pVirtObj->isVisible()!=bVisible)
		{
			pVirtObj->setVisible(bVisible); 
			pVirtObj->setUpdated();
		}
		
		switch(stVideo.eCmd)
		{
		case IHM_VIDEO_ON:
			{
				MIhmMsgVideoOn * pNewMsg = new MIhmMsgVideoOn(eId);
				m_pAniThread->emitMessageFromAni(pNewMsg);
			}
			break;
		case IHM_VIDEO_OFF:
			{
				MIhmMsgVideoOff * pNewMsg = new MIhmMsgVideoOff(eId);
				m_pAniThread->emitMessageFromAni(pNewMsg);
			}
			break;
		case IHM_VIDEO_FREEZE:
			{
				MIhmMsgVideoFreeze * pNewMsg = new MIhmMsgVideoFreeze(eId);
				m_pAniThread->emitMessageFromAni(pNewMsg);
			}
			break;
		case IHM_VIDEO_UNFREEZE:
			{
				MIhmMsgVideoUnfreeze * pNewMsg = new MIhmMsgVideoUnfreeze(eId);
				m_pAniThread->emitMessageFromAni(pNewMsg);
			}
			break;
		case IHM_VIDEO_ZOOM_IN:
			{
				pVirtObj->m_nZoom = pVirtObj->m_nZoom + 1;
				int iMaxZoom = m_pAniThread->getMaxVideoZoom();

				if(pVirtObj->m_nZoom > iMaxZoom)
						pVirtObj->m_nZoom = iMaxZoom;

				MIhmMsgVideoZoom * pNewMsg = new MIhmMsgVideoZoom(eId);
				pNewMsg->m_iNewZoom = pVirtObj->m_nZoom;
				m_pAniThread->emitMessageFromAni(pNewMsg);
			}
			break;
		case IHM_VIDEO_ZOOM_OUT:
			{
				pVirtObj->m_nZoom = pVirtObj->m_nZoom - 1;
				if(pVirtObj->m_nZoom<0)
					pVirtObj->m_nZoom = 0;
				MIhmMsgVideoZoom * pNewMsg = new MIhmMsgVideoZoom(eId);
				pNewMsg->m_iNewZoom = pVirtObj->m_nZoom;
				m_pAniThread->emitMessageFromAni(pNewMsg);
			}
			break;
		case IHM_VIDEO_ZOOM:
			{
				pVirtObj->m_nZoom = stVideo.cmdval.dwValeur;

				MIhmMsgVideoZoom * pNewMsg = new MIhmMsgVideoZoom(eId);
				pNewMsg->m_iNewZoom = pVirtObj->m_nZoom;
				m_pAniThread->emitMessageFromAni(pNewMsg);

			}
			break;
		case IHM_VIDEO_SELECT_SRC:
			{
				pVirtObj->m_nSource = stVideo.cmdval.dwValeur;

				MIhmMsgVideoSelectSrc * pNewMsg = new MIhmMsgVideoSelectSrc(eId);
				pNewMsg->m_iNewSource = pVirtObj->m_nSource;
				m_pAniThread->emitMessageFromAni(pNewMsg);

 				pVirtObj->setUpdated();
			}
			break;
		case IHM_VIDEO_SAVE_BMP:
		case IHM_VIDEO_SAVE_JPG:
			{
				MIhmMsgVideoSave * pNewMsg = new MIhmMsgVideoSave(eId);
				pNewMsg->m_sTargetFileName = stVideo.cmdval.szValeur;
				pNewMsg->m_bSaveJpg = (stVideo.eCmd==IHM_VIDEO_SAVE_JPG)?true:false;
				m_pAniThread->emitMessageFromAni(pNewMsg);
			}
			break;
		default:
			break;
		}
	}
}	
		

// Video response
//-----------------------------------------------------------------------

void MIhmSrv::IhmProcessMessVideoFreezeRsp(MIhmMsgVideoFreezeRsp *pMsg)
{
    struct_ihm_message * psMsgToSend = NULL;

	if(pMsg!=NULL)
	{
		enum_ihm_objets eSourceIHM_Obj = (enum_ihm_objets)MIhmVirtualObject::mapVirtObjIdToIhmObjId((MIhmVirtualObject::enumVirtualObjectId)pMsg->getTargetVirtObject());

		TRACE_D(QString("MIhmSrv::IhmProcessMessVideoFreezeRsp: Result:%1 : Object:%2 = %3")
						.arg(pMsg->m_iResult)
						.arg(MIhmVirtualObject::getNameForId((MIhmVirtualObject::enumVirtualObjectId)pMsg->getTargetVirtObject()))
						.arg((int)eSourceIHM_Obj));

		m_pAniThread->MyExitAlloue("MIhmSrv::IhmProcessMessVideoFreezeRsp",
			( struct_neutre ** )( &psMsgToSend ),
			sizeof( struct_ihm_message ),
			m_pAniThread->getPoolId());

		memset(psMsgToSend,0,sizeof( struct_ihm_message ));

		psMsgToSend->entete.service = (enum_srv_service)M_IHM_AFFICHAGE;
		psMsgToSend->entete.type_message = (enum_srv_type)IHM_VIDEO_CMD_RESULT;
		
		psMsgToSend->u.srv_affichage.objet = eSourceIHM_Obj;
		psMsgToSend->u.srv_affichage.valeur.video.eCmd = (pMsg->m_iResult==1)?	IHM_VIDEO_FREEZE_OK:IHM_VIDEO_FREEZE_NOK;

		
		SrvPourChaqueDemandeur (m_hService[M_IHM_AFFICHAGE], EnvoiDemandeurAsyncInput, (PVOID)psMsgToSend);

		ExitLibere((struct_neutre **)&psMsgToSend);
	}

}

void MIhmSrv::IhmProcessMessVideoSaveRsp(MIhmMsgVideoSaveRsp *pMsg)
{
    struct_ihm_message * psMsgToSend = NULL;

	if(pMsg!=NULL)
	{
		TRACE_D(QString("MIhmSrv::IhmProcessMessVideoSaveRsp: Result:%1").arg(pMsg->m_iResult));

		m_pAniThread->MyExitAlloue("MIhmSrv::IhmProcessMessVideoSaveRsp",
			( struct_neutre ** )( &psMsgToSend ),
			sizeof( struct_ihm_message ),
			m_pAniThread->getPoolId());

		memset(psMsgToSend,0,sizeof( struct_ihm_message ));

		psMsgToSend->entete.service = (enum_srv_service)M_IHM_AFFICHAGE;
		psMsgToSend->entete.type_message = (enum_srv_type)IHM_VIDEO_CMD_RESULT;
		
		psMsgToSend->u.srv_affichage.objet = (enum_ihm_objets)MIhmVirtualObject::mapVirtObjIdToIhmObjId((MIhmVirtualObject::enumVirtualObjectId)pMsg->getTargetVirtObject());
		psMsgToSend->u.srv_affichage.valeur.video.eCmd = (pMsg->m_iResult==1)?	IHM_VIDEO_SAVE_OK:IHM_VIDEO_SAVE_NOK;
	
		SrvPourChaqueDemandeur (m_hService[M_IHM_AFFICHAGE], EnvoiDemandeurAsyncInput, (PVOID)psMsgToSend);

		ExitLibere((struct_neutre **)&psMsgToSend);
	}
}

void MIhmSrv::IhmProcessMessRVideoUpdateSrc(MIhmMsgRVideoUpdateSrc *pMsg)
{
	if(pMsg!=NULL)
	{
		TRACE_D(QString("MIhmSrv::IhmProcessMessRVideoUpdateSrc:StreamUrl local:[%1], StreamUrl remote:[%2], BaseUrl:[%3]")
														.arg(pMsg->m_sStreamSourceUrlLocal)
														.arg(pMsg->m_sStreamSourceUrlRemote)
														.arg(pMsg->m_sBaseUrl));


		MIhmVirtVideoViewReq * pVirtObj = (MIhmVirtVideoViewReq *)m_pAniVirtObjects->
							getVirtualObjectByID((MIhmVirtualObject::enumVirtualObjectId)pMsg->getTargetVirtObject());
		
		
		if(pVirtObj!=NULL)
		{	
			if(!pVirtObj->isInitialized())	pVirtObj->setInitialized();

			pVirtObj->m_sStreamSourceUrlLocal = pMsg->m_sStreamSourceUrlLocal;
			pVirtObj->m_sStreamSourceUrlRemote = pMsg->m_sStreamSourceUrlRemote;
			pVirtObj->m_sBaseUrl = pMsg->m_sBaseUrl;
		
			pVirtObj->setUpdated();
		}
		else
		{
			TRACE_W(QString( "MIhmSrv::IhmProcessMessRVideoStatusUpdate ... object Id: %1 not found!")
														.arg((int)pMsg->getTargetVirtObject()));


		}
	}
}		

void MIhmSrv::IhmProcessMessRVideoUpdateOnOff(MIhmMsgRVideoUpdateOnOff *pMsg)
{
	if(pMsg!=NULL)
	{
		TRACE_D(QString("MIhmSrv::IhmProcessMessRVideoUpdateOnOff:VideoOn:%1")
															.arg(pMsg->m_bVideoOn));


		MIhmVirtVideoViewReq * pVirtObj = (MIhmVirtVideoViewReq *)m_pAniVirtObjects->
							getVirtualObjectByID((MIhmVirtualObject::enumVirtualObjectId)pMsg->getTargetVirtObject());
		
		
		if(pVirtObj!=NULL)
		{	
			if(!pVirtObj->isInitialized())	pVirtObj->setInitialized();

			pVirtObj->m_bVideoOn = pMsg->m_bVideoOn;
			pVirtObj->setUpdated();
		}
		else
		{
			TRACE_W(QString( "MIhmSrv::IhmProcessMessRVideoStatusUpdate ... object Id: %1 not found!")
														.arg((int)pMsg->getTargetVirtObject()));

		}
	}
}		
				
//-----------------------------------------------------------------------


void MIhmSrv::updateButtonObject(MIhmVirtualObject::enumVirtualObjectId eId, 
														const char* szImage, 
														const char* szText,
														const char* szToolTip,
														enum_ihm_object_enabled isEnabled, 
														bool bVisible)
{
	MIhmVirtButton * pVirtObj = 
				(MIhmVirtButton *)m_pAniVirtObjects->
					getVirtualObjectByID(eId);


	if(pVirtObj!=NULL)
	{	
		TRACE_D(QString( "MIhmSrv::updateButtonObject: Id: %1 Img[%2], Text[%3] Enabled[%4], Visible[%5] !")
												.arg(eId)
												.arg(IHM_STR_IS_DO_NOT_CHANGE(szImage)?"**DO_NOT_CHANGE**":szImage)
												.arg(IHM_STR_IS_DO_NOT_CHANGE(szText)?"**DO_NOT_CHANGE**":szText)
												.arg((int)isEnabled)
												.arg(bVisible?"TRUE":"FALSE"));
		
		if(!pVirtObj->isInitialized())
		{
			pVirtObj->setInitialized();
			pVirtObj->setUpdated(); 
		}
		
		bool bChanged,bTemp;
		bChanged = pVirtObj->setVisible(bVisible);

		if(!IHM_STR_IS_DO_NOT_CHANGE(szImage))
		{
			bTemp = pVirtObj->setImageID(QString::fromLocal8Bit(szImage));	bChanged = bChanged || bTemp;
		}

		if(!IHM_STR_IS_DO_NOT_CHANGE(szText))
		{
			bTemp = pVirtObj->setTextID(QString::fromLocal8Bit(szText));	bChanged = bChanged || bTemp;
		}
		
		if(!IHM_STR_IS_DO_NOT_CHANGE(szToolTip))
		{
			bTemp = pVirtObj->setToolTip(QString::fromLocal8Bit(szToolTip));bChanged = bChanged || bTemp;
		}

		if(isEnabled == enuIHM_OBJECT_ENABLED)
		{	bTemp = pVirtObj->setEnabled(true);	bChanged = bChanged || bTemp;
		
		}
		else if(isEnabled == enuIHM_OBJECT_DISABLED)
		{	
			bTemp = pVirtObj->setEnabled(false);	bChanged = bChanged || bTemp;
		}
		// else do not change the button state
			
		

		if(bChanged) 
			pVirtObj->setUpdated(); //if something was changed
	}
}

bool MIhmSrv::AssureAffichageStructStrings(struct_ihm_srv_affichage * psAff)
{
	psAff->szToolTip[IHM_LG_CHAINES_SRV-1] = '\0';


    switch (psAff->objet)
    {
		
		case IHM_OBJ_PlazaName:
		case IHM_OBJ_PlazaName1:
		case IHM_OBJ_PlazaName2:
		case IHM_OBJ_PlazaName3:
		case IHM_OBJ_PlazaNameTitle:
		case IHM_OBJ_PlazaNbr:
		case IHM_OBJ_PlazaNbrTitle:
		case IHM_OBJ_LaneNbr:
		case IHM_OBJ_LaneNbrTitle:
		case IHM_OBJ_CollectorID:
		case IHM_OBJ_CollectorName:      

		case IHM_OBJ_TxtSpare1:		
		case IHM_OBJ_TxtSpare2:			
		case IHM_OBJ_TxtSpare3:
		case IHM_OBJ_TxtSpare4:
		case IHM_OBJ_TxtSpare5:
		case IHM_OBJ_TxtSpare6:
		case IHM_OBJ_TxtSpare7:
		case IHM_OBJ_TxtSpare8:
		case IHM_OBJ_TxtSpare9:
		case IHM_OBJ_TxtSpare10:
		case IHM_OBJ_TxtSpare11:
		case IHM_OBJ_TxtSpare12:
		case IHM_OBJ_TxtSpare13:
		case IHM_OBJ_TxtSpare14:
		case IHM_OBJ_TxtSpare15:
		case IHM_OBJ_TxtSpare16:
		case IHM_OBJ_TxtSpare17:
		case IHM_OBJ_TxtSpare18:
		case IHM_OBJ_TxtSpare19:
		case IHM_OBJ_TxtSpare20:

		case IHM_OBJ_TxtSpare21:		
		case IHM_OBJ_TxtSpare22:			
		case IHM_OBJ_TxtSpare23:
		case IHM_OBJ_TxtSpare24:
		case IHM_OBJ_TxtSpare25:
		case IHM_OBJ_TxtSpare26:
		case IHM_OBJ_TxtSpare27:
		case IHM_OBJ_TxtSpare28:
		case IHM_OBJ_TxtSpare29:
		case IHM_OBJ_TxtSpare30:
		case IHM_OBJ_TxtSpare31:		
		case IHM_OBJ_TxtSpare32:			
		case IHM_OBJ_TxtSpare33:
		case IHM_OBJ_TxtSpare34:
		case IHM_OBJ_TxtSpare35:
		case IHM_OBJ_TxtSpare36:
		case IHM_OBJ_TxtSpare37:
		case IHM_OBJ_TxtSpare38:
		case IHM_OBJ_TxtSpare39:
		case IHM_OBJ_TxtSpare40:
		case IHM_OBJ_TxtSpare41:		
		case IHM_OBJ_TxtSpare42:			
		case IHM_OBJ_TxtSpare43:
		case IHM_OBJ_TxtSpare44:
		case IHM_OBJ_TxtSpare45:
		case IHM_OBJ_TxtSpare46:
		case IHM_OBJ_TxtSpare47:
		case IHM_OBJ_TxtSpare48:
		case IHM_OBJ_TxtSpare49:
		case IHM_OBJ_TxtSpare50:
		case IHM_OBJ_TxtSpare51:		
		case IHM_OBJ_TxtSpare52:			
		case IHM_OBJ_TxtSpare53:
		case IHM_OBJ_TxtSpare54:
		case IHM_OBJ_TxtSpare55:
		case IHM_OBJ_TxtSpare56:
		case IHM_OBJ_TxtSpare57:
		case IHM_OBJ_TxtSpare58:
		case IHM_OBJ_TxtSpare59:
		case IHM_OBJ_TxtSpare60:


		case IHM_OBJ_TransactionNbr:		
		case IHM_OBJ_TransactionNbrTitle:
		case IHM_OBJ_ReceiptNbr:
		case IHM_OBJ_ReceiptNbrTitle:
		case IHM_OBJ_Axles:
		case IHM_OBJ_AxlesTitle:
		case IHM_OBJ_Balance:	
		case IHM_OBJ_BalanceTitle:
		case IHM_OBJ_Class:              
		case IHM_OBJ_ClassTitle: 
		case IHM_OBJ_AdditionalRevenue:              
		case IHM_OBJ_AdditionalRevenueTitle: 

		case IHM_OBJ_NbCar:				
		case IHM_OBJ_Entry:
		case IHM_OBJ_EntryTitle:
		case IHM_OBJ_Currency:
		case IHM_OBJ_CurrencyTitle:
		case IHM_OBJ_Fare:
		case IHM_OBJ_FareTitle:		
		case IHM_OBJ_Sale:
		case IHM_OBJ_SaleTitle:
		case IHM_OBJ_SaleDue:			
		case IHM_OBJ_SaleDueTitle:	
		case IHM_OBJ_SaleDuePaid:			
		case IHM_OBJ_TransactionType:
		case IHM_OBJ_TransactionTypeTitle:
		case IHM_OBJ_LaneName:          
		case IHM_OBJ_LaneName1:          
		case IHM_OBJ_LaneName2:          
		case IHM_OBJ_LaneNameTitle:	
		case IHM_OBJ_FareType:
		case IHM_OBJ_FareTypeTitle:
		
		case IHM_OBJ_TxtModeTitle:		
		case IHM_OBJ_TxtMode:		
		case IHM_OBJ_TxtCollectorTitle:		
		case IHM_OBJ_TxtInCtrlUserTitle:		
		case IHM_OBJ_TxtInCtrlUser:	
		case IHM_OBJ_TxtInCtrlUser1:	
		case IHM_OBJ_TxtTrsContainerTitle:		
		case IHM_OBJ_TxtAlarmsTitle:		
		case IHM_OBJ_TxtTrsHistoryTitle:		
		case IHM_OBJ_TxtAlertesTitle:		
		case IHM_OBJ_TxtCamNumPic:
		case IHM_OBJ_TxtScanNumPic:
		case IHM_OBJ_TxtNbCarSAS:
		case IHM_OBJ_ErrPayment:		
		case IHM_OBJ_Payment:
		case IHM_OBJ_Remark:
		case IHM_OBJ_Instruction:   
		case IHM_OBJ_Instruction2:
		case IHM_OBJ_CurrentDate:	
				psAff->valeur.szValeur[IHM_MAX_XML_BUFFER_SIZE-1] = '\0';
			break;

		case IHM_OBJ_IcoLaneStatus:
		case IHM_OBJ_IcoLaneMode:
		case IHM_OBJ_IcoCollector:       
		case IHM_OBJ_IcoEntryGate:		
		case IHM_OBJ_IcoExitGate:		
		case IHM_OBJ_IcoCar:
		case IHM_OBJ_IcoTrafficLight:
		case IHM_OBJ_IcoViolation:       
		case IHM_OBJ_IcoEntryLoop:		
		case IHM_OBJ_IcoExitLoop:
		case IHM_OBJ_IcoBeacon:
		case IHM_OBJ_IcoEntOpticalBarrier:
		case IHM_OBJ_IcoSpare1:
		case IHM_OBJ_IcoSpare2:
		case IHM_OBJ_IcoSpare3:	
		case IHM_OBJ_IcoSpare4:			
		case IHM_OBJ_IcoSpare5:	
		case IHM_OBJ_IcoSpare6:
		case IHM_OBJ_IcoSpare7:
		case IHM_OBJ_IcoSpare8:
		case IHM_OBJ_IcoSpare9:
		case IHM_OBJ_IcoSpare10:
		case IHM_OBJ_IcoSpare11:
		case IHM_OBJ_IcoSpare12:
		case IHM_OBJ_IcoSpare13:
		case IHM_OBJ_IcoSpare14:
		case IHM_OBJ_IcoSpare15:
		case IHM_OBJ_IcoSpare16:
		case IHM_OBJ_IcoSpare17:
		case IHM_OBJ_IcoSpare18:
		case IHM_OBJ_IcoSpare19:
		case IHM_OBJ_IcoSpare20:
		case IHM_OBJ_IcoSpare21:
		case IHM_OBJ_IcoSpare22:
		case IHM_OBJ_IcoSpare23:
		case IHM_OBJ_IcoSpare24:
		case IHM_OBJ_IcoSpare25:
		case IHM_OBJ_IcoSpare26:
		case IHM_OBJ_IcoSpare27:
		case IHM_OBJ_IcoSpare28:
		case IHM_OBJ_IcoSpare29:
		case IHM_OBJ_IcoSpare30:
		case IHM_OBJ_IcoSpare31:
		case IHM_OBJ_IcoSpare32:
		case IHM_OBJ_IcoSpare33:
		case IHM_OBJ_IcoSpare34:
		case IHM_OBJ_IcoSpare35:
		case IHM_OBJ_IcoSpare36:
		case IHM_OBJ_IcoSpare37:
		case IHM_OBJ_IcoSpare38:
		case IHM_OBJ_IcoSpare39:
		case IHM_OBJ_IcoSpare40:
		case IHM_OBJ_IcoSpare41:
		case IHM_OBJ_IcoSpare42:
		case IHM_OBJ_IcoSpare43:
		case IHM_OBJ_IcoSpare44:
		case IHM_OBJ_IcoSpare45:
		case IHM_OBJ_IcoSpare46:
		case IHM_OBJ_IcoSpare47:
		case IHM_OBJ_IcoSpare48:
		case IHM_OBJ_IcoSpare49:
		case IHM_OBJ_IcoSpare50: 
		case IHM_OBJ_IcoCamera1:
		case IHM_OBJ_IcoCamera2:

		case IHM_OBJ_IcoBeacon2:
		case IHM_OBJ_IcoSignalLight:
		case IHM_OBJ_IcoExtOpticalBarrier:
		case IHM_OBJ_Image:	
		case IHM_OBJ_Image2:	
				psAff->valeur.szValeur[IHM_MAX_XML_BUFFER_SIZE-1] = '\0';
			break;

		case IHM_OBJ_LstAlarms:
		case IHM_OBJ_LstWarnings:
				psAff->valeur.szValeur[IHM_MAX_XML_BUFFER_SIZE-1] = '\0';
			break;

		case IHM_OBJ_btnClass:
		case IHM_OBJ_btnEntryPoint:
		case IHM_OBJ_btnEntryPointNbr:
		case IHM_OBJ_btnMode:
		case IHM_OBJ_btnArrowOn:
		case IHM_OBJ_btnArrowOff:
		case IHM_OBJ_btnCrossOn:
		case IHM_OBJ_btnCrossOff:
		case IHM_OBJ_btnDsrcOn:
		case IHM_OBJ_btnDsrcOff:
		case IHM_OBJ_btnMagneticOn:
		case IHM_OBJ_btnMagneticOff:
		case IHM_OBJ_btnCollectorOn:
		case IHM_OBJ_btnCollectorOff:

		case IHM_OBJ_btnCamera:
		case IHM_OBJ_btnScanner:
		case IHM_OBJ_btnComment:

		case IHM_OBJ_btnGabarite:
		case IHM_OBJ_btnEntryGate:
		case IHM_OBJ_btnExitGate:
		case IHM_OBJ_btnCar3:
		case IHM_OBJ_btnCar2:
		case IHM_OBJ_btnCar:
		case IHM_OBJ_btnEntryLoop:
		case IHM_OBJ_btnExitLoop:
		case IHM_OBJ_btnTblBeacon:

		case IHM_OBJ_btnCamGrab:
		case IHM_OBJ_btnCamHide:
		case IHM_OBJ_btnScanGrab:
		case IHM_OBJ_btnScanHide:	
		case IHM_OBJ_btnScanSend:
		case IHM_OBJ_btnScanSave:
		case IHM_OBJ_btnCommHide:
		case IHM_OBJ_btnExternalDisplay:
		case IHM_OBJ_btnExternalDisplay2:
		case IHM_OBJ_btnTakeControl:
		case IHM_OBJ_btnReturnControl:

		//spare buttons
		case IHM_OBJ_Product1:
		case IHM_OBJ_Product2:
		case IHM_OBJ_Product3:
		case IHM_OBJ_Product4:
		case IHM_OBJ_Product5:	
		
		case IHM_OBJ_btnSpeedLimitOn:	
		case IHM_OBJ_btnSpeedLimitOff:	
		case IHM_OBJ_btnWarningOn:	
		case IHM_OBJ_btnWarningOff:	
		case IHM_OBJ_btnDsrcTSAOn:
		case IHM_OBJ_btnDsrcTSAOff:

		case IHM_OBJ_btnSpare1:
		case IHM_OBJ_btnSpare2:
		case IHM_OBJ_btnSpare3:
		case IHM_OBJ_btnSpare4:
		case IHM_OBJ_btnSpare5:
		case IHM_OBJ_btnSpare6:
		case IHM_OBJ_btnSpare7:
		case IHM_OBJ_btnSpare8:
		case IHM_OBJ_btnSpare9:
		case IHM_OBJ_btnSpare10:

		case IHM_OBJ_btnSpare11:
		case IHM_OBJ_btnSpare12:
		case IHM_OBJ_btnSpare13:
		case IHM_OBJ_btnSpare14:
		case IHM_OBJ_btnSpare15:
		case IHM_OBJ_btnSpare16:
		case IHM_OBJ_btnSpare17:
		case IHM_OBJ_btnSpare18:
		case IHM_OBJ_btnSpare19:
		case IHM_OBJ_btnSpare20:

		case IHM_OBJ_btnSpare21:
		case IHM_OBJ_btnSpare22:
		case IHM_OBJ_btnSpare23:
		case IHM_OBJ_btnSpare24:
		case IHM_OBJ_btnSpare25:
		case IHM_OBJ_btnSpare26:
		case IHM_OBJ_btnSpare27:
		case IHM_OBJ_btnSpare28:
		case IHM_OBJ_btnSpare29:
		case IHM_OBJ_btnSpare30:
				psAff->valeur.button.szValeurImg[IHM_LG_CHAINES_SRV-1] = '\0';
				psAff->valeur.button.szValeurText[IHM_LG_CHAINES_SRV-1] = '\0';
			break;

		case IHM_OBJ_TableView1:
		case IHM_OBJ_TableView2:
		case IHM_OBJ_TableView3:
				psAff->valeur.table_view.szContent[IHM_MAX_XML_BUFFER_SIZE-1] = '\0';
			break;

		//horizontal menu
		case IHM_OBJ_HMenuView1:
		case IHM_OBJ_HMenuView2:
				psAff->valeur.menu.szMenuContent[IHM_MAX_XML_BUFFER_SIZE-1] = '\0';
			break;
				
		case IHM_OBJ_RVideoView1:
		case IHM_OBJ_RVideoView2:
				psAff->valeur.video.cmdval.szValeur[IHM_LG_CHAINES_SRV-1] = '\0';
			break;

		case IHM_OBJ_TabControl1:
		case IHM_OBJ_TabControl2:
		case IHM_OBJ_TabControl3:
		case IHM_OBJ_TabControl4:
				psAff->valeur.szValeur[IHM_MAX_XML_BUFFER_SIZE-1] = '\0';
			break;
				
		case IHM_OBJ_CNT_HeaderGroup:
		case IHM_OBJ_CNT_InstructionGroup:
		case IHM_OBJ_CNT_ModeGroup:	
		case IHM_OBJ_CNT_PaymentGroup:	
		case IHM_OBJ_CNT_TrsGroup:	

		case IHM_OBJ_CNT_Comments:
		case IHM_OBJ_CNT_Camera:
		case IHM_OBJ_CNT_Scanner:

		case IHM_OBJ_CNT_Spare1:
		case IHM_OBJ_CNT_Spare2:
		case IHM_OBJ_CNT_Spare3:
		case IHM_OBJ_CNT_Spare4:
		case IHM_OBJ_CNT_Spare5:
		case IHM_OBJ_CNT_Spare6:
		case IHM_OBJ_CNT_Spare7:
		case IHM_OBJ_CNT_Spare8:
		case IHM_OBJ_CNT_Spare9:
		case IHM_OBJ_CNT_Spare10:
		case IHM_OBJ_CNT_Spare11:
		case IHM_OBJ_CNT_Spare12:
		case IHM_OBJ_CNT_Spare13:
		case IHM_OBJ_CNT_Spare14:
		case IHM_OBJ_CNT_Spare15:
		case IHM_OBJ_CNT_Spare16:
		case IHM_OBJ_CNT_Spare17:
		case IHM_OBJ_CNT_Spare18:
		case IHM_OBJ_CNT_Spare19:
		case IHM_OBJ_CNT_Spare20:
		case IHM_OBJ_CNT_MiniWeb:
		case IHM_OBJ_CNT_AlarmsGroup:
				; //NOP	
			break;
		case IHM_OBJ_OpenLink:
				psAff->valeur.link.szUrlValue[IHM_LG_CHAINES_SRV-1] = '\0';
			break;
		case IHM_OBJ_OpenLoginDlg:
				psAff->valeur.login_dlg.szLogin[IHM_LG_CHAINES_SRV-1] = '\0'; 
			break;
		case IHM_OBJ_AboutDlg:
				psAff->valeur.szValeur[IHM_MAX_XML_BUFFER_SIZE-1] = '\0'; 
			break;

		default:
			break;
		
	}

	return true;
}



BOOL MIhmSrv::IhmSrvTraitementAffichage ( IN struct_ihm_message * psMsg )
{
    struct_ihm_srv_affichage * psAff = &( psMsg->u.srv_affichage );
    BOOL bRes = TRUE;

	//assure that strings are zero terminated
	if(!AssureAffichageStructStrings(psAff))
		return FALSE;

	bool bVisible = (psAff->visibility==enuIHM_AFF_OBJ_HIDDEN)?false:true;

    TRACE_D(QString( "MIhmSrv::IhmSrvTraitementAffichage ... object %1 Id: %2 ")
												.arg(MHelpFuncs::getStringFromObjectId((int)psAff->objet))
												.arg((int)psAff->objet));

    switch ( psAff->objet )
    {

		case IHM_OBJ_All_Object:
			;  //not used
		break;

		//Text labels
		case IHM_OBJ_PlazaName :
		case IHM_OBJ_PlazaName1 :
		case IHM_OBJ_PlazaName2 :
		case IHM_OBJ_PlazaName3 :
		case IHM_OBJ_LaneName :
		case IHM_OBJ_LaneName1 :
		case IHM_OBJ_LaneName2 :
		case IHM_OBJ_PlazaNbr :
		case IHM_OBJ_LaneNbr :
		case IHM_OBJ_CollectorID :
		case IHM_OBJ_CollectorName :
		case IHM_OBJ_TxtSpare1 :
		case IHM_OBJ_TxtSpare2 :
		case IHM_OBJ_TxtSpare3 :
		case IHM_OBJ_TxtSpare4 :
		case IHM_OBJ_TxtSpare5 :
		case IHM_OBJ_TxtSpare6 :
		case IHM_OBJ_TxtSpare7 :
		case IHM_OBJ_TxtSpare8 :
		case IHM_OBJ_TxtSpare9 :
		case IHM_OBJ_TxtSpare10:
		case IHM_OBJ_TxtSpare11:
		case IHM_OBJ_TxtSpare12:
		case IHM_OBJ_TxtSpare13:
		case IHM_OBJ_TxtSpare14:
		case IHM_OBJ_TxtSpare15:
		case IHM_OBJ_TxtSpare16:
		case IHM_OBJ_TxtSpare17:
		case IHM_OBJ_TxtSpare18:
		case IHM_OBJ_TxtSpare19:
		case IHM_OBJ_TxtSpare20:
		case IHM_OBJ_TxtSpare21 :
		case IHM_OBJ_TxtSpare22 :
		case IHM_OBJ_TxtSpare23 :
		case IHM_OBJ_TxtSpare24 :
		case IHM_OBJ_TxtSpare25 :
		case IHM_OBJ_TxtSpare26 :
		case IHM_OBJ_TxtSpare27 :
		case IHM_OBJ_TxtSpare28 :
		case IHM_OBJ_TxtSpare29 :
		case IHM_OBJ_TxtSpare30:
		case IHM_OBJ_TxtSpare31 :
		case IHM_OBJ_TxtSpare32 :
		case IHM_OBJ_TxtSpare33 :
		case IHM_OBJ_TxtSpare34 :
		case IHM_OBJ_TxtSpare35 :
		case IHM_OBJ_TxtSpare36 :
		case IHM_OBJ_TxtSpare37 :
		case IHM_OBJ_TxtSpare38 :
		case IHM_OBJ_TxtSpare39 :
		case IHM_OBJ_TxtSpare40 :
		case IHM_OBJ_TxtSpare41 :
		case IHM_OBJ_TxtSpare42 :
		case IHM_OBJ_TxtSpare43 :
		case IHM_OBJ_TxtSpare44 :
		case IHM_OBJ_TxtSpare45 :
		case IHM_OBJ_TxtSpare46 :
		case IHM_OBJ_TxtSpare47 :
		case IHM_OBJ_TxtSpare48 :
		case IHM_OBJ_TxtSpare49 :
		case IHM_OBJ_TxtSpare50:
		case IHM_OBJ_TxtSpare51 :
		case IHM_OBJ_TxtSpare52 :
		case IHM_OBJ_TxtSpare53 :
		case IHM_OBJ_TxtSpare54 :
		case IHM_OBJ_TxtSpare55 :
		case IHM_OBJ_TxtSpare56 :
		case IHM_OBJ_TxtSpare57 :
		case IHM_OBJ_TxtSpare58 :
		case IHM_OBJ_TxtSpare59 :
		case IHM_OBJ_TxtSpare60:
		case IHM_OBJ_TransactionNbr:
		case IHM_OBJ_ReceiptNbr :
		case IHM_OBJ_Axles :
		case IHM_OBJ_Balance :
		case IHM_OBJ_Class :
		case IHM_OBJ_AdditionalRevenue :
		case IHM_OBJ_NbCar :
		case IHM_OBJ_Entry :
		case IHM_OBJ_Fare :
		case IHM_OBJ_Sale :
		case IHM_OBJ_SaleDue :
		case IHM_OBJ_SaleDuePaid :
		case IHM_OBJ_TransactionType :
		case IHM_OBJ_FareType :
		case IHM_OBJ_TxtMode:
		case IHM_OBJ_TxtInCtrlUser:
		case IHM_OBJ_TxtInCtrlUser1:	
		case IHM_OBJ_TxtCamNumPic :
		case IHM_OBJ_TxtScanNumPic :
		case IHM_OBJ_TxtNbCarSAS :
			updateTextObjectLabel(MIhmVirtualObject::mapIhmObjIdToVirtObjId((enum_ihm_objets)psAff->objet), 
									(char*)psAff->valeur.szValeur,
									(char*)psAff->szToolTip, 
									bVisible);
		break;

		case IHM_OBJ_PlazaNameTitle:
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtPlazaNameTitle, "Z1_PLAZA_NAME","",bVisible);
			break;

		case IHM_OBJ_LaneNameTitle :
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtLaneNameTitle, "Z1_LANE_NAME","", bVisible);
			break;
 		
		case IHM_OBJ_PlazaNbrTitle :
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtPlazaNbrTitle, "Z1_PLAZA_NUMBER","", bVisible);
			break;

		case IHM_OBJ_LaneNbrTitle :
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtLaneNbrTitle, "Z1_LANE_NUMBER","", bVisible);
			break;
		case IHM_OBJ_TransactionNbrTitle :
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtTransactionNbrTitle, "Z2_TRS_NBR","", bVisible);
		break;

		case IHM_OBJ_ReceiptNbrTitle :
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtReceiptNbrTitle, "Z2_RECEPT_NBR","", bVisible);
			break;

		case IHM_OBJ_AxlesTitle :
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtAxlesTitle, "Z2_AXLES_TITLE","", bVisible);
			break;

		case IHM_OBJ_BalanceTitle :
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtBalanceTitle, "Z2_BALANCE_TITLE", "", bVisible);
			break;

		case IHM_OBJ_ClassTitle :
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtClassTitle, "Z2_CLASS_TITLE","", bVisible);
			break;

		case IHM_OBJ_AdditionalRevenueTitle :
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtAddRevenueTitle, "Z2_ADDITIONAL_REVENUE_TITLE","", bVisible);
			break;

		case IHM_OBJ_EntryTitle :
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtEntryTitle, "Z2_ENTRY_TITLE","", bVisible);
			break;

		case IHM_OBJ_Currency :
			//NOT USED
			//updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtCurrency, QString((char*)psAff->valeur.szValeur));
			//updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtCurrencyTitle, "Z2_ENTRY");
		break;


		case IHM_OBJ_FareTitle :
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtFareTitle, "Z2_FARE_TITLE", "", bVisible);
			break;
		case IHM_OBJ_SaleTitle :
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtSaleTitle, "Z2_SALE_TITLE", "", bVisible);
			break;
		case IHM_OBJ_SaleDueTitle :
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtSaleDueTitle, "Z2_SALE_DUE_TITLE", "", bVisible);
			break;
		case IHM_OBJ_TransactionTypeTitle:
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtTransactionTypeTitle, "Z2_TRS_TYPE_TITLE","", bVisible);
			break;
		case IHM_OBJ_FareTypeTitle:
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtFareTypeTitle, "Z2_FARE_TYPE_TITLE","", bVisible);
			break;
		case IHM_OBJ_TxtModeTitle:
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtTxtModeTitle, "Z1_MODE_TITLE", "", bVisible);
			break;
		case IHM_OBJ_TxtCollectorTitle:
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtTxtCollectorTitle, "Z1_COLLECTOR_TITLE","", bVisible);
			break;
		case IHM_OBJ_TxtInCtrlUserTitle:
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtTxtInCtrlUserTitle, "Z1_IN_CONTROL_TITLE","", bVisible);
			break;
		case IHM_OBJ_TxtTrsContainerTitle:
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtTxtTrsContainerTitle, "Z1_TRS_CONTAINER_TITLE", "", bVisible);
			break;
		case IHM_OBJ_TxtAlarmsTitle:
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtTxtAlarmsTitle, "Z1_TRS_ALARMS_TITLE","", bVisible);
			break;
		case IHM_OBJ_TxtTrsHistoryTitle :
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtTxtTrsHistoryTitle, "Z1_TRS_HISTORY_TITLE","", bVisible);
			break;
		case IHM_OBJ_TxtAlertesTitle:
			updateTextObjectLabel(MIhmVirtualObject::enuIhmVirtTxtAlertesTitle, "Z1_TRS_ALERTES_TITLE","", bVisible);
			break;

		//-------------------------------------------------
		//buttons...
		case IHM_OBJ_btnClass:
		case IHM_OBJ_btnEntryPoint:
		case IHM_OBJ_btnEntryPointNbr:
		case IHM_OBJ_btnMode:
		case IHM_OBJ_btnArrowOn:
		case IHM_OBJ_btnArrowOff:
		case IHM_OBJ_btnCrossOn:
		case IHM_OBJ_btnCrossOff:
		case IHM_OBJ_btnDsrcOn:
		case IHM_OBJ_btnDsrcOff:
		case IHM_OBJ_btnMagneticOn:
		case IHM_OBJ_btnMagneticOff:
		case IHM_OBJ_btnCollectorOn:
		case IHM_OBJ_btnCollectorOff:
		
		case IHM_OBJ_btnCamera:
		case IHM_OBJ_btnScanner:
		case IHM_OBJ_btnComment:
		
		case IHM_OBJ_btnGabarite:
		case IHM_OBJ_btnEntryGate:
		case IHM_OBJ_btnExitGate:
		case IHM_OBJ_btnCar3:
		case IHM_OBJ_btnCar2:
		case IHM_OBJ_btnCar:
		case IHM_OBJ_btnEntryLoop:
		case IHM_OBJ_btnExitLoop:
		case IHM_OBJ_btnTblBeacon:
		
		case IHM_OBJ_btnCamGrab:
		case IHM_OBJ_btnCamHide:
		case IHM_OBJ_btnScanGrab:
		case IHM_OBJ_btnScanHide:	
		case IHM_OBJ_btnScanSend:
		case IHM_OBJ_btnScanSave:
		case IHM_OBJ_btnCommHide:
		case IHM_OBJ_btnExternalDisplay:
		case IHM_OBJ_btnExternalDisplay2:
		case IHM_OBJ_btnTakeControl:
		case IHM_OBJ_btnReturnControl:
			
		//spare buttons
		case IHM_OBJ_Product1:
		case IHM_OBJ_Product2:
		case IHM_OBJ_Product3:
		case IHM_OBJ_Product4:
		case IHM_OBJ_Product5:	
		
		case IHM_OBJ_btnSpeedLimitOn:	
		case IHM_OBJ_btnSpeedLimitOff:	
		case IHM_OBJ_btnWarningOn:	
		case IHM_OBJ_btnWarningOff:	
		case IHM_OBJ_btnDsrcTSAOn:
		case IHM_OBJ_btnDsrcTSAOff:
		
		case IHM_OBJ_btnSpare1:
		case IHM_OBJ_btnSpare2:
		case IHM_OBJ_btnSpare3:
		case IHM_OBJ_btnSpare4:
		case IHM_OBJ_btnSpare5:
		case IHM_OBJ_btnSpare6:
		case IHM_OBJ_btnSpare7:
		case IHM_OBJ_btnSpare8:
		case IHM_OBJ_btnSpare9:
		case IHM_OBJ_btnSpare10:
		
		case IHM_OBJ_btnSpare11:
		case IHM_OBJ_btnSpare12:
		case IHM_OBJ_btnSpare13:
		case IHM_OBJ_btnSpare14:
		case IHM_OBJ_btnSpare15:
		case IHM_OBJ_btnSpare16:
		case IHM_OBJ_btnSpare17:
		case IHM_OBJ_btnSpare18:
		case IHM_OBJ_btnSpare19:
		case IHM_OBJ_btnSpare20:
		
		case IHM_OBJ_btnSpare21:
		case IHM_OBJ_btnSpare22:
		case IHM_OBJ_btnSpare23:
		case IHM_OBJ_btnSpare24:
		case IHM_OBJ_btnSpare25:
		case IHM_OBJ_btnSpare26:
		case IHM_OBJ_btnSpare27:
		case IHM_OBJ_btnSpare28:
		case IHM_OBJ_btnSpare29:
		case IHM_OBJ_btnSpare30:
			{
				updateButtonObject(MIhmVirtualObject::mapIhmObjIdToVirtObjId((enum_ihm_objets)psAff->objet), 
									(char*)psAff->valeur.button.szValeurImg, 
									(char*)psAff->valeur.button.szValeurText, 
									(char*)psAff->szToolTip,
									psAff->is_enabled, 
									bVisible);
			}
		//END buttons
		break;

		case IHM_OBJ_IcoLaneMode :        
		case IHM_OBJ_IcoLaneStatus :        
		case IHM_OBJ_IcoCollector :        
		case IHM_OBJ_IcoEntryGate :        
		case IHM_OBJ_IcoExitGate :        
		case IHM_OBJ_IcoCar :        
		case IHM_OBJ_IcoTrafficLight :        
		case IHM_OBJ_IcoViolation :        
		case IHM_OBJ_IcoEntryLoop :        
		case IHM_OBJ_IcoExitLoop :        
		case IHM_OBJ_IcoBeacon :        
		case IHM_OBJ_IcoEntOpticalBarrier :        
		case IHM_OBJ_IcoSpare1 :        
		case IHM_OBJ_IcoSpare2 :        
		case IHM_OBJ_IcoSpare3 :        
		case IHM_OBJ_IcoSpare4 :        
		case IHM_OBJ_IcoSpare5 :        
		case IHM_OBJ_IcoSpare6:
		case IHM_OBJ_IcoSpare7:
		case IHM_OBJ_IcoSpare8:
		case IHM_OBJ_IcoSpare9:
		case IHM_OBJ_IcoSpare10:
		case IHM_OBJ_IcoSpare11:
		case IHM_OBJ_IcoSpare12:
		case IHM_OBJ_IcoSpare13:
		case IHM_OBJ_IcoSpare14:
		case IHM_OBJ_IcoSpare15:
		case IHM_OBJ_IcoSpare16:
		case IHM_OBJ_IcoSpare17:
		case IHM_OBJ_IcoSpare18:
		case IHM_OBJ_IcoSpare19:
		case IHM_OBJ_IcoSpare20:
		case IHM_OBJ_IcoSpare21:
		case IHM_OBJ_IcoSpare22:
		case IHM_OBJ_IcoSpare23:
		case IHM_OBJ_IcoSpare24:
		case IHM_OBJ_IcoSpare25:
		case IHM_OBJ_IcoSpare26:
		case IHM_OBJ_IcoSpare27:
		case IHM_OBJ_IcoSpare28:
		case IHM_OBJ_IcoSpare29:
		case IHM_OBJ_IcoSpare30:
		case IHM_OBJ_IcoSpare31:
		case IHM_OBJ_IcoSpare32:
		case IHM_OBJ_IcoSpare33:
		case IHM_OBJ_IcoSpare34:
		case IHM_OBJ_IcoSpare35:
		case IHM_OBJ_IcoSpare36:
		case IHM_OBJ_IcoSpare37:
		case IHM_OBJ_IcoSpare38:
		case IHM_OBJ_IcoSpare39:
		case IHM_OBJ_IcoSpare40:
		case IHM_OBJ_IcoSpare41:
		case IHM_OBJ_IcoSpare42:
		case IHM_OBJ_IcoSpare43:
		case IHM_OBJ_IcoSpare44:
		case IHM_OBJ_IcoSpare45:
		case IHM_OBJ_IcoSpare46:
		case IHM_OBJ_IcoSpare47:
		case IHM_OBJ_IcoSpare48:
		case IHM_OBJ_IcoSpare49:
		case IHM_OBJ_IcoSpare50: 
		case IHM_OBJ_IcoCamera1:
		case IHM_OBJ_IcoCamera2:     
		case IHM_OBJ_IcoBeacon2:     
		case IHM_OBJ_IcoSignalLight:     
		case IHM_OBJ_IcoExtOpticalBarrier:   
		case IHM_OBJ_Image:
		case IHM_OBJ_Image2:
			updateIconObjectImage(MIhmVirtualObject::mapIhmObjIdToVirtObjId((enum_ihm_objets)psAff->objet), 
									(char*)psAff->valeur.szValeur,
									(char*)psAff->szToolTip , 
									bVisible);
		break;
			
		// INSTRUCTIONS
		case IHM_OBJ_Payment :
		case IHM_OBJ_Remark :
		case IHM_OBJ_Instruction :
		case IHM_OBJ_Instruction2 :
		case IHM_OBJ_ErrPayment :
			updateTextObjectLabel(MIhmVirtualObject::mapIhmObjIdToVirtObjId((enum_ihm_objets)psAff->objet), 
									((char*)psAff->valeur.szValeur),
									(char*)psAff->szToolTip, 
									bVisible);
		break;

		
		//LISTS...
		case IHM_OBJ_LstAlarms :
			updateListObject(MIhmVirtualObject::enuIhmVirtLstAlarms, ((char*)psAff->valeur.szValeur), (int)psAff->propriete, bVisible);
			break;

		case IHM_OBJ_LstWarnings :
			updateListObject(MIhmVirtualObject::enuIhmVirtLstWarnings, ((char*)psAff->valeur.szValeur), (int)psAff->propriete, bVisible);
			break;
 
		case IHM_OBJ_TableView1:
		case IHM_OBJ_TableView2:
		case IHM_OBJ_TableView3:
			updateTableViewReq(MIhmVirtualObject::mapIhmObjIdToVirtObjId((enum_ihm_objets)psAff->objet) , 
								((char*)psAff->valeur.table_view.szContent), 
								(psAff->valeur.table_view.bUseExternalFile==1)?true:false, 
								psAff->valeur.table_view.eTableCmd,
								bVisible);
			break;		

		case IHM_OBJ_HMenuView1:
			updateHMenuViewReq(MIhmVirtualObject::enuIhmVirtHMenuView1 , &(psAff->valeur.menu), bVisible);
			break;		
		case IHM_OBJ_HMenuView2:
			updateHMenuViewReq(MIhmVirtualObject::enuIhmVirtHMenuView2 , &(psAff->valeur.menu), bVisible);
			break;		

		case IHM_OBJ_RVideoView1:
			updateVideoViewReq(MIhmVirtualObject::enuIhmVirtRestreamVideoView1 , psAff->valeur.video, bVisible);
			break;		

		case IHM_OBJ_RVideoView2:
			updateVideoViewReq(MIhmVirtualObject::enuIhmVirtRestreamVideoView2 , psAff->valeur.video, bVisible);
			break;		

		case IHM_OBJ_CNT_HeaderGroup:
		case IHM_OBJ_CNT_InstructionGroup:
		case IHM_OBJ_CNT_ModeGroup:
		case IHM_OBJ_CNT_PaymentGroup:
		case IHM_OBJ_CNT_TrsGroup:
		case IHM_OBJ_CNT_Comments:
		case IHM_OBJ_CNT_Camera:
		case IHM_OBJ_CNT_Scanner:
		case IHM_OBJ_CNT_Spare1:
		case IHM_OBJ_CNT_Spare2:
		case IHM_OBJ_CNT_Spare3:
		case IHM_OBJ_CNT_Spare4:
		case IHM_OBJ_CNT_Spare5:
		case IHM_OBJ_CNT_Spare6:
		case IHM_OBJ_CNT_Spare7:
		case IHM_OBJ_CNT_Spare8:
		case IHM_OBJ_CNT_Spare9:
		case IHM_OBJ_CNT_Spare10:
		case IHM_OBJ_CNT_Spare11:
		case IHM_OBJ_CNT_Spare12:
		case IHM_OBJ_CNT_Spare13:
		case IHM_OBJ_CNT_Spare14:
		case IHM_OBJ_CNT_Spare15:
		case IHM_OBJ_CNT_Spare16:
		case IHM_OBJ_CNT_Spare17:
		case IHM_OBJ_CNT_Spare18:
		case IHM_OBJ_CNT_Spare19:
		case IHM_OBJ_CNT_Spare20:
		case IHM_OBJ_CNT_MiniWeb:
		case IHM_OBJ_CNT_AlarmsGroup:
			updateContainerReq(MIhmVirtualObject::mapIhmObjIdToVirtObjId((enum_ihm_objets)psAff->objet) , bVisible);
			break;		


		case IHM_OBJ_TabControl1:
		case IHM_OBJ_TabControl2:
		case IHM_OBJ_TabControl3:
		case IHM_OBJ_TabControl4:
			updateTabControl(MIhmVirtualObject::mapIhmObjIdToVirtObjId((enum_ihm_objets)psAff->objet) , psAff->valeur.szValeur, (int)psAff->propriete, bVisible);
			break;	
			
			//Creates and send a command to open a link in one of the task bar browsers 
		case IHM_OBJ_OpenLink:
			processOpenLink(psAff->valeur.link.iTargetDialog, psAff->valeur.link.szUrlValue, bVisible);
			break;	
		
		case IHM_OBJ_OpenLoginDlg:
			processLoginDlg(QString(psAff->valeur.login_dlg.szLogin), 
								(psAff->valeur.login_dlg.bManualLogin==1)?true:false, 
								(psAff->valeur.login_dlg.bLDAPOffline==1)?true:false,
								bVisible);
			break;	
		
		case IHM_OBJ_AboutDlg:
			processAboutDlg(psAff->valeur.szValeur, bVisible);
			break;	

    default :
        TRACE_W(QString("MIhmSrv::IhmSrvTraitementAffichage: Unknown object Id: %1")
												.arg((int)psAff->objet));
        bRes = FALSE;
        break;
    }

    return bRes;
}


void MIhmSrv::processReloadDynConfig()
{
	TRACE_D("MIhmSrv::processReloadDynConfig:...");

	MIhmMsgReloadDynConfigReq * pNewMsg = new MIhmMsgReloadDynConfigReq();
	m_pAniThread->emitMessageFromAni(pNewMsg);
}
	

void MIhmSrv::processOpenLink(int iTarget, QString sUrl, bool bVisible)
{
	TRACE_D(QString("MIhmSrv::processOpenLink: target:%1 url:[%2]")
												.arg(iTarget)
												.arg(sUrl));

	MIhmMsgOpenLinkReq * pNewMsg = new MIhmMsgOpenLinkReq();
	
	pNewMsg->m_iTargetWin = iTarget;
	pNewMsg->m_sUrl = sUrl;
	pNewMsg->m_bVisible = bVisible;

	m_pAniThread->emitMessageFromAni(pNewMsg);

}

void MIhmSrv::processAboutDlg(QString szAboutDialogCfg, bool bVisible)
{
	TRACE_D(QString("MIhmSrv::processAboutDlg: szAboutDialogCfg:%1 bVisible:[%2]")
		.arg(szAboutDialogCfg)
		.arg(bVisible?"true":"false"));
	
	if(bVisible)
	{
		MIhmMsgOpenAboutDlgReq * pNewMsg = new MIhmMsgOpenAboutDlgReq();
		pNewMsg->m_sAboutDlgCnf = szAboutDialogCfg;
		m_pAniThread->emitMessageFromAni(pNewMsg);
	}
	else
	{
		MIhmMsgCloseAboutDlgReq * pNewMsg = new MIhmMsgCloseAboutDlgReq();
		m_pAniThread->emitMessageFromAni(pNewMsg);
	}
}


void MIhmSrv::processLoginDlg(QString sLoginId, bool bManualLogin, bool bLDAPOffline, bool bOpen)
{
	TRACE_D(QString("MIhmSrv::processLoginDlg: sLoginId:[%1] , show[%2]")
												.arg(sLoginId)
												.arg((int)bOpen));

	MIhmMsgOpenLoginDlgReq * pNewMsg = new MIhmMsgOpenLoginDlgReq();
	
	pNewMsg->m_sLoginId = sLoginId;
	pNewMsg->m_bLDAPOffline = bLDAPOffline;
	pNewMsg->m_bManualLogin = bManualLogin;
	pNewMsg->m_bShow = bOpen;

	m_pAniThread->emitMessageFromAni(pNewMsg);

}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void IhmMessEnvoiAcquitement (
 *                           IN noyau_bal_id iBalDest,
 *                           IN enum_ihm_service eService,
 *                           IN enum_ihm_type eTypeMessage )
 * PARAMETERS: IN noyau_bal_id iBalDest      : Bal de destination du message
 *             IN enum_ihm_service eService  : Service conserne
 *             IN enum_ihm_type eTypeMessage : Acquitement / non-acquitement
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Envoyer un message d'acquittement vers l'application
 * --------------------------------------------------------------------
 */
void MIhmSrv::IhmMessEnvoiAcquitement(IN noyau_bal_id iBalDest, 
                                       IN enum_srv_service eService,
                                       IN enum_srv_type eTypeMessage)
{
	m_pAniThread->IhmMessEnvoiAcquitement(iBalDest, eService, eTypeMessage);
}

//------------------------------------------------------------------------------------------------------



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED enum_ihm_type IhmDebutService(
 *                                    IN enum_ihm_service eService,
 *                                    IN noyau_bal_id dwBalDest)
 * PARAMETERS: IN enum_ihm_service eService  : Service conserne
 *             IN noyau_bal_id iBalDest      : Bal de destination du message
  * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : 
 * --------------------------------------------------------------------
 */
enum_srv_type MIhmSrv::IhmDebutService(IN enum_ihm_service eService,
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

	IhmMessEnvoiAcquitement(dwBalDest, (enum_srv_service)eService, eResult);

	if(eResult == SRV_TYP_DEBUT_NACQ)
	{ 
		TRACE_W(QString("MIhmSrv::IhmDebutService ==> Debut Service: Pb Ajout Demandeur: %1")
												.arg(SRV_TYP_DEBUT_NACQ));
	}

	return eResult;
}

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED enum_ihm_type IhmFinService(
 *                                    IN enum_ihm_service eService,
 *                                    IN noyau_bal_id dwBalDest)
 * PARAMETERS: IN enum_ihm_service eService  : Service conserne
 *             IN noyau_bal_id iBalDest      : Bal de destination du message
  * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : 
 * --------------------------------------------------------------------
 */
enum_srv_type MIhmSrv::IhmFinService(IN enum_ihm_service eService,
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
		IhmMessEnvoiAcquitement(dwBalDest,
								(enum_srv_service)eService,
								SRV_TYP_FIN_NACQ);

		return SRV_TYP_FIN_NACQ;
	}

    // Removing a user from the list of users
	SrvEnleveDemandeur(m_hService[eService], &pvSrvData);
	IhmMessEnvoiAcquitement(dwBalDest, (enum_srv_service)eService, SRV_TYP_FIN_ACQ);

	return SRV_TYP_FIN_ACQ;
}



//-----------------------------------------------------------------


void MIhmSrv::IhmTreatMessageTCL(MIhmMsg * pMsg)
{
	if(pMsg!=NULL)	
	{
		switch(pMsg->getType())
		{
		case MIhmMsg::enuIhmMsgKeyAndCmdReq:
				IhmMessEnvoiTouche(((MIhmMsgKeyAndCmdReq *)pMsg));
			break;
		case MIhmMsg::enuIhmMsgStringDetection:
				IhmMessEnvoiDetectionChaine((MIhmMsgStringDetection *)pMsg);
			break;
		case MIhmMsg::enuIhmMsgInputDialogRes:
				IhmMessEnvoiSasieVisu((MIhmMsgInputDialogRes *)pMsg);
			break;
		case MIhmMsg::enuIhmMsgInputDialogInitErr:
				IhmSasieVisuInitErr((MIhmMsgInputDialogInitErr *)pMsg);
			break;
		case MIhmMsg::enuIhmMsgInputDlgCanceled:
				IhmSasieVisuCanceled((MIhmMsgInputDlgCanceled *)pMsg);
			break;
		case MIhmMsg::enuIhmMsgInputDlgExRes:
				IhmMessEnvoiSasieVisuEx((MIhmMsgInputDlgExRes *)pMsg);
			break;
		case MIhmMsg::enuIhmMsgInputDlgExInitErr:
				IhmSasieVisuExInitErr((MIhmMsgInputDlgExInitErr *)pMsg);
			break;
		case MIhmMsg::enuIhmMsgInputDlgExCanceled:
				IhmSasieVisuExCanceled((MIhmMsgInputDlgExCanceled *)pMsg);
			break;
		case MIhmMsg::enuIhmMsgInputDlgExOpened:
				IhmSasieVisuExOpened((MIhmMsgInputDlgExOpened *)pMsg);
			break;
		case MIhmMsg::enuIhmMsgLoginDlgOpened:
			IhmLoginDlgOpened((MIhmMsgLoginDlgOpened *)pMsg);
			break;
		case MIhmMsg::enuIhmMsgLoginDlgClosed:
			IhmLoginDlgClosed((MIhmMsgLoginDlgClosed *)pMsg);
			break;


		case MIhmMsg::enuIhmMsgAuthReq:
				IhmMessEnvoiAuthReq((MIhmMsgAuthReq *)pMsg);
			break;
		case MIhmMsg::enuIhmMsgAuthLogoffReq:
				IhmMessEnvoiAuthLogoffReq((MIhmMsgAuthLogOffReq *)pMsg);
			break;
		case MIhmMsg::enuIhmMsgAuthDlgCanceled:
			IhmMessEnvoiAuthDlgCanceled((MIhmMsgAuthDlgCanceled *)pMsg);
			break;


		case MIhmMsg::enuIhmMsgTakeCtrlReq:
				IhmMessEnvoiTakeCtrlReq((MIhmMsgTakeCtrlReq *)pMsg);
			break;
		case MIhmMsg::enuIhmMsgReturnCtrlReq:
				IhmMessEnvoiReturnCtrlReq((MIhmMsgReturnCtrlReq *)pMsg);
			break;

		case MIhmMsg::enuIhmMsgAsyncInputRsp:
				IhmProcessMessAsyncInputRsp((MIhmMsgAsyncInputRsp *)pMsg);
			break;
			
		case MIhmMsg::enuIhmMsgVideo:
			{
				MIhmMsgVideo *pMsgVideo = (MIhmMsgVideo *)pMsg;
					
				switch(pMsgVideo->getVideoMsgType())
				{
					case MIhmMsgVideo::enuIhmMsgVideoFreezeRsp:
							IhmProcessMessVideoFreezeRsp((MIhmMsgVideoFreezeRsp *)pMsgVideo);
						break;

					case MIhmMsgVideo::enuIhmMsgVideoSaveRsp:
							IhmProcessMessVideoSaveRsp((MIhmMsgVideoSaveRsp *)pMsgVideo);
						break;
					
					case MIhmMsgVideo::enuIhmMsgRVideoUpdateSrc:
							IhmProcessMessRVideoUpdateSrc((MIhmMsgRVideoUpdateSrc *)pMsgVideo);
						break;
					case MIhmMsgVideo::enuIhmMsgRVideoUpdateOnOff:
							IhmProcessMessRVideoUpdateOnOff((MIhmMsgRVideoUpdateOnOff *)pMsgVideo);
						break;


					default:
						TRACE_W(QString("MIhmSrv::IhmTreatMessageTCL: Unknown video message type: %1").
								arg((int)pMsgVideo->getVideoMsgType()));
						break;
				}

			}
			break;
			
		default:
			TRACE_W(QString("MIhmSrv::IhmTreatMessageTCL: Unknown message type: %1").
											arg((int)pMsg->getType()));
		    break;
		}
	}
}




void MIhmSrv::IhmSrvReceptionAuthorisation (struct_ihm_message *psMsg)
{

	switch ( psMsg->entete.type_message )
    {
	case SRV_TYP_DEBUT :
        TRACE_D("MIhmSrv::IhmSrvReceptionAuthorisation: SRV_TYP_DEBUT" );
		IhmDebutService(M_IHM_AUTHORIZATION, psMsg->entete.neutre.bl_retour);
		break;

	case SRV_TYP_FIN :
        TRACE_D("MIhmSrv::IhmSrvReceptionAuthorisation: SRV_TYP_FIN" );
		IhmFinService(M_IHM_AUTHORIZATION, psMsg->entete.neutre.bl_retour);
		break;
     case SRV_TYP_EFFECTUE:  
        TRACE_D("MIhmSrv::IhmSrvReceptionAuthorisation: SRV_TYP_EFFECTUE" );
		

        if ( IhmSrvTraitementAuthorization( psMsg ) )
        {
            // Pas d'erreur, Acq de la demande
            IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
                                      (enum_srv_service)M_IHM_AUTHORIZATION, 
                                      SRV_TYP_SET_ACQ);
        }
        else
        {
            TRACE_W("MIhmSrv::IhmSrvReceptionAuthorisation: ERREUR IhmSrvTraitementAuthorization retourne FALSE" );
            IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
                                      (enum_srv_service)M_IHM_AUTHORIZATION, 
                                      SRV_TYP_SET_NACQ);
        }
        break;



    // INCONNU
    default :
        TRACE_W(QString("MIhmSrv::IhmSrvReceptionAuthorisation ERREUR type inconnu %1").
										arg((int)psMsg->entete.type_message));

        // NAcq de la demande
        IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
                                  (enum_srv_service)M_IHM_AUTHORIZATION, 
                                  SRV_TYP_DEMANDE_INCONNUE);
        break;
    }
}



int MIhmSrv::getAuthBalId()
{
	int iRet = NOYAU_ID_NOK;
	int iBalId;

    for ( iBalId = 0 ; iBalId < IHM_KB_NB_TACHES_MAX ; iBalId++ )
    {
		if((bool)SrvEstDemandeur(m_hService[M_IHM_AUTHORIZATION],iBalId))
		{
			iRet = iBalId;
			break;
		}
	}

	return iRet;
}


void MIhmSrv::IhmMessEnvoiAuthReq(MIhmMsgAuthReq *pMsg)
{
    struct_ihm_message * psMsgEmis = NULL;

	TRACE_D(QString("MIhmSrv::IhmMessEnvoiAuthReq: user:%1; session:%2; is_desktop:%3").
											arg(pMsg->m_usrData.m_sUserId).
											arg(pMsg->m_usrData.m_sInternalSessionID).
											arg(pMsg->m_usrData.m_bIsDesktopUser?1:0));
	
	if(!SrvEstVide(m_hService[M_IHM_AUTHORIZATION]))
	{
		noyau_bal_id iBalDest = getAuthBalId();

		if(iBalDest!=NOYAU_ID_NOK)
		{
			m_pAniThread->MyExitAlloue("MIhmSrv::IhmMessEnvoiAuthReq",
			   ( struct_neutre ** )( &psMsgEmis ),
				sizeof( struct_ihm_message ),
				m_pAniThread->getPoolId() );

			psMsgEmis->entete.service = (enum_srv_service)M_IHM_AUTHORIZATION;
			psMsgEmis->entete.type_message = SRV_TYP_DEMANDE;

            memset( &psMsgEmis->u, 0, sizeof( psMsgEmis->u ) );
			
			psMsgEmis->u.srv_auth.cmd_type = IHM_AUTH_REQ;
            strncpy(psMsgEmis->u.srv_auth.user_id, pMsg->m_usrData.m_sUserId.toLatin1().data(), IHM_LG_CHAINES_SRV);
            strncpy(psMsgEmis->u.srv_auth.sec_code, pMsg->m_usrData.m_sSecCode.toLatin1().data(), IHM_LG_CHAINES_SRV);
			psMsgEmis->u.srv_auth.is_sec_code_pwd = pMsg->m_usrData.m_bIsSecCodePwd?1:0;
			psMsgEmis->u.srv_auth.is_user_id_auth = 0; //TODO to be used in case of auth request with hash autentificator
            strncpy(psMsgEmis->u.srv_auth.session_id, pMsg->m_usrData.m_sInternalSessionID.toLatin1().data(), IHM_LG_CHAINES_SRV);
			psMsgEmis->u.srv_auth.is_desktop = pMsg->m_usrData.m_bIsDesktopUser?1:0;

			m_pAniThread->MyExitEnvoie("MIhmSrv::IhmMessEnvoiAuthReq", iBalDest, m_pAniThread->getAniBalId(), (struct_neutre *)psMsgEmis );
		}
		else
			TRACE_W("MIhmSrv::IhmMessEnvoiAuthReq:getAuthBalId returned NOYAU_ID_NOK!");

	}
	else
	{
		TRACE_W("MIhmSrv::IhmMessEnvoiAuthReq:SrvEstVide return true!");

		MIhmMsgAuthRsp * pNewMsg = new MIhmMsgAuthRsp();
		
		pNewMsg->m_iAuthRsp = enuAUTH_NOK;
		pNewMsg->m_usrData.m_bIsDesktopUser = pMsg->m_usrData.m_bIsDesktopUser;
		pNewMsg->m_usrData.m_sInternalSessionID = pMsg->m_usrData.m_sInternalSessionID;
		pNewMsg->m_sError = IHM_LABEL_MSG_NO_AUTHORIZATION_SERVICE;

		m_pAniThread->emitMessageFromAni(pNewMsg);


	}
}



void MIhmSrv::IhmMessEnvoiAuthLogoffReq(MIhmMsgAuthLogOffReq *pMsg)
{
    struct_ihm_message * psMsgEmis = NULL;

	TRACE_D(QString("MIhmSrv::IhmMessEnvoiAuthLogoffReq: user:%1; session:%2; is_desktop:%3").
											arg(pMsg->m_usrData.m_sUserId).
											arg(pMsg->m_usrData.m_sInternalSessionID).
											arg(pMsg->m_usrData.m_bIsDesktopUser?1:0));
	
	if(!SrvEstVide(m_hService[M_IHM_AUTHORIZATION]))
	{
		noyau_bal_id iBalDest = getAuthBalId();

		if(iBalDest!=NOYAU_ID_NOK)
		{
			m_pAniThread->MyExitAlloue("MIhmSrv::IhmMessEnvoiAuthLogoffReq",
				( struct_neutre ** )( &psMsgEmis ),
				sizeof( struct_ihm_message ),
				m_pAniThread->getPoolId() );

			psMsgEmis->entete.service = (enum_srv_service)M_IHM_AUTHORIZATION;
			psMsgEmis->entete.type_message = SRV_TYP_DEMANDE;

            memset( &psMsgEmis->u, 0, sizeof( psMsgEmis->u ) );
			
			psMsgEmis->u.srv_auth.cmd_type = IHM_AUTH_LOGOFF_REQ;
			psMsgEmis->u.srv_auth.is_desktop = pMsg->m_usrData.m_bIsDesktopUser?1:0;
            strncpy(psMsgEmis->u.srv_auth.user_id, pMsg->m_usrData.m_sUserId.toLatin1().data(), IHM_LG_CHAINES_SRV);
            strncpy(psMsgEmis->u.srv_auth.sec_code, pMsg->m_usrData.m_sSecCode.toLatin1().data(), IHM_LG_CHAINES_SRV);
			psMsgEmis->u.srv_auth.is_sec_code_pwd = pMsg->m_usrData.m_bIsSecCodePwd?1:0;
            strncpy(psMsgEmis->u.srv_auth.session_id, pMsg->m_usrData.m_sInternalSessionID.toLatin1().data(), IHM_LG_CHAINES_SRV);
			psMsgEmis->u.srv_auth.reason = 	pMsg->reason;

			m_pAniThread->MyExitEnvoie("MIhmSrv::IhmMessEnvoiAuthLogoffReq", iBalDest, m_pAniThread->getAniBalId(), (struct_neutre *)psMsgEmis );
		}
		else
			TRACE_W("MIhmSrv::IhmMessEnvoiAuthLogoffReq:getAuthBalId returned NOYAU_ID_NOK!");

	}
	else
	{
		TRACE_W("MIhmSrv::IhmMessEnvoiAuthLogoffReq:SrvEstVide returned true!");
	}
}

void MIhmSrv::IhmMessEnvoiAuthDlgCanceled(MIhmMsgAuthDlgCanceled *pMsg)
{
    struct_ihm_message * psMsgEmis = NULL;

	TRACE_D("MIhmSrv::IhmMessEnvoiAuthDlgCanceled:...");
	m_pAniThread->VirtualKyb_Hide();

	if(!SrvEstVide(m_hService[M_IHM_AUTHORIZATION]))
	{
		noyau_bal_id iBalDest = getAuthBalId();

		if(iBalDest!=NOYAU_ID_NOK)
		{
			m_pAniThread->MyExitAlloue("MIhmSrv::IhmMessEnvoiAuthDlgCanceled",
				( struct_neutre ** )( &psMsgEmis ),
				sizeof( struct_ihm_message ),
				m_pAniThread->getPoolId() );

			psMsgEmis->entete.service = (enum_srv_service)M_IHM_AUTHORIZATION;
			psMsgEmis->entete.type_message = SRV_TYP_EFFECTUE;
            memset( &psMsgEmis->u, 0, sizeof( psMsgEmis->u ) );

			psMsgEmis->u.srv_auth.cmd_type = IHM_AUTH_DLG_CANCELED;
			psMsgEmis->u.srv_auth.is_desktop = pMsg->m_usrData.m_bIsDesktopUser?1:0;
			psMsgEmis->u.srv_auth.reason = 	pMsg->reason;

			m_pAniThread->MyExitEnvoie("MIhmSrv::IhmMessEnvoiAuthDlgCanceled", iBalDest, m_pAniThread->getAniBalId(), (struct_neutre *)psMsgEmis );
		}
		else
			TRACE_W("MIhmSrv::IhmMessEnvoiAuthDlgCanceled:getAuthBalId returned NOYAU_ID_NOK!");

	}
	else
	{
		TRACE_W("MIhmSrv::IhmMessEnvoiAuthDlgCanceled:SrvEstVide return true!");
	}
}




void MIhmSrv::IhmMessEnvoiTakeCtrlReq(MIhmMsgTakeCtrlReq *pMsg)
{
    struct_ihm_message * psMsgEmis = NULL;

	TRACE_D(QString("MIhmSrv::IhmMessEnvoiTakeCtrlReq: user:%1; session:%2; is_desktop:%3").
											arg(pMsg->m_usrData.m_sUserId).
											arg(pMsg->m_usrData.m_sInternalSessionID).
											arg(pMsg->m_usrData.m_bIsDesktopUser?1:0));
	
	if(!SrvEstVide(m_hService[M_IHM_AUTHORIZATION]))
	{
		noyau_bal_id iBalDest = getAuthBalId();

		if(iBalDest!=NOYAU_ID_NOK)
		{
			m_pAniThread->MyExitAlloue("MIhmSrv::IhmMessEnvoiTakeCtrlReq",
			   ( struct_neutre ** )( &psMsgEmis ),
				sizeof( struct_ihm_message ),
				m_pAniThread->getPoolId() );

			psMsgEmis->entete.service = (enum_srv_service)M_IHM_AUTHORIZATION;
			psMsgEmis->entete.type_message = SRV_TYP_DEMANDE;

            memset( &psMsgEmis->u, 0, sizeof( psMsgEmis->u ) );
			
			psMsgEmis->u.srv_auth.cmd_type = IHM_TAKE_CTRL_REQ;
			psMsgEmis->u.srv_auth.is_desktop = pMsg->m_usrData.m_bIsDesktopUser?1:0;
            strncpy(psMsgEmis->u.srv_auth.user_id, pMsg->m_usrData.m_sUserId.toLatin1().data(), IHM_LG_CHAINES_SRV);
            strncpy(psMsgEmis->u.srv_auth.sec_code, pMsg->m_usrData.m_sSecCode.toLatin1().data(), IHM_LG_CHAINES_SRV);
			psMsgEmis->u.srv_auth.is_sec_code_pwd = pMsg->m_usrData.m_bIsSecCodePwd?1:0;
            strncpy(psMsgEmis->u.srv_auth.session_id, pMsg->m_usrData.m_sInternalSessionID.toLatin1().data(), IHM_LG_CHAINES_SRV);

			m_pAniThread->MyExitEnvoie("MIhmSrv::IhmMessEnvoiTakeCtrlReq", iBalDest, m_pAniThread->getAniBalId(), (struct_neutre *)psMsgEmis );
		}
		else
			TRACE_W("MIhmSrv::IhmMessEnvoiTakeCtrlReq:getAuthBalId returned NOYAU_ID_NOK!");

	}
	else
	{
		TRACE_W("MIhmSrv::IhmMessEnvoiTakeCtrlReq:SrvEstVide return true!");
	}
}

void MIhmSrv::IhmMessEnvoiReturnCtrlReq(MIhmMsgReturnCtrlReq *pMsg)
{
    struct_ihm_message * psMsgEmis = NULL;

	TRACE_D(QString("MIhmSrv::IhmMessEnvoiReturnCtrlReq: user:%1; session:%2; is_desktop:%3").
											arg(pMsg->m_usrData.m_sUserId).
											arg(pMsg->m_usrData.m_sInternalSessionID).
											arg(pMsg->m_usrData.m_bIsDesktopUser?1:0));
	
	if(!SrvEstVide(m_hService[M_IHM_AUTHORIZATION]))
	{
		noyau_bal_id iBalDest = getAuthBalId();

		if(iBalDest!=NOYAU_ID_NOK)
		{
			m_pAniThread->MyExitAlloue("MIhmSrv::IhmMessEnvoiReturnCtrlReq",
			   ( struct_neutre ** )( &psMsgEmis ),
				sizeof( struct_ihm_message ),
				m_pAniThread->getPoolId() );

			psMsgEmis->entete.service = (enum_srv_service)M_IHM_AUTHORIZATION;
			psMsgEmis->entete.type_message = SRV_TYP_DEMANDE;

            memset( &psMsgEmis->u, 0, sizeof( psMsgEmis->u ) );
			
			psMsgEmis->u.srv_auth.cmd_type = IHM_RETURN_CTRL_REQ;
            strncpy(psMsgEmis->u.srv_auth.user_id, pMsg->m_usrData.m_sUserId.toLatin1().data(), IHM_LG_CHAINES_SRV);
            strncpy(psMsgEmis->u.srv_auth.sec_code, pMsg->m_usrData.m_sSecCode.toLatin1().data(), IHM_LG_CHAINES_SRV);
 			psMsgEmis->u.srv_auth.is_sec_code_pwd = pMsg->m_usrData.m_bIsSecCodePwd?1:0;
            strncpy(psMsgEmis->u.srv_auth.session_id, pMsg->m_usrData.m_sInternalSessionID.toLatin1().data(), IHM_LG_CHAINES_SRV);
			psMsgEmis->u.srv_auth.is_desktop = pMsg->m_usrData.m_bIsDesktopUser?1:0;
			psMsgEmis->u.srv_auth.reason = (enum_ihm_auth_ret_ctrl_reason)pMsg->m_iReason;

			m_pAniThread->MyExitEnvoie("MIhmSrv::IhmMessEnvoiReturnCtrlReq", iBalDest, m_pAniThread->getAniBalId(), (struct_neutre *)psMsgEmis );
		}
		else
			TRACE_W("MIhmSrv::IhmMessEnvoiReturnCtrlReq:getAuthBalId returned NOYAU_ID_NOK!");

	}
	else
	{
		TRACE_W("MIhmSrv::IhmMessEnvoiReturnCtrlReq:SrvEstVide return true!");
	}

}


bool MIhmSrv::IhmSrvTraitementAuthorization ( IN struct_ihm_message * psMsg )
{
    struct_ihm_authorization * psAuth=&(psMsg->u.srv_auth);
    bool bRes = false;

	//assure that strings are zero terminated
	psAuth->user_id[IHM_LG_CHAINES_SRV-1] = '\0';
	psAuth->user_name[IHM_LG_CHAINES_SRV-1] = '\0';
	psAuth->sec_code[IHM_LG_CHAINES_SRV-1] = '\0';
	psAuth->session_id[IHM_LG_CHAINES_SRV-1] = '\0';
	psAuth->errmsg[IHM_LG_CHAINES_SRV-1] = '\0';

	TRACE_D(QString("MIhmSrv::IhmSrvTraitementAuthorization: user:[%1],sec_code:[%2],is_sec_code_pwd:[%3],session_id:[%4],is_desktop:[%5],ret_val:[%6], err_msg[%7]").
										arg((char*)psAuth->user_id).
										arg((char*)psAuth->sec_code).
										arg((int)psAuth->is_sec_code_pwd).
										arg((char*)psAuth->session_id).
										arg((int)psAuth->is_desktop).
										arg((int)psAuth->ret_val).
										arg((char*)psAuth->errmsg));

	if((bool)SrvEstDemandeur(m_hService[M_IHM_AUTHORIZATION], psMsg->entete.neutre.bl_retour))
	{
		switch(psAuth->cmd_type)
		{
			case IHM_AUTH_RSP :
				{
					TRACE_D("MIhmSrv::IhmSrvTraitementAuthorization:IHM_AUTH_RSP");

					MIhmMsgAuthRsp * pNewMsg = new MIhmMsgAuthRsp();
					
					pNewMsg->m_iAuthRsp = psAuth->ret_val;
					pNewMsg->m_usrData.m_bIsDesktopUser = (psAuth->is_desktop==1)?true:false;
					pNewMsg->m_usrData.m_sInternalSessionID = psAuth->session_id;
					pNewMsg->m_sError = psAuth->errmsg;
					pNewMsg->m_usrData.m_sUserId = psAuth->user_id;
					pNewMsg->m_usrData.m_sUserName = psAuth->user_name;
					pNewMsg->m_usrData.m_sSecCode = psAuth->sec_code;
					pNewMsg->m_usrData.m_bIsSecCodePwd = (psAuth->is_sec_code_pwd==1)?true:false;
					m_pAniThread->emitMessageFromAni(pNewMsg);

					bRes = true;
					
				}
				break;
			case IHM_TAKE_CTRL_RSP :
				{
					TRACE_D("MIhmSrv::IhmSrvTraitementAuthorization:IHM_TAKE_CTRL_RSP" );
					MIhmMsgTakeCtrlRsp * pNewMsg = new MIhmMsgTakeCtrlRsp();
					
					pNewMsg->m_iAuthRsp = psAuth->ret_val;
					pNewMsg->m_usrData.m_bIsDesktopUser = (psAuth->is_desktop==1)?true:false;
					pNewMsg->m_usrData.m_sInternalSessionID = psAuth->session_id;
					pNewMsg->m_usrData.m_sUserId = psAuth->user_id;
					pNewMsg->m_usrData.m_sUserName = psAuth->user_name;
					pNewMsg->m_usrData.m_sSecCode = psAuth->sec_code;
					pNewMsg->m_usrData.m_bIsSecCodePwd = (psAuth->is_sec_code_pwd==1)?true:false;
					pNewMsg->m_sError = psAuth->errmsg;
					
					m_pAniThread->emitMessageFromAni(pNewMsg);
					bRes = true;

				}
				break;
			case IHM_RETURN_CTRL_RSP :
				{
					TRACE_D("MIhmSrv::IhmSrvTraitementAuthorization:IHM_RETURN_CTRL_RSP" );
					MIhmMsgReturnCtrlRsp * pNewMsg = new MIhmMsgReturnCtrlRsp();
					
					pNewMsg->m_iAuthRsp = psAuth->ret_val;
					pNewMsg->m_usrData.m_bIsDesktopUser = (psAuth->is_desktop==1)?true:false;
					pNewMsg->m_usrData.m_sInternalSessionID = psAuth->session_id;

					m_pAniThread->emitMessageFromAni(pNewMsg);
					bRes = true;

				}
				break;
			default :
				TRACE_W(QString("MIhmSrv::IhmSrvTraitementAuthorization:ERREUR: Unknown auth message type %1").
										arg((int)psAuth->cmd_type));
				bRes = false;
				break;
		}
	}
	else
	{
		TRACE_W("MIhmSrv::IhmSrvTraitementAuthorization: Returned response from wrong bal_id!");
		bRes = false;
	}

	return bRes;
}


//------------------------------
//Async input ...
//------------------------------


void MIhmSrv::IhmSrvReceptionAsyncInput( IN struct_ihm_message *psMsg )
{

    switch ( psMsg->entete.type_message )
    {
	case SRV_TYP_DEBUT :
        TRACE_D("MIhmSrv::IhmSrvReceptionAsyncInput: SRV_TYP_DEBUT" );
		IhmDebutService(M_IHM_ASYNC_INPUT, psMsg->entete.neutre.bl_retour);
		break;

	case SRV_TYP_FIN :
        TRACE_D("MIhmSrv::IhmSrvReceptionAsyncInput: SRV_TYP_FIN" );
		IhmFinService(M_IHM_ASYNC_INPUT, psMsg->entete.neutre.bl_retour);
		break;
        
    case SRV_TYP_SET:  
        TRACE_D("MIhmSrv::IhmSrvReceptionAsyncInput: SRV_TYP_SET" );

        // Routage de la demande vers l'interface VB (Msg evenementiel)
        if ( IhmSrvTraitementAsyncInputSet( psMsg ) )
        {
            // Pas d'erreur, Acq de la demande
            IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
                                      (enum_srv_service)M_IHM_ASYNC_INPUT, 
                                      SRV_TYP_SET_ACQ);
        }
        else
        {
            TRACE_W("MIhmSrv::IhmSrvReceptionAsyncInput: ERREUR IhmSrvTraitementAsyncInput retourne FALSE" );
            // Erreur, NAcq de la demande
            IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
                                      (enum_srv_service)M_IHM_ASYNC_INPUT, 
                                      SRV_TYP_SET_NACQ);
        }
        break;

    case SRV_TYP_GET:  
        TRACE_D("MIhmSrv::IhmSrvReceptionAsyncInput: SRV_TYP_GET");

        if (SrvEstDemandeur(m_hService[M_IHM_ASYNC_INPUT],psMsg->entete.neutre.bl_retour))
        {
            IhmSrvTraitementAsyncInputGet(psMsg);

        }
        else
        {
            TRACE_W(QString("MIhmSrv::IhmSrvReceptionAsyncInput: ERREUR service M_IHM_ASYNC_INPUT not started for bl:%1").
										arg((int)psMsg->entete.neutre.bl_retour));
            // Erreur, NAcq de la demande
            IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
                                      (enum_srv_service)M_IHM_ASYNC_INPUT, 
                                      SRV_TYP_GET_NACQ);
        }
        break;
    // INCONNU
    default :
        TRACE_W(QString("MIhmSrv::IhmSrvReceptionAsyncInput:ERREUR type inconnu %1").
										arg((int)psMsg->entete.type_message));

        // NAcq de la demande
        IhmMessEnvoiAcquitement ( psMsg->entete.neutre.bl_retour, 
                                  (enum_srv_service)M_IHM_ASYNC_INPUT, 
                                  SRV_TYP_DEMANDE_INCONNUE);
        break;
    }
}




bool MIhmSrv::IhmSrvTraitementAsyncInputSet( IN struct_ihm_message * psMsg )
{
	struct_ihm_async_input* pAsync = &(psMsg->u.srv_async_input);
    bool bRes = false;

	//assure that strings are zero terminated
	pAsync->data[IHM_LG_SAISIE_MAX-1] = '\0';

    TRACE_D(QString("MIhmSrv::IhmSrvTraitementAsyncInputSet:  data:%1 isFile:%2")
					.arg((char*)pAsync->data).arg((int)pAsync->is_file));
	
	MIhmVirtAsyncInput * pVirtObj = 
		(MIhmVirtAsyncInput *)m_pAniVirtObjects->getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtAsyncInputsID);

	if(pVirtObj!=NULL)
	{	
		if(!pVirtObj->isInitialized())	pVirtObj->setInitialized();

		pVirtObj->update((char*)pAsync->data, (int)pAsync->is_file);
		pVirtObj->setUpdated(); //used to be updated to the GUI
		bRes = true;
	}	

    return bRes;
}

bool MIhmSrv::IhmSrvTraitementAsyncInputGet( IN struct_ihm_message * psMsg )
{
	struct_ihm_async_input* pAsync = &(psMsg->u.srv_async_input);
    BOOL bRes = TRUE;

    TRACE_D("MIhmSrv::IhmSrvTraitementAsyncInputGet ... ");
	
	MIhmVirtAsyncInput * pVirtObj = 
		(MIhmVirtAsyncInput *)m_pAniVirtObjects->getVirtualObjectByID(MIhmVirtualObject::enuIhmVirtAsyncInputsID);

	if(pVirtObj!=NULL)
	{	

		struct_ihm_message * psMsgToSend;

        m_pAniThread->MyExitAlloue("MIhmSrv::IhmMessEnvoiReturnCtrlReq",
			(struct_neutre **)(&psMsgToSend),
            sizeof(struct_ihm_message),getPoolId());
        
		memset(psMsgToSend,0,sizeof( struct_ihm_message ));

		psMsgToSend->entete.service = (enum_srv_service)M_IHM_ASYNC_INPUT;
		psMsgToSend->entete.type_message = (enum_srv_type)SRV_TYP_GET;

		QString sNewValues = pVirtObj->getEncStrValues();

		psMsgToSend->u.srv_async_input.is_file = 0;
		strncpy((char*)psMsgToSend->u.srv_async_input.data, sNewValues.toLatin1().data(),
					qMin((int)sizeof(psMsgToSend->u.srv_async_input.data)-1, sNewValues.size()));


		if(Envoie(psMsg->entete.neutre.bl_retour, getAniBalId(),(struct_neutre *)(psMsgToSend)) != NOYAU_OK)
		{
			TRACE_W("EnvoiDemandeurAsyncInput: Error not sent!");
			ExitLibere((struct_neutre **)&psMsgToSend);
		}
    
	}	

    return bRes;
}



void MIhmSrv::IHMSaisieExShowVirtualKeyboard(QString sVirtKybCfg, int iVirtKybPosX, int iVirtKybPosY)
{

}


void MIhmSrv::IHMSaisieExHideVirtualKeyboard()
{

}
