/******************* (v) 2006 CSSI - All rights reserved *********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE:   EMI_PIC                                                         */
/* FILE:     emi_pic_mess.c                                                  */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             This file contains the functions for starting services,       */
/*             stopping services and sending messages between the threads.   */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/
// VC++ interface
#include <windows.h>
#include <wingdi.h>
// CS Route interface
#include <noyau.h>
#include <debug.h>
#include <run.h>
#include <module.h>
#include <csr_srv.h>
// Module interface
#include <csr_emi_pic.h>
#include <emi_pic_glob.h>
#define LOC_DEF
#include <emi_pic_mess.h>
#undef LOC_DEF
/*-------------------------------- RESERVED:  -------------------------------*/
#include <memclass.h>
/*-------------------------------- EXTERNALS: -------------------------------*/
/*-------------------------------- DEFINES:   -------------------------------*/
/*-------------------------------- TYPEDEFS:  -------------------------------*/
/*-------------------------------- FUNCTIONS: -------------------------------*/
PRIVATE BOOL WINAPI EnvoiDemandeur(PVOID pvParam,
                                   noyau_bal_id dwBalDemandeur,
                                   PVOID pvDataDuDemandeur);
/*-------------------------------- VARIABLES: -------------------------------*/
/*-------------------------------- CODE: ------------------------------------*/
/**/
/*****************************************************************************/
/*SYNTAX: enum_emi_pic_type EmiPicDebutService(short siInstId,               */
/*                                             enum_emi_pic_service eService,*/
/*                                             noyau_bal_id dwBalDest)       */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function starts the service.                               */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId                 - Instance ID.                    */
/*      IN  enum_emi_pic_service eService  - Service type.                   */
/*      IN  noyau_bal_id dwBalDest         - Destination mailbox ID.         */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  SRV_TYP_DEBUT_ACQ     Service is started successfully.                   */
/*  SRV_TYP_DEBUT_NACQ    Service is not started successfully.               */
/*****************************************************************************/
PROTECTED enum_emi_pic_type EmiPicDebutService(short siInstId,
                                               enum_emi_pic_service eService,
                                               noyau_bal_id dwBalDest)
{
	enum_emi_pic_type  eResult;

	eResult = SRV_TYP_DEBUT_NACQ;

	// Verifing if the user already exists in the list of users 
	if((SrvEstDemandeur(EMI_PIC[siInstId].hService[eService],
						dwBalDest) == FALSE))
		// Adding user to the list	
		if(SrvAjouteDemandeur(EMI_PIC[siInstId].hService[eService],
							  dwBalDest, 0L) != NULL)
		{
			eResult = SRV_TYP_DEBUT_ACQ;
		}

	EmiPicEnvoiAcquittement(siInstId, dwBalDest, eService, eResult);

	if(eResult == SRV_TYP_DEBUT_NACQ)
	{
		EmiPicFichierTrace(siInstId,
						   "EMI_PIC_MESS : EmiPicDebutService ==> Debut Service: Pb Ajout Demandeur: %d",
						   SRV_TYP_DEBUT_NACQ);
	}

	return eResult;
}

/**/
/*****************************************************************************/
/*SYNTAX: enum_emi_pic_type EmiPicFinService(short siInstId,                 */
/*                                           enum_emi_pic_service eService,  */
/*                                           noyau_bal_id dwBalDest)         */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function stops the service.                                */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId                 - Instance ID.                    */
/*      IN  enum_emi_pic_service eService  - Service type.                   */
/*      IN  noyau_bal_id dwBalDest         - Destination mailbox ID.         */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  SRV_TYP_FIN_ACQ       Service is stopped successfully.                   */
/*  SRV_TYP_FIN_NACQ      Service is not stopped successfully.               */
/*****************************************************************************/
PROTECTED enum_emi_pic_type EmiPicFinService(short siInstId,
                                             enum_emi_pic_service eService,
                                             noyau_bal_id dwBalDest)
{
	PVOID pvSrvData;

    // Searching the list of users
	pvSrvData = SrvRechercheDemandeur(EMI_PIC[siInstId].hService[eService], 
									  dwBalDest,
									  NULL,
									  NULL);
	if(pvSrvData == NULL)
	{
		EmiPicEnvoiAcquittement(siInstId,
								dwBalDest,
								eService,
								SRV_TYP_FIN_NACQ);

		return SRV_TYP_FIN_NACQ;
	}

    // Removing a user from the list of users
	SrvEnleveDemandeur(EMI_PIC[siInstId].hService[eService], &pvSrvData);
	EmiPicEnvoiAcquittement(siInstId, dwBalDest, eService, SRV_TYP_FIN_ACQ);

	return SRV_TYP_FIN_ACQ;
}

/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicEnvoiAcquittement(short siInstId,                       */
/*                                     noyau_bal_id dwBalDest,               */
/*                                     enum_emi_pic_service eService,        */
/*                                     enum_emi_pic_type eTypeMessage)       */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function sends the message to the destination mailbox.     */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId                 - Instance ID.                    */
/*      IN  noyau_bal_id dwBalDest         - Destination mailbox ID.         */
/*      IN  enum_emi_pic_service eService  - Service type.                   */
/*      IN  enum_emi_pic_type eTypeMessage - Message type.                   */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PROTECTED void EmiPicEnvoiAcquittement(short siInstId,
                                       noyau_bal_id dwBalDest,
                                       enum_emi_pic_service eService,
                                       enum_emi_pic_type eTypeMessage)
{
	struct_emi_pic_message  *pMessage = NULL;

	// Allocating the memory
	ExitAlloue((struct_neutre **)&(pMessage),
				sizeof(struct_emi_pic_message),
				EMI_PIC[siInstId].hPool);

	// Filling the message structure
	pMessage->entete.neutre.bl_retour= EMI_PIC[siInstId].dwAniBalId;
	pMessage->entete.service = eService;
	pMessage->entete.type_message = eTypeMessage;

	EmiPicFichierTrace(siInstId,
					   "EMI_PIC_MESS : EmiPicEnvoiAcquittement ==> vers BAL %d, msg_id %d, type %d",
					   dwBalDest,
					   pMessage->entete.service,
					   pMessage->entete.type_message);

	// Sending message
	ExitEnvoie(dwBalDest,
			   EMI_PIC[siInstId].dwAniBalId,
			   (struct_neutre *)pMessage);
}

/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicEnvoiEmi(short siInstId,                                */
/*                            struct_emi_pic_int_message *pMessage,          */
/*                            enum_emi_pic_type eTypeMessage)                */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function sends the message to the all subscribers of the   */
/*            EMISSION service.                                              */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId                        - Instance ID.             */
/*      IN  struct_emi_pic_int_message *pMessage  - Pointer to the message.  */
/*      IN  enum_emi_pic_type eTypeMessage        - Message type.            */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PROTECTED void EmiPicEnvoiEmi(short siInstId,
                              struct_emi_pic_int_message *pMessage,
                              enum_emi_pic_type eTypeMessage)
{
	struct_emi_pic_message       sMessage;
	struct_emi_pic_envoi_service sData = { siInstId, &sMessage };

	switch(eTypeMessage)
	{
		case M_EMI_PIC_TYP_EMISSION_PURGE_EFFECTUE:
		case M_EMI_PIC_TYP_EMISSION_PURGE_NON_EFFECTUE:
			if(!SrvEstVide(EMI_PIC[siInstId].hService[M_EMI_PIC_EMISSION]))
			{
				// Filling the structure
				sMessage.entete.neutre.bl_retour = EMI_PIC[siInstId].dwAniBalId;
				sMessage.entete.service = M_EMI_PIC_EMISSION;
				sMessage.entete.type_message = eTypeMessage;

				memcpy(&sMessage.u.srv_emi,
					   &pMessage->u.sAniEmi.sEmi,
					   sizeof(struct_emi_pic_emi));

				SrvPourChaqueDemandeur(EMI_PIC[siInstId].hService[M_EMI_PIC_EMISSION],
									   EnvoiDemandeur, 
									   &sData);
			}
			else
				EmiPicFichierTrace(siInstId,
								   "EMI_PIC_MESS : EmiPicEnvoiEmi ==> Liste demandeurs vide EMI_PIC No : %d",
								   siInstId);
			break;

		default:
			break;
	}
}
/*****************************************************************************/
/*SYNTAX: void EmiPicEnvoiEmi(short siInstId,                                */
/*                            struct_emi_pic_int_message *pMessage,          */
/*                            enum_emi_pic_type eTypeMessage)                */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function sends the message to the all subscribers of the   */
/*            EMISSION service.                                              */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId                        - Instance ID.             */
/*      IN  struct_emi_pic_int_message *pMessage  - Pointer to the message.  */
/*      IN  enum_emi_pic_type eTypeMessage        - Message type.            */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PROTECTED void EmiPicEnvoiEtat(short siInstId,
                              struct_emi_pic_etat *pMessage,
                              enum_emi_pic_type eTypeMessage)
{
	struct_emi_pic_message       sMessage;
	struct_emi_pic_envoi_service sData = { siInstId, &sMessage };
	
	switch(eTypeMessage)
	{
		case SRV_TYP_NOUVEL_ETAT:
			if( !SrvEstVide(EMI_PIC[siInstId].hService[M_SRV_ETAT]) )
			{
				// Filling the structure
				sMessage.entete.neutre.bl_retour = EMI_PIC[siInstId].dwAniBalId;
				sMessage.entete.service = M_SRV_ETAT;
				sMessage.entete.type_message = eTypeMessage;
				
				memcpy(&(sMessage.u.srv_etat.status),
					pMessage,
					sizeof(struct_emi_pic_etat));
				
				SrvPourChaqueDemandeur(EMI_PIC[siInstId].hService[M_SRV_ETAT],
					EnvoiDemandeur, 
					&sData);
			}
			else
				EmiPicFichierTrace(siInstId,
				"EMI_PIC_MESS : EmiPicEnvoiEtat ==> Liste demandeurs vide EMI_PIC No : %d",
				siInstId);
			break;
			
		default:
			break;
	}
}
/*****************************************************************************/
/*SYNTAX:																	 */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*																			 */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*		(IN)																 */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*                        Function does not return value					 */
/*****************************************************************************/
PROTECTED void EmiPicEnvoiMaintMode( short siInstId,
									 struct_emi_pic_int_message *pMessage,
									 enum_emi_pic_type eTypeMessage)
{
	struct_emi_pic_message       sMessage;
	struct_emi_pic_envoi_service sData = { siInstId, &sMessage };
	
	switch(eTypeMessage)
	{
	case M_EMI_PIC_MAINT_MODE_START_ACK:
	case M_EMI_PIC_MAINT_MODE_START_NACK:
	case M_EMI_PIC_MAINT_MODE_UPDATE:
	case M_EMI_PIC_MAINT_MODE_EFFECTUE:
	case M_EMI_PIC_MAINT_MODE_NON_EFFECTUE:
		
		if( !SrvEstVide(EMI_PIC[siInstId].hService[M_EMI_PIC_MAINT_MODE]) )
		{
			// Filling the structure
			sMessage.entete.neutre.bl_retour = EMI_PIC[siInstId].dwAniBalId;
			sMessage.entete.service = M_EMI_PIC_MAINT_MODE;
			sMessage.entete.type_message = eTypeMessage;
			
			memcpy(&(sMessage.u.srv_maint_mode),
				&(pMessage->u.sAniMaintMode),
				sizeof(struct_maint_mode));
			
			SrvPourChaqueDemandeur(EMI_PIC[siInstId].hService[M_EMI_PIC_MAINT_MODE],
				EnvoiDemandeur, 
				&sData);
		}
		else
			EmiPicFichierTrace(siInstId,
			"EMI_PIC_MESS : EmiPicEnvoiMaintMode ==> Liste demandeurs vide EMI_PIC No : %d",
			siInstId);
		break;
		
	default:
		break;
	}
}

/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicEnvoiRestit(short siInstId,                             */
/*                               struct_emi_pic_int_message *pMessage,       */
/*                               enum_emi_pic_type eTypeMessage)             */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function sends the message to the all subscribers of the   */
/*            RESTITUTION service.                                           */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId                        - Instance ID.             */
/*      IN  struct_emi_pic_int_message *pMessage  - Pointer to the message.  */
/*      IN  enum_emi_pic_type eTypeMessage        - Message type.            */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PROTECTED void EmiPicEnvoiRestit(short siInstId,
                                 struct_emi_pic_int_message *pMessage,
                                 enum_emi_pic_type eTypeMessage)
{
	struct_emi_pic_message       sMessage;
	struct_emi_pic_envoi_service sData = { siInstId, &sMessage };

	switch(eTypeMessage)
	{
		case M_EMI_PIC_TYP_RESTIT_BY_DATE_EFFECTUE:
		case M_EMI_PIC_TYP_RESTIT_BY_DATE_NON_EFFECTUE:
		case M_EMI_PIC_TYP_RESTIT_BY_FILE_EFFECTUE:
		case M_EMI_PIC_TYP_RESTIT_BY_FILE_NON_EFFECTUE:
		case M_EMI_PIC_TYP_RESTIT_BY_DATETIME_EFFECTUE:
		case M_EMI_PIC_TYP_RESTIT_BY_DATETIME_NON_EFFECTUE:
		case M_EMI_PIC_TYP_RESTIT_BY_RECORD_ID_EFFECTUE:
		case M_EMI_PIC_TYP_RESTIT_BY_RECORD_ID_NON_EFFECTUE:	

			if(!SrvEstVide(EMI_PIC[siInstId].hService[M_EMI_PIC_RESTITUTION]))
			{
				// Filling the structure
				sMessage.entete.neutre.bl_retour = EMI_PIC[siInstId].dwAniBalId;
				sMessage.entete.service = M_EMI_PIC_RESTITUTION;
				sMessage.entete.type_message = eTypeMessage;

				memcpy(&sMessage.u.srv_restit,
					   &pMessage->u.sAniRestit.sRestit,
					   sizeof(struct_emi_pic_restit));

				SrvPourChaqueDemandeur(EMI_PIC[siInstId].hService[M_EMI_PIC_RESTITUTION],
									   EnvoiDemandeur, 
									   &sData);
			}
			else
				EmiPicFichierTrace(siInstId,
								   "EMI_PIC_MESS : EmiPicEnvoiRestit ==> Liste demandeurs vide EMI_PIC No : %d",
								   siInstId);
			break;

		default:
			break;
	}
}

/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicEnvoiTrf(short siInstId,                                */
/*                            struct_emi_pic_int_message *pMessage,          */
/*                            enum_emi_pic_type eTypeMessage)                */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function sends the message to the all subscribers of the   */
/*            TRANSFER service.                                              */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId                        - Instance ID.             */
/*      IN  struct_emi_pic_int_message *pMessage  - Pointer to the message.  */
/*      IN  enum_emi_pic_type eTypeMessage        - Message type.            */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PROTECTED void EmiPicEnvoiTrf(short siInstId,
                              struct_emi_pic_int_message *pMessage,
                              enum_emi_pic_type eTypeMessage)
{
	struct_emi_pic_message       sMessage;
	struct_emi_pic_envoi_service sData = { siInstId, &sMessage };

	switch(eTypeMessage)
	{
		case M_EMI_PIC_TYP_TRANSFER_SEND_EFFECTUE:
		case M_EMI_PIC_TYP_TRANSFER_SEND_NON_EFFECTUE:
		case M_EMI_PIC_TYP_TRANSFER_DEL_EFFECTUE:
		case M_EMI_PIC_TYP_TRANSFER_DEL_NON_EFFECTUE:
			if(!SrvEstVide(EMI_PIC[siInstId].hService[M_EMI_PIC_TRANSFER]))
			{
				// Filling the structure
				sMessage.entete.neutre.bl_retour = EMI_PIC[siInstId].dwAniBalId;
				sMessage.entete.service = M_EMI_PIC_TRANSFER;
				sMessage.entete.type_message = eTypeMessage;

				memcpy(&sMessage.u.srv_trf,
					   &pMessage->u.sAniTrf.sTrf,
					   sizeof(struct_emi_pic_trf));

				SrvPourChaqueDemandeur(EMI_PIC[siInstId].hService[M_EMI_PIC_TRANSFER],
									   EnvoiDemandeur, 
									   &sData);
			}
			else
				EmiPicFichierTrace(siInstId,
								   "EMI_PIC_MESS : EmiPicEnvoiTrf ==> Liste demandeurs vide EMI_PIC No : %d",
								   siInstId);
			break;

		default:
			break;
	}
}

/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicAniEnvoiEmi(short siInstId,                             */
/*                               enum_emi_pic_internal_type eTypeMessage,    */
/*                               void *pvParam)                              */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function sends the message to the EMI thread.              */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId                           - Instance ID.          */
/*      IN  enum_emi_pic_internal_type eTypeMessage  - Message type.         */
/*      IN  void *pvParam                            - Pointer to the        */
/*                                                     message.              */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PROTECTED void EmiPicAniEnvoiEmi(short siInstId,
                                 enum_emi_pic_internal_type eTypeMessage,
                                 void *pvParam)
{
	struct_emi_pic_int_message *pAni = NULL;
	struct_emi_pic_message *pDat = (struct_emi_pic_message *)pvParam;

	// Allocating the memory
	ExitAlloue((struct_neutre **)(&pAni),
			   sizeof(struct_emi_pic_int_message),
			   EMI_PIC[siInstId].hPool);

	// Filling the message type field
	pAni->entete.type_message = eTypeMessage;

	// Message type
	switch(eTypeMessage)
	{
		case M_EMI_PIC_MAINT_REQUEST:
			strcpy_s( pAni->u.sAniMaintMode.u.szPath,sizeof(pAni->u.sAniMaintMode.u.szPath), pDat->u.srv_maint_mode.u.szTargetDir );
			break;

		case M_ARRET_DEMANDE:
		case M_EMISSION_PURGE:
		default :
			break;
	}

	
	EmiPicFichierTrace(siInstId,
					   "EMI_PIC_MESS : EmiPicAniEnvoiEmi ==>  type_message '%d'",
					   pAni->entete.type_message);

	// Sending the message
	ExitEnvoie(EMI_PIC[siInstId].dwEmiBalId,
			   EMI_PIC[siInstId].dwAniBalId,
			   (struct_neutre *)pAni);
}
/*****************************************************************************/
/*SYNTAX: void EmiPicAniEnvoiImport(short siInstId,                          */
/*                               enum_emi_pic_internal_type eTypeMessage,    */
/*                               void *pvParam)                              */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function sends the message to the EMI thread.              */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId                           - Instance ID.          */
/*      IN  enum_emi_pic_internal_type eTypeMessage  - Message type.         */
/*      IN  void *pvParam                            - Pointer to the        */
/*                                                     message.              */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PROTECTED void EmiPicAniEnvoiImport( short siInstId,
									 enum_emi_pic_internal_type eTypeMessage,
									 void *pvParam )
{
	struct_emi_pic_int_message *pAni = NULL;
	struct_emi_pic_message *pDat = (struct_emi_pic_message *)pvParam;
	
	// Allocating the memory
	ExitAlloue((struct_neutre **)(&pAni),
		sizeof(struct_emi_pic_int_message),
		EMI_PIC[siInstId].hPool);
	
	// Filling the message type field
	pAni->entete.type_message = eTypeMessage;
	
	// Message type
	switch(eTypeMessage)
	{
		case M_ARRET_DEMANDE:
		default :
			break;
	}
	
	
	EmiPicFichierTrace(siInstId,
					   "EMI_PIC_MESS : EmiPicAniEnvoiImport ==>  type_message '%d'",
					   pAni->entete.type_message);
	
	// Sending the message
	ExitEnvoie(EMI_PIC[siInstId].dwImportBalId,
		EMI_PIC[siInstId].dwAniBalId,
		(struct_neutre *)pAni);
}

/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicAniEnvoiRestit(short siInstId,                          */
/*                                  enum_emi_pic_internal_type eTypeMessage, */
/*                                  void *pvParam)                           */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function sends the message to the RESTIT thread.           */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId                           - Instance ID.          */
/*      IN  enum_emi_pic_internal_type eTypeMessage  - Message type.         */
/*      IN  void *pvParam                            - Pointer to the        */
/*                                                     message.              */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PROTECTED void EmiPicAniEnvoiRestit(short siInstId,
                                    enum_emi_pic_internal_type eTypeMessage,
                                    void *pvParam)
{
	struct_emi_pic_int_message *pAni = NULL;
	struct_emi_pic_message *pDat = (struct_emi_pic_message *)pvParam;

	// Allocating the memory
	ExitAlloue((struct_neutre **)(&pAni),
			   sizeof(struct_emi_pic_int_message),
			   EMI_PIC[siInstId].hPool );

	// Filling the message type field
	pAni->entete.type_message = eTypeMessage;

	// Message type
	switch(eTypeMessage)
	{
		case M_RESTIT_DATE_REQUEST:
			memcpy(&pAni->u.sAniRestit.sRestit.u.sRequestByDate,
				   &pDat->u.srv_restit.u.sRequestByDate, 
				   sizeof(struct_emi_pic_restit_date));
			break;

		case M_RESTIT_FILE_REQUEST:
			memcpy(&pAni->u.sAniRestit.sRestit.u.sRequestByFile,
				   &pDat->u.srv_restit.u.sRequestByFile, 
				   sizeof(struct_emi_pic_restit_file));
			break;

		case M_ARRET_DEMANDE:
		default :
			break;
	}

	EmiPicFichierTrace(siInstId,
					   "EMI_PIC_MESS : EmiPicAniEnvoiDrv ==> type_message '%d'",
					   pAni->entete.type_message);

	// Sending the message
	ExitEnvoie(EMI_PIC[siInstId].dwRestitBalId,
			   EMI_PIC[siInstId].dwAniBalId,
			   (struct_neutre *)pAni);
}
/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicAniEnvoiTrf(short siInstId,                             */
/*                               enum_emi_pic_internal_type eTypeMessage,    */
/*                               void *pvParam)                              */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            The function sends the message to the TRF thread.              */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  short siInstId                           - Instance ID.          */
/*      IN  enum_emi_pic_internal_type eTypeMessage  - Message type.         */
/*      IN  void *pvParam                            - Pointer to the        */
/*                                                     message.              */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PROTECTED void EmiPicAniEnvoiTrf(short siInstId,
                                 enum_emi_pic_internal_type eTypeMessage,
                                 void *pvParam)
{
	struct_emi_pic_int_message *pAni = NULL;
	struct_emi_pic_message *pDat = (struct_emi_pic_message *)pvParam;

	// Allocating the memory
	ExitAlloue((struct_neutre **)(&pAni),
			   sizeof(struct_emi_pic_int_message),
			   EMI_PIC[siInstId].hPool );

	// Filling the message type field
	pAni->entete.type_message = eTypeMessage;

	// Message type
	switch(eTypeMessage)
	{
		case M_TRANSFER_SEND:
		case M_TRANSFER_DEL:
			memcpy(&pAni->u.sAniTrf.sTrf,
				   &pDat->u.srv_trf, 
				   sizeof(struct_emi_pic_trf));
			break;

		case M_ARRET_DEMANDE:
		default :
			break;
	}

	EmiPicFichierTrace(siInstId,
					   "EMI_PIC_MESS : EmiPicAniEnvoiTrf ==>  type_message '%d'",
					   pAni->entete.type_message);

	// Send the message
	ExitEnvoie(EMI_PIC[siInstId].dwTrfBalId,
			   EMI_PIC[siInstId].dwAniBalId,
			   (struct_neutre *)pAni);
}

/**/
/*****************************************************************************/
/*SYNTAX: BOOL WINAPI EnvoiDemandeur(PVOID pvParam,                          */
/*                                   noyau_bal_id dwBalDemandeur,            */
/*                                   PVOID pvDataDuDemandeur)                */
/*===========================================================================*/
/*TYPE:   Local function.                                                    */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function sends data to each subscriber of the service.    */
/*            We use the pointer of this function as the  parameter of the   */
/*            "SrvPourChaqueDemandeur" function.                             */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  PVOID pvParam               - Pointer to the data.               */
/*      IN  noyau_bal_id dwBalDemandeur - Destination mailbox ID.            */
/*      IN  PVOID pvDataDuDemandeur     - Pointer to the service data.       */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  TRUE                  The function allways returns TRUE.                 */
/*****************************************************************************/
PRIVATE BOOL WINAPI EnvoiDemandeur(PVOID pvParam,
                                   noyau_bal_id dwBalDemandeur,
                                   PVOID pvDataDuDemandeur)
{
	struct_emi_pic_envoi_service *pData = (PVOID)pvParam;
	struct_emi_pic_message *pMessage;

	ExitAlloue((struct_neutre **)(&pMessage),
				sizeof(struct_emi_pic_message),
				EMI_PIC[pData->siInstId].hPool);
    
	// Filling the message structure
	*pMessage = *(pData->pMsg);

	ExitEnvoie(dwBalDemandeur,
			   EMI_PIC[pData->siInstId].dwAniBalId,
			   (struct_neutre *)(pMessage));

	return TRUE;
}

/**/
/*****************************************************************************/
/*SYNTAX: void EmiPicLibereBAL(noyau_bal_id dwBal)                           */
/*===========================================================================*/
/*TYPE:   Public function.                                                   */
/*===========================================================================*/
/*DESCRIPTION:                                                               */
/*            This function deallocates memory in the mailbox  occupied by   */
/*            the messages.                                                  */
/*===========================================================================*/
/*PARAMETERS:                                                                */
/*      IN  noyau_bal_id dwBal  - Mailbox ID.                                */
/*===========================================================================*/
/*  Return                Description                                        */
/*---------------------------------------------------------------------------*/
/*  void                  This function does not return a value.             */
/*****************************************************************************/
PROTECTED void EmiPicLibereBAL(noyau_bal_id dwBal)
{
	struct_neutre  *pNeutre;

	while((TestRecoit(dwBal, (struct_neutre **)(&pNeutre)) == NOYAU_BAL_MESS))
	{
		ExitLibere((struct_neutre **)(&pNeutre));
	}
}
/*-------------------------------- END OF FILE ------------------------------*/